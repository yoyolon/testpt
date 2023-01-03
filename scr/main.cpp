//---------------------------------------------------------------------------------------
// testpt
// yoyolon - December 2022
// 
// Development
// Microsoft Visual Studio 2019
// C++17
// 
// External Library
// stb: https://github.com/nothings/stb
// 
// Reference(this soft ware is based on listed below.)
// Raytracing in one weekend: https://raytracing.github.io/
// pbrt-v3: https://pbrt.org/
// 
// More information on these licenses can be found in NOTICE.txt
//---------------------------------------------------------------------------------------

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
Sampling sampling_strategy = Sampling::MIS;

// デバッグ用
constexpr bool DEBUG_MODE           = false; // 法線可視化を有効にする
constexpr bool GLOBAL_ILLUMINATION  = true;  // 大域照明効果(GI)を有効にする
constexpr bool IMAGE_BASED_LIGHTING = false; // IBLを有効にする
constexpr bool IS_GAMMA_CORRECTION  = true;  // ガンマ補正を有効にする
constexpr int  SAMPLES = 4;                // 1ピクセル当たりのサンプル数


/**
* @brief シンプルな球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
//void make_scene_simple(Scene& world, Camera& cam) {
//    world.clear();
//    // マテリアル
//    auto dist_ggx       = std::make_shared<GGXDistribution>(0.025f);
//    auto fres_schlick   = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
//    auto mat_microfacet = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_schlick);
//    auto mat_mirr       = std::make_shared<Mirror>(Vec3(0.9f,0.9f,0.9f));
//
//    // オブジェクト
//    auto obj_sphere       = std::make_shared<Sphere>(Vec3(0.0f,2.0f,0.0f), 3.0f, mat_microfacet);
//    auto light_shape_disk = std::make_shared<Disk>(Vec3(0.0f,20.0f,0.0f), 50.0f, nullptr);
//    auto light_disk       = std::make_shared<AreaLight>(Vec3(1.0f,1.0f,1.0f), light_shape_disk);
//    world.add(obj_sphere);
//    world.add(light_disk);
//
//    // カメラ設定
//    auto film = std::make_shared<Film>(600, 600, 3, "simple.png"); // フィルム
//    auto fd = 2.5f; // 焦点距離
//    Vec3 cam_pos(0.0f,2.0f,15.0f);
//    Vec3 cam_target(0.0f,2.0f,0.0f);
//    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
//    cam = Camera(film, fd, cam_pos, cam_forward);
//}

/**
* @brief 円柱のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
//void make_scene_cylinder(Scene& world, Camera& cam) {
//    world.clear();
//    // マテリアル
//    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
//    auto fres = std::make_shared<FresnelSchlick>(Vec3(1.00f, 0.71f, 0.29f));
//    auto mat_gold = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres);
//    auto mat_mirr  = std::make_shared<Mirror>(Vec3(0.9f,0.9f,0.9f));
//
//    // オブジェクト
//    auto obj_disk_top = std::make_shared<Disk>(Vec3(0.0f,4.0f,0.0f), 4.0f, mat_gold, true);
//    auto obj_disk_btm = std::make_shared<Disk>(Vec3::zero, 4.0f, mat_gold);
//    auto obj_cylinder = std::make_shared<Cylinder>(Vec3(0.0f,-4.0f,0.0f), 4.0f, 8.0f, mat_gold);
//    auto light_shape  = std::make_shared<Disk>(Vec3(0.0f,7.5f,0.0f), 4.0f, nullptr);
//    auto light        = std::make_shared<AreaLight>(Vec3(10.0f,10.0f,10.0f), light_shape);
//    world.add(obj_disk_top);
//    world.add(obj_disk_btm);
//    world.add(obj_cylinder);
//    world.add(light);
//
//    // カメラ設定
//    auto film = std::make_shared<Film>(768, 512, 3, "cylinder.png"); // フィルム
//    auto fd = 2.5f; // 焦点距離
//    Vec3 cam_pos(0.0f,10.0f,20.0f);
//    Vec3 cam_target(0.0f,0.0f,0.0f);
//    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
//    cam = Camera(film, fd, cam_pos, cam_forward);
//}

/**
* @brief 重点的サンプリング検証用シーン
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note モデル出典: https://github.com/wjakob/nori/tree/master/scenes/pa5/veach_mi
*       このソフトウェアに上記モデルは付属しておりません.
*/
//void make_scene_MIS(Scene& world, Camera& cam) {
//    world.clear();
//    // マテリアル
//    auto dist1 = std::make_shared<GGXDistribution>(0.100f);
//    auto dist2 = std::make_shared<GGXDistribution>(0.050f);
//    auto dist3 = std::make_shared<GGXDistribution>(0.020f);
//    auto dist4 = std::make_shared<GGXDistribution>(0.005f);
//    auto fres  = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
//    auto mat_vrough = std::make_shared<Microfacet>(Vec3(0.03f, 0.03f, 0.03f), dist1, fres);
//    auto mat_rough  = std::make_shared<Microfacet>(Vec3(0.03f, 0.03f, 0.03f), dist2, fres);
//    auto mat_normal = std::make_shared<Microfacet>(Vec3(0.03f, 0.03f, 0.03f), dist3, fres);
//    auto mat_smooth = std::make_shared<Microfacet>(Vec3(0.03f, 0.03f, 0.03f), dist4, fres);
//    auto mat_diff   = std::make_shared<Diffuse>(Vec3(0.1f,0.1f,0.1f));
//    auto mat_mirr   = std::make_shared<Mirror>(Vec3(1.0f, 1.0f, 1.0f));
//
//    // 光源
//    auto r = Vec3(1.00f,0.00f,0.00f) * 10;
//    auto y = Vec3(1.00f,1.00f,0.00f) * 10;
//    auto g = Vec3(0.00f,1.00f,0.00f) * 10;
//    auto b = Vec3(0.00f,0.00f,1.00f) * 10;
//    auto sphere_XL = std::make_shared<Sphere>(Vec3( 3.75f,0.0f,0.0f), 0.90f, nullptr);
//    auto sphere_L  = std::make_shared<Sphere>(Vec3( 1.25f,0.0f,0.0f), 0.30f, nullptr);
//    auto sphere_M  = std::make_shared<Sphere>(Vec3(-1.25f,0.0f,0.0f), 0.10f, nullptr);
//    auto sphere_S  = std::make_shared<Sphere>(Vec3(-3.75f,0.0f,0.0f), 0.03f, nullptr);
//    auto sphere    = std::make_shared<Sphere>(Vec3(  0.0f,4.0f,3.0f), 1.0f, nullptr);
//    auto light_XL  = std::make_shared<AreaLight>(Vec3(  1.2f,  1.2f,  1.2f)*b, sphere_XL);
//    auto light_L   = std::make_shared<AreaLight>(Vec3( 11.1f, 11.1f, 11.1f)*g, sphere_L);
//    auto light_M   = std::make_shared<AreaLight>(Vec3(100.0f,100.0f,100.0f)*y, sphere_M);
//    auto light_S   = std::make_shared<AreaLight>(Vec3(900.0f,900.0f,900.0f)*r, sphere_S);
//    auto light     = std::make_shared<AreaLight>(Vec3(100.0f,100.0f,100.0f), sphere);
//
//    // オブジェクト
//    auto plate1 = std::make_shared<TriangleMesh>("asset/veach_mis/plate1.obj", mat_smooth);
//    auto plate2 = std::make_shared<TriangleMesh>("asset/veach_mis/plate2.obj", mat_normal);
//    auto plate3 = std::make_shared<TriangleMesh>("asset/veach_mis/plate3.obj", mat_rough);
//    auto plate4 = std::make_shared<TriangleMesh>("asset/veach_mis/plate4.obj", mat_vrough);
//    auto floor  = std::make_shared<TriangleMesh>("asset/veach_mis/floor.obj" , mat_diff);
//    world.add(light_XL);
//    world.add(light_L);
//    world.add(light_M);
//    world.add(light_S);
//    //world.add(light);
//    world.add(floor);
//    world.add(plate1);
//    world.add(plate2);
//    world.add(plate3);
//    world.add(plate4);
//
//    // カメラ設定
//    auto film = std::make_shared<Film>(1280, 720, 3, "veach_mis.png"); // フィルム
//    auto fd = 4.732f * film->get_aspect();
//    Vec3 cam_pos(0.0f, 6.0f, 27.5f);
//    Vec3 cam_target(0.0f, -1.5f, 2.5f);
//    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
//    cam = Camera(film, fd, cam_pos, cam_forward);
//}

