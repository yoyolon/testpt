#include "Microfacet.h"
#include "Material.h"

MicrofacetDistribution::~MicrofacetDistribution() {}


// *** ベックマン分布 ***
BeckmannDistribution::BeckmannDistribution(float _alpha)
	: alpha(_alpha) {}

float BeckmannDistribution::D(const Vec3& h) const {
	float tan2Theta = Tan2Theta(h);
	if (std::isinf(tan2Theta)) return 0;
	float cos4Theta = Cos2Theta(h) * Cos2Theta(h);
	float alpha2 = alpha * alpha;
	return std::exp(-tan2Theta / alpha2) / (pi * cos4Theta * alpha2);
}

float BeckmannDistribution::Lambda(const Vec3& w) const {
	float tanTheta = std::abs(TanTheta(w));
	if (std::isinf(tanTheta)) return 0;
	// PBRTの有理多項式近似を用いる
	float a = 1 / (alpha * tanTheta);
	if (a >= 1.6f) return 0;
	return (1.0f - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}


// *** GGX分布 ***
GGXDistribution::GGXDistribution(float _alpha)
	: alpha(_alpha) {}

float GGXDistribution::D(const Vec3& h) const {
	float tan2Theta = Tan2Theta(h);
	if (std::isinf(tan2Theta)) return 0;
	float cos4Theta = Cos2Theta(h) * Cos2Theta(h);
	float alpha2 = alpha * alpha;
	return 1 / (pi * cos4Theta * alpha2 * (1.0f + tan2Theta / alpha2) * (1.0f + tan2Theta / alpha2));
}

float GGXDistribution::Lambda(const Vec3& w) const {
	float tanTheta = std::abs(TanTheta(w));
	if (std::isinf(tanTheta)) return 0;
	return (-1.0f + std::sqrt(1.0f + alpha * alpha * tanTheta * tanTheta)) / 2;
}