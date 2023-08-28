#include "BxDF.h"
#include "Fresnel.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"

#include "external/stb_image_write.h"
#include "external/stb_image.h"


// *** Lambert���� ***

LambertianReflection::LambertianReflection(const Vec3& _scale)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Diffuse)),
    scale(_scale)
{}

float LambertianReflection::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
}

Vec3 LambertianReflection::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return scale * invpi;
}

Vec3 LambertianReflection::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    wi = Random::cosine_hemisphere_sample();
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}


// *** ���S���ʔ��� ***

SpecularReflection::SpecularReflection(Vec3 _scale, std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Specular)),
    scale(_scale),
    fres(_fres)
{}

SpecularReflection::SpecularReflection(Vec3 _scale, float _n_inside, float _n_outside)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Specular)),
    scale(_scale)
{
    fres = std::make_shared<FresnelDielectric>(_n_inside, _n_outside);
}

float SpecularReflection::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return 0.f; // �f���^�֐��̂���
}

Vec3 SpecularReflection::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return Vec3::zero; // �f���^�֐��̂���
}

Vec3 SpecularReflection::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    // �����˕����𖾎��I�ɏd�_�I�T���v�����O����̂�eval���\�b�h�͎g��Ȃ�
    wi = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // �����˕���
    pdf = 1.0f;
    auto cos_term = get_cos(wi); // ���S���ʂȂ̂�cos(wi)��cos(wos)�͓�����
    auto F = fres->eval(cos_term, p);
    auto brdf = F / std::abs(cos_term);
    return scale * brdf;
}


// *** ���S���ʓ��� ***

SpecularTransmission::SpecularTransmission(Vec3 _scale, float _n_inside, float _n_outside,
    std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Transmission | (uint8_t)BxDFType::Specular)),
    scale(_scale),
    n_inside(_n_inside),
    n_outside(_n_outside),
    fres(_fres)
{
    if (fres == nullptr) {
        fres = std::make_shared<FresnelDielectric>(n_inside, n_outside);
    }
}

float SpecularTransmission::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return 0.f; // �f���^�֐��̂���
}

Vec3 SpecularTransmission::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return Vec3::zero; // �f���^�֐��̂���
}

Vec3 SpecularTransmission::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    // ���ߕ����𖾎��I�ɏd�_�I�T���v�����O����
    // note: wo����˕����Ƃ���BTDF���v�Z����wi�����˕����ɂȂ�悤�ɕϊ�
    auto eta = p.is_front ? n_outside / n_inside : n_inside / n_outside; // ���΋���
    wi = refract(wo, Vec3(0.f, 0.f, 1.0f), eta); // ���ܕ���
    // �S���˂̏ꍇ(note: refract()�͑S���˂̏ꍇ�[���x�N�g����Ԃ����C�����ׂ�)
    if (is_zero(wi)) {
        pdf = 0.f;
        return Vec3::zero;
    }
    pdf = 1.0f;
    auto cos_term = get_cos(wo);
    auto F = Vec3::one - fres->eval(cos_term, p); // �t���l�����ߗ�
    // note: eta^2�̏�Z�͋��ܗ��̈Ⴂ�ɂ��G�l���M�[���x�̕ω����l��
    auto btdf = eta * eta * F / std::abs(cos_term);
    return scale * btdf;
}


// *** Phong���ʔ��� ***

PhongReflection::PhongReflection(Vec3 _scale, float _shine)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)),
    scale(_scale),
    shine(_shine)
{}

float PhongReflection::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    auto specular = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // �����˕���
    if (!is_same_hemisphere(specular, wi)) {
        return 0.f;
    }
    auto lobe = std::pow(dot(specular, wi), shine); // Phong���[�u
    return (shine + 1.0f) / 2 * invpi * std::max(lobe, epsilon);
}

Vec3 PhongReflection::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    auto specular = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // �����˕���
    if (!is_same_hemisphere(specular, wi)) {
        return Vec3::zero;
    }
    auto lobe = std::pow(dot(specular, wi), shine); // Phong���[�u
    auto brdf = (shine + 2.0f) / (2 * pi) * lobe;
    return scale * brdf;
}

Vec3 PhongReflection::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    wi = phong_sample(shine);
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}

Vec3 PhongReflection::phong_sample(float shine) const {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto z = std::pow(u, 1 / (shine + 1.0f));
    auto r = std::sqrt(std::max(1.0f - z * z, 0.f));
    auto phi = 2 * pi * v;
    auto x = std::cos(phi) * r;
    auto y = std::sin(phi) * r;
    return Vec3(x, y, z);
}


