//--------------------------------------------------------------------------------------------------------------------
// testpt
// yoyolon - September 2022
// 
// Develop
// C++17
// MSVS 2019
// 
// External Library
// stb_image: https://github.com/nothings/stb
//
// Reference
// [MT97]  T.Moller, B.Trumbore. "Fast, minimum storage ray-triangle intersection" 1997.
// [PR08]  P.Shirley, R.K.Morley. "Realistic Ray Tracing" 2008.
// [PJH16] M.Pharr, W.Jakob, G.Humphre. "Physically Based Rendering: From Theory To Implementation" 2016.
// [Shi20] P.Shirley. "Raytracing in one weekend" 2020.
// 
// This soft is based on Raytracing in one weekend(https://raytracing.github.io/) and pbrt-v3(https://pbrt.org/).
//--------------------------------------------------------------------------------------------------------------------

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define __STDC_LIB_EXT1__

#include "external/stb_image_write.h"
#include "external/stb_image.h"
#include "Vec3.h"
#include "Ray.h"
#include "Scene.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Material.h"
#include "Camera.h"
#include "Random.h"
#include "ONB.h"
#include "Microfacet.h"
#include "Fresnel.h"

#include <sstream>
#include<iostream>
#include<string>
#include<vector>
#include <memory>
#include <chrono>

// ���ˋP�x���v�Z
// r        : �ǐՂ��郌�C
// bounce   : ���C�̃o�E���X��
// MaxDepth : ���C�̍ő�o�E���X��
// wordl    : �����_�����O����V�[��
// contrib  : ���݂̃��C�̊�^
Vec3 L(const Ray& r, int bounces, int MaxDepth, const scene& world, Vec3 contrib) {
	if (bounces >= MaxDepth) return Vec3(0.0f, 0.0f, 0.0f);

	// ��������
	intersection isect;
	if (world.intersect(r, 0.001f, inf, isect)) {
		// �V�F�[�f�B���O���W�̍\�z
		ONB shadingCoord;
		shadingCoord.build_ONB(isect.normal);
		Vec3 wi_local = -shadingCoord.world2local(r.get_dir());
		Vec3 wo_local;
		Vec3 brdf;
		float pdf;
		// �U���}�e���A���̏ꍇ
		if (isect.mat->f(wi_local, isect, brdf, wo_local, pdf)) {
			auto wo = shadingCoord.local2world(wo_local);
			float cos_term = dot(isect.normal, unit_vector(wo));
			contrib = contrib * brdf * cos_term / pdf;
			// ���V�A�����[���b�g
			if (bounces >= 3) {
				float p_rr = std::max(0.5f, contrib.average()); // ���C�ǐՂ̌p���m��
				if (p_rr < Random::uniform_float()) return Vec3(0.0f, 0.0f, 0.0f);
				else contrib = contrib / p_rr;
			}
			return L(Ray(isect.pos, wo), ++bounces, MaxDepth, world, contrib);
		}
		// �����}�e���A���̏ꍇ
		else {
			return contrib * isect.mat->emitte();
		}
	}

	// �����Ȃ��̏ꍇ�w�i����T���v��
	//if (bounces==0) return Vec3(0.0, 0.0, 1.0);
	//else return Vec3(0.0, 0.0, 0.0);

	auto t = 0.5f * (unit_vector(r.get_dir()).get_y() + 1.0f);
	return contrib * ((1.0f - t) * Vec3(0.3f, 0.3f, 0.3f) + t * Vec3(0.5f, 0.5f, 0.7f));
}

// �@����\��
Vec3 L_normal(const Ray& r, const scene& world) {
	intersection isect;
	if (world.intersect(r, 0.001f, inf, isect)) {
		return 0.5f * (isect.normal + Vec3(1.0f, 1.0f, 1.0f));
	}
	return Vec3(1.0f, 1.0f, 1.0f);
}

