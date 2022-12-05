#pragma once

#include "Vec3.h"

// *** ���C�N���X ***
class Ray {
public:
	Ray() {}
	Ray(Vec3 _origin, Vec3 _dir) : origin(_origin), dir(_dir) {};

	// �Q�b�^
	Vec3 get_dir() const { return dir; }
	Vec3 get_origin() const { return origin; }

	// �ʒu�̎擾
	inline Vec3 at(float t) const { return origin + t * dir; }

private:
	Vec3 origin, dir;
};