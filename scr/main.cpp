//--------------------------------------------------------------------------------------------------------------------
// testpt
// yoyolon - December 2022
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
    BRDF  = 1, // BRDF�ɂ��d�_�I�T���v�����O
    LIGHT = 2, // �����ɂ��d�_�I�T���v�����O
    MIS   = 3  // ���d�d�_�I�T���v�����O
};
Sampling sampling_strategy = Sampling::MIS;

// �f�o�b�O�p
constexpr bool DEBUG_MODE           = false; // �@���̉�����L���ɂ���
constexpr bool GLOBAL_ILLUMINATION  = false;  // �O���[�o���C���~�l�[�V������L���ɂ���
constexpr bool IMAGE_BASED_LIGHTING = false; // Image-based lighting��L���ɂ���
constexpr bool IS_GAMMA_CORRECTION  = true;  // �K���}�␳��L���ɂ���
constexpr int  SAMPLES = 128;                // 1�s�N�Z��������̃T���v�����̃f�t�H���g�l


/**
* @brief �V���v���ȋ��̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
*/
void make_scene_simple(Scene& world, Camera& cam) {
    world.clear();
    // �}�e���A��
    auto dist_ggx       = std::make_shared<GGXDistribution>(0.025f);
    auto fres_schlick   = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
    auto mat_microfacet = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_schlick);
    auto mat_mirr       = std::make_shared<Mirror>(Vec3(0.9f,0.9f,0.9f));
    auto mat_light      = std::make_shared<Emitter>(Vec3(10.00f,10.00f,10.00f));

    // �I�u�W�F�N�g
    auto obj_sphere       = std::make_shared<Sphere>(Vec3(0.0f,2.0f,0.0f), 3.0f, mat_microfacet);
    auto light_shape_disk = std::make_shared<Disk>(Vec3(0.0f,20.0f,0.0f), 50.0f, mat_light);
    auto light_disk       = std::make_shared<AreaLight>(Vec3(1.0f,1.0f,1.0f), light_shape_disk);
    world.add(obj_sphere);
    world.add(light_disk);

    // �J�����ݒ�
    auto film = std::make_shared<Film>(600, 600, 3, "simple.png"); // �t�B����
    auto fd = 2.5f; // �œ_����
    Vec3 cam_pos(0.0f,2.0f,15.0f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief �~���̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
*/
void make_scene_cylinder(Scene& world, Camera& cam) {
    world.clear();
    // �}�e���A��
    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
    auto fres = std::make_shared<FresnelSchlick>(Vec3(1.00f, 0.71f, 0.29f));
    auto mat_gold = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres);
    auto mat_mirr  = std::make_shared<Mirror>(Vec3(0.9f,0.9f,0.9f));
    auto mat_light = std::make_shared<Emitter>(Vec3(10.00f,10.00f,10.00f));

    // �I�u�W�F�N�g
    auto obj_disk_top = std::make_shared<Disk>(Vec3(0.0f,4.0f,0.0f), 4.0f, mat_gold, true);
    auto obj_disk_btm = std::make_shared<Disk>(Vec3::zero, 4.0f, mat_gold);
    auto obj_cylinder = std::make_shared<Cylinder>(Vec3(0.0f,-4.0f,0.0f), 4.0f, 8.0f, mat_gold);
    auto light_shape  = std::make_shared<Disk>(Vec3(0.0f,7.5f,0.0f), 4.0f, mat_light);
    auto light        = std::make_shared<AreaLight>(Vec3(10.0f,10.0f,10.0f), light_shape);
    world.add(obj_disk_top);
    world.add(obj_disk_btm);
    world.add(obj_cylinder);
    world.add(light);

    // �J�����ݒ�
    auto film = std::make_shared<Film>(768, 512, 3, "cylinder.png"); // �t�B����
    auto fd = 2.5f; // �œ_����
    Vec3 cam_pos(0.0f,10.0f,20.0f);
    Vec3 cam_target(0.0f,0.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief �d�_�I�T���v�����O���ؗp�V�[��
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
* @note ���f���o�T: https://github.com/wjakob/nori/tree/master/scenes/pa5/veach_mi
*/
void make_scene_MIS(Scene& world, Camera& cam) {
    world.clear();
    // �}�e���A��
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

    // ����
    auto r = Vec3(1.00f,0.00f,0.00f);
    auto y = Vec3(1.00f,1.00f,0.00f);
    auto g = Vec3(0.00f,1.00f,0.00f);
    auto b = Vec3(0.00f,0.00f,1.00f);
    auto sphere_XL = std::make_shared<Sphere>(Vec3( 3.75f,0.0f,0.0f), 0.90f, mat_light);
    auto sphere_L  = std::make_shared<Sphere>(Vec3( 1.25f,0.0f,0.0f), 0.30f, mat_light);
    auto sphere_M  = std::make_shared<Sphere>(Vec3(-1.25f,0.0f,0.0f), 0.10f, mat_light);
    auto sphere_S  = std::make_shared<Sphere>(Vec3(-3.75f,0.0f,0.0f), 0.03f, mat_light);
    auto sphere    = std::make_shared<Sphere>(Vec3(  0.0f,4.0f,3.0f), 1.0f, mat_light);
    auto light_XL  = std::make_shared<AreaLight>(Vec3(  1.2f,  1.2f,  1.2f)*b, sphere_XL);
    auto light_L   = std::make_shared<AreaLight>(Vec3( 11.1f, 11.1f, 11.1f)*g, sphere_L);
    auto light_M   = std::make_shared<AreaLight>(Vec3(100.0f,100.0f,100.0f)*y, sphere_M);
    auto light_S   = std::make_shared<AreaLight>(Vec3(900.0f,900.0f,900.0f)*r, sphere_S);
    auto light     = std::make_shared<AreaLight>(Vec3(100.0f,100.0f,100.0f), sphere);

    // �I�u�W�F�N�g
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

    // �J�����ݒ�
    auto film = std::make_shared<Film>(768, 512, 3, "veach_mis.png"); // �t�B����
    auto fd = 4.732f * film->get_aspect();
    Vec3 cam_pos(0.0f, 6.0f, 27.5f);
    Vec3 cam_target(0.0f, -1.5f, 2.5f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief �R�[�l���{�b�N�X�̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
* @note �Q�l: http://www.graphics.cornell.edu/online/box/data.html
*             https://benedikt-bitterli.me/resources/
*/
void make_scene_cornell(Scene& world, Camera& cam) {
    world.clear();
    //  �}�e���A��
    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
    auto fres_schlick = std::make_shared<FresnelSchlick>(Vec3(0.9f, 0.9f, 0.9f));
    auto mat_ggx   = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_ggx, fres_schlick);
    auto mat_mirr  = std::make_shared<Mirror>(Vec3(0.9f, 0.9f, 0.9f));
    //auto mat_red   = std::make_shared<Diffuse>(Vec3( 0.5694f,  0.0430f, 0.0451f));
    //auto mat_green = std::make_shared<Diffuse>(Vec3( 0.1039f,  0.3778f, 0.0768f));
    //auto mat_white = std::make_shared<Diffuse>(Vec3( 0.8860f,  0.6977f, 0.6676f));
    //auto mat_light = std::make_shared<Emitter>(Vec3(20.6904f, 10.8669f, 2.7761f));
    //auto radiance = Vec3(20.6904f, 10.8669f, 2.7761f);
    auto mat_red   = std::make_shared<Diffuse>(Vec3(0.630f, 0.065f, 0.050f));
    auto mat_green = std::make_shared<Diffuse>(Vec3(0.140f, 0.450f, 0.091f));
    auto mat_white = std::make_shared<Diffuse>(Vec3(0.725f, 0.710f, 0.680f));
    auto mat_light = std::make_shared<Emitter>(Vec3(20.6904f, 10.8669f, 2.7761f));
    auto radiance = Vec3(17.0f, 12.0f, 4.0f);

    // Light sorce
    auto light_shape = std::make_shared<TriangleMesh>(
        std::vector<Vec3>{
            Vec3(-343.0f, 543.7f, -227.0f),
            Vec3(-343.0f, 543.7f, -332.0f),
            Vec3(-213.0f, 543.7f, -332.0f),
            Vec3(-213.0f, 543.7f, -227.0f)},
        std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
        mat_light);
    //light_shape->move(Vec3(0.0f,-5.0f,0.0f));
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
    // �I�u�W�F�N�g���V�[���ɒǉ�
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

    //// �J�����ݒ�
    auto film = std::make_shared<Film>(1000, 1000, 3, "cornell_box.png"); // �t�B����
    auto deg_to_rad = [](float deg) { return deg * pi / 180; };
    auto fov = deg_to_rad(35.0f);
    auto fd = 2.0f * std::cos(fov) / std::sin(fov); // �œ_����
    Vec3 cam_pos(-278.0f, 273.0f, 700.0f);
    Vec3 cam_target(-278.0f, 273.0f, 0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos); // z�����̕������J�����̑O��
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief �قȂ閌���̔������R�[�e�B���O���ꂽ���̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
*/
void make_scene_sphere(Scene& world, Camera& cam) {
    world.clear();

    // �}�e���A��
    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
    auto fres_schlick = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
    auto fres_irid1 = std::make_shared<FresnelThinfilm>(200.f, 1.0f, 1.6f, 1.0f);
    auto fres_irid2 = std::make_shared<FresnelThinfilm>(300.f, 1.0f, 1.6f, 1.0f);
    auto fres_irid3 = std::make_shared<FresnelThinfilm>(400.f, 1.0f, 1.6f, 1.0f);
    auto fres_irid4 = std::make_shared<FresnelThinfilm>(500.f, 1.0f, 1.6f, 1.0f);
    auto fres_irid5 = std::make_shared<FresnelThinfilm>(600.f, 1.0f, 1.6f, 1.0f);
    auto fres_irid6 = std::make_shared<FresnelThinfilm>(700.f, 1.0f, 1.6f, 1.0f);
    auto fres_irid7 = std::make_shared<FresnelThinfilm>(800.f, 1.0f, 1.6f, 1.0f);
    auto fres_irid8 = std::make_shared<FresnelThinfilm>(900.f, 1.0f, 1.6f, 1.0f);
    auto fres_irid9 = std::make_shared<FresnelThinfilm>(999.f, 1.0f, 1.6f, 1.0f);
    auto mat_ggx  = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_schlick);
    auto mat_ggx1 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid1);
    auto mat_ggx2 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid2);
    auto mat_ggx3 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid3);
    auto mat_ggx4 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid4);
    auto mat_ggx5 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid5);
    auto mat_ggx6 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid6);
    auto mat_ggx7 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid7);
    auto mat_ggx8 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid8);
    auto mat_ggx9 = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres_irid9);

    // �I�u�W�F�N�g
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

    // �J�����ݒ�
    auto film = std::make_shared<Film>(600, 600, 3, "iridescence_spehre.png"); // �t�B����
    auto fd = 2.5f; // �œ_����
    Vec3 cam_pos(0.0f,2.0f,10.0f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}

