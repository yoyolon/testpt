//--------------------------------------------------------------------------------------------------------------------
// testpt
// yoyolon - October 2022
// 
// Development
// Microsoft Visual Studio 2019
// C++17
// 
// External Library
// stb_image: https://github.com/nothings/stb
//
// This software is based on Raytracing in one weekend(https://raytracing.github.io/).
// Part of this software is based on pbrt(https://pbrt.org/) licensed under the BSD 2-Clause "Simplified" License
// (https://github.com/mmp/pbrt-v3/blob/master/LICENSE.txt).
//--------------------------------------------------------------------------------------------------------------------

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define __STDC_LIB_EXT1__

#include "external/stb_image_write.h"
#include "external/stb_image.h"
#include "Camera.h"
#include "Fresnel.h"
#include "Light.h"
#include "Material.h"
#include "Microfacet.h"
#include "ONB.h"
#include "Random.h"
#include "Ray.h"
#include "Scene.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Vec3.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

enum class Sampling {
    BRDF  = 1, // BRDFによる重点的サンプリング
    LIGHT = 2, // 光源による重点的サンプリング
    MIS   = 3  // 多重重点的サンプリング
};

constexpr bool DEBUG_MODE           = false;
constexpr bool DIRECT_ILLUMINATION  = true;
constexpr bool IMAGE_BASED_LIGHTING = false;
constexpr bool IS_GAMMA_CORRECTION  = true;
constexpr int  SAMPLES = 128;
// デバッグ用
int raycontrib = 0; // 光源からの寄与
int raydeath = 0;   // ロシアンルーレット打ち切り
int raybg = 0;      // 背景からの寄与
Sampling sampling_strategy = Sampling::BRDF;

/**
* @brief シンプルな球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
*/
void make_scene_simple(Scene& world, Camera& cam, float aspect) {
    world.clear();
    // マテリアル
    auto dist_ggx = std::make_shared<GGXDistribution>(0.025f);
    auto fres_schlick = std::make_shared<FresnelSchlick>(Vec3(0.9f, 0.9f, 0.9f));
    auto mat_microfacet = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_schlick);
    auto mat_mirr = std::make_shared<Mirror>(Vec3(0.9f, 0.9f, 0.9f));
    auto mat_light = std::make_shared<Emitter>(Vec3(10.00f, 10.00f, 10.00f));
    // オブジェクト
    auto obj_sphere = std::make_shared<Sphere>(Vec3(0.0f, 2.0f, 0.0f), 3.0f, mat_microfacet);
    auto light_shape_disk = std::make_shared<Disk>(Vec3(0.0f, 20.0f, 0.0f), 50.0f, mat_light);
    auto light_disk = std::make_shared<AreaLight>(Vec3(1.0f, 1.0f, 1.0f), light_shape_disk);
    world.add(obj_sphere);
    //world.add(light_disk);

    // カメラの設定
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f, 2.0f, 15.0f);
    Vec3 cam_target(0.0f, 2.0f, 0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(2.0f, aspect, fd, cam_pos, cam_forward);
}

