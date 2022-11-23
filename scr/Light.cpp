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
AreaLight::AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape, LightType _type) 
	: Light(_type), intensity(_intensity), shape(_shape)
{
	area = shape->area();
}

Vec3 AreaLight::emitte() const {
	return intensity;
}

Vec3 AreaLight::power() const {
	return intensity * area;
}

bool AreaLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
	return shape->intersect(r, t_min, t_max, p);
}

float AreaLight::sample_pdf(const Vec3& wi, const intersection& p) const {
	// NOTE: �����菇
	// wi�����[�J�����W�n�ɕϊ�
	// �ʌ����̓_�������_���ɃT���v��
	// pdf���v�Z
	// �����_�ƃT���v���_�̉�����(TODO: ������Ȃ�������ǂ�����?)
	// pdf���v�Z: pdf=1/shape.area()�ɂȂ�(��l�T���v�����O�̂���)
	// wi���O���[�o�����W�n�ɕϊ�
	return 1.0;
}