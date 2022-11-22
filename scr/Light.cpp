#include "Light.h"
#include "Shape.h"
#include "Random.h"
#include "Scene.h"

// *** ���� ***
bool Light::IsVisible(const intersection& p1, const intersection& p2, const Scene& world) {
	Ray r = Ray(p1.pos, unit_vector(p2.pos - p1.pos));
	intersection isect;
	return !world.intersect(r, 0.001f, inf, isect);
}

// *** �ʌ��� ***
AreaLight::AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape, int _type) 
	: Light(_type), intensity(_intensity), shape(_shape) {}

Vec3 AreaLight::emitte(const Vec3& wi, const intersection& p, float& pdf) {
	// �����菇
	// pdf���v�Z
	// �����_�ƃT���v���_�̉�����(TODO: ������Ȃ�������ǂ�����?)
	// pdf���v�Z: pdf=1/shape.area()�ɂȂ�(��l�T���v�����O�̂���)
	return intensity;
}

float AreaLight::sample_pdf(const Vec3& wi, const intersection& p) const {
	// wi�����[�J�����W�n�ɕϊ�
	// �ʌ����̓_�������_���ɃT���v��
	// wi���O���[�o�����W�n�ɕϊ�
}