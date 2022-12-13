#include "Light.h"
#include "Shape.h"
#include "Random.h"
#include "Scene.h"

// *** ���� ***
bool Light::is_visible(const intersection& p1, const intersection& p2, const Scene& world) {
	Ray r = Ray(p1.pos, unit_vector(p2.pos - p1.pos));
	intersection isect;
	return !world.intersect(r, 0.001f, inf, isect);
}

// *** �ʌ��� ***
AreaLight::AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape)
	: Light(LightType::Area), intensity(_intensity), shape(_shape)
{
	area = shape->area();
}

Vec3 AreaLight::emitte() const {
	return intensity;
}

Vec3 AreaLight::power() const {
	return intensity * area * 4 * pi;
}

bool AreaLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
	return shape->intersect(r, t_min, t_max, p);
}

Vec3 AreaLight::sample_light(const intersection& p, Vec3& wo, float& pdf) {
	// NOTE: �����菇
	// 1. �ʌ����̓_�������_���ɃT���v��
	// 2. �����_�ƃT���v���_�̉�����
	// 3. �����̕��ˋP�x��Ԃ�
	auto pl = shape->sample(p, pdf);
	wo = unit_vector(pl.pos - p.pos);
	return emitte();
}