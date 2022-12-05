#pragma once

#include "Material.h"
#include "Shape.h"

// *** ‹… ***
class Sphere : public Shape {
public:
	Sphere() : radius(1.0f) {};
	Sphere(Vec3 c, float r, std::shared_ptr<Material> m) : center(c), radius(r), mat(m) {};

	bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;
	float area() const override;

private:
	Vec3 center;
	float radius;
	std::shared_ptr<Material> mat;
};