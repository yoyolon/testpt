#pragma once

#include "Vec3.h"
#include <complex>

// *** フレネルクラス ***
class Fresnel {
public:
	virtual ~Fresnel();
	virtual Vec3 Evaluate(float cosTheta) const = 0;
};

// *** Schlick近似 ***
class FresnelSchlick : public Fresnel {
public:
	FresnelSchlick(Vec3 _F0);
	Vec3 Evaluate(float cosTheta) const override;

private:
	Vec3 F0;
};

// *** 誘電体 ***
class FresnelDielectric : public Fresnel {
public:
	FresnelDielectric(float _ni, float _no);
	Vec3 Evaluate(float cosTheta) const override;

private:
	float ni, no;
};

// *** 単層薄膜干渉 ***
class FresnelThinfilm : public Fresnel {
public:
	FresnelThinfilm(float d, float ni, float _nf, float _no);
	Vec3 Evaluate(float cosTheta) const override;

private:
		float d;    // 膜厚
		float ni;   // 入射媒質の屈折率
		float nf;	// 薄膜の屈折率
		float no;   // ベース媒質の屈折率
};