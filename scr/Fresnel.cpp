#include "Fresnel.h"


// *** フレネルの式 ***
inline float rs(float ni, float nj, float cosi, float cosj) {
	return (ni * cosi - nj * cosj) / (ni * cosi + nj * cosj);
}
inline float ts(float ni, float nj, float cosi, float cosj) {
	return (ni * cosi + ni * cosi) / (ni * cosi + nj * cosj);
}
inline float rp(float ni, float nj, float cosi, float cosj) {
	return (nj * cosi - ni * cosj) / (ni * cosj + nj * cosi);
}
inline float tp(float ni, float nj, float cosi, float cosj) {
	return (ni * cosi + ni * cosi) / (ni * cosj + nj * cosi);
}

// *** フレネルクラス ***
Fresnel::~Fresnel() {}


// *** Schlick近似 ***
FresnelSchlick::FresnelSchlick(float _ni, float _no)
	: ni(_ni), no(_no) 
{
	float f0 = (ni - no) / (ni + no);
	f0 *= f0;
	F0 = Vec3(f0, f0, f0);
}

Vec3 FresnelSchlick::Evaluate(float cosTheta) const {
	return F0 + (Vec3(1.0f,1.0f,1.0f) - F0) * (float)std::pow(1.0f - cosTheta, 5);
}


// *** 誘電体 ***
FresnelDielectric::FresnelDielectric(float _ni, float _no) 
	: ni(_ni), no(_no) {}

Vec3 FresnelDielectric::Evaluate(float cosTheta) const {
	float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
	float sinO = no / ni * sinTheta;
	if (sinO >= 1.0f) return Vec3(0.0f, 0.0f, 0.0f);
	float cosO = std::sqrt(std::max(0.0f, 1.0f - sinO * sinO));
	float Rs = rs(ni, no, cosTheta, cosO);
	float Rp = rp(ni, no, cosTheta, cosO);
	float R =  (Rs * Rs + Rp * Rp) / 2;
	return Vec3(R, R, R);
}