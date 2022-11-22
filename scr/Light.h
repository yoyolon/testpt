#pragma once

#include "Ray.h"

struct intersection;

enum class LightType {
	Area = 1, IBL = 2
};


// *** ŒõŒ¹ ***
class Light {
public:
	virtual ~Light() {};
	Light(int _type) : type(_type) {};
	virtual Vec3 emitte(const Vec3& wi, const intersection& p, float& pdf) = 0;
	virtual Vec3 power() const = 0;
	virtual float sample_pdf(const Vec3& wi, const intersection& p) const = 0;
	bool IsVisible(const intersection& p1, const intersection& p2, const class Scene& world);

	const int type;
};


// *** –ÊŒõŒ¹ ***
class AreaLight : public Light {
public:
	AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape, int type);
	Vec3 emitte(const Vec3& wi, const intersection& p, float& pdf) override;
	float sample_pdf(const Vec3& wi, const intersection& p) const override;

private:
	Vec3 intensity;
	std::shared_ptr<class Shape> shape; // –ÊŒõŒ¹‚Ì–Ê
};