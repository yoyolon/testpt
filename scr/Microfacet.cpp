#include "Microfacet.h"
#include "Material.h"
#include "Random.h"


// *** ベックマン分布 ***
Beckmann::Beckmann(float _alpha)
    : alpha(_alpha) {}

float Beckmann::D(const Vec3& h) const {
    float tan2_theta = get_tan2(h);
    if (std::isinf(tan2_theta)) return 0;
    float cos4_theta = get_cos2(h) * get_cos2(h);
    float alpha2 = alpha * alpha;
    return std::exp(-tan2_theta / alpha2) / (pi * cos4_theta * alpha2);
}

float Beckmann::lambda(const Vec3& w) const {
    float tan_theta = std::abs(get_tan(w));
    if (std::isinf(tan_theta)) return 0;
    // PBRTの有理多項式近似を用いる
    float a = 1 / (alpha * tan_theta);
    if (a >= 1.6f) return 0;
    return (1.0f - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}

Vec3 Beckmann::sample_halfvector(const Vec3& wo) const {
    return beckmann_sample(alpha);
}

float Beckmann::eval_pdf(const Vec3& h, const Vec3 & wo) const {
    return D(h) * std::abs(get_cos(h));
}

Vec3 Beckmann::beckmann_sample(float alpha) const {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto logs = std::log(1.0f - u);
    if (std::isinf(logs)) logs = 0.f;
    auto tan2_theta = -alpha * alpha * logs;
    auto cos2_theta = 1 / (1 + tan2_theta);
    auto sin2_theta = 1 - cos2_theta;
    auto sin_theta = std::sqrt(std::max(sin2_theta, 0.f));
    auto phi = 2 * pi * v;
    auto z = std::sqrt(std::max(cos2_theta, 0.f));
    auto x = std::cos(phi) * sin_theta;
    auto y = std::sin(phi) * sin_theta;
    return Vec3(x, y, z);
}


// *** Trowbridge-Reitz(GGX)分布 ***

GGX::GGX(float _alpha, bool _is_vsible_sampling)
    : alpha(_alpha), is_vsible_sampling(_is_vsible_sampling) {}

float GGX::D(const Vec3& h) const {
    float tan2_theta = get_tan2(h);
    if (std::isinf(tan2_theta)) return 0;
    float cos4_theta = get_cos2(h) * get_cos2(h);
    float alpha2 = alpha * alpha;
    float tan_term = 1.0f + tan2_theta / alpha2;
    return 1 / (pi * cos4_theta * alpha2 * tan_term * tan_term);
}

float GGX::lambda(const Vec3& w) const {
    float tan_theta = std::abs(get_tan(w));
    if (std::isinf(tan_theta)) return 0;
    return 0.5f * (-1.0f + std::sqrt(1.0f + alpha * alpha * tan_theta * tan_theta));
}

Vec3 GGX::sample_halfvector(const Vec3& wo) const {
    if (is_vsible_sampling) {
        return visible_ggx_sample(wo, alpha);
    }
    return ggx_sample(alpha);
}

float GGX::eval_pdf(const Vec3& h, const Vec3& wo) const {
    if (is_vsible_sampling) {
        return D(h) * G1(wo) * std::max(0.f, dot(wo, h)) / std::abs(get_cos(wo));
    }
    return D(h) * std::abs(get_cos(h));
}

Vec3 GGX::ggx_sample(float alpha) const {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto tan2_theta = alpha * alpha * u / (1.0f - u); // NOTE: atan2は遅いので未使用
    auto cos2_theta = 1 / (1 + tan2_theta);
    auto sin2_theta = 1 - cos2_theta;
    auto sin_theta = std::sqrt(std::max(sin2_theta, 0.f));
    auto phi = 2 * pi * v;
    auto z = std::sqrt(std::max(cos2_theta, 0.f));
    auto x = std::cos(phi) * sin_theta;
    auto y = std::sin(phi) * sin_theta;
    return Vec3(x, y, z);
}

Vec3 GGX::visible_ggx_sample(const Vec3& wo, float alpha) const {
    // 楕円体での出射方向を半球に変換
    Vec3 wo_hemi = unit_vector(Vec3(alpha * wo[0], alpha * wo[1], wo[2]));
    // 正規直交基底を構築
    float denominator_squared = wo_hemi[0] * wo_hemi[0] + wo_hemi[1] * wo_hemi[1];
    Vec3 T1(1.0f, 0.f, 0.f);
    if (denominator_squared > 0) {
        T1 = Vec3(-wo_hemi[1], wo_hemi[0], 0.f) / std::sqrt(denominator_squared);
    }
    Vec3 T2 = cross(wo_hemi, T1);
    // 半球での法線をサンプリング
    Vec2 uv = Random::uniform_disk_sample();
    float s = 0.5f * (1.0f + wo_hemi[2]);
    float t1 = uv[0];
    float t2 = (1.0f - s) * std::sqrt(1.0f - t1 * t1) + s * uv[1];
    Vec3 wh_hemi = t1 * T1 + t2 * T2 + std::sqrt(std::max(0.f, 1 - t1 * t1 - t2 * t2)) * wo_hemi;
    // 半球での法線を楕円体での法線に変換
    Vec3 wh = Vec3(alpha * wh_hemi[0], alpha * wh_hemi[1], wh_hemi[2]);
    return unit_vector(wh);
}