/**
* @brief �ԕr���f���̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
* @note ���f���o�T: https://polyhaven.com/a/ceramic_vase_01
*/
void make_scene_vase(Scene& world, Camera& cam) {
    world.clear();
    // �}�e���A��
    auto dist_ggx = std::make_shared<GGXDistribution>(0.05f);
    auto fres     = std::make_shared<FresnelSchlick>(Vec3(0.9f,0.9f,0.9f));
    auto mat_ggx  = std::make_shared<Microfacet>(Vec3(1.0f,1.0f,1.0f), dist_ggx, fres);
    // �I�u�W�F�N�g
    auto obj_pot = std::make_shared<TriangleMesh>("asset/model.obj", mat_ggx);
    world.add(obj_pot);

    // �J�����̐ݒ�
    auto film = std::make_shared<Film>(600, 600, 3, "vase.png"); // �t�B����
    auto fd = 2.5f; // �œ_����
    Vec3 cam_pos(0.0f,2.0f,7.5f);
    Vec3 cam_target(0.0f,2.0f,0.0f);
    Vec3 cam_forward = unit_vector(cam_target - cam_pos);
    cam = Camera(film, fd, cam_pos, cam_forward);
}


/**
* @brief BRDF�ɉ��������ڌ����T���v�����O
* @pram[in] r             :���˃��C
* @pram[in] isect         :�����_���
* @pram[in] world         :�V�[��
* @pram[in] shading_coord :�V�F�[�f�B���O���W�n
* @return Vec3 :�����̏d�ݕt�����ˋP�x
*/
Vec3 explict_brdf(const Ray& r, const intersection& isect, const Scene& world,
                  const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    Vec3 wo_local;
    float pdf_scattering, pdf_light, weight = 1.0f;

    // BRDF�̃T���v�����O
    auto wi = unit_vector(r.get_dir());          // ���˕����͕K�����K������
    auto wi_local = -shading_coord.to_local(wi); // ���[�J���ȓ��˕���
    auto brdf = isect.mat->sample_f(wi_local, isect, wo_local, pdf_scattering);
    if (pdf_scattering == 0 || is_zero(brdf)) {
        return Ld;
    }

    // �����ƌ������Ȃ���Ί�^�̓[��
    auto wo = shading_coord.to_world(wo_local); // �o�˕����͐��K���ς�
    auto r_new = Ray(isect.pos, wo);
    intersection isect_light;
    if (!world.intersect_light(r_new, eps_isect, inf, isect_light)) {
        return Ld;
    }

    // �����̕��ˋP�x���v�Z
    auto L = isect_light.light->emitte();
    pdf_light = isect_light.light->sample_pdf(isect, wo);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // �������Օ������Ɗ�^�̓[��
    if (world.intersect_object(r_new, eps_isect, isect_light.t)) {
        return Ld;
    }

    // ��^�̌v�Z
    if (sampling_strategy == Sampling::MIS) {
        weight = Random::power_heuristic(1, pdf_scattering, 1, pdf_light);
    }
    auto cos_term = dot(isect.normal, wo);
    Ld += brdf * cos_term * weight * L / pdf_scattering;
    return Ld;
}

