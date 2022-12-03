#pragma once

#include "Ray.h"

// *** シェーディング規則 ***
// 1. 法線をz軸正の方向としたシェーディング座標系で行う
// 2. 入射/出射方向は物体表面から離れる方向を正とする
// 3. z軸とベクトルがなす角をthetaとする．

struct intersection;

// *** ユーティリティ関数 ***
inline float get_cos(const Vec3& w) { return w.get_z(); }
inline float get_cos2(const Vec3& w) { return w.get_z() * w.get_z(); }
inline float get_sin2(const Vec3& w) { return std::max(0.0f, 1.0f - get_cos2(w)); }
inline float get_sin(const Vec3& w) { return std::sqrt(get_sin2(w)); }
inline float get_tan(const Vec3& w) { return get_sin(w) / get_cos(w); }
inline float get_tan2(const Vec3& w) { return get_sin2(w) / get_cos2(w); }


// *** マテリアルクラス ***
class Material {
public:
	virtual ~Material() {};
	virtual bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const = 0;
	virtual Vec3 emitte() const { return Vec3(0.0f, 0.0f, 0.0f); }
	virtual float sample_pdf(const Vec3& wi, const Vec3& wo) const = 0;
};


// *** 拡散反射 ***
class Diffuse : public Material {
public:
	Diffuse(Vec3 _albedo);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;
private:
	Vec3 albedo;
};


// *** 鏡面反射 ***
class Mirror : public Material {
public:
	Mirror(Vec3 _albedo);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;
private:
	Vec3 albedo;
};


// *** Phongモデル ***
class Phong : public Material {
public:
	Phong(Vec3 _albedo, Vec3 Kd, Vec3 Ks, float shin);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;
private:
	Vec3 albedo;
	Vec3 Kd; // 拡散反射係数
	Vec3 Ks; // 鏡面反射係数
	float shin; // 光沢度
};


// *** 発光 ***
class Emitter : public Material {
public:
	Emitter(Vec3 _intensity);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;
	Vec3 emitte() const;
private:
	Vec3 intensity; // 強度
};


// *** マイクロファセットBRDF ***
// 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
class Microfacet : public Material {
public:
	Microfacet(Vec3 _albedo, std::shared_ptr<class MicrofacetDistribution> _distribution, 
		       std::shared_ptr<class Fresnel> _fresnel);
	bool f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;
	float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
	Vec3 albedo;
	std::shared_ptr<class MicrofacetDistribution> distribution; // マイクロファセット分布
	std::shared_ptr<class Fresnel> fresnel; // フレネル項
};