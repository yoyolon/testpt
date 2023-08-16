#include "Renderer.h"
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
#include "Math.h"
#include "Microfacet.h"
#include "ONB.h"
#include "Random.h"
#include "Ray.h"
#include "Scene.h"
#include "Shape.h"
#include "Math.h"


// �f�o�b�O�p
constexpr bool DEBUG_MODE = false; // (�f�o�b�O���[�h)�@��������L���ɂ���
constexpr bool IS_GAMMA_CORRECTION = true;  // �K���}�␳��L���ɂ���

/**
* @brief  Low-Discrepancy����̐����ɗ��p����֐�
* @param[in] i  :
* @return float :[0,1]�̎���
* @note // Ref. T. Kollig and A. Keller. "Efficient Multidimensional Sampling" 2002.
*/
float radical_inverse(unsigned int i) {
    i = (i << 16) | (i >> 16);
    i = ((i & 0x00FF00FF) << 8) | ((i & 0xFF00FF00) >> 8);
    i = ((i & 0x0F0F0F0F) << 4) | ((i & 0xF0F0F0F0) >> 4);
    i = ((i & 0x33333333) << 2) | ((i & 0xCCCCCCCC) >> 2);
    i = ((i & 0x55555555) << 1) | ((i & 0xAAAAAAAA) >> 1);
    return (float)i / (float)0x100000000;
}


Renderer::Renderer(int _spp, Sampling _strategy)
    : spp(_spp), strategy(_strategy)
{}