/**
* @brief ���ڌ�����̌�������T���v�����O
* @pram[in] r             :���˃��C
* @pram[in] isect         :�����_���
* @pram[in] world         :�V�[��
* @pram[in] shading_coord :�V�F�[�f�B���O���W�n
* @return Vec3 :�����̏d�ݕt�����ˋP�x
*/
Vec3 explict_one_light(const Ray& r, const intersection& isect, const Scene& world,
                       const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    Vec3 wo;
    float pdf_scattering, pdf_light, weight = 1.0f;

    // �����������_���Ɉ�I��
    const auto& lights = world.get_light();
    auto num_lights = lights.size();
    if (num_lights == 0) {
        return Ld;
    }
    auto light_index = Random::uniform_int(0, num_lights - 1);
    const auto& light = lights[light_index];

    // �����̃T���v�����O
    auto L = light->sample_light(isect, wo, pdf_light);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // �������Օ������Ɗ�^�̓[��
    auto r_light = Ray(isect.pos, wo);
    intersection isect_light;
    light->intersect(r_light, eps_isect, inf, isect_light); // �����̌����_�����擾
    if (world.intersect_object(r_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // BRDF��]��
    auto wi       = unit_vector(r.get_dir());    // ���˕����͕K�����K������
    auto wi_local = -shading_coord.to_local(wi); // �T�[�t�@�C�X�ւ̃��[�J���ȓ��˕���
    auto wo_local =  shading_coord.to_local(unit_vector(wo));
    auto brdf = isect.mat->f(wi_local, wo_local);
    pdf_scattering = isect.mat->sample_pdf(wi_local, wo_local);
    if (pdf_scattering == 0 || is_zero(brdf)) {
        return Ld;
    }

    // ��^�̌v�Z
    if (sampling_strategy == Sampling::MIS) {
        weight = Random::power_heuristic(1, pdf_light, 1, pdf_scattering);
    }
    auto cos_term = std::abs(dot(isect.normal, unit_vector(wo)));
    Ld += brdf * L * cos_term * weight / pdf_light;
    return Ld;
}

/**
* @brief �����I�ɒ��ڌ����T���v�����O
* @pram[in] r             :���˃��C
* @pram[in] isect         :�����_���
* @pram[in] world         :�V�[��
* @pram[in] shading_coord :�V�F�[�f�B���O���W�n
* @return Vec3 :�����̏d�ݕt�����ˋP�x
*/
Vec3 explicit_direct_light(const Ray& r, const intersection& isect, const Scene& world,
                           const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    auto contrib = Vec3::one;
    // ���S���ʔ��˂ł͒��ڌ��𖳎�
    if (isect.mat->get_type() == MaterialType::Specular) {
        return Ld;
    }

    // BRDF����T���v�����O
    if ((sampling_strategy == Sampling::BRDF) || (sampling_strategy == Sampling::MIS)) {
        Ld += contrib * explict_brdf(r, isect, world, shading_coord);
    }

    // ��������T���v�����O
    if ((sampling_strategy == Sampling::LIGHT) || (sampling_strategy == Sampling::MIS)) {
        Ld += contrib * explict_one_light(r, isect, world, shading_coord);
    }
    return Ld;
}

/**
* @brief ���C�ɉ��������ˋP�x�`�����v�Z����֐�
* @param[in]  r_in      :�J������������̃��C
* @param[in]  bouunce   :���݂̃��C�̃o�E���X��
* @param[in]  max_depth :���C�̍ő�o�E���X��
* @param[in]  world     :�����_�����O����V�[���̃f�[�^
* @param[in]  contrib   :���݂̃��C�̊�^
* @return Vec3          :���C�ɉ��������ˋP�x
* @note �Q�l: https://www.pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/Path_Tracing
*/
Vec3 L_pathtracing(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    bool is_specular_ray = false;
    // �p�X�g���[�V���O
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // �����_���
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        // �J�������C�ƃX�y�L�������C�͌������̊�^�����Z
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
        // �����������ɂ͊�^�����Z���Ȃ�(�����I�Ɍ����T���v�����O���s������)
        if (isect.type == IsectType::Light) {
            break;
        }
        // ���}�b�v�̃T���v�����O
        // NOTE: ���}�b�v�������Ƃ��Ď����������^�͌v�Z���Ȃ�
        if (!is_intersect) {
            L += contrib * world.sample_envmap(r);
            break;
        }

        // ���ڌ��̃T���v�����O
        ONB shading_coord(isect.normal);
        L += contrib * explicit_direct_light(r, isect, world, shading_coord);

        // BRDF�Ɋ�Â��o�˕����̃T���v�����O
        // NOTE: �J����������wi�C����������wo�ɂ��Ă���
        Vec3 wi_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wo_local;
        float pdf;
        auto brdf = isect.mat->sample_f(wi_local, isect, wo_local, pdf);
        auto wo = shading_coord.to_world(unit_vector(wo_local));
        auto cos_term = std::abs(dot(isect.normal, wo));
        contrib = contrib * brdf * cos_term / pdf;
        is_specular_ray = (isect.mat->get_type() == MaterialType::Specular) ? true : false;
        r = Ray(isect.pos, wo); // ���̃��C�𐶐�

        //���V�A�����[���b�g
        if (bounces >= 3) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // �ł��؂�m��
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }
    }
    return L;
}

