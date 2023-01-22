#include "BxDF.h"
#include "Fresnel.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Vcabvty.h"

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


// *** Lambert���� ***
LambertianTransmission::LambertianTransmission(const Vec3& _scale)
    : BxDF(BxDFType((uint8_t)BxDFType::Transmission | (uint8_t)BxDFType::Diffuse)),
    scale(_scale)
{}

float LambertianTransmission::eval_pdf(const Vec3& wo, const Vec3& wi, 
                                       const intersection& p) const {
    if (is_same_hemisphere(wo, wi)) {
        return 0.0f; // BTDF�Ȃ̂œ��ꔼ�����Ȃ�T���v������Ȃ�
    }
    else {
        return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
    }
}

Vec3 LambertianTransmission::eval_f(const Vec3& wo, const Vec3& wi, 
                                    const intersection& p) const {
    return scale * invpi;
}

Vec3 LambertianTransmission::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, 
                                      float& pdf) const {
    auto temp = Random::cosine_hemisphere_sample();
    wi = Vec3(temp.get_x(), temp.get_y(), -temp.get_z()); // ���ߕ����͔����O
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}


// *** ���S���ʔ��� ***
SpecularReflection::SpecularReflection(Vec3 _scale, std::shared_ptr<class Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Specular)), 
      scale(_scale),
      fres(_fres)
{}

SpecularReflection::SpecularReflection(Vec3 _scale, float _ni, float _no)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Specular)),
    scale(_scale)
{
    fres = std::make_shared<FresnelDielectric>(_ni, _no);
}

float SpecularReflection::eval_pdf(const Vec3& wo, const Vec3& wi, 
                                   const intersection& p) const {
    return 0.0f; // �f���^�֐��̂���
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
    auto cos_term = get_cos(wi); // ���S���ʂȂ̂�cos(wi)��cos(wo)�͓�����
    auto F = fres->eval(cos_term, p);
    auto brdf = scale * F / std::abs(cos_term);
    return brdf;
}


// *** ���S���ʓ��� ***
SpecularTransmission::SpecularTransmission(Vec3 _scale, float _n_inside, float _n_outside)
    : BxDF(BxDFType((uint8_t)BxDFType::Transmission | (uint8_t)BxDFType::Specular)),
    scale(_scale),
    n_inside(_n_inside),
    n_outside(_n_outside)
{
    fres = std::make_shared<FresnelDielectric>(n_inside, n_outside);
}

float SpecularTransmission::eval_pdf(const Vec3& wo, const Vec3& wi, 
                                     const intersection& p) const {
    return 0.0f; // �f���^�֐��̂���
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
    wi = refract(wo, Vec3(0.0f,0.0f,1.0f), eta); // ���ܕ���
    // �S���˂̏ꍇ
    if (is_zero(wi)) {
        pdf = 0.0f;
        return Vec3::zero;
    }
    pdf = 1.0f;
    auto cos_term = get_cos(wo);
    auto F = Vec3::one - fres->eval(cos_term, p); // �t���l�����ߗ�
    auto btdf = scale * eta * eta * F / std::abs(cos_term);
    return btdf;
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
        return 0.0f;
    }
    auto cos_term = std::pow(dot(specular, wi), shine);
    return (shine + 1.0f) / 2 * invpi * std::max(cos_term, epsilon);
}

Vec3 PhongReflection::eval_f(const Vec3& wo, const Vec3& wi, 
                             const intersection& p) const {
    auto specular = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // �����˕���
    if (!is_same_hemisphere(specular, wi)) {
        return Vec3::zero;
    }
    auto cos_term = std::pow(dot(specular, wi), shine);
    auto f = (shine + 2.0f) / (2 * pi) * cos_term;
    return scale * f;
}

Vec3 PhongReflection::sample_f(const Vec3& wo, const intersection& p, 
                               Vec3& wi, float& pdf) const {
    wi = Random::phong_sample(shine);
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}


// *** v-cavity�}�C�N���t�@�Z�b�g���� ***
VcavityReflection::VcavityReflection(Vec3 _scale, std::shared_ptr<Vcavity> _dist, 
                                     std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)), 
      scale(_scale),
      dist(_dist), 
      fres(_fres) 
{}

float VcavityReflection::eval_pdf(const Vec3& wo, const Vec3& wi, 
                                     const intersection& p) const {
    if (!is_same_hemisphere(wo, wi)) {
        return 0.0f;
    }
    auto h = unit_vector(wo + wi);
    return dist->eval_pdf(h, wo) / (4 * dot(wo, h)); // �m�����x�̕ϊ�
}