/**
* @brief 様々な形状のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
*/
void make_scene_cylinder(Scene& world, Camera& cam, float aspect) {
    world.clear();
    // マテリアル
    auto mat_mirr = std::make_shared<Mirror>(Vec3(0.9f, 0.9f, 0.9f));
    auto mat_light = std::make_shared<Emitter>(Vec3(10.00f, 10.00f, 10.00f));
    // オブジェクト
    auto obj_disk_top = std::make_shared<Disk>(Vec3(0.0f, 8.0f, 0.0f), 4.0f, mat_mirr);
    auto obj_disk_btm = std::make_shared<Disk>(Vec3::zero, 4.0f, mat_mirr);
    auto obj_cylinder = std::make_shared<Cylinder>(Vec3::zero, 4.0f, 8.0f, mat_mirr);
    auto light_shape_disk = std::make_shared<Disk>(Vec3(0.0f, 20.0f, 0.0f), 50.0f, mat_light);
    auto light_disk = std::make_shared<AreaLight>(Vec3(10.0f, 10.0f, 10.0f), light_shape_disk);
    world.add(obj_disk_top);
    world.add(obj_disk_btm);
    world.add(obj_cylinder);
    //world.add(light_shape_disk);

    // カメラの設定
    auto fd = 1.5f; // 焦点距離
    Vec3 cam_pos(0.0f, 10.0f, 20.0f);
    Vec3 cam_target(0.0f, 2.0f, 0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(2.0f, aspect, fd, cam_pos, cam_forward);
}

/**
* @brief 重点的サンプリング検証用シーン
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
* @note モデル出典: https://github.com/wjakob/nori/tree/master/scenes/pa5/veach_mi
*/
void make_scene_MIS(Scene& world, Camera& cam, float aspect) {
    world.clear();
    // マテリアル
    auto dist_ggx_vr = std::make_shared<GGXDistribution>(0.100f);
    auto dist_ggx_r  = std::make_shared<GGXDistribution>(0.050f);
    auto dist_ggx_n  = std::make_shared<GGXDistribution>(0.020f);
    auto dist_ggx_s  = std::make_shared<GGXDistribution>(0.005f);
    auto fres        = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
    auto mat_vrough  = std::make_shared<Microfacet>(Vec3(0.0175f, 0.0225f, 0.0325f), dist_ggx_vr, fres);
    auto mat_rough   = std::make_shared<Microfacet>(Vec3(0.0175f, 0.0225f, 0.0325f), dist_ggx_r,  fres);
    auto mat_normal  = std::make_shared<Microfacet>(Vec3(0.0175f, 0.0225f, 0.0325f), dist_ggx_n,  fres);
    auto mat_smooth  = std::make_shared<Microfacet>(Vec3(0.0175f, 0.0225f, 0.0325f), dist_ggx_s,  fres);
    auto mat_light   = std::make_shared<Emitter>(Vec3(10.0f,10.0f,10.0f));
    auto mat_diff    = std::make_shared<Diffuse>(Vec3(0.1f,0.1f,0.1f));
    auto mat_diff2   = std::make_shared<Diffuse>(Vec3(1.0f,0.1f,0.1f));
    auto mat_mirr    = std::make_shared<Mirror>(Vec3(1.0f, 1.0f, 1.0f));
    // 光源
    auto sphere_L  = std::make_shared<Sphere>(Vec3( 3.75f, 0.0f, 0.0f), 0.9f, mat_light);
    auto sphere_M  = std::make_shared<Sphere>(Vec3( 1.25f, 0.0f, 0.0f), 0.3f, mat_light);
    auto sphere_S  = std::make_shared<Sphere>(Vec3(-1.25f, 0.0f, 0.0f), 0.1f, mat_light);
    auto sphere_SS = std::make_shared<Sphere>(Vec3(-3.75f, 0.0f, 0.0f), 0.1f / 3, mat_light);
    auto sphere    = std::make_shared<Sphere>(Vec3(  0.0f, 4.0f, 3.0f), 1.0f , mat_light);
    auto light_L   = std::make_shared<AreaLight>(Vec3(100.0f, 100.0f, 100.0f)/81, sphere_L);
    auto light_M   = std::make_shared<AreaLight>(Vec3(100.0f, 100.0f, 100.0f)/9 , sphere_M);
    auto light_S   = std::make_shared<AreaLight>(Vec3(100.0f, 100.0f, 100.0f)   , sphere_S);
    auto light_SS  = std::make_shared<AreaLight>(Vec3(100.0f, 100.0f, 100.0f)*9 , sphere_SS);
    auto light     = std::make_shared<AreaLight>(Vec3(100.0f, 100.0f, 100.0f)   , sphere);
    auto plate1 = std::make_shared<TriangleMesh>("asset/veach_mis/plate1.obj", mat_smooth);
    auto plate2 = std::make_shared<TriangleMesh>("asset/veach_mis/plate2.obj", mat_normal);
    auto plate3 = std::make_shared<TriangleMesh>("asset/veach_mis/plate3.obj", mat_rough);
    auto plate4 = std::make_shared<TriangleMesh>("asset/veach_mis/plate4.obj", mat_vrough);
    auto floor  = std::make_shared<TriangleMesh>("asset/veach_mis/floor.obj" , mat_diff);
    world.add(light_L);
    world.add(light_M);
    world.add(light_S);
    world.add(light_SS);
    //world.add(light);
    world.add(floor);
    world.add(plate1);
    world.add(plate2);
    world.add(plate3);
    world.add(plate4);

    // カメラの設定
    auto fd = 4.732f * aspect;
    Vec3 cam_pos(0.0f, 6.0f, 27.5f);
    Vec3 cam_target(0.0f, -1.5f, 2.5f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(2.0f, aspect, fd, cam_pos, cam_forward);
}

/**
* @brief コーネルボックスのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
* @note 参考: http://www.graphics.cornell.edu/online/box/data.html
*/
void make_scene_cornell(Scene& world, Camera& cam, float aspect) {
    world.clear();
    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
    auto fres_schlick = std::make_shared<FresnelSchlick>(Vec3(0.9f, 0.9f, 0.9f));
    auto mat_ggx   = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_schlick);
    auto mat_mirr  = std::make_shared<Mirror>(Vec3(0.9f, 0.9f, 0.9f));
    auto mat_red   = std::make_shared<Diffuse>(Vec3( 0.5694f,  0.0430f, 0.0451f));
    auto mat_green = std::make_shared<Diffuse>(Vec3( 0.1039f,  0.3778f, 0.0768f));
    auto mat_white = std::make_shared<Diffuse>(Vec3( 0.8860f,  0.6977f, 0.6676f));
    auto mat_light = std::make_shared<Emitter>(Vec3(20.6904f, 10.8669f, 2.7761f));
    // light sorce
    auto light_shape = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-343.0f, 548.7f, -227.0f),
            Vec3(-343.0f, 548.7f, -332.0f),
            Vec3(-213.0f, 548.7f, -332.0f),
            Vec3(-213.0f, 548.7f, -227.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_light);
    auto radiance = Vec3(20.6904f, 10.8669f, 2.7761f);
    auto light = std::make_shared<AreaLight>(radiance, light_shape);
    // light sorce2
    auto light_shape_disk = std::make_shared<Disk>(Vec3(-278.0f, 548.7f, -279.6f), 50.0f, mat_light);
    auto light_disk = std::make_shared<AreaLight>(radiance, light_shape_disk);
    // light sorce3
    auto light_shape_spehre = std::make_shared<Sphere>(Vec3(-278.0f, 548.7f - 50.0f, -279.6f), 50.0f, mat_light);
    auto light_sphere = std::make_shared<AreaLight>(radiance, light_shape_spehre);
    // ceiling
    auto ceiling = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-556.0f, 548.8f,   0.0f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(  0.0f,  548.8f, -559.2f),
            Vec3(  0.0f,  548.8f,   0.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    // floor
    auto floor = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f, 0.0f,    0.0f),
            Vec3(  0.0f,  0.0f,    0.0f),
            Vec3(  0.0f,  0.0f, -559.2f),
            Vec3(-549.6f, 0.0f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    // back wall
    auto back = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-549.6f,   0.0f, -559.2f),
            Vec3(   0.0f,   0.0f, -559.2f),
            Vec3(   0.0f, 548.8f, -559.2f),
            Vec3(-556.0f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    // right wall
    auto right = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(0.0f,   0.0f, -559.2f),
            Vec3(0.0f,   0.0f,    0.0f),
            Vec3(0.0f, 548.8f,    0.0f),
            Vec3(0.0f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_green);
    // left wall
    auto left = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f,   0.0f,    0.0f),
            Vec3(-549.6f,   0.0f, -559.2f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(-556.0f, 548.8f,    0.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_red);
    // Short block
    auto short_top = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-130.0f, 165.0f,  -65.0f),
            Vec3( -82.0f, 165.0f, -225.0f),
            Vec3(-240.0f, 165.0f, -272.0f),
            Vec3(-290.0f, 165.0f, -114.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    auto short_rgt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-290.0f,   0.0f, -114.0f),
            Vec3(-290.0f, 165.0f, -114.0f),
            Vec3(-240.0f, 165.0f, -272.0f),
            Vec3(-240.0f,   0.0f, -272.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    auto short_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-130.0f,   0.0f,  -65.0f),
            Vec3(-130.0f, 165.0f,  -65.0f),
            Vec3(-290.0f, 165.0f, -114.0f),
            Vec3(-290.0f,   0.0f, -114.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    auto short_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3( -82.0f,   0.0f, -225.0f),
            Vec3( -82.0f, 165.0f, -225.0f),
            Vec3(-130.0f, 165.0f,  -65.0f),
            Vec3(-130.0f,   0.0f,  -65.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    auto short_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-240.0f,   0.0f, -272.0f),
            Vec3(-240.0f, 165.0f, -272.0f),
            Vec3( -82.0f, 165.0f, -225.0f),
            Vec3( -82.0f,   0.0f, -225.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    // Tall block
    auto tall_top = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-423.0f, 330.0f, -247.0f),
            Vec3(-265.0f, 330.0f, -296.0f),
            Vec3(-314.0f, 330.0f, -456.0f),
            Vec3(-472.0f, 330.0f, -406.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    auto tall_rgt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-423.0f,   0.0f, -247.0f),
            Vec3(-423.0f, 330.0f, -247.0f),
            Vec3(-472.0f, 330.0f, -406.0f),
            Vec3(-472.0f,   0.0f, -406.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    auto tall_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-472.0f,   0.0f, -406.0f),
            Vec3(-472.0f, 330.0f, -406.0f),
            Vec3(-314.0f, 330.0f, -456.0f),
            Vec3(-314.0f,   0.0f, -456.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    auto tall_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-314.0f,   0.0f, -456.0f),
            Vec3(-314.0f, 330.0f, -456.0f),
            Vec3(-265.0f, 330.0f, -296.0f),
            Vec3(-265.0f,   0.0f, -296.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    auto tall_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-265.0f,   0.0f, -296.0f),
            Vec3(-265.0f, 330.0f, -296.0f),
            Vec3(-423.0f, 330.0f, -247.0f),
            Vec3(-423.0f,   0.0f, -247.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    // オブジェクトと光源をシーンに追加
    //auto sphere = std::make_shared<Sphere>(Vec3(-255.0f, 255.0f, -255.0f), 50.0f, mat_white);
    //world.add(sphere);
    world.add(left);
    world.add(right);
    world.add(back);
    world.add(ceiling);
    world.add(floor);
    world.add(short_top);
    world.add(short_rgt);
    world.add(short_frt);
    world.add(short_lft);
    world.add(short_bck);
    world.add(tall_top);
    world.add(tall_rgt);
    world.add(tall_frt);
    world.add(tall_lft);
    world.add(tall_bck);
    world.add(light);
    //world.add(light_disk);
    //world.add(light_sphere);

    //// カメラの設定
    auto fd = 0.035f; // 焦点距離
    Vec3 cam_pos(-278.0f, 273.0f, 800.0f);
    Vec3 cam_target(-278.0f, 273.0f, 0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos); // z軸負の方向がカメラの前方
    cam = Camera(0.025f, aspect, fd, cam_pos, cam_forward);
}

/**
* @brief 異なる膜厚の薄膜がコーティングされた球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
*/
void make_scene_sphere(Scene& world, Camera& cam, float aspect) {
    world.clear();

    // マイクロファセット分布
    auto dist_ggx = std::make_shared<GGXDistribution>(0.1f);
    // フレネルの式
    auto fres_thinfilm1 = std::make_shared<FresnelThinfilm>(200.f, 1.0f, 1.6f, 1.2f);
    auto fres_thinfilm2 = std::make_shared<FresnelThinfilm>(300.f, 1.0f, 1.6f, 1.2f);
    auto fres_thinfilm3 = std::make_shared<FresnelThinfilm>(400.f, 1.0f, 1.6f, 1.2f);
    auto fres_thinfilm4 = std::make_shared<FresnelThinfilm>(500.f, 1.0f, 1.6f, 1.2f);
    auto fres_thinfilm5 = std::make_shared<FresnelThinfilm>(600.f, 1.0f, 1.6f, 1.2f);
    auto fres_thinfilm6 = std::make_shared<FresnelThinfilm>(700.f, 1.0f, 1.6f, 1.2f);
    auto fres_thinfilm7 = std::make_shared<FresnelThinfilm>(800.f, 1.0f, 1.6f, 1.2f);
    auto fres_thinfilm8 = std::make_shared<FresnelThinfilm>(900.f, 1.0f, 1.6f, 1.2f);
    auto fres_thinfilm9 = std::make_shared<FresnelThinfilm>(1000.f, 1.0f, 1.6f, 1.2f);
    // マテリアル
    auto mat_microfacet1 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_thinfilm1);
    auto mat_microfacet2 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_thinfilm2);
    auto mat_microfacet3 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_thinfilm3);
    auto mat_microfacet4 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_thinfilm4);
    auto mat_microfacet5 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_thinfilm5);
    auto mat_microfacet6 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_thinfilm6);
    auto mat_microfacet7 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_thinfilm7);
    auto mat_microfacet8 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_thinfilm8);
    auto mat_microfacet9 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_thinfilm9);
    // 球
    auto obj_sphere1 = std::make_shared<Sphere>(Vec3(-2.2f, -0.2f, 0.0f), 1.0f, mat_microfacet1);
    auto obj_sphere2 = std::make_shared<Sphere>(Vec3(0.0f, -0.2f, 0.0f), 1.0f, mat_microfacet2);
    auto obj_sphere3 = std::make_shared<Sphere>(Vec3(2.2f, -0.2f, 0.0f), 1.0f, mat_microfacet3);
    auto obj_sphere4 = std::make_shared<Sphere>(Vec3(-2.2f, 2.0f, 0.0f), 1.0f, mat_microfacet4);
    auto obj_sphere5 = std::make_shared<Sphere>(Vec3(0.0f, 2.0f, 0.0f), 1.0f, mat_microfacet5);
    auto obj_sphere6 = std::make_shared<Sphere>(Vec3(2.2f, 2.0f, 0.0f), 1.0f, mat_microfacet6);
    auto obj_sphere7 = std::make_shared<Sphere>(Vec3(-2.2f, 4.2f, 0.0f), 1.0f, mat_microfacet7);
    auto obj_sphere8 = std::make_shared<Sphere>(Vec3(0.0f, 4.2f, 0.0f), 1.0f, mat_microfacet8);
    auto obj_sphere9 = std::make_shared<Sphere>(Vec3(2.2f, 4.2f, 0.0f), 1.0f, mat_microfacet9);
    // オブジェクトをシーンに追加
    world.add(obj_sphere1);
    world.add(obj_sphere2);
    world.add(obj_sphere3);
    world.add(obj_sphere4);
    world.add(obj_sphere5);
    world.add(obj_sphere6);
    world.add(obj_sphere7);
    world.add(obj_sphere8);
    world.add(obj_sphere9);
    // カメラの設定
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f, 2.0f, -10.0f);
    Vec3 cam_target(0.0f, 2.0f, 0.0f);
    Vec3 cam_forward = -unit_vector(cam_target - cam_pos);
    cam = Camera(2.0f, aspect, fd, cam_pos, cam_forward);
}

