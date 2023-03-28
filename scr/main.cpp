//-------------------------------------------------------------------------------------------------
// testpt
// yoyolon - March 2022
// 
// Development
// Microsoft Visual Studio 2019
// C++17
// 
// External Library
// stb: https://github.com/nothings/stb
// 
// Reference(this software is based on listed below.)
// Raytracing in one weekend: https://raytracing.github.io/
// pbrt-v3: https://pbrt.org/
// 
// More information on these licenses can be found in NOTICE.txt
//-------------------------------------------------------------------------------------------------

#include "external/stb_image_write.h"
#include "external/stb_image.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "Camera.h"
#include "Film.h"
#include "Fresnel.h"
#include "Light.h"
#include "MakeScene.h"
#include "Material.h"
#include "Microfacet.h"
#include "ONB.h"
#include "Random.h"
#include "Ray.h"
#include "Scene.h"
#include "Math.h"

// ���ڌ��̃T���v�����O�헪
enum class Sampling {
    BSDF  = 1, // BSDF�ɂ��d�_�I�T���v�����O
    LIGHT = 2, // �����ɂ��d�_�I�T���v�����O
    MIS   = 3  // ���d�d�_�I�T���v�����O
};
Sampling sampling_strategy = Sampling::MIS;

// �f�o�b�O�p
constexpr bool DEBUG_MODE           = false;  // (�f�o�b�O���[�h)�@��������L���ɂ���
constexpr bool GLOBAL_ILLUMINATION  = true;   // ���Ɩ�����(GI)��L���ɂ���
constexpr bool IS_GAMMA_CORRECTION  = true;   // �K���}�␳��L���ɂ���
constexpr bool BIASED_DENOISING     = false;  // ��^�ɏ���l��ݒ肷�邱�ƂŃf�m�C�Y
constexpr int  RUSSIAN_ROULETTE     = 5;      // ���V�A�����[���b�g�K�p�܂ł̃��C�̃o�E���X��
constexpr int  SAMPLES              = 128;    // 1�s�N�Z��������̃T���v����


/**
* @brief �P�x���疳���Ȓl(NaN��inf)�����O����֐�
* @param[in]  color :�P�x
* @return Vec3      :�L���l�ɕϊ���̒l
*/
Vec3 exclude_invalid(const Vec3& color) {
    float r = color.get_x();
    float g = color.get_y();
    float b = color.get_z();
    // �����Ȓl���[���ɂ���
    if (!isfinite(r)) r = 0.0f;
    if (!isfinite(g)) g = 0.0f;
    if (!isfinite(b)) b = 0.0f;
    return Vec3(r, g, b);
}


/**
* @brief ���ڌ���BSDF�ɉ��������˕�������T���v�����O����֐�
* @pram[in] r             :�ǐՃ��C
* @pram[in] isect         :�����_���
* @pram[in] world         :�V�[��
* @pram[in] shading_coord :�V�F�[�f�B���O���W�n
* @return Vec3 :���ڌ��̏d�ݕt�����˕��ˋP�x
* @note �X�y�L�������C�ł͎��s����Ȃ�
*/
Vec3 explict_bsdf(const Ray& r, const intersection& isect, const Scene& world,
                  const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    Vec3 wi_local; // ���ڌ��̓��˕���(�V�F�[�f�B���O���W�n)
    float pdf_scattering, pdf_light, weight = 1.0f;

    // BSDF�Ɋ�Â����ڌ��̓��˕������T���v�����O
    auto wo = unit_vector(r.get_dir()); // �o�˕����͕K�����K������
    auto wo_local = -shading_coord.to_local(wo);
    BxDFType sampled_type;
    auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf_scattering, 
                                    sampled_type);
    if (pdf_scattering == 0 || is_zero(bsdf)) {
        return Ld;
    }

    // �����ƌ������Ȃ���Ί�^�̓[��
    auto wi = shading_coord.to_world(wi_local); // ���ڌ��̓��˕���
    auto r_new = Ray(isect.pos, wi); // �����֌������̃��C
    intersection isect_light;
    if (!world.intersect_light(r_new, eps_isect, inf, isect_light)) {
        return Ld;
    }

    // �������������̕��ˋP�x���v�Z
    auto L = isect_light.light->evel_light(wi);
    pdf_light = isect_light.light->eval_pdf(isect, wi);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // �����ւ̃��C���Օ������Ɗ�^�̓[��
    if (world.intersect_object(r_new, eps_isect, isect_light.t)) {
        return Ld;
    }

    // ��^�̌v�Z
    if (sampling_strategy == Sampling::MIS) {
        weight = Random::power_heuristic(1, pdf_scattering, 1, pdf_light);
    }
    auto cos_term = dot(isect.normal, wi);
    Ld += bsdf * cos_term * weight * L / pdf_scattering;
    return Ld;
}

