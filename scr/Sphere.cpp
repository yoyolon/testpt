#include "Sphere.h"

// CÆÌð·»è
bool Sphere::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
	// ñûö®Ì»Ê®D = b^2 - 4acðp
	auto temp = r.get_orig() - center;
	auto a = r.get_dir().length2();
	auto b = 2 * dot(r.get_dir(), temp);
	auto c = temp.length2() - radius * radius;
	auto D = b * b - 4 * a * c;

	if (D < 0) return false;
	auto d = std::sqrt(D);
	auto t = (-b - d) / (2 * a);
	if (t < t_min || t > t_max) {
		t = (-b + d) / (2 * a);
		if (t < t_min || t > t_max) {
			return false;
		}
	}
	// ð·_îñÌXV
	p.t = t;
	p.pos = r.at(p.t);
	p.normal = unit_vector(p.pos - center);
	p.mat = mat;
	return true;
};