// �R�[�l���{�b�N�X�V�[��
// Reference: http://www.graphics.cornell.edu/online/box/data.html
void makeScene_cornell(scene& world, Camera& cam, float aspect) {
	world.clear();

	// �}�e���A��
	auto mat_red = std::make_shared<Diffuse>(Vec3(0.65f,0.05f,0.05f));
	auto mat_green = std::make_shared<Diffuse>(Vec3(0.12f,0.45f,0.15f));
	auto mat_white = std::make_shared<Diffuse>(Vec3(0.73f,0.73f,0.73f));
	auto mat_mirr = std::make_shared<Mirror>(Vec3(0.99f,0.99f,0.99f));
	auto mat_light = std::make_shared<Emitter>(Vec3(25.00f,25.00f,25.00f));
	auto mat_phong = std::make_shared<Phong>(Vec3(1.0,1.0,1.0), Vec3(0.5f,0.5f,0.5f), Vec3(0.5f,0.5f,0.5f), 20.0f);
	
	// Floor
	auto floor = std::make_shared<TriangleMesh>(
		std::vector<Vec3>{
		Vec3(552.8f, 0.0f, 0.0f),
		Vec3(0.0f,   0.0f, 0.0f),
		Vec3(0.0f,   0.0f, 559.2f),
		Vec3(549.6f, 0.0f, 559.2f)},
		std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
		mat_white,
		Vec3(0.0f, 0.0f, 0.0f)
	);
	// light sorce
	auto light = std::make_shared<TriangleMesh>(
		std::vector<Vec3>{
		Vec3(343.0f, 548.7f, 227.0f),
		Vec3(343.0f, 548.7f, 332.0f),
		Vec3(213.0f, 548.7f, 332.0f),
		Vec3(213.0f, 548.7f, 227.0f)},
		std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
		mat_light,
		Vec3(0.0f, 0.0f, 0.0f)
	);
	// Ceiling
	auto ceiling = std::make_shared<TriangleMesh>(
		std::vector<Vec3>{
		Vec3(556.0f, 548.8f,   0.0f),
		Vec3(556.0f, 548.8f, 559.2f),
		Vec3(  0.0f, 548.8f, 559.2f),
		Vec3(  0.0f, 548.8f,   0.0f)},
		std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
		mat_white,
		Vec3(0.0f, 0.0f, 0.0f)
	);
	// back wall
	auto back = std::make_shared<TriangleMesh>(
		std::vector<Vec3>{
		Vec3(549.6f,   0.0f,   559.2f),
		Vec3(  0.0f,   0.0f,   559.2f),
		Vec3(  0.0f,   548.8f, 559.2f),
		Vec3(556.0f,   548.8f, 559.2f)},
		std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
		mat_white,
		Vec3(0.0f, 0.0f, 0.0f)
	);
	// right wall
	auto right = std::make_shared<TriangleMesh>(
		std::vector<Vec3>{
		Vec3(0.0f, 0.0f,   559.2f),
		Vec3(0.0f, 0.0f,     0.0f),
		Vec3(0.0f, 548.8f,   0.0f),
		Vec3(0.0f, 548.8f, 559.2f)},
		std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
		mat_green,
		Vec3(0.0f, 0.0f, 0.0f)
	);
	// left wall
	auto left = std::make_shared<TriangleMesh>(
		std::vector<Vec3>{
		Vec3(552.8f, 0.0f,     0.0f),
		Vec3(549.6f, 0.0f,   559.2f),
		Vec3(556.0f, 548.8f, 559.2f),
		Vec3(556.0f, 548.8f,   0.0f)},
		std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
		mat_red,
		Vec3(0.0f, 0.0f, 0.0f)
	);

	// Spehre
	auto obj_mirr = std::make_shared<Sphere>(Vec3(278.0f-110.0f, 75.0f, 227.0f), 75.0f, mat_mirr);
	auto obj_diff = std::make_shared<Sphere>(Vec3(278.0f+110.0f, 75.0f, 227.0f), 75.0f, mat_phong);

	// �I�u�W�F�N�g���V�[���ɒǉ�
	world.add(left);
	world.add(right);
	world.add(back);
	world.add(ceiling);
	world.add(floor);
	world.add(obj_mirr);
	world.add(obj_diff);
	world.add(light);

	// �J�����̐ݒ�
	auto fd = 3.5f; // �œ_����
	Vec3 camPos(278.0f, 273.0f, -800.0f);
	Vec3 camTarget(278.0f, 273.0f, 0.0f);
	Vec3 camDir = -unit_vector(camTarget - camPos);
	std::cout << camDir << '\n';
	cam = Camera(2.0f, aspect, fd, camPos, camDir);
}

