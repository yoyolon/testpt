#include "Sphere.h"

// レイと球の交差判定
bool Sphere::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
	// 二次方程式の判別式D = (b/2)^2 - acを利用(bは偶数)
	auto temp = r.get_origin() - center;
	auto a = r.get_dir().length2();
	auto b_half = dot(r.get_dir(), temp);
	auto c = temp.length2() - radius * radius;
	auto D = b_half * b_half - a * c;

	if (D < 0) return false;
	auto b = b_half * 2;
	auto d = 2 * std::sqrt(D);
	auto t = (-b - d) / (2 * a);
	if (t < t_min || t > t_max) {
		t = (-b + d) / (2 * a);
		if (t < t_min || t > t_max) {
			return false;
		}
	}
	// 交差点情報の更新
	p.t = t;
	p.pos = r.at(p.t);
	p.normal = unit_vector(p.pos - center);
	p.mat = mat;
	return true;
};

float Sphere::area() const {
	return 4 * pi * radius;
}