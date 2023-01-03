#include "Material.h"
#include <complex>
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** ランバート反射 ***
Diffuse::Diffuse(Vec3 _albedo) : Material(MaterialType::Diffuse), albedo(_albedo) {}

float Diffuse::sample_pdf(const Vec3& wi, const Vec3& wo) const {
    return std::max(std::abs(get_cos(wo)) * invpi, epsilon);
}

Vec3 Diffuse::f(const Vec3& wi, const Vec3& wo) const {
    return albedo * invpi;
}

Vec3 Diffuse::sample_f(const Vec3& wi, const intersection& p, Vec3& wo, float& pdf) const {
    wo = Random::cosine_hemisphere_sample();
    pdf = sample_pdf(wi, wo);
    auto brdf = f(wi, wo);
    return brdf;
}


// *** 完全鏡面反射 ***
Mirror::Mirror(Vec3 _albedo) : Material(MaterialType::Specular), albedo(_albedo) {}


float Mirror::sample_pdf(const Vec3& wi, const Vec3& wo) const {
    return 1.0f;
}

Vec3 Mirror::f(const Vec3& wi, const Vec3& wo) const {
    return albedo / dot(Vec3(0, 0, 1), unit_vector(wo));
}

Vec3 Mirror::sample_f(const Vec3& wi, const intersection& p, Vec3& wo, float& pdf) const {
    wo = Vec3(-wi.get_x(), -wi.get_y(), wi.get_z()); // 正反射方向
    pdf = sample_pdf(wi, wo);
    auto brdf = f(wi, wo);
    return brdf;
}


// *** Phongモデル ***
// TODO: 正規化/より効率的な重点的サンプリングの実装
Phong::Phong(Vec3 _albedo, Vec3 _Kd, Vec3 _Ks, float _shin) 
    : Material(MaterialType::Glossy), albedo(_albedo), Kd(_Kd), Ks(_Ks), shin(_shin) {}

float Phong::sample_pdf(const Vec3& wi, const Vec3& wo) const {
    return std::max(std::abs(get_cos(wo)) * invpi, epsilon);
}

Vec3 Phong::f(const Vec3& wi, const Vec3& wo) const {
    auto dir_spec = Vec3(-wi.get_x(), -wi.get_y(), wi.get_z()); // 正反射方向
    Vec3 diffuse = Kd * invpi;
    Vec3 specular = Ks * std::pow(dot(dir_spec, wo), shin) * invpi;
    return albedo * (diffuse + specular);
}

Vec3 Phong::sample_f(const Vec3& wi, const intersection& p, Vec3& wo, float& pdf) const {
    wo = Random::cosine_hemisphere_sample();
    pdf = sample_pdf(wi, wo);
    auto brdf = f(wi, wo);
    return brdf;
}


// *** マイクロファセットモデル ***
Microfacet::Microfacet(Vec3 _albedo, std::shared_ptr<MicrofacetDistribution> _distribution, 
                       std::shared_ptr<Fresnel> _fresnel)
    : Material(MaterialType::Glossy), albedo(_albedo), distribution(_distribution), fresnel(_fresnel) {}

float Microfacet::sample_pdf(const Vec3& wi, const Vec3& wo) const {
    auto h = unit_vector(wi + wo);
    return distribution->sample_pdf(wi, h) / (4 * dot(wi, h)); // 確率密度の変換
}

Vec3 Microfacet::f(const Vec3& wi, const Vec3& wo) const {
    if (wi.get_z() * wo.get_z() < 0) {
        return Vec3::zero; // 同一半球内に存在しないなら反射しない(単散乱仮定のため)
    }
    auto h = unit_vector(wi + wo);
    float cos_wi = std::abs(get_cos(wi));
    float cos_wo = std::abs(get_cos(wo));
    if (cos_wi == 0 || cos_wo == 0) {
        return Vec3::zero;
    }
    if (is_zero(h)) {
        return Vec3::zero;
    }
    float D = distribution->D(h);
    float G = distribution->G(wi, wo);
    Vec3 F = fresnel->evaluate(dot(wi, h));
    return albedo * (D * G * F) / (4 * cos_wi * cos_wo);
}

Vec3 Microfacet::sample_f(const Vec3& wi, const intersection& p, Vec3& wo, float& pdf) const {
    Vec3 h = distribution->sample_halfvector();
    wo = unit_vector(reflect(wi, h)); // reflect()では正規化しない
    pdf = sample_pdf(wi, wo);
    auto brdf = f(wi, wo);
    return brdf;
}