/**
* @brief ���ڌ�����̌�������T���v�����O
* @pram[in] r             :�ǐՃ��C
* @pram[in] isect         :�����_���
* @pram[in] world         :�V�[��
* @pram[in] shading_coord :�V�F�[�f�B���O���W�n
* @return Vec3 :���ڌ��̏d�ݕt�����˕��ˋP�x
* @note �X�y�L�������C�ł͎��s����Ȃ�
*/
Vec3 explict_one_light(const Ray& r, const intersection& isect, const Scene& world,
                       const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    Vec3 wi;
    float pdf_scattering, pdf_light, weight = 1.0f;

    // �����������_���Ɉ�I��
    const auto& lights = world.get_light();
    int num_lights = (int)lights.size();
    if (num_lights == 0) {
        return Ld;
    }
    auto light_index = Random::uniform_int(0, num_lights - 1);
    const auto& light = lights[light_index];

    // �����̃W�I���g��������˕������T���v�����O
    auto L = light->sample_light(isect, wi, pdf_light);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // �������Օ������Ɗ�^�̓[��
    auto r_light = Ray(isect.pos, wi); // �����ւ̓��˕���
    intersection isect_light;
    light->intersect(r_light, eps_isect, inf, isect_light); // �����̌����_���擾
    if (world.intersect_object(r_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // BSDF��]��
    auto wo       = unit_vector(r.get_dir());
    auto wo_local = -shading_coord.to_local(wo);
    auto wi_local =  shading_coord.to_local(wi);
    auto bsdf = isect.mat->eval_f(wo_local, wi_local, isect);
    pdf_scattering = isect.mat->eval_pdf(wo_local, wi_local, isect);
    if (pdf_scattering == 0 || is_zero(bsdf)) {
        return Ld;
    }

    // ��^�̌v�Z
    if (sampling_strategy == Sampling::MIS) {
        weight = Random::power_heuristic(1, pdf_light, 1, pdf_scattering);
    }
    auto cos_term = std::abs(dot(isect.normal, wi));
    Ld += bsdf * L * cos_term * weight / pdf_light;
    return Ld;
}

/**
* @brief �����I�ɒ��ڌ����T���v�����O
* @pram[in] r             :�ǐՃ��C
* @pram[in] isect         :�����_���
* @pram[in] world         :�V�[��
* @pram[in] shading_coord :�V�F�[�f�B���O���W�n
* @return Vec3 :�����̏d�ݕt�����ˋP�x
*/
Vec3 explicit_direct_light_sampling(const Ray& r, const intersection& isect, 
                                    const Scene& world, const ONB& shading_coord) {
    auto Ld = Vec3::zero;

    // BSDF�Ɋ�Â��T���v�����O
    if ((sampling_strategy == Sampling::BSDF) || (sampling_strategy == Sampling::MIS)) {
        auto L = explict_bsdf(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
    }

    // �����̃W�I���g���Ɋ�Â��T���v�����O
    if ((sampling_strategy == Sampling::LIGHT) || (sampling_strategy == Sampling::MIS)) {
        auto L = explict_one_light(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
    }
    return Ld;
}

/**
* @brief �����T���v�����O���s���p�X�g���[�V���O�����s����֐�
* @param[in]  r_in      :�J������������̃��C
* @param[in]  max_depth :���C�̍ő�o�E���X��
* @param[in]  world     :�����_�����O����V�[���̃f�[�^
* @return Vec3          :���C�ɉ��������ˋP�x
* @note �Q�l: pbrt-v3
*/
Vec3 L_pathtracing(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    bool is_specular_ray = false;
    // �p�X�g���[�V���O
    for (int bounces = 0; bounces < max_depth; bounces++) {
        // ��������
        intersection isect; // �����_���
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        if (is_intersect == false) {
            break;
        }
        // �J�������C�ƃX�y�L�������C�͌����̊�^�����Z
        if (bounces == 0 || is_specular_ray) {
            if (isect.type == IsectType::Light) {
                L += contrib * isect.light->evel_light(r.get_dir());
                break;
            }
        }
        // �����W�I���g���ƌ������ɂ͊�^�����Z���Ȃ�(�����I�Ɍ����T���v�����O���s������)
        if (isect.type == IsectType::Light) {
            break;
        }
        // ���ڌ��̃T���v�����O
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal);// �@���𔽓]
        L += contrib * explicit_direct_light_sampling(r, isect, world, shading_coord);
        // BSDF�Ɋ�Â����˕����̃T���v�����O
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        if (pdf == 0.0f || is_zero(bsdf)) break;
        auto wi = shading_coord.to_world(wi_local); // �T���v�����O�������˕���
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;
        is_specular_ray = is_spacular_type(sampled_type);
        r = Ray(isect.pos, wi); // ���̃��C�𐶐�
        //���V�A�����[���b�g
        if (bounces >= RUSSIAN_ROULETTE) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // �ł��؂�m��
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }
    }
    return L;
}


/**
* @brief �i�C�[�u�ȃp�X�g���[�V���O�����s����֐�
* @param[in]  r_in      :�J������������̃��C
* @param[in]  max_depth :���C�̍ő�o�E���X��
* @param[in]  world     :�����_�����O����V�[���̃f�[�^
* @return Vec3          :���C�ɉ��������ˋP�x
*/
Vec3 L_naive_pathtracing(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    bool is_specular_ray = false;
    // �p�X�g���[�V���O
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // �����_���
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        if (is_intersect == false) {
            break;
        }
        // �����ƌ����������^�����Z
        if (isect.type == IsectType::Light) {
            if (isect.type == IsectType::Light) {
                L += contrib * isect.light->evel_light(r.get_dir());
                break;
            }
        }
        // BSDF�Ɋ�Â����˕����̃T���v�����O
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal);
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        if (pdf == 0.0f || is_zero(bsdf)) break;
        auto wi = shading_coord.to_world(wi_local); // �T���v�����O�������˕���
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;
        is_specular_ray = is_spacular_type(sampled_type);
        r = Ray(isect.pos, wi); // ���̃��C�𐶐�
        //���V�A�����[���b�g
        if (bounces >= RUSSIAN_ROULETTE) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // �ł��؂�m��
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }
    }
    return L;
}


