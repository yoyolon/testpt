#include "Sphere.h"
#include "Random.h"
#include "ONB.h"

// *** �� ***
Sphere::Sphere(Vec3 c, float r, std::shared_ptr<Material> m)
    : center(c), radius(r), mat(m) {};

bool Sphere::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // �񎟕������̔��ʎ�D/4 = (b/2)^2 - a*c�𗘗p(b�͋���)
    auto temp = r.get_origin() - center;
    auto a = r.get_dir().length2();
    auto b_half = dot(r.get_dir(), temp);
    auto c = temp.length2() - radius * radius;
    auto D = b_half * b_half - a * c;
    if (D < 0) {
        return false;
    }
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
    p.pos = r.at(t);
    p.normal = unit_vector(p.pos - center);
    p.is_front = is_front(r, p.normal);
    p.mat = mat;
    return true;
};

float Sphere::area() const {
    return 4 * pi * radius * radius;
}

intersection Sphere::sample(const intersection& ref) const {
    // �������l�T���v�����O(�v)
    //intersection isect;
    //isect.normal = Random::uniform_sphere_sample();
    //isect.pos = radius * isect.normal + center;
    //return isect;
    
    // ���̉��̈���l�����ăT���v�����O
    // ���[�J���Ȕ�����������T���v�����O
    auto z = unit_vector(ref.pos - center);
    auto sampling_coord = ONB(z);
    auto sampling_local_pos = Random::uniform_hemisphere_sample();
    // �O���[�o���Ȕ����ɕϊ�
    intersection isect;
    isect.normal = sampling_coord.to_world(sampling_local_pos);
    isect.pos = radius * isect.normal + center;
    return isect;
}

float Sphere::eval_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // ref����W�I���g���ւ̃��C
    intersection isect;                    // �W�I���g���̌����_
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.0f;
    }
    // �ʂ̗������T���v�������ꍇpdf���[���ɂ���
    if (dot(isect.normal, -w) < 0) {
        return 0.0f;
    }
    auto d = ref.pos - isect.pos;
    return d.length2() / (std::abs(dot(isect.normal, -w)) * area());
}

// *** �~�� ***
const Vec3 Disk::normal = Vec3(0.0f, -1.0f, 0.0f);

Disk::Disk(Vec3 c, float r, std::shared_ptr<Material> m, bool is_flip)
    : center(c), radius(r), mat(m), is_flip_normal(is_flip) {};

bool Disk::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // ���C��xz���ʂ̌����_���~�����ɂ��邩����
    if (r.get_dir().get_y() == 0) {
        return false;
    }
    auto t = (center.get_y() - r.get_origin().get_y()) / r.get_dir().get_y();
    if (t < t_min || t > t_max) {
        return false;
    }
    auto pos = r.at(t); // xz���ʂƂ̌����_
    // �~�Ƃ̌�������
    auto dx = pos.get_x() - center.get_x();
    auto dz = pos.get_z() - center.get_z();
    auto dist2 = dx * dx + dz * dz;
    if (dist2 > radius * radius) {
        return false;
    }
    // �����_���̍X�V
    p.t = t;
    p.pos = pos;
    p.normal = normal;
    if (is_flip_normal) p.normal *= -1; // �@���̔��]
    p.is_front = is_front(r, p.normal);
    p.mat = mat;
    return true;
};

float Disk::area() const {
    return pi * radius * radius;
}

intersection Disk::sample(const intersection& ref) const {
    auto disk_sample = Random::concentric_disk_sample();
    auto x = disk_sample.get_x() * radius + center.get_x();
    auto z = disk_sample.get_y() * radius + center.get_z();
    auto y = center.get_y();
    intersection isect;
    isect.normal = normal;
    if (is_flip_normal) isect.normal *= -1; // �@���̔��]
    isect.pos = Vec3(x, y, z);
    return isect;
}


// *** �~�� ***
Cylinder::Cylinder(Vec3 c, float r, float h, std::shared_ptr<Material> m)
    : center(c), radius(r), height(h), mat(m) {};

bool Cylinder::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // �񎟕������̔��ʎ�D/4 = (b/2)^2 - a*c�𗘗p(b�͋���)
    auto temp = r.get_origin() - center;
    auto dir_x = r.get_dir().get_x();
    auto dir_z = r.get_dir().get_z();
    auto temp_x = temp.get_x();
    auto temp_z = temp.get_z();
    auto a = dir_x * dir_x + dir_z * dir_z;
    auto b_half = dir_x * temp_x + dir_z * temp_z;
    auto c = temp_x * temp_x + temp_z * temp_z - radius * radius;
    auto D = b_half * b_half - a * c;
    if (D < 0) {
        return false;
    }
    auto b = b_half * 2;
    auto d = 2 * std::sqrt(D);
    auto t = (-b - d) / (2 * a);
    if (t < t_min || t > t_max) {
        t = (-b + d) / (2 * a);
        if (t < t_min || t > t_max) {
            return false;
        }
    }
     // �����_��y���W���~���͈͓̔�������
    auto y = r.at(t).get_y();
    auto y_min = center.get_y();
    auto y_max = y_min + height;
    if (y <= y_min || y >= y_max) {
        return false;
        t = (-b + d) / (2 * a); // �~���̓���
        auto y = r.at(t).get_y();
        if (y <= y_min || y >= y_max) {
            return false;
        }
    }
     // �����_���̍X�V
    p.t = t;
    p.pos = r.at(t);
    auto diff = p.pos - center;
    p.normal = unit_vector(Vec3(diff.get_x(), 0.0f, diff.get_z()));
    p.is_front = is_front(r, p.normal);
    p.mat = mat;
    return true;
}

float Cylinder::area() const {
    return 2 * pi * radius * height;
}

intersection Cylinder::sample(const intersection& p) const {
    // NOTE: y�����̕����������
    float u = Random::uniform_float();
    float v = Random::uniform_float();
    float y = center.get_y() + u * height;
    float phi = 2 * pi * v;
    float x = center.get_x() + std::sin(phi);
    float z = center.get_y() + std::cos(phi);
    intersection isect;
    isect.pos = Vec3(x, y, z);
    isect.normal = unit_vector(Vec3(x, 0, z));
    return isect;
}