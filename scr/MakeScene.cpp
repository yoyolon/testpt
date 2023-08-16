#include "MakeScene.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
#include "Camera.h"
#include "Film.h"
#include "Light.h"
#include "Material.h"
#include "Math.h"
#include "Random.h"
#include "Scene.h"
#include "Shape.h"


void make_scene_simple(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto gold = Vec3(1.00f, 0.71f, 0.29f);
    auto copper = Vec3(0.95f, 0.64f, 0.54f);
    auto mat_smith = std::make_shared<Metal>(Vec3::one, gold, 0.05f);
    auto mat_diff = std::make_shared<Diffuse>(Vec3(0.1f,0.1f,0.1f));
    auto mat_irid = std::make_shared<Thinfilm>(Vec3::one, 500.f, 1.0f, 1.34f, 0.1f, false);
    auto mat_glass = std::make_shared<Glass>(Vec3::one, Vec3::one, Vec3::one, 1.4f, 0.05f);

    // シェイプ
    auto obj_sphere = std::make_shared<Sphere>(Vec3(0.f,2.0f,0.f), 2.0f, mat_smith);
    // Floor
    auto obj_plane = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3( 10.f, 0.f, -10.f),
            Vec3(-10.f, 0.f, -10.f),
            Vec3(-10.f, 0.f,  10.f),
            Vec3( 10.f, 0.f,  10.f)},
            std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_diff);
    world.add(obj_sphere);
    //world.add(obj_plane);
    // 光源
    auto light_env = std::make_shared<EnvironmentLight>("asset/envmap.hdr", 270.f);
    world.add(light_env);
    //auto light_parallel = std::make_shared<ParallelLight>(Vec3::one, Vec3(0.f, 2.f, 10.f));
    //world.add(light_parallel);
    // カメラ設定
    auto film = std::make_shared<Film>(600, 600, 3, "simple.png");
    //auto fd = 2.5f; // 焦点距離
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(30.f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
    Vec3 cam_pos(0.f,2.0f,10.f);
    Vec3 cam_target(0.f,2.0f,0.f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

void make_scene_simple2(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto gold = Vec3(1.00f, 0.71f, 0.29f);
    auto silver = Vec3(0.95f, 0.93f, 0.88f);
    auto copper = Vec3(0.95f, 0.64f, 0.54f);
    auto mat_diff_blue = std::make_shared<Diffuse>(Vec3(0.1f, 0.1f, 0.8f));
    auto mat_diff_green = std::make_shared<Diffuse>(Vec3(0.1f, 0.8f, 0.1f));
    auto mat_diff_red = std::make_shared<Diffuse>(Vec3(0.8f, 0.1f, 0.1f));
    auto mat_gold = std::make_shared<Metal>(Vec3::one, gold, 0.3f);
    auto mat_silver = std::make_shared<Metal>(Vec3::one, silver, 0.1f);
    auto mat_copper = std::make_shared<Metal>(Vec3::one, copper, 0.1f);
    auto mat_plastic = std::make_shared<Plastic>(Vec3::one, Vec3(0.5f, 0.f, 0.f), 
                                                 Vec3(0.5f, 0.5f, 0.5f), 0.05f);
    auto mat_glass = std::make_shared<Glass>(Vec3::one, Vec3::one, Vec3::one, 1.6f, 0.f);
    auto mat_irid = std::make_shared<Thinfilm>(Vec3::one, 500.f, 1.0f, 1.34f, 0.0f, false);
    auto mat_mirr = std::make_shared<Mirror>(Vec3::one);
    auto mat_plane = std::make_shared<Diffuse>(Vec3(0.1f, 0.1f, 0.1f));
    // シェイプ
    auto obj_sphere1 = std::make_shared<Sphere>(Vec3(5.0f, 2.0f, 0.f), 2.0f, mat_plastic);
    auto obj_sphere2 = std::make_shared<Sphere>(Vec3( 0.f, 2.0f, 0.f), 2.0f, mat_glass);
    //auto obj_sphere2 = std::make_shared<Sphere>(Vec3( 0.f, 2.0f, 0.f), 2.0f, mat_diff_blue);
    auto obj_sphere3 = std::make_shared<Sphere>(Vec3(-5.0f, 2.0f, 0.f), 2.0f, mat_silver);
    //// Floor
    //auto obj_plane = std::make_shared<TriangleMesh>(
    //    std::vector<Vec3>{
    //        Vec3( 20.f, 0.f, -20.f),
    //        Vec3(-20.f, 0.f, -20.f),
    //        Vec3(-20.f, 0.f,  20.f),
    //        Vec3( 20.f, 0.f,  20.f)},
    //    std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
    //    mat_plane);
    world.add(obj_sphere1);
    world.add(obj_sphere2);
    world.add(obj_sphere3);
    //world.add(obj_plane);
    //// 光源
    //auto obj_light_plane = std::make_shared<TriangleMesh>(
    //    std::vector<Vec3>{
    //        Vec3(-15.0f, 10.f,  1.0f),
    //        Vec3(-15.0f, 10.f, -1.0f),
    //        Vec3( 15.0f, 10.f, -1.0f),
    //        Vec3( 15.0f, 10.f,  1.0f)},
    //    std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
    //    mat_plane);
    //auto light_plane = std::make_shared<AreaLight>(Vec3(10.f,10.f,10.f), obj_light_plane);
    //world.add(light_plane);
    auto light_env = std::make_shared<EnvironmentLight>("asset/envmap.hdr", 270.f);
    world.add(light_env);
    //auto light_parallel = std::make_shared<ParallelLight>(Vec3::one, Vec3(10.f, 10.f, 10.f));
    //world.add(light_parallel);
    // カメラ設定
    auto film = std::make_shared<Film>(600, 600, 3, "simple2.png");
    //auto fd = 2.5f; // 焦点距離
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(30.f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
    Vec3 cam_pos(0.f, 5.0f, 25.0f);
    Vec3 cam_target(0.f, 2.0f, 0.f);
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
    auto sphere    = std::make_shared<Sphere>(Vec3(10.f, 10.f, 4.0f), 0.5f, nullptr);
    auto sphere_XL = std::make_shared<Sphere>(Vec3( 3.75f,0.f,0.f), 0.90f, nullptr);
    auto sphere_L  = std::make_shared<Sphere>(Vec3( 1.25f,0.f,0.f), 0.30f, nullptr);
    auto sphere_M  = std::make_shared<Sphere>(Vec3(-1.25f,0.f,0.f), 0.10f, nullptr);
    auto sphere_S  = std::make_shared<Sphere>(Vec3(-3.75f,0.f,0.f), 0.03f, nullptr);
    auto light = std::make_shared<AreaLight>(Vec3(800.f, 800.f, 800.f), sphere);
    auto light_XL  = std::make_shared<AreaLight>(Vec3(  1.2f,  1.2f,  1.2f), sphere_XL);
    auto light_L   = std::make_shared<AreaLight>(Vec3( 11.1f, 11.1f, 11.1f), sphere_L);
    auto light_M   = std::make_shared<AreaLight>(Vec3(100.f,100.f,100.f), sphere_M);
    auto light_S   = std::make_shared<AreaLight>(Vec3(900.f,900.f,900.f), sphere_S);
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
    Vec3 cam_pos(0.f, 2.0f, 15.0f);
    Vec3 cam_target(0.f, -1.0f, 4.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}


void make_scene_cornell_box(Scene& world, Camera& cam) {
    world.clear();
    // マテリアル
    auto mat_red   = std::make_shared<Diffuse>(Vec3( 0.5694f,  0.0430f, 0.0451f));
    auto mat_green = std::make_shared<Diffuse>(Vec3( 0.1039f,  0.3778f, 0.0768f));
    auto mat_white = std::make_shared<Diffuse>(Vec3( 0.8860f,  0.6977f, 0.6676f));
    auto radiance = Vec3(20.f, 15.0f, 6.0f);
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
            Vec3(-556.0f, 548.8f, 0.f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(0.f, 548.8f, -559.2f),
            Vec3(0.f, 548.8f, 0.f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Floor
    auto floor = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f, 0.f, 0.f),
            Vec3(0.f, 0.f, 0.f),
            Vec3(0.f, 0.f, -559.2f),
            Vec3(-549.6f, 0.f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Back wall
    auto back = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-549.6f, 0.f, -559.2f),
            Vec3(0.f, 0.f, -559.2f),
            Vec3(0.f, 548.8f, -559.2f),
            Vec3(-556.0f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Right wall
    auto right = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(0.f, 0.f, -559.2f),
            Vec3(0.f, 0.f, 0.f),
            Vec3(0.f, 548.8f, 0.f),
            Vec3(0.f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_green);
    // Left wall
    auto left = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f, 0.f, 0.f),
            Vec3(-549.6f, 0.f, -559.2f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(-556.0f, 548.8f, 0.f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_red);
    // Short block
    auto short_top = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-130.f, 165.0f, -65.0f),
            Vec3(-82.0f, 165.0f, -225.0f),
            Vec3(-240.f, 165.0f, -272.0f),
            Vec3(-290.f, 165.0f, -114.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_short);
    auto short_rgt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-290.f, 0.f, -114.0f),
            Vec3(-290.f, 165.0f, -114.0f),
            Vec3(-240.f, 165.0f, -272.0f),
            Vec3(-240.f, 0.f, -272.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_short);
    auto short_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-130.f, 0.f, -65.0f),
            Vec3(-130.f, 165.0f, -65.0f),
            Vec3(-290.f, 165.0f, -114.0f),
            Vec3(-290.f, 0.f, -114.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_short);
    auto short_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-82.0f, 0.f, -225.0f),
            Vec3(-82.0f, 165.0f, -225.0f),
            Vec3(-130.f, 165.0f, -65.0f),
            Vec3(-130.f, 0.f, -65.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_short);
    auto short_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-240.f, 0.f, -272.0f),
            Vec3(-240.f, 165.0f, -272.0f),
            Vec3(-82.0f, 165.0f, -225.0f),
            Vec3(-82.0f, 0.f, -225.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_short);
    // Tall block
    auto tall_top = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-423.0f, 330.f, -247.0f),
            Vec3(-265.0f, 330.f, -296.0f),
            Vec3(-314.0f, 330.f, -456.0f),
            Vec3(-472.0f, 330.f, -406.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_tall);
    auto tall_rgt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-423.0f, 0.f, -247.0f),
            Vec3(-423.0f, 330.f, -247.0f),
            Vec3(-472.0f, 330.f, -406.0f),
            Vec3(-472.0f, 0.f, -406.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_tall);
    auto tall_bck = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-472.0f, 0.f, -406.0f),
            Vec3(-472.0f, 330.f, -406.0f),
            Vec3(-314.0f, 330.f, -456.0f),
            Vec3(-314.0f, 0.f, -456.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_tall);
    auto tall_lft = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-314.0f, 0.f, -456.0f),
            Vec3(-314.0f, 330.f, -456.0f),
            Vec3(-265.0f, 330.f, -296.0f),
            Vec3(-265.0f, 0.f, -296.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_tall);
    auto tall_frt = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-265.0f, 0.f, -296.0f),
            Vec3(-265.0f, 330.f, -296.0f),
            Vec3(-423.0f, 330.f, -247.0f),
            Vec3(-423.0f, 0.f, -247.0f)},
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
    auto film = std::make_shared<Film>(400, 400, 3, "cornell_box.png"); // フィルム
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(35.0f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
    Vec3 cam_pos(-278.0f, 273.0f, 700.f);
    Vec3 cam_target(-278.0f, 273.0f, 0.f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos); // z軸負の方向がカメラの前方
    cam = Camera(film, fd, cam_pos, cam_forward);
}


