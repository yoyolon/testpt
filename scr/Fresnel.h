#pragma once

#include "Vec3.h"
#include <complex>

// *** ƒtƒŒƒlƒ‹ƒNƒ‰ƒX ***
class Fresnel {
public:
	virtual ~Fresnel();
	virtual Vec3 evaluate(float cos_theta) const = 0;
};

// *** Schlick‹ß— ***
class FresnelSchlick : public Fresnel {
public:
	FresnelSchlick(Vec3 _F0);
	Vec3 evaluate(float cos_theta) const override;

private:
	Vec3 F0;
};

// *** —U“d‘Ì ***
class FresnelDielectric : public Fresnel {
public:
	FresnelDielectric(float _ni, float _no);
	Vec3 evaluate(float cos_theta) const override;

private:
	float ni, no;
};

// *** ’P‘w”––ŒŠ±Â ***
// float d  : –ŒŒú
// float ni : “üË”}¿‚Ì‹üÜ—¦
// float nf : ”––Œ”}¿‚Ì‹üÜ—¦
// float no : oË”}¿‚Ì‹üÜ—¦
class FresnelThinfilm : public Fresnel {
public:
	FresnelThinfilm(float d, float ni, float _nf, float _no);
	Vec3 evaluate(float cos_theta) const override;

private:
		float d;    // –ŒŒú
		float ni;   // “üË”}¿‚Ì‹üÜ—¦
		float nf;	// ”––Œ‚Ì‹üÜ—¦
		float no;   // ƒx[ƒX”}¿‚Ì‹üÜ—¦
};