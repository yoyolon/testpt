#include "Sphere.h"
#include "Random.h"

// ���C�Ƌ��̌�������
bool Sphere::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
	// �񎟕������̔��ʎ�D = (b/2)^2 - ac�𗘗p(b�͋���)
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
	// �����_���̍X�V
	p.t = t;
	p.pos = r.at(p.t);
	p.normal = unit_vector(p.pos - center);
	p.mat = mat;
	return true;
};

float Sphere::area() const {
	return 4 * pi * radius;
}

intersection Sphere::sample(const intersection& p, float& pdf) const {
	intersection isect;
	return isect;
}


// ���C�Ɖ~�Ղ̌�������
bool Disk::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
	// ���C��z���ʏ�̌����_���~�����ɂ��邩����
	if (r.get_dir().get_y() == 0) {
		return false;
	}
	auto t = (center.get_y() - r.get_origin().get_y()) / r.get_dir().get_y();
	if (t < t_min || t > t_max) {
		return false;
	}
	auto pos = r.at(t);
	auto dist2 = (pos.get_x() - center.get_x()) * (pos.get_x() - center.get_x())
			   + (pos.get_z() - center.get_z()) * (pos.get_z() - center.get_z());
	if (dist2 > radius * radius) return false;
	// �����_���̍X�V
	p.t = t;
	p.pos = pos;
	p.normal = unit_vector(Vec3(0.0f,0.0f,-r.get_dir().get_y()));
	p.mat = mat;
	return true;
};

float Disk::area() const {
	return 2 * pi * radius;
}


intersection Disk::sample(const intersection& p, float& pdf) const {
	// NOTE: �菇
	// 1. �ʌ�����̓_����l�T���v�����O
	// 2. �����_�𐶐�
	// 3. ���˕����𐶐�
	// 4. ���̊p�Ɋւ���pdf���v�Z
	auto disk_sample = Random::concentric_disk_sample();
	auto x = disk_sample.get_x() * radius - center.get_x();
	auto z = disk_sample.get_y() * radius - center.get_z();
	auto y = center.get_y();
	intersection isect;
	isect.normal = Vec3(0, -1, 0);
	isect.pos = Vec3(x, y, z);
	auto wi = isect.pos - p.pos;
	//if (dot(isect.normal, unit_vector(-wi)) < 0) std::cout << "Error\n";
	pdf = wi.length2() / (std::abs(dot(isect.normal, unit_vector(-wi))) * area());
	return isect;
}