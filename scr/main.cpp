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
#include "Film.h"
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
constexpr bool DIRECT_ILLUMINATION  = false;
constexpr bool IMAGE_BASED_LIGHTING = false;
constexpr bool IS_GAMMA_CORRECTION  = true;
constexpr int  SAMPLES = 32;
// デバッグ用
Sampling sampling_strategy = Sampling::MIS;

/**
* @brief シンプルな球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
*/
void make_scene_simple(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto dist_ggx       = std::make_shared<GGXDistribution>(0.025f);
    auto fres_schlick   = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
    auto mat_microfacet = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_schlick);
    auto mat_mirr       = std::make_shared<Mirror>(Vec3(0.9f,0.9f,0.9f));
    auto mat_light      = std::make_shared<Emitter>(Vec3(10.00f,10.00f,10.00f));

    // オブジェクト
    auto obj_sphere       = std::make_shared<Sphere>(Vec3(0.0f,2.0f,0.0f), 3.0f, mat_microfacet);
    auto light_shape_disk = std::make_shared<Disk>(Vec3(0.0f,20.0f,0.0f), 50.0f, mat_light);
    auto light_disk       = std::make_shared<AreaLight>(Vec3(1.0f,1.0f,1.0f), light_shape_disk);
    world.add(obj_sphere);
    world.add(light_disk);

    // カメラ設定
    auto film = std::make_shared<Film>(600, 600, 3, "simple.png"); // フィルム
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f,2.0f,15.0f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief 様々な形状のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
*/
void make_scene_cylinder(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto mat_mirr  = std::make_shared<Mirror>(Vec3(0.9f,0.9f,0.9f));
    auto mat_light = std::make_shared<Emitter>(Vec3(10.00f,10.00f,10.00f));

    // オブジェクト
    auto obj_disk_top = std::make_shared<Disk>(Vec3(0.0f,8.0f,0.0f), 4.0f, mat_mirr);
    auto obj_disk_btm = std::make_shared<Disk>(Vec3::zero, 4.0f, mat_mirr);
    auto obj_cylinder = std::make_shared<Cylinder>(Vec3::zero, 4.0f, 8.0f, mat_mirr);
    auto light_shape  = std::make_shared<Disk>(Vec3(0.0f,20.0f,0.0f), 50.0f, mat_light);
    auto light_disk   = std::make_shared<AreaLight>(Vec3(10.0f,10.0f,10.0f), light_shape);
    world.add(obj_disk_top);
    world.add(obj_disk_btm);
    world.add(obj_cylinder);
    world.add(light_shape);

    // カメラ設定
    auto film = std::make_shared<Film>(768, 512, 3, "cylinder.png"); // フィルム
    auto fd = 1.5f; // 焦点距離
    Vec3 cam_pos(0.0f,10.0f,20.0f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief 重点的サンプリング検証用シーン
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
* @note モデル出典: https://github.com/wjakob/nori/tree/master/scenes/pa5/veach_mi
*/
void make_scene_MIS(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto dist1 = std::make_shared<GGXDistribution>(0.100f);
    auto dist2 = std::make_shared<GGXDistribution>(0.050f);
    auto dist3 = std::make_shared<GGXDistribution>(0.020f);
    auto dist4 = std::make_shared<GGXDistribution>(0.005f);
    auto fres  = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
    auto mat_vrough = std::make_shared<Microfacet>(Vec3(0.0175f, 0.0225f, 0.0325f), dist1, fres);
    auto mat_rough  = std::make_shared<Microfacet>(Vec3(0.0175f, 0.0225f, 0.0325f), dist2, fres);
    auto mat_normal = std::make_shared<Microfacet>(Vec3(0.0175f, 0.0225f, 0.0325f), dist3, fres);
    auto mat_smooth = std::make_shared<Microfacet>(Vec3(0.0175f, 0.0225f, 0.0325f), dist4, fres);
    auto mat_light  = std::make_shared<Emitter>(Vec3(10.0f,10.0f,10.0f));
    auto mat_diff   = std::make_shared<Diffuse>(Vec3(0.1f,0.1f,0.1f));
    auto mat_mirr   = std::make_shared<Mirror>(Vec3(1.0f, 1.0f, 1.0f));
    // 光源
    auto sphere_XL = std::make_shared<Sphere>(Vec3( 3.75f,0.0f,0.0f), 0.90f, mat_light);
    auto sphere_L  = std::make_shared<Sphere>(Vec3( 1.25f,0.0f,0.0f), 0.30f, mat_light);
    auto sphere_M  = std::make_shared<Sphere>(Vec3(-1.25f,0.0f,0.0f), 0.10f, mat_light);
    auto sphere_S  = std::make_shared<Sphere>(Vec3(-3.75f,0.0f,0.0f), 0.03f, mat_light);
    auto sphere    = std::make_shared<Sphere>(Vec3(  0.0f,4.0f,3.0f), 1.0f, mat_light);
    auto light_XL  = std::make_shared<AreaLight>(Vec3(  1.2f,  1.2f,  1.2f), sphere_XL);
    auto light_L   = std::make_shared<AreaLight>(Vec3( 11.1f, 11.1f, 11.1f), sphere_L);
    auto light_M   = std::make_shared<AreaLight>(Vec3(100.0f,100.0f,100.0f), sphere_M);
    auto light_S   = std::make_shared<AreaLight>(Vec3(900.0f,900.0f,900.0f), sphere_S);
    auto light     = std::make_shared<AreaLight>(Vec3(100.0f,100.0f,100.0f), sphere);
    // オブジェクト
    auto plate1 = std::make_shared<TriangleMesh>("asset/veach_mis/plate1.obj", mat_smooth);
    auto plate2 = std::make_shared<TriangleMesh>("asset/veach_mis/plate2.obj", mat_normal);
    auto plate3 = std::make_shared<TriangleMesh>("asset/veach_mis/plate3.obj", mat_rough);
    auto plate4 = std::make_shared<TriangleMesh>("asset/veach_mis/plate4.obj", mat_vrough);
    auto floor  = std::make_shared<TriangleMesh>("asset/veach_mis/floor.obj" , mat_diff);
    world.add(light_XL);
    world.add(light_L);
    world.add(light_M);
    world.add(light_S);
    //world.add(light);
    world.add(floor);
    world.add(plate1);
    world.add(plate2);
    world.add(plate3);
    world.add(plate4);
    // カメラの設定
    auto film = std::make_shared<Film>(768, 512, 3, "veach_mis.png"); // フィルム
    auto fd = 4.732f * film->get_aspect();
    Vec3 cam_pos(0.0f, 6.0f, 27.5f);
    Vec3 cam_target(0.0f, -1.5f, 2.5f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief コーネルボックスのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
* @note 参考: http://www.graphics.cornell.edu/online/box/data.html
*/
void make_scene_cornell(Scene& world, Camera& cam) {
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

    //// カメラの設定
    auto film = std::make_shared<Film>(600, 600, 3, "cornell_box.png"); // フィルム
    auto fd = 0.035f / 0.025f * 2.0f; // 焦点距離
    Vec3 cam_pos(-278.0f, 273.0f, 800.0f);
    Vec3 cam_target(-278.0f, 273.0f, 0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos); // z軸負の方向がカメラの前方
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief 異なる膜厚の薄膜がコーティングされた球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
*/
void make_scene_sphere(Scene& world, Camera& cam) {
    world.clear();

    // マイクロファセット分布
    auto dist_ggx = std::make_shared<GGXDistribution>(0.1f);
    // フレネルの式
    auto fres_irid1 = std::make_shared<FresnelThinfilm>(200.f, 1.0f, 1.6f, 1.2f);
    auto fres_irid2 = std::make_shared<FresnelThinfilm>(300.f, 1.0f, 1.6f, 1.2f);
    auto fres_irid3 = std::make_shared<FresnelThinfilm>(400.f, 1.0f, 1.6f, 1.2f);
    auto fres_irid4 = std::make_shared<FresnelThinfilm>(500.f, 1.0f, 1.6f, 1.2f);
    auto fres_irid5 = std::make_shared<FresnelThinfilm>(600.f, 1.0f, 1.6f, 1.2f);
    auto fres_irid6 = std::make_shared<FresnelThinfilm>(700.f, 1.0f, 1.6f, 1.2f);
    auto fres_irid7 = std::make_shared<FresnelThinfilm>(800.f, 1.0f, 1.6f, 1.2f);
    auto fres_irid8 = std::make_shared<FresnelThinfilm>(900.f, 1.0f, 1.6f, 1.2f);
    auto fres_irid9 = std::make_shared<FresnelThinfilm>(999.f, 1.0f, 1.6f, 1.2f);
    // マテリアル
    auto mat_ggx1 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid1);
    auto mat_ggx2 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid2);
    auto mat_ggx3 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid3);
    auto mat_ggx4 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid4);
    auto mat_ggx5 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid5);
    auto mat_ggx6 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid6);
    auto mat_ggx7 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid7);
    auto mat_ggx8 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid8);
    auto mat_ggx9 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid9);
    // 球
    auto spehre1 = std::make_shared<Sphere>(Vec3(-2.2f,-0.2f,0.0f), 1.0f, mat_ggx1);
    auto spehre2 = std::make_shared<Sphere>(Vec3( 0.0f,-0.2f,0.0f), 1.0f, mat_ggx2);
    auto spehre3 = std::make_shared<Sphere>(Vec3( 2.2f,-0.2f,0.0f), 1.0f, mat_ggx3);
    auto spehre4 = std::make_shared<Sphere>(Vec3(-2.2f, 2.0f,0.0f), 1.0f, mat_ggx4);
    auto spehre5 = std::make_shared<Sphere>(Vec3( 0.0f, 2.0f,0.0f), 1.0f, mat_ggx5);
    auto spehre6 = std::make_shared<Sphere>(Vec3( 2.2f, 2.0f,0.0f), 1.0f, mat_ggx6);
    auto spehre7 = std::make_shared<Sphere>(Vec3(-2.2f, 4.2f,0.0f), 1.0f, mat_ggx7);
    auto spehre8 = std::make_shared<Sphere>(Vec3( 0.0f, 4.2f,0.0f), 1.0f, mat_ggx8);
    auto spehre9 = std::make_shared<Sphere>(Vec3( 2.2f, 4.2f,0.0f), 1.0f, mat_ggx9);
    // オブジェクトをシーンに追加
    world.add(spehre1);
    world.add(spehre2);
    world.add(spehre3);
    world.add(spehre4);
    world.add(spehre5);
    world.add(spehre6);
    world.add(spehre7);
    world.add(spehre8);
    world.add(spehre9);
    // カメラの設定
    auto film = std::make_shared<Film>(600, 600, 3, "spehre.png"); // フィルム
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f,2.0f,-10.0f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = -unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief 花瓶モデルのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
* @note モデル出典: https://polyhaven.com/a/ceramic_vase_01
*/
void make_scene_vase(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
    auto fres     = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
    auto mat_ggx  = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres);
    // オブジェクト
    auto obj_pot = std::make_shared<TriangleMesh>("asset/model.obj", mat_ggx);
    world.add(obj_pot);

    // カメラの設定
    auto film = std::make_shared<Film>(600, 600, 3, "vase.png"); // フィルム
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f,2.0f,7.5f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief 明示的に直接光をサンプリング
* @pram[in] r     :入射レイ
* @pram[in] isect :交差点情報
* @pram[in] world :シーン
* @return Vec3 :光源の重み付き放射輝度
*/
Vec3 explicit_direct_light(const Ray& r, const intersection& isect, const Scene& world) {
    auto Ld = Vec3::zero;
    auto contrib = Vec3::one;
    // 完全鏡面反射
    if (isect.mat->get_type() == MaterialType::Specular) {
        return Ld;
    }
    // シェーディング座標の構築
    ONB shadingCoord(isect.normal);
    Vec3 wi = unit_vector(r.get_dir());
    Vec3 wi_local = -shadingCoord.to_local(wi);
    Vec3 wo_scattering_local;
    Vec3 brdf;
    float pdf_scattering;
    isect.mat->sample_f(wi_local, isect, brdf, wo_scattering_local, pdf_scattering);
    Vec3 wo_scattering = unit_vector(shadingCoord.to_world(wo_scattering_local)); // 散乱方向
    float cos_term = dot(isect.normal, wo_scattering);
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
    // シーン中の光源を一つサンプリング
    if ((sampling_strategy == Sampling::LIGHT) || (sampling_strategy == Sampling::MIS)) {
        const auto& lights = world.get_light();
        auto light_index = Random::uniform_int(0, lights.size() - 1);
        const auto& light = lights[light_index];
        float pdf_light = 0.0f;
        Vec3 wo_light;
        Vec3 L = light->sample_light(isect, wo_light, pdf_light);
        if (is_zero(L) || pdf_light == 0) {
            return Ld;
        }
        // 光源の可視判定
        auto r_light = Ray(isect.pos, wo_light);
        intersection isect_light;
        light->intersect(r_light, eps_isect, inf, isect_light); // 光源の交差点情報を取得
        if (world.intersect_object(r_light, eps_isect, isect_light.t)) {
            return Ld;
        }
        // 光源サンプリング時のBRDFを評価
        auto wo_light_local = shadingCoord.to_local(unit_vector(wo_light));
        auto brdf_light = isect.mat->f(wi_local, wo_light_local);
        auto cos_term_light = std::abs(dot(isect.normal, unit_vector(wo_light)));
        if (sampling_strategy == Sampling::MIS) {
            pdf_scattering = isect.mat->sample_pdf(wi_local, wo_light_local);
            weight = Random::power_heuristic(1, pdf_light, 1, pdf_scattering);
        }
        Ld += contrib * brdf_light * L * cos_term_light * weight / pdf_light;
    }
    return Ld;
}

