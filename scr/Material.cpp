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

float Diffuse::f_pdf(const Vec3& wi, const intersection& p, const Vec3& wo) const {
	//return std::max(unit_vector(wo).get_z() * invpi, 0.0f);
	return 0.5f * invpi;
}

bool Diffuse::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	//wo = Random::cosine_hemisphere_sample(); // cos�ɏ]�����T���v�����O
	wo = unit_vector(Random::uniform_hemisphere_sample()); // �����_���Ȕ��˕������T���v��
	pdf = f_pdf(wi, p, wo);
	brdf = albedo * invpi;
	return true;
}


// *** ���S���ʔ��� ***
Mirror::Mirror(Vec3 _albedo) : albedo(_albedo) {}

float Mirror::f_pdf(const Vec3& wi, const intersection& p, const Vec3& wo) const {
	return 1.0;
}

bool Mirror::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	wo = unit_vector(Vec3(-wi.get_x(), -wi.get_y(), wi.get_z())); // �����˕���
	pdf = f_pdf(wi, p, wo);
	float cos_term = dot(Vec3(0,0,1), unit_vector(wo));
	brdf = albedo / cos_term;
	return true;
}


// *** Phong���f�� ***
Phong::Phong(Vec3 _albedo, Vec3 _Kd, Vec3 _Ks, float _shin) 
	: albedo(_albedo), Kd(_Kd), Ks(_Ks), shin(_shin) {}

float Phong::f_pdf(const Vec3& wi, const intersection& p, const Vec3& wo) const {
	return 0.5f * invpi;
	//return std::max(unit_vector(wo).get_z() * invpi, 0.0f);
}

bool Phong::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	wo = unit_vector(Random::uniform_hemisphere_sample()); // �����_���Ȕ��˕������T���v��
	//wo = Random::cosine_hemisphere_sample(); // cos�ɏ]�����T���v�����O
	auto dir_spec = unit_vector(Vec3(-wi.get_x(), -wi.get_y(), wi.get_z())); // �����˕���
	pdf = f_pdf(wi, p, wo);
	Vec3 diffuse = Kd * invpi;
	float cos_term = dot(Vec3(0, 0, 1), unit_vector(wo));
	float sp = std::pow(std::max(0.0f, dot(dir_spec, wo)), shin);
	Vec3 specular = Ks / cos_term  * sp * invpi;
	brdf = albedo * (diffuse + specular);
	return true;
}


// *** ���� ***
Emitter::Emitter(Vec3 _intensity) : intensity(_intensity) {}

bool Emitter::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	return false;
}

Vec3 Emitter::emitte() const {
	return intensity;
}

// *** �}�C�N���t�@�Z�b�g���f�� ***
// Reference: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
Microfacet::Microfacet(Vec3 _albedo, std::shared_ptr<MicrofacetDistribution> _distribution, 
					   std::shared_ptr<Fresnel> _fresnel)
	: albedo(_albedo), distribution(_distribution), fresnel(_fresnel) {}

bool Microfacet::f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const {
	wo = unit_vector(Random::uniform_hemisphere_sample()); // �����_���Ȕ��˕������T���v��
	Vec3 h = wi + wo;
	h = unit_vector(h);
	float cosThetaI = std::abs(CosTheta(wi));
	float cosThetaO = std::abs(CosTheta(wo));
	if (cosThetaI == 0 || cosThetaO == 0) return false;
	if (h.get_x() == 0 && h.get_y() == 0 && h.get_z() == 0) return false;
	Vec3 numerator = distribution->D(h) * distribution->G(wi, wo) * fresnel->Evaluate(dot(wi, h));
	float denominator = 4 * cosThetaI * cosThetaO;
	brdf = albedo *  numerator / denominator;
	pdf = f_pdf(wi, p, wo);
	return true;
}

float Microfacet::f_pdf(const Vec3& wi, const intersection& p, const Vec3& wo) const {
	return 0.5f * invpi;
}