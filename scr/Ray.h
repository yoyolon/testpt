#pragma once

#include "Vec3.h"

// *** レイクラス ***
class Ray {
public:
	Ray() {}
	Ray(Vec3 _orig, Vec3 _dir) : orig(_orig), dir(_dir) {};

	// ゲッタ
	Vec3 get_dir() const { return dir; }
	Vec3 get_orig() const { return orig; }

	// 位置の取得
	inline Vec3 at(float t) const { return orig + t * dir; }

private:
	Vec3 orig, dir;
};