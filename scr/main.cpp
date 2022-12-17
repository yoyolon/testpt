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
#define DEBUG

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


// デバッグ用
int raycontrib = 0; // 光源からの寄与
int raydeath = 0;   // ロシアンルーレット打ち切り
int raybg = 0;      // 背景からの寄与

/**
* @brief シンプルな球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @param[in]  float :カメラのアスペクト比
*/
void make_scene_simple(Scene& world, Camera& cam, float aspect) {
    world.clear();
    // マテリアル
    auto dist_ggx = std::make_shared<GGXDistribution>(0.25f);
    auto fres_schlick = std::make_shared<FresnelSchlick>(Vec3(0.9f, 0.9f, 0.9f));
    auto mat_microfacet = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_schlick);
    auto mat_light = std::make_shared<Emitter>(Vec3(10.00f, 10.00f, 10.00f));
    // オブジェクト
    auto obj_sphere = std::make_shared<Sphere>(Vec3(0.0f, 2.0f, 0.0f), 3.0f, mat_microfacet);
    auto light_shape_disk = std::make_shared<Disk>(Vec3(0.0f, 20.0f, 0.0f), 50.0f, mat_light);
    auto light_disk = std::make_shared<AreaLight>(Vec3(10.0f, 10.0f, 10.0f), light_shape_disk);
    world.add(obj_sphere);
    world.add(light_disk);

    // カメラの設定
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f, 2.0f, 5.0f);
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
void make_scene_shape_test(Scene& world, Camera& cam, float aspect) {
    world.clear();
    // マテリアル
    auto mat_mirr = std::make_shared<Mirror>(Vec3(0.9f, 0.9f, 0.9f));
    auto mat_light = std::make_shared<Emitter>(Vec3(10.00f, 10.00f, 10.00f));
    // オブジェクト
    auto obj_sphere = std::make_shared<Sphere>(Vec3(0.0f, 0.0f, 0.0f), 2.0f, mat_mirr);
    auto obj_disk_top = std::make_shared<Disk>(Vec3(0.0f, 8.0f, 0.0f), 4.0f, mat_mirr);
    auto obj_disk_btm = std::make_shared<Disk>(Vec3(0.0f, 0.0f, 0.0f), 4.0f, mat_mirr);
    auto obj_cylinder = std::make_shared<Cylinder>(Vec3(0.0f, 0.0f, 0.0f), 4.0f, 8.0f, mat_mirr);
    world.add(obj_disk_top);
    world.add(obj_disk_btm);
    world.add(obj_sphere);
    world.add(obj_cylinder);

    // カメラの設定
    auto fd = 1.5f; // 焦点距離
    Vec3 cam_pos(0.0f, 2.0f, 20.0f);
    Vec3 cam_target(0.0f, 2.0f, 0.0f);
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
    auto mat_microfacet = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_schlick);
    auto mat_red   = std::make_shared<Diffuse>(Vec3(0.5694f, 0.043f, 0.0451f));
    auto mat_green = std::make_shared<Diffuse>(Vec3(0.1039f, 0.3778f, 0.0768f));
    auto mat_white = std::make_shared<Diffuse>(Vec3(0.886f, 0.6977f, 0.6676f));
    auto mat_mirr  = std::make_shared<Mirror>(Vec3(0.99f, 0.99f, 0.99f));
    auto mat_light = std::make_shared<Emitter>(Vec3(20.6904f, 10.8669f, 2.7761f));
    auto mat_phong = std::make_shared<Phong>(Vec3(1.0, 1.0, 1.0), Vec3(0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), 20.0f);
    // light sorce
    auto light_shape = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(343.0f, 548.7f, 227.0f),
            Vec3(343.0f, 548.7f, 332.0f),
            Vec3(213.0f, 548.7f, 332.0f),
            Vec3(213.0f, 548.7f, 227.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_light,
        Vec3(0.0f, 0.0f, 0.0f));
    auto radiance = Vec3(20.6904f, 10.8669f, 2.7761f);
    auto light = std::make_shared<AreaLight>(radiance, light_shape);
    // light sorce2
    auto light_shape_disk = std::make_shared<Disk>(Vec3(278.0f, 548.7f, 279.6f), 50.0f, mat_light);
    auto light_disk = std::make_shared<AreaLight>(radiance, light_shape_disk);
    // ceiling
    auto ceiling = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(556.0f, 548.8f,   0.0f),
            Vec3(556.0f, 548.8f, 559.2f),
            Vec3(  0.0f, 548.8f, 559.2f),
            Vec3(  0.0f, 548.8f,   0.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    // floor
    auto floor = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(552.8f, 0.0f,   0.0f),
            Vec3(  0.0f, 0.0f,   0.0f),
            Vec3(  0.0f, 0.0f, 559.2f),
            Vec3(549.6f, 0.0f, 559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    // back wall
    auto back = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(549.6f,   0.0f, 559.2f),
            Vec3(  0.0f,   0.0f, 559.2f),
            Vec3(  0.0f, 548.8f, 559.2f),
            Vec3(556.0f, 548.8f, 559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    // right wall
    auto right = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(0.0f,   0.0f, 559.2f),
            Vec3(0.0f,   0.0f,   0.0f),
            Vec3(0.0f, 548.8f,   0.0f),
            Vec3(0.0f, 548.8f, 559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_green,
        Vec3(0.0f, 0.0f, 0.0f));
    // left wall
    auto left = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(552.8f,   0.0f,   0.0f),
            Vec3(549.6f,   0.0f, 559.2f),
            Vec3(556.0f, 548.8f, 559.2f),
            Vec3(556.0f, 548.8f,   0.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_red,
        Vec3(0.0f, 0.0f, 0.0f));
    // Short block
    auto short_top = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(130.0f, 165.0f,  65.0f),
            Vec3( 82.0f, 165.0f, 225.0f),
            Vec3(240.0f, 165.0f, 272.0f),
            Vec3(290.0f, 165.0f, 114.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    auto short_rgt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(290.0f,   0.0f, 114.0f),
            Vec3(290.0f, 165.0f, 114.0f),
            Vec3(240.0f, 165.0f, 272.0f),
            Vec3(240.0f,   0.0f, 272.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    auto short_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(130.0f,   0.0f,  65.0f),
            Vec3(130.0f, 165.0f,  65.0f),
            Vec3(290.0f, 165.0f, 114.0f),
            Vec3(290.0f,   0.0f, 114.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    auto short_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3( 82.0f,   0.0f, 225.0f),
            Vec3( 82.0f, 165.0f, 225.0f),
            Vec3(130.0f, 165.0f,  65.0f),
            Vec3(130.0f,   0.0f,  65.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    auto short_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(240.0f,   0.0f, 272.0f),
            Vec3(240.0f, 165.0f, 272.0f),
            Vec3( 82.0f, 165.0f, 225.0f),
            Vec3( 82.0f,   0.0f, 225.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    // Tall block
    auto tall_top = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(423.0f, 330.0f, 247.0f),
            Vec3(265.0f, 330.0f, 296.0f),
            Vec3(314.0f, 330.0f, 456.0f),
            Vec3(472.0f, 330.0f, 406.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    auto tall_rgt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(423.0f,   0.0f, 247.0f),
            Vec3(423.0f, 330.0f, 247.0f),
            Vec3(472.0f, 330.0f, 406.0f),
            Vec3(472.0f,   0.0f, 406.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    auto tall_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(472.0f,   0.0f, 406.0f),
            Vec3(472.0f, 330.0f, 406.0f),
            Vec3(314.0f, 330.0f, 456.0f),
            Vec3(314.0f,   0.0f, 456.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    auto tall_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(314.0f,   0.0f, 456.0f),
            Vec3(314.0f, 330.0f, 456.0f),
            Vec3(265.0f, 330.0f, 296.0f),
            Vec3(265.0f,   0.0f, 296.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
    auto tall_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(265.0f,   0.0f, 296.0f),
            Vec3(265.0f, 330.0f, 296.0f),
            Vec3(423.0f, 330.0f, 247.0f),
            Vec3(423.0f,   0.0f, 247.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white,
        Vec3(0.0f, 0.0f, 0.0f));
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
    //world.add(light);
    world.add(light_disk);

    //// カメラの設定
    auto fd = 0.035f; // 焦点距離
    Vec3 cam_pos(278.0f, 273.0f, -800.0f);
    Vec3 cam_target(278.0f, 273.0f, 0.0f);
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
    auto fres_Schlick = std::make_shared<FresnelSchlick>(Vec3(1.00f, 0.86f, 0.57f));
    auto mat_microfacet = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_Schlick);
    // オブジェクト
    std::string vasepath = "asset/model.obj";
    std::cout << vasepath << '\n';
    auto obj_pot = std::make_shared<TriangleMesh>(vasepath, mat_microfacet, Vec3(0.0f, 0.0f, 0.0f));
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
    if (bounces >= max_depth) return Vec3(0.0f, 0.0f, 0.0f);

    // 交差判定
    // NOTE: カメラ方向をwi，光源方向をwoにしている(あとで修正)
    intersection isect;
    if (world.intersect(r, epsilon, inf, isect)) {
        // シェーディング座標の構築
        ONB shadingCoord;
        shadingCoord.build_ONB(isect.normal);
        Vec3 wi_local = -shadingCoord.to_local(r.get_dir());
        Vec3 wo_local;
        Vec3 wo;
        Vec3 brdf;
        float pdf;
        // マテリアルの場合
        if (isect.type == IsectType::Material) {
            isect.mat->f(wi_local, isect, brdf, wo_local, pdf);
            // 発光
            if (isect.mat->get_type() == MaterialType::Emitter) {
                std::cout << "emitter\n";
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
                 //明示的な光源のサンプリング
                    //float pdfl = 0.0f;
                    //Vec3 wl;
                    //for (const auto& light : world.get_light()) {
                    //    Vec3 L = light->sample_light(isect, wl, pdfl);
                    //    // 可視判定
                    //    Ray r_l(isect.pos, wl);
                    //    intersection null_p;
                    //    if (world.intersect_object(r_l, epsilon, inf, null_p)) {
                    //        pdfl = 0.0f;
                    //    }
                    //    Vec3 brdfl;
                    //    auto wl_local = -shadingCoord.to_local(wl);
                    //    isect.mat->f(wl_local, isect, brdfl, wl_local, pdfl);
                    //    auto cos_term_l = std::abs(dot(isect.normal, unit_vector(-wl)));
                    //    //return Vec3(1.0f, 0.0f, 0.0f);
                    //    contrib = contrib * brdfl * L * cos_term_l / pdfl;
                    //    //return contrib * brdf_l * L * cos_term_l / pdf_l;
                    //}
            }
            //ロシアンルーレット
            //TODO: あとでPBRTを確認
            if (bounces >= 5) {
                float p_rr = std::max(0.5f, contrib.average()); // レイ追跡の継続確率
                if (p_rr < Random::uniform_float()) {
                    raydeath++;
                    return Vec3(0.0f, 0.0f, 0.0f);
                }
                else {
                    contrib /= p_rr;
                }
            }
            return L(Ray(isect.pos, wo), ++bounces, max_depth, world, contrib);
        }
        // 光源の場合
        else if (isect.type == IsectType::Light) {
            raycontrib++;
            return contrib * isect.light->emitte();
        }
    }
    raybg++;
    return contrib * world.sample_envmap(r);
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
    if (world.intersect(r, epsilon, inf, isect)) {
        return 0.5f * (isect.normal + Vec3(1.0f, 1.0f, 1.0f));
    }
    return Vec3(1.0f, 1.0f, 1.0f);
}


/**
* @brief main関数
*/
int main(int argc, char* argv[]) {
    Random::init(); // 乱数の初期化
    // 出力画像
    const char* filename = "image.png";  // パス
    constexpr auto aspect = 3.0f / 3.0f;     // アスペクト比
    constexpr int h = 600;                   // 高さ
    constexpr int w = int(h * aspect);       // 幅
    constexpr int c = 3;                     // チャンネル数
    std::vector<uint8_t> img(w * h * c); // 画像データ
    // パラメータ
    const int nsample = (argc == 2) ? atoi(argv[1]) : 1; // レイのサンプル数
    constexpr auto max_depth = 100; // レイの最大追跡数
    constexpr auto gamma = 1 / 2.2f;    // ガンマ補正用
    // シーン
    //int w_envmap, h_envmap, c_envmap;
    //float* envmap = stbi_loadf("asset/envmap3.hdr", &w_envmap, &h_envmap, &c_envmap, 0);
    //Scene world(envmap, w_envmap, h_envmap, c_envmap);
    auto world = Scene();
    Camera cam;
    //make_scene_simple(world, cam, aspect);
    make_scene_shape_test(world, cam, aspect);
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
                #ifndef DEGUB
                I += L_normal(r, world);
                #else
                I += L(r, 0, max_depth, world, Vec3(1.0f,1.0f,1.0f));
                #endif
            }
            I *= 1.0f / nsample;
            float r = std::pow(I.get_x(), gamma);
            float g = std::pow(I.get_y(), gamma);
            float b = std::pow(I.get_z(), gamma);
            img[index++] = static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255);
            img[index++] = static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255);
            img[index++] = static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255);
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