#include "BxDF.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** Lambert反射 ***
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


// *** Lambert透過 ***
LambertianTransmission::LambertianTransmission(const Vec3& _scale)
    : BxDF(BxDFType((uint8_t)BxDFType::Transmission | (uint8_t)BxDFType::Diffuse)),
    scale(_scale)
{}

float LambertianTransmission::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    if (is_same_hemisphere(wo, wi)) {
        return 0.0f; // BTDFなので同一半球内ならサンプルされない
    }
    else {
        return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
    }
}

Vec3 LambertianTransmission::eval_f(const Vec3& wo, const Vec3& wi) const {
    return scale * invpi;
}

Vec3 LambertianTransmission::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    auto temp = Random::cosine_hemisphere_sample();
    wi = Vec3(temp.get_x(), temp.get_y(), -temp.get_z()); // 透過方向は半球外
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

SpecularReflection::SpecularReflection(Vec3 _scale, float _ni, float _no)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Specular)),
    scale(_scale)
{
    fres = std::make_shared<FresnelDielectric>(_ni, _no);
}

float SpecularReflection::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    return 0.0f; // デルタ関数のため
}

Vec3 SpecularReflection::eval_f(const Vec3& wo, const Vec3& wi) const {
    return Vec3::zero; // デルタ関数のため
}

Vec3 SpecularReflection::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    // 正反射方向を明示的に重点的サンプリングするのでevalメソッドは使わない
    wi = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向
    pdf = 1.0f;
    auto cos_term = get_cos(wi); // 完全鏡面なのでcos(wi)とcos(wo)は等しい
    auto F = fres->evaluate(cos_term);
    auto brdf = scale * F / std::abs(cos_term);
    return brdf;
}


// *** 完全鏡面透過 ***
SpecularTransmission::SpecularTransmission(Vec3 _scale, float _ni, float _no)
    : BxDF(BxDFType((uint8_t)BxDFType::Transmission | (uint8_t)BxDFType::Specular)),
    scale(_scale),
    ni(_ni),
    no(_no)
{
    fres = std::make_shared<FresnelDielectric>(ni, no);
}

float SpecularTransmission::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    return 0.0f; // デルタ関数のため
}

Vec3 SpecularTransmission::eval_f(const Vec3& wo, const Vec3& wi) const {
    return Vec3::zero; // デルタ関数のため
}

Vec3 SpecularTransmission::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    // 透過方向を明示的に重点的サンプリングするのでevalメソッドは使わない
    // 出射方向(入射レイ)が媒質の内側にあるか判定
    auto cos_out = get_cos(wo);
    bool is_inside = cos_out < 0;
    auto n_inside  = is_inside ? no : ni;
    auto n_outside = is_inside ? ni : no;
    auto normal = is_inside ? Vec3(0.0f,0.0f,-1.0f) : Vec3(0.0f,0.0f,1.0f);
    auto eta = n_outside * n_outside; // 相対屈折率
    wi = refract(wo, normal, eta); // 屈折方向
    // 全反射の場合
    if (is_zero(wi)) {
        pdf = 0.0f;
        return Vec3::zero;

    }
    pdf = 1.0f;
    auto cos_term = get_cos(wi);
    auto F = Vec3::one - fres->evaluate(cos_out); // フレネル透過率
    auto brdf = scale * eta * eta * F / std::abs(cos_term);
    return brdf;
}


// *** Phong鏡面反射 ***
PhongReflection::PhongReflection(Vec3 _scale, float _shine) 
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)),
      scale(_scale),
      shine(_shine) 
{}

float PhongReflection::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    auto specular = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向
    if (!is_same_hemisphere(specular, wi)) {
        return 0.0f;
    }
    auto cos_term = std::pow(dot(specular, wi), shine);
    return (shine + 1.0f) / 2 * invpi * std::max(cos_term, epsilon);
}

Vec3 PhongReflection::eval_f(const Vec3& wo, const Vec3& wi) const {
    auto specular = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向
    if (!is_same_hemisphere(specular, wi)) {
        return Vec3::zero;
    }
    auto cos_term = std::pow(dot(specular, wi), shine);
    auto f = (shine + 2.0f) / (2 * pi) * cos_term;
    return scale * f;
}

Vec3 PhongReflection::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    wi = Random::phong_sample(shine);
    //wi = Random::cosine_hemisphere_sample();
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