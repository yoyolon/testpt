#pragma once

#include "Ray.h"

struct intersection;

// NOTE: Material��Object��"���L�����"���CLight��Object��"���L����"

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
	virtual float sample_pdf(const Vec3& wi, const intersection& p) const = 0;
	virtual Vec3 sample_Li(Vec3& wo, float& pdf) = 0; // �����������T���v�������ˋP�x��Ԃ�
	virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;
	bool IsVisible(const intersection& p1, const intersection& p2, const class Scene& world);

private:
	const LightType type;
};


// *** �ʌ��� ***
class AreaLight : public Light {
public:
	AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape, LightType type);
	Vec3 emitte() const override;
	Vec3 power() const override;
	float sample_pdf(const Vec3& wi, const intersection& p) const override;
	Vec3 sample_Li(Vec3& wo, float& pdf) override;
	bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
	Vec3 intensity;
	std::shared_ptr<class Shape> shape; // �ʌ����̖�
	float area;
};