/**
* @brief ���C�ɉ��������ˋP�x�`�����v�Z����֐�
* @param[in]  r_in      :�J������������̃��C
* @param[in]  bouunce   :���݂̃��C�̃o�E���X��
* @param[in]  max_depth :���C�̍ő�o�E���X��
* @param[in]  world     :�����_�����O����V�[���̃f�[�^
* @param[in]  contrib   :���݂̃��C�̊�^
* @return Vec3          :���C�ɉ��������ˋP�x
* @note ���V�A�����[���b�g�ɂ��ł��؂���������Ă��Ȃ��̂�max_depth�͏����߂ɂ��Ă���
*/
Vec3 L_direct(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    // �X�y�L�������C�݂̂�ǐՂ���
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // �����_���
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);

        // �������Ȃ��ꍇ���}�b�v���T���v�����O
        if (!is_intersect) {
            L += contrib * world.sample_envmap(r);
            break;
        }
        // �����̏ꍇ��^��ǉ�
        if (isect.type == IsectType::Light) {
            L += contrib * isect.light->emitte();
            break;
        }
        // ���S���ʂłȂ���Β��ڌ����T���v�����O
        ONB shading_coord(isect.normal);
        if (isect.mat->get_type() != MaterialType::Specular) {
            L += contrib * explicit_direct_light(r, isect, world, shading_coord);
            break;
        }
        else {
            // BRDF�Ɋ�Â��o�˕����̃T���v�����O
            // NOTE: �J����������wi�C����������wo�ɂ��Ă���
            Vec3 wi_local = -shading_coord.to_local(unit_vector(r.get_dir()));
            Vec3 wo_local;
            float pdf;
            auto brdf = isect.mat->sample_f(wi_local, isect, wo_local, pdf);
            auto wo = shading_coord.to_world(unit_vector(wo_local));
            auto cos_term = std::abs(dot(isect.normal, wo));
            contrib = contrib * brdf * cos_term / pdf;
            r = Ray(isect.pos, wo); // ���̃��C�𐶐�
        }
    }
    return L;
}