/**
* @brief 花瓶モデルのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
* @note モデル出典: https://polyhaven.com/a/ceramic_vase_01
*/
void make_scene_vase(Scene& world, Camera& cam, float aspect) {
    world.clear();
    // マテリアル
    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
    auto fres_Schlick = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
    auto mat_microfacet = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_Schlick);
    auto mat_phong = std::make_shared<Phong>(Vec3(1.0f,1.0f,1.0f), Vec3(0.0f,0.0f,0.0f), Vec3(1.0f,1.0f,1.0f), 45.0f);
    // オブジェクト
    std::string vasepath = "asset/model.obj";
    auto obj_pot = std::make_shared<TriangleMesh>(vasepath, mat_microfacet);
    world.add(obj_pot);

    // カメラの設定
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f, 2.0f, 7.5f);
    Vec3 cam_target(0.0f, 2.0f, 0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(2.0f, aspect, fd, cam_pos, cam_forward);
}


/**
* @brief レイに沿った放射輝度伝搬を計算する関数
* @param[in]  r         :追跡するレイ
* @param[in]  bouunce   :現在のレイのバウンス回数
* @param[in]  max_depth :レイの最大バウンス回数
* @param[in]  world     :レンダリングするシーンのデータ
* @param[in]  contrib   :現在のレイの寄与
* @return Vec3          :レイに沿った放射輝度
*/
Vec3 L(const Ray& r, int bounces, int max_depth, const Scene& world, Vec3 contrib) {
    if (bounces >= max_depth) {
        return Vec3::zero;
    }
    intersection isect; // 交差点情報
    if (!world.intersect(r, eps_isect, inf, isect)) {
        raybg++;
        return contrib * world.sample_envmap(r);
    }
    // シェーディング座標の構築
    // NOTE: カメラ方向をwi，光源方向をwoにしている(あとで修正)
    ONB shadingCoord(isect.normal);
    //shadingCoord.build_ONB(isect.normal);
    Vec3 wi = unit_vector(r.get_dir());
    Vec3 wi_local = -shadingCoord.to_local(wi);
    Vec3 wo_local, wo, brdf;
    float pdf;
    // マテリアルの場合
    if (isect.type == IsectType::Material) {
        isect.mat->sample_f(wi_local, isect, brdf, wo_local, pdf);
        // 発光
        if (isect.mat->get_type() == MaterialType::Emitter) {
            return contrib * isect.mat->emitte();
        }
        // 完全鏡面反射
        else if (isect.mat->get_type() == MaterialType::Specular) {
            wo = shadingCoord.to_world(wo_local);
            float cos_term = dot(isect.normal, unit_vector(wo));
            contrib = contrib * brdf * cos_term / pdf;
        }
        // 拡散・光沢反射
        else {
            wo = shadingCoord.to_world(wo_local);
            float cos_term = dot(isect.normal, unit_vector(wo));
            contrib = contrib * brdf * cos_term / pdf;
        }
        //ロシアンルーレット
        if (bounces >= 5) {
            float p_rr = std::max(0.5f, contrib.average());
            if (p_rr > Random::uniform_float()) {
                raydeath++;
                return Vec3::zero;
            }
            else {
                contrib /= p_rr;
            }
        }
        return L(Ray(isect.pos, wo), ++bounces, max_depth, world, contrib);
    }
    // 光源の場合
    else {
        raycontrib++;
        return contrib * isect.light->emitte();
    }
}

