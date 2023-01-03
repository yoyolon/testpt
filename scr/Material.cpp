#include "Material.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** ランバート反射 ***
Diffuse::Diffuse(Vec3 _albedo) : Material(MaterialType::Diffuse), albedo(_albedo) {}

float Diffuse::sample_pdf(const Vec3& wo, const Vec3& wi) const {
    return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
}

Vec3 Diffuse::f(const Vec3& wo, const Vec3& wi) const {
    return albedo * invpi;
}

Vec3 Diffuse::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    wi = Random::cosine_hemisphere_sample();
    pdf = sample_pdf(wo, wi);
    auto brdf = f(wo, wi);
    return brdf;
}


// *** 完全鏡面反射 ***
Mirror::Mirror(Vec3 _albedo) : Material(MaterialType::Specular), albedo(_albedo) {}


float Mirror::sample_pdf(const Vec3& wo, const Vec3& wi) const {
    return 1.0f;
}

Vec3 Mirror::f(const Vec3& wo, const Vec3& wi) const {
    return albedo / dot(Vec3(0, 0, 1), unit_vector(wi));
}

Vec3 Mirror::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    wi = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向(reflectより速い)
    pdf = sample_pdf(wo, wi);
    auto brdf = f(wo, wi);
    return brdf;
}


// *** Phongモデル ***
// TODO: 正規化/より効率的な重点的サンプリングの実装
Phong::Phong(Vec3 _albedo, Vec3 _Kd, Vec3 _Ks, float _shin) 
    : Material(MaterialType::Glossy), albedo(_albedo), Kd(_Kd), Ks(_Ks), shin(_shin) {}

float Phong::sample_pdf(const Vec3& wo, const Vec3& wi) const {
    return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
}

Vec3 Phong::f(const Vec3& wo, const Vec3& wi) const {
    auto dir_spec = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向
    Vec3 diffuse = Kd * invpi;
    Vec3 specular = Ks * std::pow(dot(dir_spec, wi), shin) * invpi;
    return albedo * (diffuse + specular);
}

Vec3 Phong::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    wi = Random::cosine_hemisphere_sample();
    pdf = sample_pdf(wo, wi);
    auto brdf = f(wo, wi);
    return brdf;
}


// *** マイクロファセットモデル ***
/** @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
Microfacet::Microfacet(Vec3 _albedo, std::shared_ptr<MicrofacetDistribution> _distribution, 
                       std::shared_ptr<Fresnel> _fresnel)
    : Material(MaterialType::Glossy), albedo(_albedo), distribution(_distribution), fresnel(_fresnel) {}

float Microfacet::sample_pdf(const Vec3& wo, const Vec3& wi) const {
    auto h = unit_vector(wo + wi);
    return distribution->sample_pdf(wo, h) / (4 * dot(wo, h)); // 確率密度の変換
}

Vec3 Microfacet::f(const Vec3& wo, const Vec3& wi) const {
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
    float D = distribution->D(h);
    float G = distribution->G(wo, wi);
    Vec3 F = fresnel->evaluate(dot(wo, h));
    return albedo * (D * G * F) / (4 * cos_wo * cos_wi);
}

Vec3 Microfacet::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    Vec3 h = distribution->sample_halfvector();
    wi = unit_vector(reflect(wo, h)); // reflect()では正規化しないので明示的に正規化
    pdf = sample_pdf(wo, wi);
    auto brdf = f(wo, wi);
    return brdf;
}