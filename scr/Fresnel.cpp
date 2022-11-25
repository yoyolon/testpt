#include "Fresnel.h"
#include <complex>

// *** UtilityŠÖ” ***
// ƒtƒŒƒlƒ‹‚Ì®
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
// ”½ËŒW”
static std::complex<float> composit_r(float r, float R, float Phi) {
	std::complex<float>i(0.f, 1.f);
	return (r + R * exp(i * 2.0f * Phi)) / (1.0f + r * R * exp(i * 2.0f * Phi));
}
// “§‰ßŒW”
static std::complex<float> composit_t(float r, float R, float t, float T, float Phi) {
	std::complex<float>i(0.f, 1.f);
	return (t * T * exp(i * Phi)) / (1.0f + r * R * exp(i * 2.0f * Phi));
}
// ”––ŒŠ±Â”½Ë—¦
Vec3 iridterm(float cos0, float d, float n0, float n1, float n2) {
	// “§‰ßŠpŒvZ
	float sin0 = std::sqrt(std::max(0.0f, 1.0f - cos0 * cos0));
	float sin1 = n0 / n1 * sin0;
	if (sin1 >= 1.0) return  Vec3(1.0f, 1.0f, 1.0f); // ‘S”½Ë
	float cos1 = std::sqrt(std::max(0.0f, 1.0f - sin1 * sin1));
	float sin2 = n0 / n2 * sin0;
	if (sin2 >= 1.0) return Vec3(1.0f, 1.0f, 1.0f);  // ‘S”½Ë
	float cos2 = std::sqrt(std::max(0.0f, 1 - sin2 * sin2));
	// ƒtƒŒƒlƒ‹”½ËŒW”ŒvZ
	float r01s = rs(n0, n1, cos0, cos1);
	float r12s = rs(n1, n2, cos1, cos2);
	float r01p = rp(n0, n1, cos0, cos1);
	float r12p = rp(n1, n2, cos1, cos2);
	// ‡¬”½Ë—¦ŒvZ
	float lambda[3] = { 640.0, 540.0, 450.0 };
	float rgb[3] = { 0.0 };
	std::complex<float>r_s[3];
	std::complex<float>r_p[3];
	for (int i = 0; i < 3; i++) {
		float phi = 2 * pi / lambda[i] * n1 * d * cos1;
		//std::complex<float> phi_c(phi, 0.f);
		r_s[i] = composit_r(r01s, r12s, phi); // ‡¬”½ËŒW”(s”g)
		r_p[i] = composit_r(r01p, r12p, phi); // ‡¬”½ËŒW”(p”g)
		rgb[i] = (norm(r_s[i]) + norm(r_p[i])) / 2;
	}
	return Vec3(rgb);
}


// *** ƒtƒŒƒlƒ‹ƒNƒ‰ƒX ***
Fresnel::~Fresnel() {}


// *** Schlick‹ß— ***
FresnelSchlick::FresnelSchlick(Vec3 _F0)
	: F0(_F0) {}

Vec3 FresnelSchlick::Evaluate(float cosTheta) const {
	return F0 + (Vec3(1.0f,1.0f,1.0f) - F0) * (float)std::pow(1.0f - cosTheta, 5);
}


// *** —U“d‘Ì ***
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


// *** ”––ŒŠ±Â ***
FresnelThinfilm::FresnelThinfilm(float _d, float _ni, float _nf, float _no)
	: d(_d), ni(_ni), nf(_nf), no(_no) {}

Vec3 FresnelThinfilm::Evaluate(float cosTheta) const {
	return iridterm(cosTheta, d, ni, nf, no);
}