/**
* @brief �V�[�����̃I�u�W�F�N�g�̖@����\������֐�
* @param[in]  r         :�ǐՂ��郌�C
* @param[in]  world     :�����_�����O����V�[���̃f�[�^
* @return Vec3          :�@���̉���
* @note z�����̕������J�����֌����������Ƃ���
*/
Vec3 L_normal(const Ray& r, const Scene& world) {
    intersection isect;
    if (world.intersect(r, eps_isect, inf, isect)) {
        return 0.5f * (isect.normal + Vec3(1.0f, 1.0f, 1.0f));
    }
    return Vec3(1.0f, 1.0f, 1.0f);
}

/**
* @brief �F���K���}�␳
* @param[in]  color :�K���}�␳�O�̐F
* @param[in]  gamma :�K���}�l
* @return Vec3      :�K���}�␳��̐F
*/
Vec3 gamma_correction(const Vec3& color, float gamma) {
    float r = std::pow(color.get_x(), gamma);
    float g = std::pow(color.get_y(), gamma);
    float b = std::pow(color.get_z(), gamma);
    return Vec3(r, g, b);
}


/**
* @brief main�֐�
*/
int main(int argc, char* argv[]) {
    Random::init(); // �����̏�����
    // �p�����[�^
    const int nsample = (argc == 2) ? atoi(argv[1]) : SAMPLES; // ���C�̃T���v����
    constexpr auto max_depth = 100;  // ���C�̍ő�ǐՐ�
    constexpr auto gamma = 1 / 2.2f; // �K���}�␳�p
    // �V�[��
    Scene world;
    if (IMAGE_BASED_LIGHTING) {
        int w_envmap, h_envmap, c_envmap;
        float* envmap = stbi_loadf("asset/envmap3.hdr", &w_envmap, &h_envmap, &c_envmap, 0);
        world = Scene(envmap, w_envmap, h_envmap, c_envmap);
    }
    Camera cam;
    //make_scene_simple(world, cam);
    //make_scene_cylinder(world, cam);
    make_scene_MIS(world, cam); // TODO: �������̃T���v�����O����
    //make_scene_cornell(world, cam);
    //make_scene_sphere(world, cam);
    //make_scene_vase(world, cam);
    // �o�͉摜
    const auto w = cam.get_w(); // ����
    const auto h = cam.get_h(); // ��
    const auto c = cam.get_c(); // �`�����l����
    std::vector<uint8_t> img(w * h * c);  // �摜�f�[�^

    // ���C�g���[�V���O
    auto start_time = std::chrono::system_clock::now(); // �v���J�n����
    int index = 0;
    for (int i = 0; i < h; i++) {
        std::cout << '\r' << i+1 << '/' << h << std::flush;
        for (int j = 0; j < w; j++) {
            Vec3 I(0.0f, 0.0f, 0.0f);
            // index�Ԗڂ̃s�N�Z���̃T���v���𐶐�
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

    // �o��
    auto end_time = std::chrono::system_clock::now(); // �v���I������
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << '\n' <<  time_ms / 1000 << "sec\n";
    stbi_write_png(cam.get_filename(), w, h, 3, img.data(), w * c * sizeof(uint8_t));
}