// *** �}�C�N���t�@�Z�b�gBRDF ***

MicrofacetReflection::MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist,
    std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)),
    scale(_scale),
    dist(_dist),
    fres(_fres)
{
    create_multiple_scattering_table(); // ���d�U���e�[�u���̐���
}

MicrofacetReflection::MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist,
    float _n_inside, float _n_outside)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)),
    scale(_scale),
    dist(_dist)
{
    fres = std::make_shared<FresnelDielectric>(_n_inside, _n_outside);
}

float MicrofacetReflection::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    if (!is_same_hemisphere(wo, wi)) {
        return 0.f;
    }
    auto h = unit_vector(wo + wi);
    // �n�[�u�x�N�g���̊m�����x���T���v�����O�����̊m�����x�ɕϊ�
    return dist->eval_pdf(h, wo) / (4 * dot(wo, h));
}

Vec3 MicrofacetReflection::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    if (!is_same_hemisphere(wo, wi)) {
        return Vec3::zero;
    }
    float cos_wo = std::abs(get_cos(wo));
    float cos_wi = std::abs(get_cos(wi));
    if (cos_wo == 0 || cos_wi == 0) {
        return Vec3::zero;
    }
    // �n�[�t�����̎擾
    auto h = unit_vector(wo + wi);
    if (is_zero(h)) {
        return Vec3::zero;
    }
    float D = dist->D(h);
    float G = dist->G(wo, wi);
    Vec3 F = fres->eval(dot(wo, h), p);
    auto brdf = (D * G * F) / (4 * cos_wo * cos_wi);
    // ���d�U�����l������ꍇ��U����
    if (is_multiple_scattering) {
        //int index = std::clamp(int(t*(table_size-1)), 0, table_size-2);
        //auto E = t * table[index] + (1-t) * table[index+1];
        int index_wo = int(cos_wo * (table_size - 1));
        int index_wi = int(cos_wi * (table_size - 1));
        auto E_wo = E[index_wo];
        auto E_wi = E[index_wi];
        auto f_ms = F * (1.0f - E_wo) * (1.0f - E_wi) / (1.0f - E_ave) * invpi;
        brdf += f_ms;
    }
    return scale * brdf;
}

Vec3 MicrofacetReflection::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    // �n�[�u�x�N�g�����T���v�����O���ē��˕����ɕϊ�����
    Vec3 h = dist->sample_halfvector(wo);
    wi = unit_vector(reflect(wo, h)); // reflect()�ł͐��K�����Ȃ��̂Ŗ����I�ɐ��K��
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}

float MicrofacetReflection::weight(float cos_theta, float phi, 
    const std::shared_ptr<NDF>& dist_alpha) const {
    auto sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);
    Vec3 wo(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);
    // ���ˊp���T���v�����O
    Vec3 h = dist_alpha->sample_halfvector(wo);
    auto wi = unit_vector(reflect(wo, h));
    // ���˕����Əo�˕������������ɂȂ��ꍇ�͖���
    if (!is_same_hemisphere(wo, wi)) {
        return 0.f;
    }
    // �댯�ȃP�[�X�����O
    if (std::abs(get_cos(wo)) == 0 || std::abs(get_cos(wi)) == 0) {
        return 0.f;
    }
    float weight = 0.f;
    // VNDF�̏d�݌v�Z
    if (dist_alpha->get_is_visible_sampling()) {
        weight = dist_alpha->G(wo, wi) / dist_alpha->G1(wo);
    }
    // NDF�̏d�݌v�Z
    else {
        weight = dist_alpha->G(wo, wi) * std::abs(dot(wo, h)) / std::abs(get_cos(wo)) / std::abs(get_cos(h));
    }
    if (!isfinite(weight)) weight = 0.f; // �����Ȓl�����O
    return weight;
}

