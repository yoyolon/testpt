#include "ONB.h"

// �@������ONB���\�z
void ONB::build_ONB(const Vec3& normal) {
	n = unit_vector(normal);
	s = unit_vector(cross(n, Vec3(1.0f,0.0,0.0)));
	if (s.length2() <= epsilon) s = unit_vector(cross(n, Vec3(0.0f,1.0,0.0)));
	t = unit_vector(cross(n, s));
}

// ���[���h���W�����[�J�����W�ɕϊ�
Vec3 ONB::world2local(const Vec3& a) const {
	return Vec3(dot(a, s), dot(a, t), dot(a, n));
}

// ���[�J�����W�����[���h���W�ɕϊ�
Vec3 ONB::local2world(const Vec3& a) const {
	return a.get_x() * s + a.get_y() * t + a.get_z() * n;
}