Vec3 VcavityReflection::eval_f(const Vec3& wo, const Vec3& wi, 
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
    auto D = dist->D(h);
    auto G = dist->G(wo, wi, h); // v-cavity�p
    Vec3 F = fres->eval(dot(wo, h), p);
    return scale * (D * G * F) / (4 * cos_wo * cos_wi);
}

Vec3 VcavityReflection::sample_f(const Vec3& wo, const intersection& p, 
                                 Vec3& wi, float& pdf) const {
    Vec3 h = dist->sample_halfvector(wo);
    wi = unit_vector(reflect(wo, h)); // reflect()�ł͐��K�����Ȃ��̂Ŗ����I�ɐ��K��
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}


// *** �}�C�N���t�@�Z�b�gBRDF ***
/** @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
MicrofacetReflection::MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist, 
                                           std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)), 
      scale(_scale),
      dist(_dist), 
      fres(_fres) 
{}

MicrofacetReflection::MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist,
                                           float _ni, float _no)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)),
      scale(_scale),
      dist(_dist)
{
    fres = std::make_shared<FresnelDielectric>(_ni, _no);
}

float MicrofacetReflection::eval_pdf(const Vec3& wo, const Vec3& wi, 
                                     const intersection& p) const {
    if (!is_same_hemisphere(wo, wi)) {
        return 0.0f;
    }
    auto h = unit_vector(wo + wi);
    return dist->eval_pdf(h) / (4 * dot(wo, h)); // �m�����x�̕ϊ�
}

Vec3 MicrofacetReflection::eval_f(const Vec3& wo, const Vec3& wi, 
                                  const intersection& p) const {
    if (!is_same_hemisphere(wo, wi)) {
        return Vec3::zero; // ���ꔼ�����ɑ��݂���Ȃ瓧�߂��Ȃ�(�P�U������̂���)
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
    return scale * (D * G * F) / (4 * cos_wo * cos_wi);
}

Vec3 MicrofacetReflection::sample_f(const Vec3& wo, const intersection& p, 
                                    Vec3& wi, float& pdf) const {
    Vec3 h = dist->sample_halfvector();
    wi = unit_vector(reflect(wo, h)); // reflect()�ł͐��K�����Ȃ��̂Ŗ����I�ɐ��K��
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}


// *** �}�C�N���t�@�Z�b�gBTDF ***
/** @note �Q�l: https://www.pbr-book.org/3ed-2018/Transmission_Models/Microfacet_Models */
MicrofacetTransmission::MicrofacetTransmission(Vec3 _scale, std::shared_ptr<NDF> _dist,
                                               float _n_inside, float _n_outside)
    : BxDF(BxDFType((uint8_t)BxDFType::Transmission | (uint8_t)BxDFType::Glossy)),
    scale(_scale),
    dist(_dist),
    n_inside(_n_inside),
    n_outside(_n_outside)
{
    fres = std::make_shared<FresnelDielectric>(n_inside, n_outside);
}

float MicrofacetTransmission::eval_pdf(const Vec3& wo, const Vec3& wi, 
                                       const intersection& p) const {
    if (is_same_hemisphere(wo, wi)) {
        return 0.0f;
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
    return dist->eval_pdf(h) * cos_factor * eta_factor * eta_factor; // �m�����x�̕ϊ�
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
        return Vec3::green;
    }
    float cos_hi = std::abs(dot(wi, h));
    float cos_ho = std::abs(dot(wo, h));
    float D = dist->D(h);
    float G = dist->G(wo, wi);
    Vec3 F = Vec3::one - fres->eval(cos_ho, p);
    float cos_factor = cos_hi * cos_ho / cos_wi / cos_wo;
    float eta_factor = 1 / (cos_ho + eta * cos_hi);
    return scale * cos_factor * (D * G * F) * eta_factor * eta_factor;
}

Vec3 MicrofacetTransmission::sample_f(const Vec3& wo, const intersection& p, 
                                      Vec3& wi, float& pdf) const {
    Vec3 h = dist->sample_halfvector();
    auto eta = p.is_front ? n_outside / n_inside : n_inside / n_outside; // ���΋���
    wi = unit_vector(refract(wo, h, eta)); // reflect()�ł͐��K�����Ȃ��̂Ŗ����I�ɐ��K��
    // �S���˂̏ꍇ
    if (is_zero(wi)) {
        pdf = 0.0f;
        return Vec3::zero;
    }
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}