
#include "Microfacet.h"
#include "Material.h"
#include "Random.h"

MicrofacetDistribution::~MicrofacetDistribution() {}


// *** ベックマン分布 ***
BeckmannDistribution::BeckmannDistribution(float _alpha)
    : alpha(_alpha) {}

float BeckmannDistribution::D(const Vec3& h) const {
    float tan2_theta = get_tan2(h);
    if (std::isinf(tan2_theta)) return 0;
    float cos4_theta = get_cos2(h) * get_cos2(h);
    float alpha2 = alpha * alpha;
    return std::exp(-tan2_theta / alpha2) / (pi * cos4_theta * alpha2);
}

float BeckmannDistribution::lambda(const Vec3& w) const {
    float tan_theta = std::abs(get_tan(w));
    if (std::isinf(tan_theta)) return 0;
    // PBRTの有理多項式近似を用いる
    float a = 1 / (alpha * tan_theta);
    if (a >= 1.6f) return 0;
    return (1.0f - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}

Vec3 BeckmannDistribution::sample_halfvector() const {
    return Random::beckmann_sample(alpha);
}

float BeckmannDistribution::sample_pdf(const Vec3& wi, const Vec3& h) const {
    return D(h) * std::abs(get_cos(h));
}


// *** Trowbridge-Reitz(GGX)分布 ***
GGXDistribution::GGXDistribution(float _alpha)
    : alpha(_alpha) {}

float GGXDistribution::D(const Vec3& h) const {
    float tan2_theta = get_tan2(h);
    if (std::isinf(tan2_theta)) return 0;
    float cos4_theta = get_cos2(h) * get_cos2(h);
    float alpha2 = alpha * alpha;
    return 1 / (pi * cos4_theta * alpha2 * (1.0f + tan2_theta / alpha2) * (1.0f + tan2_theta / alpha2));
}

float GGXDistribution::lambda(const Vec3& w) const {
    float tan_theta = std::abs(get_tan(w));
    if (std::isinf(tan_theta)) return 0;
    return (-1.0f + std::sqrt(1.0f + alpha * alpha * tan_theta * tan_theta)) / 2;
}

Vec3 GGXDistribution::sample_halfvector() const {
    return Random::ggx_sample(alpha);
}

float GGXDistribution::sample_pdf(const Vec3& wi, const Vec3& h) const {
    return D(h) * std::abs(get_cos(h));
}