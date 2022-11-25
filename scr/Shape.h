#pragma once

#include "Ray.h"

enum class IsectType {
	None = 1, Material = 2, Light = 4
};

// *** �����_��� ***
// TODO: �ʌ����ւ̑Ή�
struct intersection {
	Vec3 pos;    // �����_
	Vec3 normal; // �@��
	float t;     // ���C�̃p�����[�^
	std::shared_ptr<class Material> mat; // �}�e���A��
	std::shared_ptr<class Light> light; // ����
	IsectType type; // �����_�̎��(����or�}�e���A��)
};


// *** �`�󃂃f���N���X ***
class Shape {
public:
	virtual ~Shape() {};
	// ���C�ƃV�F�C�v�̌�������
	virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;
	// �ʐ�
	virtual float area() const = 0;
};