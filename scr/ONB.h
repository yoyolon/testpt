#pragma once

#include "Vec3.h"

// *** ���K�������(ONB) ***
class ONB {
public:
	// �R���X�g���N�^
	ONB() {}
	ONB(const Vec3& _s, const Vec3& _t, const Vec3& _n) : s(_s), t(_t), n(_n) {}

	// �Q�b�^
	Vec3 get_s() const { return s; }
	Vec3 get_t() const { return t; }
	Vec3 get_n() const { return n; }

	// �@������ONB���\�z
	void build_ONB(const Vec3& normal);

	// ���W�ϊ�
	Vec3 to_local(const Vec3& a) const;
	Vec3 to_world(const Vec3& a) const;

private:
	Vec3 s, t, n; // ���K�����x�N�g��
};