/**
* @brief コーネルボックスのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note 参考: http://www.graphics.cornell.edu/online/box/data.html
*/
void make_scene_cornell_box(Scene& world, Camera& cam) {
    world.clear();
    //  マテリアル
    //auto dist_ggx = std::make_shared<GGXDistribution>(0.15f);
    //auto fres     = std::make_shared<FresnelSchlick>(Vec3(1.00f, 0.71f, 0.29f));
    //auto mat_ggx  = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres);
    //auto dist_ggx2 = std::make_shared<GGXDistribution>(0.1f);
    //auto fres2 = std::make_shared<FresnelSchlick>(Vec3(1.00f, 1.00f, 1.00f));
    //auto mat_ggx2 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx2, fres2);
    //auto mat_mirr = std::make_shared<Mirror>(Vec3(0.9f, 0.9f, 0.9f));

    // カラーバリエーション1
    //auto mat_red   = std::make_shared<Diffuse>(Vec3( 0.5694f,  0.0430f, 0.0451f));
    //auto mat_green = std::make_shared<Diffuse>(Vec3( 0.1039f,  0.3778f, 0.0768f));
    //auto mat_white = std::make_shared<Diffuse>(Vec3( 0.8860f,  0.6977f, 0.6676f));
    //auto radiance = Vec3(20.6904f, 10.8669f, 2.7761f);

    // カラーバリエーション2
    auto mat_red   = std::make_shared<Diffuse>(Vec3(1.000f, 0.065f, 0.065f));
    auto mat_green = std::make_shared<Diffuse>(Vec3(0.065f, 0.065f, 1.000f));
    auto mat_white = std::make_shared<Diffuse>(Vec3(0.710f, 0.710f, 0.710f));
    auto radiance  = Vec3(10.0f, 10.0f, 10.0f);

    // ボックスの色
    auto& mat_tall  = mat_white;
    auto& mat_short = mat_white;

    // Light sorce
    auto light_shape = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-343.0f, 543.7f, -227.0f),
            Vec3(-343.0f, 543.7f, -332.0f),
            Vec3(-213.0f, 543.7f, -332.0f),
            Vec3(-213.0f, 543.7f, -227.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        nullptr);
    auto light = std::make_shared<AreaLight>(radiance, light_shape);
    // Ceiling
    auto ceiling = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-556.0f, 548.8f,   0.0f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(  0.0f,  548.8f, -559.2f),
            Vec3(  0.0f,  548.8f,   0.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    // Floor
    auto floor = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f, 0.0f,    0.0f),
            Vec3(  0.0f,  0.0f,    0.0f),
            Vec3(  0.0f,  0.0f, -559.2f),
            Vec3(-549.6f, 0.0f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    // Back wall
    auto back = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-549.6f,   0.0f, -559.2f),
            Vec3(   0.0f,   0.0f, -559.2f),
            Vec3(   0.0f, 548.8f, -559.2f),
            Vec3(-556.0f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_white);
    // Right wall
    auto right = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(0.0f,   0.0f, -559.2f),
            Vec3(0.0f,   0.0f,    0.0f),
            Vec3(0.0f, 548.8f,    0.0f),
            Vec3(0.0f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_green);
    // Left wall
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
        mat_short);
    auto short_rgt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-290.0f,   0.0f, -114.0f),
            Vec3(-290.0f, 165.0f, -114.0f),
            Vec3(-240.0f, 165.0f, -272.0f),
            Vec3(-240.0f,   0.0f, -272.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_short);
    auto short_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-130.0f,   0.0f,  -65.0f),
            Vec3(-130.0f, 165.0f,  -65.0f),
            Vec3(-290.0f, 165.0f, -114.0f),
            Vec3(-290.0f,   0.0f, -114.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_short);
    auto short_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3( -82.0f,   0.0f, -225.0f),
            Vec3( -82.0f, 165.0f, -225.0f),
            Vec3(-130.0f, 165.0f,  -65.0f),
            Vec3(-130.0f,   0.0f,  -65.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_short);
    auto short_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-240.0f,   0.0f, -272.0f),
            Vec3(-240.0f, 165.0f, -272.0f),
            Vec3( -82.0f, 165.0f, -225.0f),
            Vec3( -82.0f,   0.0f, -225.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_short);
    // Tall block
    auto tall_top = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-423.0f, 330.0f, -247.0f),
            Vec3(-265.0f, 330.0f, -296.0f),
            Vec3(-314.0f, 330.0f, -456.0f),
            Vec3(-472.0f, 330.0f, -406.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_tall);
    auto tall_rgt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-423.0f,   0.0f, -247.0f),
            Vec3(-423.0f, 330.0f, -247.0f),
            Vec3(-472.0f, 330.0f, -406.0f),
            Vec3(-472.0f,   0.0f, -406.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_tall);
    auto tall_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-472.0f,   0.0f, -406.0f),
            Vec3(-472.0f, 330.0f, -406.0f),
            Vec3(-314.0f, 330.0f, -456.0f),
            Vec3(-314.0f,   0.0f, -456.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_tall);
    auto tall_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-314.0f,   0.0f, -456.0f),
            Vec3(-314.0f, 330.0f, -456.0f),
            Vec3(-265.0f, 330.0f, -296.0f),
            Vec3(-265.0f,   0.0f, -296.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_tall);
    auto tall_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-265.0f,   0.0f, -296.0f),
            Vec3(-265.0f, 330.0f, -296.0f),
            Vec3(-423.0f, 330.0f, -247.0f),
            Vec3(-423.0f,   0.0f, -247.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_tall);
    // オブジェクトをシーンに追加
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

    //// カメラ設定
    auto film = std::make_shared<Film>(1000, 1000, 3, "cornell_box.png"); // フィルム
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(35.0f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
    Vec3 cam_pos(-278.0f, 273.0f, 700.0f);
    Vec3 cam_target(-278.0f, 273.0f, 0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos); // z軸負の方向がカメラの前方
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief 異なる膜厚の薄膜がコーティングされた球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
//void make_scene_sphere(Scene& world, Camera& cam) {
//    world.clear();
//
//    // マテリアル
//    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
//    auto fres_schlick = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
//    auto fres_irid1 = std::make_shared<FresnelThinfilm>(200.f, 1.0f, 1.6f, 1.0f);
//    auto fres_irid2 = std::make_shared<FresnelThinfilm>(300.f, 1.0f, 1.6f, 1.0f);
//    auto fres_irid3 = std::make_shared<FresnelThinfilm>(400.f, 1.0f, 1.6f, 1.0f);
//    auto fres_irid4 = std::make_shared<FresnelThinfilm>(500.f, 1.0f, 1.6f, 1.0f);
//    auto fres_irid5 = std::make_shared<FresnelThinfilm>(600.f, 1.0f, 1.6f, 1.0f);
//    auto fres_irid6 = std::make_shared<FresnelThinfilm>(700.f, 1.0f, 1.6f, 1.0f);
//    auto fres_irid7 = std::make_shared<FresnelThinfilm>(800.f, 1.0f, 1.6f, 1.0f);
//    auto fres_irid8 = std::make_shared<FresnelThinfilm>(900.f, 1.0f, 1.6f, 1.0f);
//    auto fres_irid9 = std::make_shared<FresnelThinfilm>(999.f, 1.0f, 1.6f, 1.0f);
//    auto mat_ggx  = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_schlick);
//    auto mat_ggx1 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid1);
//    auto mat_ggx2 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid2);
//    auto mat_ggx3 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid3);
//    auto mat_ggx4 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid4);
//    auto mat_ggx5 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid5);
//    auto mat_ggx6 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid6);
//    auto mat_ggx7 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid7);
//    auto mat_ggx8 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid8);
//    auto mat_ggx9 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid9);
//
//    // オブジェクト
//    auto spehre1 = std::make_shared<Sphere>(Vec3(-2.2f,-0.2f,0.0f), 1.0f, mat_ggx1);
//    auto spehre2 = std::make_shared<Sphere>(Vec3( 0.0f,-0.2f,0.0f), 1.0f, mat_ggx2);
//    auto spehre3 = std::make_shared<Sphere>(Vec3( 2.2f,-0.2f,0.0f), 1.0f, mat_ggx3);
//    auto spehre4 = std::make_shared<Sphere>(Vec3(-2.2f, 2.0f,0.0f), 1.0f, mat_ggx4);
//    auto spehre5 = std::make_shared<Sphere>(Vec3( 0.0f, 2.0f,0.0f), 1.0f, mat_ggx5);
//    auto spehre6 = std::make_shared<Sphere>(Vec3( 2.2f, 2.0f,0.0f), 1.0f, mat_ggx6);
//    auto spehre7 = std::make_shared<Sphere>(Vec3(-2.2f, 4.2f,0.0f), 1.0f, mat_ggx7);
//    auto spehre8 = std::make_shared<Sphere>(Vec3( 0.0f, 4.2f,0.0f), 1.0f, mat_ggx8);
//    auto spehre9 = std::make_shared<Sphere>(Vec3( 2.2f, 4.2f,0.0f), 1.0f, mat_ggx9);
//    world.add(spehre1);
//    world.add(spehre2);
//    world.add(spehre3);
//    world.add(spehre4);
//    world.add(spehre5);
//    world.add(spehre6);
//    world.add(spehre7);
//    world.add(spehre8);
//    world.add(spehre9);
//
//    // カメラ設定
//    auto film = std::make_shared<Film>(600, 600, 3, "iridescence_spehre.png"); // フィルム
//    auto fd = 2.5f; // 焦点距離
//    Vec3 cam_pos(0.0f,2.0f,10.0f);
//    Vec3 cam_target(0.0f,2.0f,0.0f);
//    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
//    cam = Camera(film, fd, cam_pos, cam_forward);
//}

/**
* @brief コーネルボックスと球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note 参考: http://www.graphics.cornell.edu/online/box/data.html
*/
//void make_scene_box_with_sphere(Scene& world, Camera& cam) {
//    world.clear();
//    //  マテリアル
//    auto dist_ggx1 = std::make_shared<GGXDistribution>(0.10f);
//    auto dist_ggx2 = std::make_shared<GGXDistribution>(0.15f);
//    auto dist_ggx3 = std::make_shared<GGXDistribution>(0.30f);
//    auto fres = std::make_shared<FresnelSchlick>(Vec3(1.00f, 0.71f, 0.29f));
//    auto mat_ggx1 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx1, fres);
//    auto mat_ggx2 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx2, fres);
//    auto mat_ggx3 = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx3, fres);
//    auto mat_mirr = std::make_shared<Mirror>(Vec3(0.9f, 0.9f, 0.9f));
//
//    // カラーバリエーション2
//    auto mat_red = std::make_shared<Diffuse>(Vec3(1.000f, 0.065f, 0.065f));
//    auto mat_green = std::make_shared<Diffuse>(Vec3(0.065f, 0.065f, 1.000f));
//    auto mat_white = std::make_shared<Diffuse>(Vec3(0.710f, 0.710f, 0.710f));
//    auto radiance = Vec3(10.0f, 10.0f, 10.0f);
//
//    auto sphere1 = std::make_shared<Sphere>(Vec3(-123, 50.0f, -200.0f), 50, mat_ggx1);
//    auto sphere2 = std::make_shared<Sphere>(Vec3(-273, 50.0f, -200.0f), 50, mat_ggx2);
//    auto sphere3 = std::make_shared<Sphere>(Vec3(-423, 50.0f, -200.0f), 50, mat_ggx3);
//
//
//    // Light sorce
//    auto light_shape = std::make_shared<TriangleMesh>(
//        std::vector<Vec3>{
//            Vec3(-343.0f, 543.7f, -227.0f),
//            Vec3(-343.0f, 543.7f, -332.0f),
//            Vec3(-213.0f, 543.7f, -332.0f),
//            Vec3(-213.0f, 543.7f, -227.0f)},
//        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
//            nullptr);
//    auto light = std::make_shared<AreaLight>(radiance, light_shape);
//    // Ceiling
//    auto ceiling = std::make_shared<TriangleMesh>(
//        std::vector<Vec3>{
//            Vec3(-556.0f, 548.8f,    0.0f),
//            Vec3(-556.0f, 548.8f, -559.2f),
//            Vec3(   0.0f, 548.8f, -559.2f),
//            Vec3(   0.0f, 548.8f,    0.0f)},
//        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
//            mat_white);
//    // Floor
//    auto floor = std::make_shared<TriangleMesh>(
//        std::vector<Vec3>{
//            Vec3(-552.8f, 0.0f,    0.0f),
//            Vec3(   0.0f, 0.0f,    0.0f),
//            Vec3(   0.0f, 0.0f, -559.2f),
//            Vec3(-549.6f, 0.0f, -559.2f)},
//        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
//            mat_white);
//    // Back wall
//    auto back = std::make_shared<TriangleMesh>(
//        std::vector<Vec3>{
//            Vec3(-549.6f,   0.0f, -559.2f),
//            Vec3(   0.0f,   0.0f, -559.2f),
//            Vec3(   0.0f, 548.8f, -559.2f),
//            Vec3(-556.0f, 548.8f, -559.2f)},
//        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
//            mat_white);
//    // Right wall
//    auto right = std::make_shared<TriangleMesh>(
//        std::vector<Vec3>{
//            Vec3(0.0f,   0.0f, -559.2f),
//            Vec3(0.0f,   0.0f,    0.0f),
//            Vec3(0.0f, 548.8f,    0.0f),
//            Vec3(0.0f, 548.8f, -559.2f)},
//        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
//            mat_green);
//    // Left wall
//    auto left = std::make_shared<TriangleMesh>(
//        std::vector<Vec3>{
//            Vec3(-552.8f,   0.0f,    0.0f),
//            Vec3(-549.6f,   0.0f, -559.2f),
//            Vec3(-556.0f, 548.8f, -559.2f),
//            Vec3(-556.0f, 548.8f,    0.0f)},
//        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
//            mat_red);
//    // オブジェクトをシーンに追加
//    world.add(sphere1);
//    world.add(sphere2);
//    world.add(sphere3);
//    world.add(left);
//    world.add(right);
//    world.add(back);
//    world.add(ceiling);
//    world.add(floor);
//    world.add(light);
//
//    //// カメラ設定
//    auto film = std::make_shared<Film>(1000, 1000, 3, "box_with_sphere.png"); // フィルム
//    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
//    auto fov = deg_to_rad(35.0f);
//    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
//    Vec3 cam_pos(-278.0f, 273.0f, 700.0f);
//    Vec3 cam_target(-278.0f, 273.0f, 0.0f);
//    Vec3 cam_forward = unit_vector(cam_target - cam_pos); // z軸負の方向がカメラの前方
//    cam = Camera(film, fd, cam_pos, cam_forward);
//}

/**
* @brief 花瓶モデルのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note モデル出典: https://polyhaven.com/a/ceramic_vase_01
*       このソフトウェアに上記モデルは付属しておりません.
*/
//void make_scene_vase(Scene& world, Camera& cam) {
//    world.clear();
//    // マテリアル
//    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
//    auto fres     = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
//    auto mat_ggx  = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres);
//
//    // オブジェクト
//    auto obj_pot = std::make_shared<TriangleMesh>("asset/model.obj", mat_ggx);
//    world.add(obj_pot);
//
//    // カメラの設定
//    auto film = std::make_shared<Film>(600, 600, 3, "vase.png"); // フィルム
//    auto fd = 2.5f; // 焦点距離
//    Vec3 cam_pos(0.0f,2.0f,7.5f);
//    Vec3 cam_target(0.0f,2.0f,0.0f);
//    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
//    cam = Camera(film, fd, cam_pos, cam_forward);
//}


/**
* @brief BRDFに沿った直接光の入射方向をサンプリング
* @pram[in] r             :追跡レイ
* @pram[in] isect         :交差点情報
* @pram[in] world         :シーン
* @pram[in] shading_coord :シェーディング座標系
* @return Vec3 :直接光の重み付き入射放射輝度
*/
Vec3 explict_brdf(const Ray& r, const intersection& isect, const Scene& world,
                  const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    Vec3 wi_local; // 直接光の入射方向
    float pdf_scattering, pdf_light, weight = 1.0f;

    // BRDFに基づき直接光の入射方向をサンプリング
    auto wo = unit_vector(r.get_dir());          // 出射方向は必ず正規化する
    auto wo_local = -shading_coord.to_local(wo); // ローカルな出射方向
    auto brdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf_scattering);
    if (pdf_scattering == 0 || is_zero(brdf)) {
        return Ld;
    }

    // 光源と交差しなければ寄与はゼロ
    auto wi = shading_coord.to_world(wi_local); // 直接光の入射方向
    auto r_new = Ray(isect.pos, wi); // 光源へ向かうのレイ
    intersection isect_light;
    if (!world.intersect_light(r_new, eps_isect, inf, isect_light)) {
        return Ld;
    }

    // 光源の放射輝度を計算
    auto L = isect_light.light->emitte();
    pdf_light = isect_light.light->eval_pdf(isect, wi);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // 光源へのレイが遮蔽されると寄与はゼロ
    if (world.intersect_object(r_new, eps_isect, isect_light.t)) {
        return Ld;
    }

    // 寄与の計算
    if (sampling_strategy == Sampling::MIS) {
        weight = Random::power_heuristic(1, pdf_scattering, 1, pdf_light);
    }
    auto cos_term = dot(isect.normal, wi);
    Ld += brdf * cos_term * weight * L / pdf_scattering;
    return Ld;
}

