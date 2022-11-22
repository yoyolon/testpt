#pragma once

#include "Ray.h"

struct intersection;

// *** ���[�e�B���e�B�֐� ***
inline float CosTheta(const Vec3& w) { return w.get_z(); }
inline float Cos2Theta(const Vec3& w) { return w.get_z() * w.get_z(); }
inline float Sin2Theta(const Vec3& w) { return std::max(0.0f, 1.0f - Cos2Theta(w)); }
inline float SinTheta(const Vec3& w) { return std::sqrt(Sin2Theta(w)); }
inline float TanTheta(const Vec3& w) { return SinTheta(w) / CosTheta(w); }
inline float Tan2Theta(const Vec3& w) { return Sin2Theta(w) / Cos2Theta(w); }


// *** �}�e���A���N���X ***
class Material {
public:
	virtual ~Material() {};
	virtual bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const = 0;
	virtual Vec3 emitte() const { return Vec3(0.0f, 0.0f, 0.0f); }
	virtual float sample_pdf(const Vec3& wi, const Vec3& wo) const = 0;
};


// *** �g�U���� ***
class Diffuse : public Material {
public:
	Diffuse(Vec3 _albedo);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;
private:
	Vec3 albedo;
};


// *** ���ʔ��� ***
class Mirror : public Material {
public:
	Mirror(Vec3 _albedo);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;
private:
	Vec3 albedo;
};


// *** Phong���f�� ***
class Phong : public Material {
public:
	Phong(Vec3 _albedo, Vec3 Kd, Vec3 Ks, float shin);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;
private:
	Vec3 albedo;
	Vec3 Kd; // �g�U���ˌW��
	Vec3 Ks; // ���ʔ��ˌW��
	float shin; // ����x
};


// *** ���� ***
class Emitter : public Material {
public:
	Emitter(Vec3 _intensity);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;
	Vec3 emitte() const;
private:
	Vec3 intensity; // ���x
};


// *** �}�C�N���t�@�Z�b�gBRDF ***
// �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
class Microfacet : public Material {
public:
	Microfacet(Vec3 _albedo, std::shared_ptr<class MicrofacetDistribution> _distribution, 
		       std::shared_ptr<class Fresnel> _fresnel);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
	Vec3 albedo;
	std::shared_ptr<class MicrofacetDistribution> distribution; // �}�C�N���t�@�Z�b�g���z
	std::shared_ptr<class Fresnel> fresnel; // �t���l����
};