/**
* @brief レイに沿った放射輝度伝搬を計算する関数
* @param[in]  r         :追跡するレイ
* @param[in]  bouunce   :現在のレイのバウンス回数
* @param[in]  max_depth :レイの最大バウンス回数
* @param[in]  world     :レンダリングするシーンのデータ
* @param[in]  contrib   :現在のレイの寄与
* @return Vec3          :レイに沿った放射輝度
*/
Vec3 L_direct(const Ray& r, int bounces, int max_depth, const Scene& world, Vec3 contrib) {
    if (bounces >= max_depth) {
        return Vec3::zero;
    }
    intersection isect;
    if (!world.intersect(r, eps_isect, inf, isect)) {
        raybg++;
        return contrib * world.sample_envmap(r);
    }
    // マテリアルの場合
    if (isect.type == IsectType::Material) {
        if (isect.mat->get_type() == MaterialType::Emitter) {
            return contrib * isect.mat->emitte();
        }
        // シェーディング座標の構築
        ONB shadingCoord(isect.normal);
        //shadingCoord.build_ONB(isect.normal);
        Vec3 wi = unit_vector(r.get_dir());
        Vec3 wi_local = -shadingCoord.to_local(wi);
        Vec3 wo_scattering_local;
        Vec3 brdf;
        float pdf_scattering;
        isect.mat->sample_f(wi_local, isect, brdf, wo_scattering_local, pdf_scattering);
        Vec3 wo_scattering = unit_vector(shadingCoord.to_world(wo_scattering_local)); // 散乱方向
        float cos_term = dot(isect.normal, wo_scattering);
        // 完全鏡面反射
        if (isect.mat->get_type() == MaterialType::Specular) {
            contrib = contrib * brdf * cos_term / pdf_scattering;
            return L(Ray(isect.pos, wo_scattering), ++bounces, max_depth, world, contrib);
        }
        // 拡散・光沢反射
        else {
            auto Ld = Vec3::zero;
            float weight = 1.0f;
            // BRDFからサンプリング
            if ((sampling_strategy == Sampling::BRDF) || (sampling_strategy == Sampling::MIS)) {
                // シーン中の全光源と交差判定
                intersection isect_light;
                if (world.intersect_light(Ray(isect.pos, wo_scattering), eps_isect, inf, isect_light)) {
                    // 光源の可視判定
                    if (!world.intersect_object(Ray(isect.pos, wo_scattering), eps_isect, isect_light.t)) {
                        if (sampling_strategy == Sampling::MIS) {
                            auto pdf_light = isect_light.light->sample_pdf(isect, wo_scattering);
                            weight = Random::power_heuristic(1, pdf_scattering, 1, pdf_light);
                        }
                        Ld += contrib * brdf * cos_term * weight * isect_light.light->emitte() / pdf_scattering;
                    }
                }
            }
            // 光源からサンプリング
            // 複数光源の場合どのようにサンプリングする?
            // 1. 光源を1つ選びサンプリング
            // 2. 放射エネルギーに基づき1点をサンプリング
            // 3. 各光源から1点サンプリング
            // 4. n点サンプリング
            // とりあえず(1)を試す
            // NOTE: 
            if ((sampling_strategy == Sampling::LIGHT) || (sampling_strategy == Sampling::MIS)) {
                const auto& lights = world.get_light();
                auto light_index = Random::uniform_int(0, lights.size() - 1);
                const auto& light = lights[light_index];
                float pdf_light = 0.0f;
                Vec3 wo_light;
                Vec3 L = light->sample_light(isect, wo_light, pdf_light);
                if (is_zero(L) || pdf_light == 0) {
                    return Vec3(0.0f, 1.0f, 0.0f);
                }
                // 光源の可視判定
                auto r_light = Ray(isect.pos, wo_light);
                intersection isect_light;
                light->intersect(r_light, eps_isect, inf, isect_light); // 光源の交差点情報を取得
                if (world.intersect_object(r_light, eps_isect, isect_light.t)) { // 遮蔽
                    return Vec3(1.0f, 0.0f, 0.0f);
                }
                // 光源サンプリング時のBRDFを評価
                auto wo_light_local = -shadingCoord.to_local(unit_vector(wo_light));
                auto brdf_light = isect.mat->f(wi_local, wo_light_local);
                auto cos_term_light = std::abs(dot(isect.normal, unit_vector(wo_light)));
                if (sampling_strategy == Sampling::MIS) {
                    pdf_scattering = isect.mat->sample_pdf(wi_local, wo_light_local);
                    weight = Random::power_heuristic(1, pdf_light, 1, pdf_scattering);
                }
                if (isect.mat->get_type() == MaterialType::Glossy) {
                    //std::cout << "brdf: " << brdf_light << '\n';
                    //std::cout << "cos_term: " << cos_term_light << '\n';
                    //std::cout << "weight: " << weight << '\n';
                }
                Ld += contrib * brdf_light * L * cos_term_light * weight / pdf_light;
            }
            return Ld;
        }
    }
    // 光源の場合
    else {
        raycontrib++;
        return contrib * isect.light->emitte();
    }
}