void make_scene_box_with_sphere(Scene& world, Camera& cam) {
    world.clear();
    //  マテリアル
    auto gold = Vec3(1.00f, 0.71f, 0.29f);
    auto mat_gold   = std::make_shared<Metal>(Vec3::one, gold, 0.20f);
    auto mat_plastic = std::make_shared<Plastic>(Vec3::one, Vec3(0.5f,0.f,0.f), Vec3(0.5f,0.5f,0.5f), 0.05f);
    auto mat_glass    = std::make_shared<Glass>(Vec3::one, Vec3::one, Vec3::one, 1.6f, 0.0f);
    auto mat_red   = std::make_shared<Diffuse>(Vec3(1.000f, 0.065f, 0.065f));
    auto mat_green = std::make_shared<Diffuse>(Vec3(0.065f, 0.065f, 1.000f));
    auto mat_white = std::make_shared<Diffuse>(Vec3(0.710f, 0.710f, 0.710f));
    auto radiance = Vec3(10.f, 10.f, 10.f);
    // 3つの球
    auto sphere1 = std::make_shared<Sphere>(Vec3(-123, 50.f, -200.f), 50.f, mat_plastic);
    auto sphere2 = std::make_shared<Sphere>(Vec3(-273, 50.f, -200.f), 50.f, mat_glass);
    auto sphere3 = std::make_shared<Sphere>(Vec3(-423, 50.f, -200.f), 50.f, mat_gold);
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
            Vec3(-556.0f, 548.8f,    0.f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(   0.f, 548.8f, -559.2f),
            Vec3(   0.f, 548.8f,    0.f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Floor
    auto floor = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f, 0.f,    0.f),
            Vec3(   0.f, 0.f,    0.f),
            Vec3(   0.f, 0.f, -559.2f),
            Vec3(-549.6f, 0.f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Back wall
    auto back = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-549.6f,   0.f, -559.2f),
            Vec3(   0.f,   0.f, -559.2f),
            Vec3(   0.f, 548.8f, -559.2f),
            Vec3(-556.0f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_white);
    // Right wall
    auto right = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(0.f,   0.f, -559.2f),
            Vec3(0.f,   0.f,    0.f),
            Vec3(0.f, 548.8f,    0.f),
            Vec3(0.f, 548.8f, -559.2f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
            mat_green);
    // Left wall
    auto left = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-552.8f,   0.f,    0.f),
            Vec3(-549.6f,   0.f, -559.2f),
            Vec3(-556.0f, 548.8f, -559.2f),
            Vec3(-556.0f, 548.8f,    0.f)},
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
    auto light_parallel = std::make_shared<ParallelLight>(Vec3::one, Vec3(10.f, 10.f, 10.f));
    //world.add(light_parallel);
    //// カメラ設定
    auto film = std::make_shared<Film>(600, 600, 3, "box_with_sphere.png");
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(35.0f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
    Vec3 cam_pos(-278.0f, 273.0f, 800.f);
    Vec3 cam_target(-278.0f, 273.0f, 0.f);
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
    // 光源
    auto light_env = std::make_shared<EnvironmentLight>("asset/envmap.hdr", 180.f);
    world.add(light_env);
    // カメラの設定
    auto film = std::make_shared<Film>(200, 200, 3, "vase.png");
    auto fd = 2.5f; // 焦点距離
    Vec3 cam_pos(0.f,2.0f,7.5f);
    Vec3 cam_target(0.f,2.0f,0.f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}


void make_scene_thinfilm(Scene& world, Camera& cam) {
    world.clear();
    // ランダムにシーンを生成
    // small
    for (int i = 0; i < 20; i++) {
        // マテリアル
        auto thickness = Random::uniform_float(200.f, 1200.f);
        auto mat = std::make_shared<Thinfilm>(Vec3::one, thickness, 1.0f, 1.34f, 0.f, true);
        // シェイプ
        auto radius = Random::uniform_float(0.1f, 1.0f);
        auto pos = Vec3(Random::uniform_float(-10.f,  10.f), 
                        Random::uniform_float(-10.f,  10.f), 
                        Random::uniform_float(-15.0f, -10.f));
        auto spehre = std::make_shared<Sphere>(pos, radius, mat);
        world.add(spehre);
    }
    // middle
    for (int i = 0; i < 10; i++) {
        // マテリアル
        auto thickness = Random::uniform_float(200.f, 700.f);
        auto mat = std::make_shared<Thinfilm>(Vec3::one, thickness, 1.0f, 1.34f, 0.f, true);
        // シェイプ
        auto radius = Random::uniform_float(0.1f, 2.0f);
        auto pos = Vec3(Random::uniform_float(-5.0f, 5.0f),
                        Random::uniform_float(-5.0f, 5.0f),
                        Random::uniform_float(-5.0f, 0.f));
        auto spehre = std::make_shared<Sphere>(pos, radius, mat);
        world.add(spehre);
    }
    // 光源
    auto light_env = std::make_shared<EnvironmentLight>("asset/envmap.hdr", 180.f);
    world.add(light_env);
    // カメラ設定
    auto film = std::make_shared<Film>(600, 600, 3, "iridescence_spehre.png");
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(45.0f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // 焦点距離
    Vec3 cam_pos(0.f,0.f,10.f);
    Vec3 cam_target(0.f,0.f,0.f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}