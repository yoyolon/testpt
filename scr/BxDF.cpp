#include "BxDF.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** ランバート反射 ***
LambertianReflection::LambertianReflection(const Vec3& _scale)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Diffuse)),
      scale(_scale)
{}

float LambertianReflection::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
}

Vec3 LambertianReflection::eval_f(const Vec3& wo, const Vec3& wi) const {
    return scale * invpi;
}

Vec3 LambertianReflection::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    wi = Random::cosine_hemisphere_sample();
    pdf = eval_pdf(wo, wi);
    auto brdf = eval_f(wo, wi);
    return brdf;
}


// *** 完全鏡面反射 ***
SpecularReflection::SpecularReflection(Vec3 _scale, std::shared_ptr<class Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Specular)), 
      scale(_scale),
      fres(_fres)
{}

float SpecularReflection::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    return 1.0f; // 正反射方向を重点的サンプリングしているためpdfは1.0
}

Vec3 SpecularReflection::eval_f(const Vec3& wo, const Vec3& wi) const {
    auto cos_term = get_cos(wi); // cos(wi)とcos(wo)が等しい
    auto F = fres->evaluate(cos_term);
    return scale * F / cos_term;
}

Vec3 SpecularReflection::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    wi = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向
    pdf = eval_pdf(wo, wi);
    auto brdf = eval_f(wo, wi);
    return brdf;
}


// *** マイクロファセットモデル ***
/** @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
MicrofacetReflection::MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist, 
                       std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)), 
      scale(_scale),
      dist(_dist), 
      fres(_fres) 
{}

float MicrofacetReflection::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    auto h = unit_vector(wo + wi);
    return dist->eval_pdf(h) / (4 * dot(wo, h)); // 確率密度の変換
}

Vec3 MicrofacetReflection::eval_f(const Vec3& wo, const Vec3& wi) const {
    if (wo.get_z() * wi.get_z() < 0) {
        return Vec3::zero; // 同一半球内に存在しないなら反射しない(単散乱仮定のため)
    }
    auto h = unit_vector(wo + wi);
    float cos_wo = std::abs(get_cos(wo));
    float cos_wi = std::abs(get_cos(wi));
    if (cos_wo == 0 || cos_wi == 0) {
        return Vec3::zero;
    }
    if (is_zero(h)) {
        return Vec3::zero;
    }
    float D = dist->D(h);
    float G = dist->G(wo, wi);
    Vec3 F = fres->evaluate(dot(wo, h));
    return scale * (D * G * F) / (4 * cos_wo * cos_wi);
}

Vec3 MicrofacetReflection::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    Vec3 h = dist->sample_halfvector();
    wi = unit_vector(reflect(wo, h)); // reflect()では正規化しないので明示的に正規化
    pdf = eval_pdf(wo, wi);
    auto brdf = eval_f(wo, wi);
    return brdf;
}