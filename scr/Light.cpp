#include "Light.h"
#include "Shape.h"
#include "Random.h"
#include "Scene.h"

// *** ŒõŒ¹ ***
bool Light::IsVisible(const intersection& p1, const intersection& p2, const Scene& world) {
	Ray r = Ray(p1.pos, unit_vector(p2.pos - p1.pos));
	intersection isect;
	return !world.intersect(r, 0.001f, inf, isect);
}

// *** –ÊŒõŒ¹ ***
AreaLight::AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape, int _type) 
	: Light(_type), intensity(_intensity), shape(_shape) {}