/**
* @brief �m���I���C�g���[�V���O
* @param[in]  r_in      :�J������������̃��C
* @param[in]  max_depth :���C�̍ő�o�E���X��
* @param[in]  world     :�����_�����O����V�[���̃f�[�^
* @return Vec3          :���C�ɉ��������ˋP�x
* @note ���V�A�����[���b�g�ɂ��ł��؂���������Ă��Ȃ��̂�max_depth�͏����߂ɂ��Ă���
*/
Vec3 L_raytracing(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    // �X�y�L�������C�݂̂�ǐՂ���
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // �����_���
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        if (is_intersect == false) {
            break;
        }
        // �����ƌ����������^��ǉ�
        if (isect.type == IsectType::Light) {
            L += contrib * isect.light->evel_light(r.get_dir());
            break;
        }
        // �o�˕����̃T���v�����O
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal);
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        // �o�˕������X�y�L�����łȂ��Ȃ�����𖾎��I�ɃT���v�����O
        if (!is_spacular_type(sampled_type)) {
            L += contrib * explicit_direct_light_sampling(r, isect, world, shading_coord);
            break;
        }
        auto wi = shading_coord.to_world(wi_local);
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;
        r = Ray(isect.pos, wi); // ���̃��C�𐶐�
    }
    return L;
}

/**
* @brief �V�[�����̃V�F�C�v�̖@����\������֐�
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
* @brief �K���}�␳�̃w���p�[�֐�
* @param[in]  c  :�K���}�␳�O�̐F�̗v�f
* @return Vfloat :�K���}�␳��̐F�̗v�f
*/
float gamma_correction_element(float c) {
    // �K���}�␳
    if (c < 0.0031308f) {
        return 12.92f * c;
    }
    return 1.055f * std::powf(c, 1/2.4f) - 0.055f;
}

/**
* @brief �K���}�␳
* @param[in]  color :�K���}�␳�O�̐F
* @return Vec3      :�K���}�␳��̐F
*/
Vec3 gamma_correction(const Vec3& color) {
    // �K���}�␳
    auto r = gamma_correction_element(color.get_x());
    auto g = gamma_correction_element(color.get_y());
    auto b = gamma_correction_element(color.get_z());
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

    // �V�[��
    Scene world;
    Camera cam;
    make_scene_simple(world, cam);
    //make_scene_cylinder(world, cam);
    //make_scene_MIS(world, cam);
    //make_scene_cornell_box(world, cam);
    //make_scene_box_with_sphere(world, cam);
    //make_scene_vase(world, cam);
    //make_scene_thinfilm(world, cam);
 
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
                Vec3 L;
                if (DEBUG_MODE) {
                    L = L_normal(r, world);
                }
                else {
                    if (GLOBAL_ILLUMINATION) {
                        L = L_pathtracing(r, max_depth, world);
                        //L = L_naive_pathtracing(r, max_depth, world);
                    }
                    else {
                        L = L_raytracing(r, max_depth, world);
                    }
                }
                if (BIASED_DENOISING) {
                    I += clamp(exclude_invalid(L)); // �m�C�Y�����邪�����x�[�X�łȂ�
                }
                else {
                    I += exclude_invalid(L);
                }
            }
            I *= 1.0f / nsample;
            I = clamp(I); // [0, 1]�ŃN�����v(TODO: �g�[���}�b�s���O�̎���)
            if (IS_GAMMA_CORRECTION) I = gamma_correction(I);
            img[index++] = static_cast<uint8_t>(I.get_x() * 255);
            img[index++] = static_cast<uint8_t>(I.get_y() * 255);
            img[index++] = static_cast<uint8_t>(I.get_z() * 255);
        }
    }

    // �o��
    auto end_time = std::chrono::system_clock::now(); // �v���I������
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << '\n' <<  time_ms / 1000 << "sec\n";
    stbi_write_png(cam.get_filename(), w, h, 3, img.data(), w * c * sizeof(uint8_t));
}