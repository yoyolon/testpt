// Reference: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models

#pragma once

#include "Vec3.h"

// *** �}�C�N���t�@�Z�b�g���z�N���X ***
class MicrofacetDistribution {
public:
	virtual ~MicrofacetDistribution();
	virtual float D(const Vec3& h) const = 0;
	virtual float lambda(const Vec3& w) const = 0;
	float G1(const Vec3& w) const {
		return 1 / (1 + lambda(w));
	}
	float G(const Vec3& wi, const Vec3& wo) const {
		return 1 / (1 + lambda(wi) + lambda(wo));
	}
	virtual Vec3 sample_halfvector() const = 0;
	virtual float sample_pdf(const Vec3& wi, const Vec3& h) const = 0;
};


// *** �x�b�N�}�����z ***
class BeckmannDistribution : public MicrofacetDistribution {
public:
	BeckmannDistribution(float alpha);
	float D(const Vec3& h) const override;
	float lambda(const Vec3& h) const override;
	Vec3 sample_halfvector() const override;
	float sample_pdf(const Vec3& wi, const Vec3& h) const override;

private: 
	float alpha;
};


// *** GGX���z ***
class GGXDistribution : public MicrofacetDistribution {
public:
	GGXDistribution(float alpha);
	float D(const Vec3& h) const override;
	float lambda(const Vec3& h) const override;
	Vec3 sample_halfvector() const override;
	float sample_pdf(const Vec3& wi, const Vec3& h) const override;

private:
	float alpha;
};