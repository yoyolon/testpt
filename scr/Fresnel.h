#pragma once

#include "Vec3.h"
#include <complex>

// *** �t���l���N���X ***
class Fresnel {
public:
	virtual ~Fresnel();
	virtual Vec3 evaluate(float cos_theta) const = 0;
};

// *** Schlick�ߎ� ***
class FresnelSchlick : public Fresnel {
public:
	FresnelSchlick(Vec3 _F0);
	Vec3 evaluate(float cos_theta) const override;

private:
	Vec3 F0;
};

// *** �U�d�� ***
class FresnelDielectric : public Fresnel {
public:
	FresnelDielectric(float _ni, float _no);
	Vec3 evaluate(float cos_theta) const override;

private:
	float ni, no;
};

// *** �P�w�������� ***
class FresnelThinfilm : public Fresnel {
public:
	FresnelThinfilm(float d, float ni, float _nf, float _no);
	Vec3 evaluate(float cos_theta) const override;

private:
		float d;    // ����
		float ni;   // ���˔}���̋��ܗ�
		float nf;	// �����̋��ܗ�
		float no;   // �x�[�X�}���̋��ܗ�
};