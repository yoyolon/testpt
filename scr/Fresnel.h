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
	FresnelSchlick(float _ni, float _no);
	Vec3 Evaluate(float cosTheta) const override;

private:
	float ni, no;
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