/**
* @brief シーン中のオブジェクトの法線を表示する関数
* @param[in]  r         :追跡するレイ
* @param[in]  world     :レンダリングするシーンのデータ
* @return Vec3          :法線の可視化
* @note z軸正の方向をカメラへ向かう方向とする
*/
Vec3 L_normal(const Ray& r, const Scene& world) {
    intersection isect;
    if (world.intersect(r, eps_isect, inf, isect)) {
        return 0.5f * (isect.normal + Vec3(1.0f, 1.0f, 1.0f));
    }
    return Vec3(1.0f, 1.0f, 1.0f);
}

/**
* @brief 色をガンマ補正
* @param[in]  color :ガンマ補正前の色
* @param[in]  gamma :ガンマ値
* @return Vec3      :ガンマ補正後の色
* @note z軸正の方向をカメラへ向かう方向とする
*/
Vec3 gamma_correction(const Vec3& color, float gamma) {
    float r = std::pow(color.get_x(), gamma);
    float g = std::pow(color.get_y(), gamma);
    float b = std::pow(color.get_z(), gamma);
    return Vec3(r, g, b);
}


/**
* @brief main関数
*/
int main(int argc, char* argv[]) {
    Random::init(); // 乱数の初期化
    // 出力画像
    const char* filename = "image.png";  // パス
    constexpr auto w = 768;              // 高さ
    constexpr auto h = 512;              // 幅
    constexpr auto aspect = (float)w / h;       // アスペクト比
    constexpr int c = 3;                 // チャンネル数
    std::vector<uint8_t> img(w * h * c); // 画像データ
    // パラメータ
    const int nsample = (argc == 2) ? atoi(argv[1]) : SAMPLES; // レイのサンプル数
    constexpr auto max_depth = 100;  // レイの最大追跡数
    constexpr auto gamma = 1 / 2.2f; // ガンマ補正用
    // シーン
    Scene world;
    if (IMAGE_BASED_LIGHTING) {
        int w_envmap, h_envmap, c_envmap;
        float* envmap = stbi_loadf("asset/envmap3.hdr", &w_envmap, &h_envmap, &c_envmap, 0);
        world = Scene(envmap, w_envmap, h_envmap, c_envmap);
    }
    Camera cam;
    //make_scene_simple(world, cam, aspect);
    //make_scene_cylinder(world, cam, aspect);
    make_scene_MIS(world, cam, aspect);
    //make_scene_cornell(world, cam, aspect);
    //make_scene_sphere(world, cam, aspect);
    //make_scene_vase(world, cam, aspect);


    // レイトレーシング
    auto start_time = std::chrono::system_clock::now(); // 計測開始時間
    int index = 0;
    for (int i = 0; i < h; i++) {
        std::cout << '\r' << i+1 << '/' << h << std::flush;
        for (int j = 0; j < w; j++) {
            Vec3 I(0.0f, 0.0f, 0.0f);
            // index番目のピクセルのサンプルを生成
            for (int k = 0; k < nsample; k++) {
                auto v = (i + Random::uniform_float()) / (h - 1);
                auto u = (j + Random::uniform_float()) / (w - 1);
                Ray r = cam.generate_ray(u, v);
                if (DEBUG_MODE) {
                    I += L_normal(r, world);
                }
                else {
                    if (DIRECT_ILLUMINATION)
                        I += L_direct(r, 0, max_depth, world, Vec3(1.0f, 1.0f, 1.0f));
                    else 
                        I += L(r, 0, max_depth, world, Vec3(1.0f, 1.0f, 1.0f));
                }
            }
            I *= 1.0f / nsample;
            if (IS_GAMMA_CORRECTION) I = gamma_correction(I, gamma);
            img[index++] = static_cast<uint8_t>(std::clamp(I.get_x(), 0.0f, 1.0f) * 255);
            img[index++] = static_cast<uint8_t>(std::clamp(I.get_y(), 0.0f, 1.0f) * 255);
            img[index++] = static_cast<uint8_t>(std::clamp(I.get_z(), 0.0f, 1.0f) * 255);
        }
    }
    auto end_time = std::chrono::system_clock::now(); // 計測終了時間
    std::cout << '\n' << std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count()/1000 << "sec\n";
    std::cout << std::setprecision(4)
              << "light hit: " << (float)raycontrib / (h * w * nsample) * 100 << '\n'
              << "bg hit:    " << (float)raybg      / (h * w * nsample) * 100 << '\n'
              << "deth:      " << (float)raydeath   / (h * w * nsample) * 100 << '\n';

     // 画像出力
    stbi_write_png(filename, w, h, 3, img.data(), w * c * sizeof(uint8_t));
}