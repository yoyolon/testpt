#pragma once

#include "Ray.h"

struct intersection;

// NOTE: Material��Object��"���L�����"��Light��Object��"���L����"

enum class LightType {
	Area = 1, IBL = 2
};

// *** ���� ***
class Light {
public:
	virtual ~Light() {};
	Light(LightType _type) : type(_type) {};
	virtual Vec3 emitte() const = 0;
	virtual Vec3 power() const = 0;
	virtual Vec3 sample_light(const intersection& p, Vec3& wo, float& pdf) = 0;
	virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;
	bool is_visible(const intersection& p1, const intersection& p2, const class Scene& world);

private:
	const LightType type;
};


// *** �ʌ��� ***
class AreaLight : public Light {
public:
	AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape);
	Vec3 emitte() const override;
	Vec3 power() const override;
	Vec3 sample_light(const intersection& p, Vec3& wo, float& pdf) override;
	bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
	Vec3 intensity;
	std::shared_ptr<class Shape> shape; // �ʌ����̃W�I���g��
	float area;
};