Vec3 Renderer::explict_uniform(const Ray& r, const intersection& isect, 
    const Scene& world, const ONB& shading_coord) const {
    auto Ld = Vec3::zero;

    // ���˕����������_���ɃT���v�����O
    Vec3 wi_local = Random::uniform_hemisphere_sample();
    Vec3 wi = shading_coord.to_world(wi_local);

    // �����ւ̃��C�������ƌ������Ȃ���Ί�^�̓[��
    auto r_to_light = Ray(isect.pos, wi); // �����֌������̃��C
    intersection isect_light;
    if (!world.intersect_light(r_to_light, eps_isect, inf, isect_light)) {
        return Ld;
    }

    // �������������̕��ˋP�x���v�Z
    auto L = isect_light.light->evel_light(wi);
    if (is_zero(L)) {
        return Ld;
    }

    // �����ւ̃��C���V�F�C�v�ɎՕ������Ɗ�^�̓[��
    if (world.intersect_object(r_to_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // �T���v�����O�����ł�BSDF��pdf��]��
    auto wo = unit_vector(r.get_dir());
    auto wo_local = -shading_coord.to_local(wo); // ���̕\�ʂ��痣����������
    auto bsdf = isect.mat->eval_f(wo_local, wi_local, isect);
    if (is_zero(bsdf)) {
        return Ld;
    }
    float pdf_scattering = 0.5f * invpi; // ��l�T���v�����O�̂���
    // ��^�̌v�Z
    auto cos_term = dot(isect.normal, wi);
    Ld += bsdf * cos_term * L / pdf_scattering;
    return Ld;
}

Vec3 Renderer::explict_bsdf(const Ray& r, const intersection& isect, 
    const Scene& world, const ONB& shading_coord) const{
    auto Ld = Vec3::zero;
    Vec3 wi_local;
    float pdf_scattering, pdf_light, weight = 1.0f;

    // BSDF�Ɋ�Â����ڌ��̓��˕������T���v�����O
    auto wo = unit_vector(r.get_dir());
    auto wo_local = -shading_coord.to_local(wo);
    BxDFType sampled_type;
    auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf_scattering, sampled_type);
    if (pdf_scattering == 0 || is_zero(bsdf)) {
        return Ld;
    }
    auto wi = shading_coord.to_world(wi_local); // ���ڌ��̓��˕���(�����_���痣����������)

    // �����ւ̃��C�������ƌ������Ȃ���Ί�^�̓[��
    auto r_to_light = Ray(isect.pos, wi); // �����֌������̃��C
    intersection isect_light;
    if (!world.intersect_light(r_to_light, eps_isect, inf, isect_light)) {
        return Ld;
    }

    // �������������̕��ˋP�x���v�Z
    auto L = isect_light.light->evel_light(wi);
    pdf_light = isect_light.light->eval_pdf(isect, wi);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // �����ւ̃��C���V�F�C�v�ɎՕ������Ɗ�^�̓[��
    if (world.intersect_object(r_to_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // ��^�̌v�Z
    bool is_delta_bxdf = is_spacular_type(sampled_type); // �f���^���z�Ȃ�MIS�d�݂�1
    if (strategy == Sampling::MIS && !is_delta_bxdf) {
        weight = Random::power_heuristic(1, pdf_scattering, 1, pdf_light);
    }
    auto cos_term = dot(isect.normal, wi);
    Ld += bsdf * cos_term * weight * L / pdf_scattering;
    return Ld;
}

Vec3 Renderer::explict_one_light(const Ray& r, const intersection& isect, 
    const Scene& world, const ONB& shading_coord) const {
    auto Ld = Vec3::zero;
    Vec3 wi; // �����̓��˕���(�����_���痣����������)
    float pdf_scattering, pdf_light, weight = 1.0f;

    // �����������_���Ɉ�I��
    // TODO: �����G�l���M�[���z���x�[�X�Ɍ�����I�т���
    const auto& lights = world.get_light();
    int num_lights = (int)lights.size();
    if (num_lights == 0) {
        return Ld;
    }
    auto light_index = Random::uniform_int(0, num_lights - 1);
    const auto& light = lights[light_index];

    // �I�񂾌���������˕������T���v�����O
    auto L = light->sample_light(isect, wi, pdf_light);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }
    pdf_light = pdf_light / num_lights; // �����̃����_���ȑI�����l��

    // �����ւ̃��C���Օ������Ɗ�^�̓[��
    auto r_to_light = Ray(isect.pos, wi); // �����֌��������C
    intersection isect_light;
    light->intersect(r_to_light, eps_isect, inf, isect_light); // �����̌����_���擾
    if (world.intersect_object(r_to_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // �T���v�����O�������˕����ł�BSDF��]��
    auto wo = unit_vector(r.get_dir());
    auto wo_local = -shading_coord.to_local(wo);
    auto wi_local = shading_coord.to_local(wi);
    auto bsdf = isect.mat->eval_f(wo_local, wi_local, isect);
    pdf_scattering = isect.mat->eval_pdf(wo_local, wi_local, isect);
    if (pdf_scattering == 0 || is_zero(bsdf)) {
        return Ld;
    }

    // ��^�̌v�Z
    bool is_delta_light = light->is_delta_light(); // �f���^���z�Ȃ�MIS�d�݂�1
    if (strategy == Sampling::MIS && !is_delta_light) {
        weight = Random::power_heuristic(1, pdf_light, 1, pdf_scattering);
    }
    auto cos_term = std::abs(dot(isect.normal, wi));
    Ld += bsdf * L * cos_term * weight / pdf_light;
    return Ld;
}

Vec3 Renderer::explicit_direct_light_sampling(const Ray& r, const intersection& isect,
    const Scene& world, const ONB& shading_coord) const {
    auto Ld = Vec3::zero;
    // ��l�T���v�����O
    if (strategy == Sampling::UNIFORM) {
        auto L = explict_uniform(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
        return Ld;
    }
    // BSDF�Ɋ�Â��T���v�����O
    if ((strategy == Sampling::BSDF) || (strategy == Sampling::MIS)) {
        auto L = explict_bsdf(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
    }
    // �����Ɋ�Â��T���v�����O
    if ((strategy == Sampling::LIGHT) || (strategy == Sampling::MIS)) {
        auto L = explict_one_light(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
    }
    return Ld;
}


Vec3 Renderer::L_raytracing(const Ray& r_in, int max_depth, const Scene& world) const {
    const int RUSSIAN_ROULETTE = 1;
    const int SPLIT_SAMPLES = 1; // �V���h�E���C�̃T���v����
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    // �X�y�L�������C�݂̂�ǐՂ���
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // �����_���
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        if (is_intersect == false) {
            L = world.get_bg_color();
            return L;
        }
        // �����ƌ����������^��ǉ�
        if (isect.type == IsectType::Light) {
            L += contrib * isect.light->evel_light(r.get_dir());
            return L;
        }

        // �V�F�[�f�B���O���W�̐���
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal);

        // �����������̂��X�y�L�����łȂ��Ȃ璼�ڌ��̃T���v�����O
        if (!isect.mat->is_perfect_specular()) {
            for (int i = 0; i < SPLIT_SAMPLES; i++) {
                L += contrib * explicit_direct_light_sampling(r, isect, world, shading_coord);
            }
            L /= SPLIT_SAMPLES;
            break;
        }

        // BSDF�Ɋ�Â��o�H(����)�̃T���v�����O
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        auto wi = shading_coord.to_world(wi_local);

        // ��^�̍X�V
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;

        //���V�A�����[���b�g
        if (bounces >= RUSSIAN_ROULETTE) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // �ł��؂�m��
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }

        // ���̃��C�𐶐�
        r = Ray(isect.pos, wi);
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
Vec3 Renderer::L_naive_pathtracing(const Ray& r_in, int max_depth, const Scene& world) const {
    const int RUSSIAN_ROULETTE = 1;
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
            // �ʌ������@�����t�����̏ꍇ�͌������T���v�����Ȃ�
            auto light_type = isect.light->get_type();
            if (light_type == LightType::Area && dot(isect.normal, -r.get_dir()) < 0) {
                return Vec3::zero;
            }
            L += contrib * isect.light->evel_light(r.get_dir());
            break;
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


Vec3 Renderer::L_pathtracing(const Ray& r_in, int max_depth, const Scene& world) const {
    const int RUSSIAN_ROULETTE = 1;
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
                // �ʌ������@�����t�����̏ꍇ�͌������T���v�����Ȃ�
                auto light_type = isect.light->get_type();
                if (light_type == LightType::Area && !isect.is_front) {
                    return Vec3::zero;
                }
                L += contrib * isect.light->evel_light(r.get_dir());
                break;
            }
        }
        // �J�������C�ƃX�y�L�������C�ȊO�͌����Ƃ̌������Ɋ�^�����Z���Ȃ�(�����I�Ɍ������T���v�����O���邽��)
        if (isect.type == IsectType::Light) {
            break;
        }

        // �V�F�[�f�B���O���W�̐���
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal); // ���ߑ��Ȃ�@���𔽓]

        // �����������̂��X�y�L�����łȂ��Ȃ璼�ڌ��̃T���v�����O
        if (!isect.mat->is_perfect_specular()) {
            L += contrib * explicit_direct_light_sampling(r, isect, world, shading_coord);
        }

        // BSDF�Ɋ�Â��o�H(����)�̃T���v�����O
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir())); // ���̕\�ʂ��痣����������
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        if (pdf == 0.0f || is_zero(bsdf)) break;
        auto wi = shading_coord.to_world(wi_local);

        // ��^�̍X�V
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;

        //���V�A�����[���b�g
        if (bounces >= RUSSIAN_ROULETTE) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // �ł��؂�m��
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }

        // ���̃��C�𐶐�
        is_specular_ray = is_spacular_type(sampled_type);
        r = Ray(isect.pos, wi); // ���̃��C�𐶐�
    }
    return L;
}

