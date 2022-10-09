#pragma once

#include "Vec3.h"
#include <complex>

// *** tlNX ***
class Fresnel {
public:
	virtual ~Fresnel();
	virtual Vec3 Evaluate(float cosTheta) const = 0;
};

// *** Schlickί ***
class FresnelSchlick : public Fresnel {
public:
	FresnelSchlick(float _ni, float _no);
	Vec3 Evaluate(float cosTheta) const override;

private:
	float ni, no;
	Vec3 F0;
};

// *** UdΜ ***
class FresnelDielectric : public Fresnel {
public:
	FresnelDielectric(float _ni, float _no);
	Vec3 Evaluate(float cosTheta) const override;

private:
	float ni, no;
};

// *** ±Β ***
class FresnelThinfilm : public Fresnel {
public:
	FresnelThinfilm(float d, float ni, float _nf, float _no);
	Vec3 Evaluate(float cosTheta) const override;

private:
		float d;    // ϊ
		float ni;   // όΛ}ΏΜόά¦
		float nf;	// Μόά¦
		float no;   // x[X}ΏΜόά¦
};