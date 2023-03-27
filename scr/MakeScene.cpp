#include "Camera.h"
#include "Film.h"
#include "Light.h"
#include "MakeScene.h"
#include "Material.h"
#include "Scene.h"
#include "Math.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>


void make_scene_simple(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto gold = Vec3(1.00f, 0.71f, 0.29f);
    auto copper = Vec3(0.95f, 0.64f, 0.54f);
    auto mat_smith = std::make_shared<Metal>(Vec3::one, gold, 0.5f);
    auto mat_diff = std::make_shared<Diffuse>(Vec3(0.75,0.75,0.75));
    //auto mat_glass   = std::make_shared<Glass>(Vec3::one, Vec3::one, Vec3::one, 1.4f, 0.0f);
    // シェイプ
    auto obj_sphere = std::make_shared<Sphere>(Vec3(0.0f,2.0f,0.0f), 3.0f, mat_smith);
    world.add(obj_sphere);
    // 光源
    auto light_env = std::make_shared<EnvironmentLight>("asset/envmap.hdr", 0.0f);
    world.add(light_env);
    // カメラ設定
    auto film = std::make_shared<Film>(600, 600, 3, "simple.png");
    //auto fd = 2.5f; // 焦点距離
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(30.0f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
    Vec3 cam_pos(0.0f,2.0f,15.0f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}


void make_scene_cylinder(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto mat_gold = std::make_shared<Metal>(Vec3::one, Vec3(1.00f, 0.71f, 0.29f), 0.05f);
    auto mat_mirr  = std::make_shared<Mirror>(Vec3(0.9f,0.9f,0.9f));
    // シェイプ
    auto obj_disk_top = std::make_shared<Disk>(Vec3(0.0f,4.0f,0.0f), 4.0f, mat_gold, true);
    auto obj_disk_btm = std::make_shared<Disk>(Vec3::zero, 4.0f, mat_gold);
    auto obj_cylinder = std::make_shared<Cylinder>(Vec3(0.0f,-4.0f,0.0f), 4.0f, 8.0f, mat_gold);
    auto light_shape  = std::make_shared<Disk>(Vec3(0.0f,7.5f,0.0f), 4.0f, nullptr);
    auto light        = std::make_shared<AreaLight>(Vec3(10.0f,10.0f,10.0f), light_shape);
    world.add(obj_disk_top);
    world.add(obj_disk_btm);
    world.add(obj_cylinder);
    // 光源
    auto light_env = std::make_shared<EnvironmentLight>("asset/envmap.hdr");
    world.add(light_env);
    // カメラ設定
    auto film = std::make_shared<Film>(768, 512, 3, "cylinder.png");
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f,10.0f,20.0f);
    Vec3 cam_target(0.0f,0.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}


void make_scene_MIS(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto mat_vrough = std::make_shared<Metal>(Vec3::one, Vec3::one, 0.100f);
    auto mat_rough  = std::make_shared<Metal>(Vec3::one, Vec3::one, 0.050f);
    auto mat_normal = std::make_shared<Metal>(Vec3::one, Vec3::one, 0.020f);
    auto mat_smooth = std::make_shared<Metal>(Vec3::one, Vec3::one, 0.005f);
    auto mat_diff   = std::make_shared<Diffuse>(Vec3(0.4f,0.4f,0.4f));
    // 光源
    auto r = Vec3(1.00f,0.00f,0.00f) * 10;
    auto y = Vec3(1.00f,1.00f,0.00f) * 10;
    auto g = Vec3(0.00f,1.00f,0.00f) * 10;
    auto b = Vec3(0.00f,0.00f,1.00f) * 10;
    auto sphere    = std::make_shared<Sphere>(Vec3(10.0f, 10.0f, 4.0f), 0.5f, nullptr);
    auto sphere_XL = std::make_shared<Sphere>(Vec3( 3.75f,0.0f,0.0f), 0.90f, nullptr);
    auto sphere_L  = std::make_shared<Sphere>(Vec3( 1.25f,0.0f,0.0f), 0.30f, nullptr);
    auto sphere_M  = std::make_shared<Sphere>(Vec3(-1.25f,0.0f,0.0f), 0.10f, nullptr);
    auto sphere_S  = std::make_shared<Sphere>(Vec3(-3.75f,0.0f,0.0f), 0.03f, nullptr);
    auto light = std::make_shared<AreaLight>(Vec3(800.0f, 800.0f, 800.0f), sphere);
    auto light_XL  = std::make_shared<AreaLight>(Vec3(  1.2f,  1.2f,  1.2f), sphere_XL);
    auto light_L   = std::make_shared<AreaLight>(Vec3( 11.1f, 11.1f, 11.1f), sphere_L);
    auto light_M   = std::make_shared<AreaLight>(Vec3(100.0f,100.0f,100.0f), sphere_M);
    auto light_S   = std::make_shared<AreaLight>(Vec3(900.0f,900.0f,900.0f), sphere_S);
    // シェイプ
    auto plate1 = std::make_shared<TriangleMesh>("asset/veach_mis/plate1.obj", mat_smooth);
    auto plate2 = std::make_shared<TriangleMesh>("asset/veach_mis/plate2.obj", mat_normal);
    auto plate3 = std::make_shared<TriangleMesh>("asset/veach_mis/plate3.obj", mat_rough);
    auto plate4 = std::make_shared<TriangleMesh>("asset/veach_mis/plate4.obj", mat_vrough);
    auto floor  = std::make_shared<TriangleMesh>("asset/veach_mis/floor.obj" , mat_diff);
    world.add(light_XL);
    world.add(light_L);
    world.add(light_M);
    world.add(light_S);
    world.add(light);
    world.add(floor);
    world.add(plate1);
    world.add(plate2);
    world.add(plate3);
    world.add(plate4);
    // カメラ設定
    auto film = std::make_shared<Film>(1200, 800, 3, "veach_mis.png");
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(28.0f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
    Vec3 cam_pos(0.0f, 2.0f, 15.0f);
    Vec3 cam_target(0.0f, -1.0f, 4.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}


void make_scene_cornell_box(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto mat_red   = std::make_shared<Diffuse>(Vec3( 0.5694f,  0.0430f, 0.0451f));
    auto mat_green = std::make_shared<Diffuse>(Vec3( 0.1039f,  0.3778f, 0.0768f));
    auto mat_white = std::make_shared<Diffuse>(Vec3( 0.8860f,  0.6977f, 0.6676f));
    auto radiance = Vec3(20.0f, 15.0f, 6.0f);
    auto& mat_tall = mat_white;
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
            Vec3(-556.0f, 548.8f, 0.0f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(0.0f, 548.8f, -559.2f),
            Vec3(0.0f, 548.8f, 0.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Floor
    auto floor = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f, 0.0f, 0.0f),
            Vec3(0.0f, 0.0f, 0.0f),
            Vec3(0.0f, 0.0f, -559.2f),
            Vec3(-549.6f, 0.0f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Back wall
    auto back = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-549.6f, 0.0f, -559.2f),
            Vec3(0.0f, 0.0f, -559.2f),
            Vec3(0.0f, 548.8f, -559.2f),
            Vec3(-556.0f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Right wall
    auto right = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(0.0f, 0.0f, -559.2f),
            Vec3(0.0f, 0.0f, 0.0f),
            Vec3(0.0f, 548.8f, 0.0f),
            Vec3(0.0f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_green);
    // Left wall
    auto left = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f, 0.0f, 0.0f),
            Vec3(-549.6f, 0.0f, -559.2f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(-556.0f, 548.8f, 0.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_red);
    // Short block
    auto short_top = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-130.0f, 165.0f, -65.0f),
            Vec3(-82.0f, 165.0f, -225.0f),
            Vec3(-240.0f, 165.0f, -272.0f),
            Vec3(-290.0f, 165.0f, -114.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_short);
    auto short_rgt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-290.0f, 0.0f, -114.0f),
            Vec3(-290.0f, 165.0f, -114.0f),
            Vec3(-240.0f, 165.0f, -272.0f),
            Vec3(-240.0f, 0.0f, -272.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_short);
    auto short_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-130.0f, 0.0f, -65.0f),
            Vec3(-130.0f, 165.0f, -65.0f),
            Vec3(-290.0f, 165.0f, -114.0f),
            Vec3(-290.0f, 0.0f, -114.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_short);
    auto short_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-82.0f, 0.0f, -225.0f),
            Vec3(-82.0f, 165.0f, -225.0f),
            Vec3(-130.0f, 165.0f, -65.0f),
            Vec3(-130.0f, 0.0f, -65.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_short);
    auto short_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-240.0f, 0.0f, -272.0f),
            Vec3(-240.0f, 165.0f, -272.0f),
            Vec3(-82.0f, 165.0f, -225.0f),
            Vec3(-82.0f, 0.0f, -225.0f)},
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
            Vec3(-423.0f, 0.0f, -247.0f),
            Vec3(-423.0f, 330.0f, -247.0f),
            Vec3(-472.0f, 330.0f, -406.0f),
            Vec3(-472.0f, 0.0f, -406.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_tall);
    auto tall_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-472.0f, 0.0f, -406.0f),
            Vec3(-472.0f, 330.0f, -406.0f),
            Vec3(-314.0f, 330.0f, -456.0f),
            Vec3(-314.0f, 0.0f, -456.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_tall);
    auto tall_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-314.0f, 0.0f, -456.0f),
            Vec3(-314.0f, 330.0f, -456.0f),
            Vec3(-265.0f, 330.0f, -296.0f),
            Vec3(-265.0f, 0.0f, -296.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_tall);
    auto tall_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-265.0f, 0.0f, -296.0f),
            Vec3(-265.0f, 330.0f, -296.0f),
            Vec3(-423.0f, 330.0f, -247.0f),
            Vec3(-423.0f, 0.0f, -247.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_tall);
    // シェイプをシーンに追加
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


