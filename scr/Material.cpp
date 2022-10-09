#include <complex>
#include "Material.h"
#include "Shape.h"
#include "Random.h"
#include "Vec3.h"
#include "Microfacet.h"
#include "Fresnel.h"

// *** �V�F�[�f�B���O�K�� ***
// 1. �@����z���Ƃ����V�F�[�f�B���O���W�n�ōs��
// 2. ���ˁE�o�˕����͕��̕\�ʂ��痣�������𐳂Ƃ���


// *** �����o�[�g���� ***
Diffuse::Diffuse(Vec3 _albedo) : albedo(_albedo) {}

float Diffuse::sample_pdf(const Vec3& wi, const Vec3& wo) const {
	//return std::max(std::abs(CosTheta(wo)) * invpi, epsilon);
	return 0.5 * invpi;
}
bool Diffuse::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	wo = Random::uniform_hemisphere_sample();
	pdf = sample_pdf(wi, wo);
	brdf = albedo * invpi;
	return true;
}


// *** ���S���ʔ��� ***
Mirror::Mirror(Vec3 _albedo) : albedo(_albedo) {}

float Mirror::sample_pdf(const Vec3& wi, const Vec3& wo) const {
	return 1.0f;
}
bool Mirror::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	// �f���^���z
	wo = unit_vector(Vec3(-wi.get_x(), -wi.get_y(), wi.get_z())); // �����˕���
	pdf = sample_pdf(wi, wo);
	float cos_term = dot(Vec3(0,0,1), unit_vector(wo));
	brdf = albedo / cos_term;
	return true;
}


// *** Phong���f�� ***
Phong::Phong(Vec3 _albedo, Vec3 _Kd, Vec3 _Ks, float _shin) 
	: albedo(_albedo), Kd(_Kd), Ks(_Ks), shin(_shin) {}

float Phong::sample_pdf(const Vec3& wi, const Vec3& wo) const {
	return std::max(std::abs(CosTheta(wo)) * invpi, epsilon);
}
bool Phong::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	// �]���ɏ]�����T���v�����O
	wo = Random::cosine_hemisphere_sample();
	pdf = sample_pdf(wi, wo);
	auto dir_spec = unit_vector(Vec3(-wi.get_x(), -wi.get_y(), wi.get_z())); // �����˕���
	Vec3 diffuse = Kd * invpi;
	float cos_term = dot(Vec3(0, 0, 1), unit_vector(wo));
	float sp = std::pow(std::max(0.0f, dot(dir_spec, wo)), shin);
	Vec3 specular = Ks / cos_term  * sp * invpi;
	brdf = albedo * (diffuse + specular);
	return true;
}


// *** �}�C�N���t�@�Z�b�g���f�� ***
// Reference: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
Microfacet::Microfacet(Vec3 _albedo, std::shared_ptr<MicrofacetDistribution> _distribution, 
					   std::shared_ptr<Fresnel> _fresnel)
	: albedo(_albedo), distribution(_distribution), fresnel(_fresnel) {}

float Microfacet::sample_pdf(const Vec3& wi, const Vec3& h) const {
	// h��PDF��wo��PDF��
	return distribution->sample_pdf(wi, h) / (4 * dot(wi, h));
}

bool Microfacet::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	Vec3 h = distribution->sample_halfvector();
	wo = unit_vector(reflect(wi, h)); // �n�[�t�x�N�g���Ɠ��˕�������o�˕������v�Z
	pdf = sample_pdf(wi, h);
	float cosThetaI = std::abs(CosTheta(wi));
	float cosThetaO = std::abs(CosTheta(wo));
	if (cosThetaI == 0 || cosThetaO == 0) return false;
	if (h.get_x() == 0 && h.get_y() == 0 && h.get_z() == 0) return false;
	float D = distribution->D(h);
	float G = distribution->G(wi, wo);
	Vec3 F = fresnel->Evaluate(dot(wi, h));
	brdf = albedo *  (D * G * F) / (4 * cosThetaI * cosThetaO);
	return true;
}


// *** ���� ***
Emitter::Emitter(Vec3 _intensity) : intensity(_intensity) {}

float Emitter::sample_pdf(const Vec3& wi, const Vec3& wo) const { return 1.0; }
bool Emitter::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	return false;
}

Vec3 Emitter::emitte() const {
	return intensity;
}