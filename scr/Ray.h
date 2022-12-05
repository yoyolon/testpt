#pragma once

#include "Vec3.h"

// *** レイクラス ***
class Ray {
public:
	Ray() {}
	Ray(Vec3 _origin, Vec3 _dir) : origin(_origin), dir(_dir) {};

	// ゲッタ
	Vec3 get_dir() const { return dir; }
	Vec3 get_origin() const { return origin; }

	// 位置の取得
	inline Vec3 at(float t) const { return origin + t * dir; }

private:
	Vec3 origin, dir;
};