/**
* @brief 直接光を一つの光源からサンプリング
* @pram[in] r             :追跡レイ
* @pram[in] isect         :交差点情報
* @pram[in] world         :シーン
* @pram[in] shading_coord :シェーディング座標系
* @return Vec3 :直接光の重み付き入射放射輝度
*/
Vec3 explict_one_light(const Ray& r, const intersection& isect, const Scene& world,
                       const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    Vec3 wi;
    float pdf_scattering, pdf_light, weight = 1.0f;

    // 光源をランダムに一つ選択
    const auto& lights = world.get_light();
    auto num_lights = lights.size();
    if (num_lights == 0) {
        return Ld;
    }
    auto light_index = Random::uniform_int(0, num_lights - 1);
    const auto& light = lights[light_index];

    // 光源のジオメトリから入射方向をサンプリング
    auto L = light->sample_light(isect, wi, pdf_light);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // 光源が遮蔽されると寄与はゼロ
    auto r_light = Ray(isect.pos, wi); // 光源への入射方向
    intersection isect_light;
    light->intersect(r_light, eps_isect, inf, isect_light); // 光源の交差点情報を取得
    if (world.intersect_object(r_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // BRDFを評価
    auto wo       = unit_vector(r.get_dir()); // 出射方向は必ず正規化する
    auto wo_local = -shading_coord.to_local(wo);
    auto wi_local =  shading_coord.to_local(wi);
    auto brdf = isect.mat->eval_f(wo_local, wi_local);
    pdf_scattering = isect.mat->eval_pdf(wo_local, wi_local);
    if (pdf_scattering == 0 || is_zero(brdf)) {
        return Ld;
    }

    // 寄与の計算
    if (sampling_strategy == Sampling::MIS) {
        weight = Random::power_heuristic(1, pdf_light, 1, pdf_scattering);
    }
    auto cos_term = std::abs(dot(isect.normal, wi));
    Ld += brdf * L * cos_term * weight / pdf_light;
    return Ld;
}

/**
* @brief 明示的に直接光をサンプリング
* @pram[in] r             :追跡レイ
* @pram[in] isect         :交差点情報
* @pram[in] world         :シーン
* @pram[in] shading_coord :シェーディング座標系
* @return Vec3 :光源の重み付き放射輝度
*/
Vec3 explicit_direct_light(const Ray& r, const intersection& isect, const Scene& world,
                           const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    auto contrib = Vec3::one;
    // 完全鏡面反射では直接光を無視
    if (isect.mat->get_type() == MaterialType::Specular) {
        return Ld;
    }

    // BRDFに基づきサンプリング
    if ((sampling_strategy == Sampling::BRDF) || (sampling_strategy == Sampling::MIS)) {
        Ld += contrib * explict_brdf(r, isect, world, shading_coord);
    }

    // 光源のジオメトリに基づきサンプリング
    if ((sampling_strategy == Sampling::LIGHT) || (sampling_strategy == Sampling::MIS)) {
        Ld += contrib * explict_one_light(r, isect, world, shading_coord);
    }
    return Ld;
}

/**
* @brief レイに沿った放射輝度伝搬を計算する関数
* @param[in]  r_in      :カメラ方向からのレイ
* @param[in]  bouunce   :現在のレイのバウンス回数
* @param[in]  max_depth :レイの最大バウンス回数
* @param[in]  world     :レンダリングするシーンのデータ
* @param[in]  contrib   :現在のレイの寄与
* @return Vec3          :レイに沿った放射輝度
* @note 参考: pbrt-v3
*/
Vec3 L_pathtracing(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    bool is_specular_ray = false;
    // パストレーシング
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // 交差点情報
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        // カメラレイとスペキュラレイは光源をの寄与を加算
        if (bounces == 0 || is_specular_ray) {
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
        // 光源ジオメトリと交差時には寄与を加算しない(明示的に光源サンプリングを行うため)
        if (isect.type == IsectType::Light) {
            break;
        }
        // 環境マップのサンプリング
        // NOTE: 環境マップを光源として実装したら寄与は計算しない
        if (!is_intersect) {
            L += contrib * world.sample_envmap(r);
            break;
        }

        // 直接光のサンプリング
        ONB shading_coord(isect.normal);
        L += contrib * explicit_direct_light(r, isect, world, shading_coord);

        // BRDFに基づく出射方向のサンプリング
        // NOTE: カメラ方向をwi，光源方向をwoにしている
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wi_local;
        float pdf;
        auto brdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf);
        auto wi = shading_coord.to_world(wi_local); // サンプリングした入射方向
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * brdf * cos_term / pdf;
        is_specular_ray = (isect.mat->get_type() == MaterialType::Specular) ? true : false;
        r = Ray(isect.pos, wi); // 次のレイを生成

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
* @param[in]  r_in      :カメラ方向からのレイ
* @param[in]  bouunce   :現在のレイのバウンス回数
* @param[in]  max_depth :レイの最大バウンス回数
* @param[in]  world     :レンダリングするシーンのデータ
* @param[in]  contrib   :現在のレイの寄与
* @return Vec3          :レイに沿った放射輝度
* @note ロシアンルーレットによる打ち切りを実装していないのでmax_depthは小さめにしておく
*/
Vec3 L_direct(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    // スペキュラレイのみを追跡する
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // 交差点情報
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);

        // 交差しない場合環境マップをサンプリング
        if (!is_intersect) {
            L += contrib * world.sample_envmap(r);
            break;
        }
        // 光源の場合寄与を追加
        if (isect.type == IsectType::Light) {
            L += contrib * isect.light->emitte();
            break;
        }
        // 完全鏡面でなければ直接光をサンプリング
        ONB shading_coord(isect.normal);
        if (isect.mat->get_type() != MaterialType::Specular) {
            L += contrib * explicit_direct_light(r, isect, world, shading_coord);
            break;
        }
        else {
            // BRDFに基づく出射方向のサンプリング
            // NOTE: カメラ方向をwi，光源方向をwoにしている
            Vec3 wi_local = -shading_coord.to_local(unit_vector(r.get_dir()));
            Vec3 wo_local;
            float pdf;
            auto brdf = isect.mat->sample_f(wi_local, isect, wo_local, pdf);
            auto wo = shading_coord.to_world(wo_local);
            auto cos_term = std::abs(dot(isect.normal, wo));
            contrib = contrib * brdf * cos_term / pdf;
            r = Ray(isect.pos, wo); // 次のレイを生成
        }
    }
    return L;
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
    //make_scene_MIS(world, cam);
    make_scene_cornell_box(world, cam);
    //make_scene_box_with_sphere(world, cam);
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
                    if (GLOBAL_ILLUMINATION)
                        I += L_pathtracing(r, max_depth, world);
                    else 
                        I += L_direct(r, max_depth, world);
                }
            }
            I *= 1.0f / nsample;
            if (IS_GAMMA_CORRECTION) I = gamma_correction(I, gamma);
            img[index++] = static_cast<uint8_t>(std::clamp(I.get_x(), 0.0f, 1.0f) * 255);
            img[index++] = static_cast<uint8_t>(std::clamp(I.get_y(), 0.0f, 1.0f) * 255);
            img[index++] = static_cast<uint8_t>(std::clamp(I.get_z(), 0.0f, 1.0f) * 255);
        }
    }

    // 出力
    auto end_time = std::chrono::system_clock::now(); // 計測終了時間
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << '\n' <<  time_ms / 1000 << "sec\n";
    stbi_write_png(cam.get_filename(), w, h, 3, img.data(), w * c * sizeof(uint8_t));
}