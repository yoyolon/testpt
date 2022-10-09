#pragma once

#include <cmath>
#include <iostream>
#include "utility.h"

// *** 3³xNgNX ***
class Vec3 {
public:
	Vec3(float _x=0.0f, float _y=0.0f, float _z=0.0f) : e{_x, _y, _z} {};
	Vec3(float* _e) : e{_e[0], _e[1], _e[2]} {};

	// Qb^
	float get_x() const { return e[0]; }
	float get_y() const { return e[1]; }
	float get_z() const { return e[2]; }

	// zρANZXZq
	float operator[](int i) const { return e[i]; }
	float& operator[](int i) { return e[i]; }

	// PZq
	Vec3 operator+() const { return *this; }
	Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }

	// ‘γόZq
	Vec3& operator+=(const Vec3 & a) {
		e[0] += a.e[0];
		e[1] += a.e[1];
		e[2] += a.e[2];
		return *this;
	}
	Vec3& operator-=(const Vec3& a) {
		e[0] -= a.e[0];
		e[1] -= a.e[1];
		e[2] -= a.e[2];
		return *this;
	}
	Vec3& operator*=(float t) {
		e[0] *= t;
		e[1] *= t;
		e[2] *= t;
		return *this;
	}
	Vec3& operator/=(float t) {
		if (t == 0) return *this; // TODO: [ZΜ
		float invt = 1.0f / t;
		return *this *= invt;
	}

	// ρZq
	friend inline Vec3 operator+(const Vec3& a, const Vec3& b) {
		return Vec3(a.e[0] + b.e[0], a.e[1] + b.e[1], a.e[2] + b.e[2]);
	}
	friend inline Vec3 operator-(const Vec3& a, const Vec3& b) {
		return Vec3(a.e[0] - b.e[0], a.e[1] - b.e[1], a.e[2] - b.e[2]);
	}
	friend inline Vec3 operator*(const Vec3& a, const Vec3& b) {
		return Vec3(a.e[0] * b.e[0], a.e[1] * b.e[1], a.e[2] * b.e[2]);
	}
	friend inline Vec3 operator*(const Vec3& a, float t) {
		return Vec3(a.e[0] * t, a.e[1] * t, a.e[2] * t);
	}
	friend inline Vec3 operator*(float t, const Vec3& a) {
		return Vec3(a.e[0] * t, a.e[1] * t, a.e[2] * t);
	}
	friend inline Vec3 operator/(const Vec3& a, float t) {
		if (t == 0) return a; // [ZΜ
		float invt = 1.0f / t;
		return Vec3(a.e[0] * invt, a.e[1] * invt, a.e[2] * invt);
	}

	// ·³ΜζΎ
	float length2() const { return e[0] * e[0] + e[1] * e[1] + e[2] * e[2]; }
	float length() const { return std::sqrt(length2()); }
	float average() const { return (e[0] + e[1] + e[2]) / 3; }
	// ΰΟ
	friend inline float dot(Vec3 a, Vec3 b) { return a.e[0] * b.e[0] + a.e[1] * b.e[1] + a.e[2] * b.e[2]; }
	// OΟ
	friend inline Vec3 cross(Vec3 a, Vec3 b) {
		return Vec3(a.e[1] * b.e[2] - a.e[2] * b.e[1],
					a.e[2] * b.e[0] - a.e[0] * b.e[2],
					a.e[0] * b.e[1] - a.e[1] * b.e[0]);
	}

private:
	float e[3];
};

// oΝ
inline std::ostream& operator<<(std::ostream& s, const Vec3& a) {
	return s << '(' << a.get_x() << ' ' << a.get_y() << ' ' << a.get_z() << ')';
}


// *** UtilityΦ ***
// PΚxNg
inline bool IsZero(const Vec3& a) { return a.length2() > 0 ? false : true; }

inline Vec3 unit_vector(const Vec3& a) { return a / a.length(); }

// ³½ΛϋόxNg(όΛpͺOό«πίφ)
inline Vec3 reflect(const Vec3& wo, const Vec3& n) {
	return -wo + 2 * dot(wo, n) * n;
}