Vec3 Renderer::L_normal(const Ray& r, const Scene& world) const {
    intersection isect;
    if (world.intersect(r, eps_isect, inf, isect)) {
        if (isect.type == IsectType::Light && isect.light->get_type() == LightType::IBL) {
            return Vec3(0.5f, 0.5f, 0.5f); // ���}�b�v
        }
        Vec3 normal_color = 0.5f * (isect.normal + Vec3(1.0f, 1.0f, 1.0f));
        return normal_color;
        //return Vec3(normal_color[0], 0.f, 0.f); // x�������̂ݕ\��
        //return Vec3(0.f, normal_color[1], 0.f); // y�������̂ݕ\��
        //return Vec3(0.f, 0.f, normal_color[2]); // z�������̂ݕ\��
    }
    return Vec3(0.f, 0.f, 0.f); // �������Ȃ�
}


void Renderer::render(const Scene& world, const Camera& cam) const {
    const int max_depth = 100;
    Random::init(); // �����̏�����

    // �o�͉摜�̐ݒ�
    const auto w = cam.get_w(); // ����
    const auto h = cam.get_h(); // ��
    const auto c = cam.get_c(); // �`�����l����
    std::vector<uint8_t> img(w * h * c);  // �摜�f�[�^

    // ���C�g���[�V���O
    auto start_time = std::chrono::system_clock::now(); // �v���J�n����
    int index = 0;
    for (int y = 0; y < h; y++) {
        std::cout << '\r' << y + 1 << '/' << h << std::flush;
        for (int x = 0; x < w; x++) {
            Vec3 I(0.f, 0.f, 0.f);
            // index�Ԗڂ̃s�N�Z���̃T���v���𐶐�
            for (int k = 0; k < spp; k++) {
                //Vec2 uv(float(k)/spp, radical_inverse(k)); // Low-Discrepancy����𗘗p
                Vec2 uv(Random::uniform_float(), Random::uniform_float()); // ��l�T���v�����O
                Ray r = cam.generate_ray((x + uv[0]) / (w - 1), (y + uv[1]) / (h - 1));
                Vec3 L;
                if (DEBUG_MODE) {
                    L = L_normal(r, world);
                }
                else {
                    //L = L_raytracing(r, max_depth, world);
                    //L = L_naive_pathtracing(r, max_depth, world);
                    L = L_pathtracing(r, max_depth, world);
                }
                I += exclude_invalid(L);
            }
            I *= 1.0f / spp;
            I = clamp(I); // [0, 1]�ŃN�����v(TODO: �g�[���}�b�s���O�̎���)
            if (IS_GAMMA_CORRECTION) I = gamma_correction(I);
            img[index++] = static_cast<uint8_t>(I.get_x() * 255);
            img[index++] = static_cast<uint8_t>(I.get_y() * 255);
            img[index++] = static_cast<uint8_t>(I.get_z() * 255);
        }
    }

    // �摜�o��
    auto end_time = std::chrono::system_clock::now(); // �v���I������
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << '\n' << time_ms / 1000 << "sec\n";
    stbi_write_png(cam.get_filename(), w, h, 3, img.data(), w * c * sizeof(uint8_t));
}