// ���V�[��
void makeScene_sphere(scene& world, Camera& cam, float aspect) {
	world.clear();

	// �}�C�N���t�@�Z�b�g���z
	auto dist_GGX = std::make_shared<GGXDistribution>(0.4f);
	// �t���l���̎�
	auto fres_Dielectric = std::make_shared<FresnelDielectric>(1.0f, 1.6f);
	auto fres_Schlick = std::make_shared<FresnelSchlick>(1.0f, 1.6f);

	// �}�e���A��
	auto mat_microfacet = std::make_shared<Microfacet>(Vec3(1.0f, 1.0f, 1.0f), dist_GGX, fres_Schlick);

	auto mat_diff = std::make_shared<Diffuse>(Vec3(0.50f,0.50f,0.50f));
	auto mat_mirr = std::make_shared<Mirror>(Vec3(1.0f,0.2f,0.2f));
	auto mat_lightR = std::make_shared<Emitter>(Vec3(50.0f, 10.0f, 10.0f));
	auto mat_lightG = std::make_shared<Emitter>(Vec3(10.0f, 50.0f, 10.0f));
	auto mat_lightB = std::make_shared<Emitter>(Vec3(10.0f, 10.0f, 50.0f));




	// �I�u�W�F�N�g
	auto obj_light = std::make_shared<TriangleMesh>(
		std::vector<Vec3>{
		Vec3(-1.0f,  5.0f, -1.0f),
		Vec3( 1.0f,  5.0f, -1.0f),
		Vec3( 1.0f,  5.0f,  1.0f),
		Vec3(-1.0f,  5.0f,  1.0f)},
		std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
		mat_lightR,
		Vec3(0.0f, 0.0f, -5.0f)
	);

	// �I�u�W�F�N�g2
	auto obj_light2 = std::make_shared<TriangleMesh>(
		std::vector<Vec3>{
		Vec3(-1.0f,  5.0f, -1.0f),
		Vec3( 1.0f,  5.0f, -1.0f),
		Vec3( 1.0f,  5.0f,  1.0f),
		Vec3(-1.0f,  5.0f,  1.0f)},
		std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
		mat_lightG,
		Vec3(5.0f, 0.0f, -5.0f)
	);

	// �I�u�W�F�N�g3
	auto obj_light3 = std::make_shared<TriangleMesh>(
		std::vector<Vec3>{
		Vec3(-1.0f, 5.0f, -1.0f),
			Vec3(1.0f, 5.0f, -1.0f),
			Vec3(1.0f, 5.0f, 1.0f),
			Vec3(-1.0f, 5.0f, 1.0f)},
		std::vector<Vec3>{Vec3(0, 1, 2), Vec3(0, 2, 3)},
			mat_lightB,
			Vec3(-5.0f, 0.0f, -5.0f)
			);

	auto obj_sphere = std::make_shared<Sphere>(Vec3(-1.5f, 2.0f, 0.0f), 1.4f, mat_microfacet);
	auto obj_sphere2 = std::make_shared<Sphere>(Vec3(1.5f, 2.0f, 0.0f), 1.4f, mat_mirr);

	// �I�u�W�F�N�g���V�[���ɒǉ�
	world.add(obj_sphere);
	world.add(obj_sphere2);
	world.add(obj_light);
	world.add(obj_light2);
	world.add(obj_light3);

	// �J�����̐ݒ�
	auto fd = 1.5f; // �œ_����
	Vec3 camPos(0.0f, 2.0f, 5.0f);
	Vec3 camTarget(0.0f, 2.0f, 0.0f);
	Vec3 camDir = -unit_vector(camTarget - camPos);
	cam = Camera(2.0f, aspect, fd, camPos, camDir);
}

// main�֐�
int main(int argc, char* argv[]) {
	// ������
	Random::init();

	// �o�͉摜
	const char* filename = "../out.png"; // �p�X
	const auto aspect = 4.0f / 3.0f;     // �A�X�y�N�g��
	const int h = 600;                   // ����
	const int w = int(h * aspect);       // ��
	const int c = 3;                     // �`�����l����
	std::vector<uint8_t> img(w * h * c); // �摜�f�[�^

	// �V�[��
	scene world;
	Camera cam;
	//makeScene_cornell(world, cam, aspect); // �R�[�l���{�b�N�X�V�[���̍쐬
	makeScene_sphere(world, cam, aspect);

	// ���̑��p�����[�^
	int nsample = (argc == 2) ? atoi(argv[1]) : 128; // ���C�̃T���v����
	constexpr auto max_depth = 100; // ���C�̍ő�ǐՐ�
	constexpr auto gamma = 1/2.2f;	// �K���}�␳�p

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
				Ray r = cam.generate_Ray(u, v);
				Vec3 contrib = Vec3(1.0f, 1.0f, 1.0f);
				I += L(r, 0, max_depth, world, contrib);
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

	// �����_�����O����
	auto end_time = std::chrono::system_clock::now();
	std::cout << '\n' << std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count()/1000 << "sec\n";

	 // �摜�o��
	stbi_write_png(filename, w, h, 3, img.data(), w * c * sizeof(uint8_t));
}