/**
* @brief レイに沿った放射輝度伝搬を計算する関数
* @param[in]  r         :追跡するレイ
* @param[in]  bouunce   :現在のレイのバウンス回数
* @param[in]  max_depth :レイの最大バウンス回数
* @param[in]  world     :レンダリングするシーンのデータ
* @param[in]  contrib   :現在のレイの寄与
* @return Vec3          :レイに沿った放射輝度
* @note 参考: https://www.pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/Path_Tracing
*
*/
// TODO: veach_misだと上手くパストレ出来ない
// 1. マイクロファセットBRDFが原因? コーネルボックスで検証(上手くいった)
// 2. 複数光源が原因? 光源を一つだけにする
Vec3 path_tracing(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    bool is_prev_specular = false;
    // パストレーシング
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // 交差点情報
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        // 光源サンプリングの例外(カメラレイとスペキュラレイの場合)
        if (bounces == 0 || is_prev_specular) {
            if (is_intersect) {
                if (isect.type == IsectType::Light) {
                    L += contrib * isect.light->emitte();
                    break;
                }
            }
            else {
                L += contrib * world.sample_envmap(r);
            }
        }
        // 光源交差時に終了(明示的に光源サンプリングを行うため)
        if (isect.type == IsectType::Light) {
            break;
        }
        if (!is_intersect) break;
        // 光源サンプリング
        L += contrib * explicit_direct_light(r, isect, world);
        // BRDFのサンプリング
        // NOTE: カメラ方向をwi，光源方向をwoにしている
        ONB shadingCoord(isect.normal);
        Vec3 wi_local = -shadingCoord.to_local(unit_vector(r.get_dir()));
        Vec3 wo_local, brdf;
        float pdf;
        isect.mat->sample_f(wi_local, isect, brdf, wo_local, pdf);
        auto wo = shadingCoord.to_world(unit_vector(wo_local)); // ワールド座標系の出射方向
        float cos_term = std::abs(dot(isect.normal, wo));
        contrib = contrib * brdf * cos_term / pdf;
        is_prev_specular = (isect.mat->get_type() == MaterialType::Specular) ? true : false;
        r = Ray(isect.pos, wo); // 次のレイを生成
        //ロシアンルーレット
        if (bounces >= 3) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // 打ち切り確率
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }
    }
    return L;
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
// TODO: 再帰でなくループで実装
Vec3 L_direct(const Ray& r, int bounces, int max_depth, const Scene& world, Vec3 contrib) {
    if (bounces >= max_depth) {
        return Vec3::zero;
    }
    intersection isect;
    if (!world.intersect(r, eps_isect, inf, isect)) {
        return contrib * world.sample_envmap(r);
    }
    // 交差点がマテリアルの場合
    if (isect.type == IsectType::Material) {
        // 発光体
        if (isect.mat->get_type() == MaterialType::Emitter) {
            return contrib * isect.mat->emitte();
        }
        // 完全鏡面反射
        if (isect.mat->get_type() == MaterialType::Specular) {
            // シェーディング座標の構築
            ONB shadingCoord(isect.normal);
            Vec3 wi = unit_vector(r.get_dir());
            Vec3 wi_local = -shadingCoord.to_local(wi);
            Vec3 wo_scattering_local;
            Vec3 brdf;
            float pdf_scattering;
            isect.mat->sample_f(wi_local, isect, brdf, wo_scattering_local, pdf_scattering);
            Vec3 wo_scattering = unit_vector(shadingCoord.to_world(wo_scattering_local)); // 散乱方向
            float cos_term = dot(isect.normal, wo_scattering);
            contrib = contrib * brdf * cos_term / pdf_scattering;
            return L_direct(Ray(isect.pos, wo_scattering), ++bounces, max_depth, world, contrib);
        }
        // 拡散・光沢反射
        else {
            return explicit_direct_light(r, isect, world);
        }
    }
    // 光源の場合
    else {
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
    //make_scene_simple(world, cam);
    //make_scene_cylinder(world, cam);
    make_scene_MIS(world, cam);
    //make_scene_cornell(world, cam);
    //make_scene_sphere(world, cam);
    //make_scene_vase(world, cam);
    // 出力画像
    const auto w = cam.get_w(); // 高さ
    const auto h = cam.get_h(); // 幅
    const auto c = cam.get_c(); // チャンネル数
    std::vector<uint8_t> img(w * h * c);  // 画像データ

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
                        I += path_tracing(r, max_depth, world);
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
     // 画像出力
    stbi_write_png(cam.get_filename(), w, h, 3, img.data(), w * c * sizeof(uint8_t));
}