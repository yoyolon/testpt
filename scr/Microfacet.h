#pragma once
// Reference: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models

#include "Vec3.h"

// *** マイクロファセット分布クラス ***
class MicrofacetDistribution {
public:
	virtual ~MicrofacetDistribution();
	virtual float D(const Vec3& h) const = 0;
	virtual float Lambda(const Vec3& w) const = 0;
	float G1(const Vec3& w) const {
		return 1 / (1 + Lambda(w));
	}
	float G(const Vec3& wi, const Vec3& wo) const {
		return 1 / (1 + Lambda(wi) + Lambda(wo));
	}
};


// *** ベックマン分布 ***
class BeckmannDistribution : public MicrofacetDistribution {
public:
	BeckmannDistribution(float alpha);
	float D(const Vec3& h) const override;
	float Lambda(const Vec3& h) const override;

private: 
	float alpha;
};


// *** GGX分布 ***
class GGXDistribution : public MicrofacetDistribution {
public:
	GGXDistribution(float alpha);
	float D(const Vec3& h) const override;
	float Lambda(const Vec3& h) const override;

private:
	float alpha;
};