void make_scene_box_with_sphere(Scene& world, Camera& cam) {
    world.clear();
    //  マテリアル
    auto gold = Vec3(1.00f, 0.71f, 0.29f);
    auto mat_gold   = std::make_shared<Metal>(Vec3::one, gold, 0.20f);
    auto mat_plastic = std::make_shared<Plastic>(Vec3::one, Vec3(0.5f,0.0f,0.0f), 
                                                 Vec3(0.5f,0.5f,0.5f), 0.05f);
    auto mat_glass    = std::make_shared<Glass>(Vec3::one, Vec3::one, Vec3::one, 1.6f, 0.0f);
    auto mat_red   = std::make_shared<Diffuse>(Vec3(1.000f, 0.065f, 0.065f));
    auto mat_green = std::make_shared<Diffuse>(Vec3(0.065f, 0.065f, 1.000f));
    auto mat_white = std::make_shared<Diffuse>(Vec3(0.710f, 0.710f, 0.710f));
    auto radiance = Vec3(10.0f, 10.0f, 10.0f);
    // 3つの球
    auto sphere1 = std::make_shared<Sphere>(Vec3(-123, 50.0f, -200.0f), 50.0f, mat_plastic);
    auto sphere2 = std::make_shared<Sphere>(Vec3(-273, 50.0f, -200.0f), 50.0f, mat_glass);
    auto sphere3 = std::make_shared<Sphere>(Vec3(-423, 50.0f, -200.0f), 50.0f, mat_gold);
    // Light sorce
    auto light_shape = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-343.0f, 543.8f, -227.0f),
            Vec3(-343.0f, 543.8f, -332.0f),
            Vec3(-213.0f, 543.8f, -332.0f),
            Vec3(-213.0f, 543.8f, -227.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            nullptr);
    auto light = std::make_shared<AreaLight>(radiance, light_shape);
    // Ceiling
    auto ceiling = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-556.0f, 548.8f,    0.0f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(   0.0f, 548.8f, -559.2f),
            Vec3(   0.0f, 548.8f,    0.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Floor
    auto floor = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f, 0.0f,    0.0f),
            Vec3(   0.0f, 0.0f,    0.0f),
            Vec3(   0.0f, 0.0f, -559.2f),
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
    // シェイプをシーンに追加
    world.add(sphere1);
    world.add(sphere2);
    world.add(sphere3);
    //world.add(large_spehre);
    world.add(left);
    world.add(right);
    world.add(back);
    world.add(ceiling);
    world.add(floor);
    world.add(light);
    //// カメラ設定
    auto film = std::make_shared<Film>(1000, 1000, 3, "box_with_sphere.png");
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(35.0f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
    Vec3 cam_pos(-278.0f, 273.0f, 700.0f);
    Vec3 cam_target(-278.0f, 273.0f, 0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}


void make_scene_vase(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto gold = Vec3(1.00f, 0.71f, 0.29f);
    auto mat_gold = std::make_shared<Metal>(Vec3::one, gold, 0.05f);
    // シェイプ
    auto obj_pot = std::make_shared<TriangleMesh>("asset/model.obj", mat_gold);
    world.add(obj_pot);
    // カメラの設定
    auto film = std::make_shared<Film>(600, 600, 3, "vase.png");
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f,2.0f,7.5f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}


/**
* @brief 異なる膜厚の薄膜がコーティングされた球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
void make_scene_thinfilm(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto mat_ggx1 = std::make_shared<Thinfilm>(Vec3::one, 100.0f, 1.6f, 0.5f);
    auto mat_ggx2 = std::make_shared<Thinfilm>(Vec3::one, 200.0f, 1.6f, 0.5f);
    auto mat_ggx3 = std::make_shared<Thinfilm>(Vec3::one, 300.0f, 1.6f, 0.5f);
    auto mat_ggx4 = std::make_shared<Thinfilm>(Vec3::one, 400.0f, 1.6f, 0.5f);
    auto mat_ggx5 = std::make_shared<Thinfilm>(Vec3::one, 500.0f, 1.6f, 0.5f);
    auto mat_ggx6 = std::make_shared<Thinfilm>(Vec3::one, 600.0f, 1.6f, 0.5f);
    auto mat_ggx7 = std::make_shared<Thinfilm>(Vec3::one, 700.0f, 1.6f, 0.5f);
    auto mat_ggx8 = std::make_shared<Thinfilm>(Vec3::one, 800.0f, 1.6f, 0.5f);
    auto mat_ggx9 = std::make_shared<Thinfilm>(Vec3::one, 900.0f, 1.6f, 0.5f);
    // シェイプ
    auto spehre1 = std::make_shared<Sphere>(Vec3(-2.2f,-0.2f,0.0f), 1.0f, mat_ggx1);
    auto spehre2 = std::make_shared<Sphere>(Vec3( 0.0f,-0.2f,0.0f), 1.0f, mat_ggx2);
    auto spehre3 = std::make_shared<Sphere>(Vec3( 2.2f,-0.2f,0.0f), 1.0f, mat_ggx3);
    auto spehre4 = std::make_shared<Sphere>(Vec3(-2.2f, 2.0f,0.0f), 1.0f, mat_ggx4);
    auto spehre5 = std::make_shared<Sphere>(Vec3( 0.0f, 2.0f,0.0f), 1.0f, mat_ggx5);
    auto spehre6 = std::make_shared<Sphere>(Vec3( 2.2f, 2.0f,0.0f), 1.0f, mat_ggx6);
    auto spehre7 = std::make_shared<Sphere>(Vec3(-2.2f, 4.2f,0.0f), 1.0f, mat_ggx7);
    auto spehre8 = std::make_shared<Sphere>(Vec3( 0.0f, 4.2f,0.0f), 1.0f, mat_ggx8);
    auto spehre9 = std::make_shared<Sphere>(Vec3( 2.2f, 4.2f,0.0f), 1.0f, mat_ggx9);
    world.add(spehre1);
    world.add(spehre2);
    world.add(spehre3);
    world.add(spehre4);
    world.add(spehre5);
    world.add(spehre6);
    world.add(spehre7);
    world.add(spehre8);
    world.add(spehre9);
    // 光源
    auto light_env = std::make_shared<EnvironmentLight>("asset/envmap.hdr", 0.0f);
    world.add(light_env);
    // カメラ設定
    auto film = std::make_shared<Film>(600, 600, 3, "iridescence_spehre.png");
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.0f,2.0f,10.0f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}