void MicrofacetReflection::create_multiple_scattering_table() {
    int nsamples = 1e4;
    E_ave = 0.f;
    for (int i = 0; i < table_size; i++) {
        auto cos_theta = (i+1.0f) / table_size; // ���ˊp�]��
        E[i] = 0.f;
        for (int j= 0; j < nsamples; j++) {
            auto phi = Random::uniform_float(0.f, 2*pi);
            E[i] += weight(cos_theta, phi, dist);
        }
        E[i] /= nsamples;
        E_ave += E[i] * cos_theta;
    }
    E_ave = 2 * E_ave / table_size;
    // ���d�U���e�[�u���̏o��
    bool is_write_table = true;
    if (is_write_table) {
        std::vector<uint8_t> img(table_size * table_size * 3, 0);  // �摜�f�[�^
        for (int h = 0; h < table_size; h++) {
            auto alpha = (h + 1.0f) / table_size; // �\�ʑe��
            alpha = alpha * alpha;
            auto dist_alpha = std::make_shared<GGX>(alpha, true);
            for (int w = 0; w < table_size; w++) {
                auto cos_theta = (w + 1.0f) / table_size; // ���ˊp�]��
                // ��^���v�Z
                float e = 0.f;
                for (int k = 0; k < nsamples; k++) {
                    auto phi = Random::uniform_float(0.f, 2 * pi);
                    e += weight(cos_theta, phi, dist_alpha);
                }
                e = e / nsamples;
                int e_int = static_cast<int>((1.0f - e) * 255);
                int index = h * table_size * 3 + w * 3;
                img[index]     = std::clamp(e_int, 0, 255);
                img[index + 1] = std::clamp(e_int, 0, 255);
                img[index + 2] = std::clamp(e_int, 0, 255);
            }
        }
        // �摜�̏o��
        stbi_write_png("table.png", table_size, table_size, 3, img.data(),
            table_size * 3 * sizeof(uint8_t));
    }
}


// *** �}�C�N���t�@�Z�b�gBTDF ***

MicrofacetTransmission::MicrofacetTransmission(Vec3 _scale, std::shared_ptr<NDF> _dist, float _n_inside,
    float _n_outside, std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Transmission | (uint8_t)BxDFType::Glossy)),
    scale(_scale),
    dist(_dist),
    n_inside(_n_inside),
    n_outside(_n_outside),
    fres(_fres)
{
    if (fres == nullptr) {
        fres = std::make_shared<FresnelDielectric>(n_inside, n_outside);
    }
}

float MicrofacetTransmission::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    if (is_same_hemisphere(wo, wi)) {
        return 0.f;
    }
    auto eta = p.is_front ? n_outside / n_inside : n_inside / n_outside; // ���΋���
    auto h = unit_vector(eta * wo + wi);
    // �S���˂̏ꍇ
    if (is_zero(refract(wo, h, eta))) {
        return 1.0f;
    }
    float cos_hi = std::abs(dot(wi, h));
    float cos_ho = std::abs(dot(wo, h));
    float cos_factor = eta * eta * cos_hi;
    float eta_factor = 1 / (cos_ho + eta * cos_hi);
    // �n�[�u�x�N�g���̊m�����x���T���v�����O�����̊m�����x�ɕϊ�
    return dist->eval_pdf(h, wo) * cos_factor * eta_factor * eta_factor;
}

Vec3 MicrofacetTransmission::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    // note: wo����˕����Ƃ���BTDF���v�Z����wi�����˕����ɂȂ�悤�ɕϊ�
    if (is_same_hemisphere(wo, wi)) {
        return Vec3::zero;
    }
    float cos_wo = std::abs(get_cos(wo));
    float cos_wi = std::abs(get_cos(wi));
    if (cos_wo == 0 || cos_wi == 0) {
        return Vec3::zero;
    }
    auto eta = p.is_front ? n_outside / n_inside : n_inside / n_outside; // ���΋���
    // �n�[�t�����̎擾
    auto h = unit_vector(eta * wo + wi);
    if (is_zero(h)) {
        return Vec3::zero;
    }
    // �S���˂̏ꍇ
    if (is_zero(refract(wo, h, eta))) {
        return Vec3::zero;
    }
    float cos_hi = std::abs(dot(wi, h));
    float cos_ho = std::abs(dot(wo, h));
    float D = dist->D(h);
    float G = dist->G(wo, wi);
    Vec3 F = Vec3::one - fres->eval(cos_ho, p); // ���ߗ�
    float cos_factor = cos_hi * cos_ho / cos_wi / cos_wo;
    float eta_factor = 1 / (cos_ho + eta * cos_hi);
    auto btdf = cos_factor * (D * G * F) * eta_factor * eta_factor;
    return scale * btdf;
}

Vec3 MicrofacetTransmission::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    Vec3 h = dist->sample_halfvector(wo);
    auto eta = p.is_front ? n_outside / n_inside : n_inside / n_outside; // ���΋���
    wi = unit_vector(refract(wo, h, eta)); // refract()�ł͐��K�����Ȃ��̂Ŗ����I�ɐ��K��
    // �S���˂̏ꍇ
    if (is_zero(wi)) {
        pdf = 0.f;
        return Vec3::zero;
    }
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}