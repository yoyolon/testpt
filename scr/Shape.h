#pragma once

#include "Ray.h"

// *** �����_��� ***
struct intersection {
	Vec3 pos;    // �����_
	Vec3 normal; // �@��
	float t;     // ���C�̃p�����[�^
	std::shared_ptr<class Material> mat; // �}�e���A��
};


// *** �`�󃂃f���N���X ***
class Shape {
public:
	virtual ~Shape() {};
	// ���C�ƃV�F�C�v�̌�������
	virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;
};