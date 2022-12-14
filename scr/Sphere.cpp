#include "Sphere.h"
#include "Random.h"

// *** �� ***
Sphere::Sphere(Vec3 c, float r, std::shared_ptr<Material> m)
    : center(c), radius(r), mat(m) {};

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
    p.pos = r.at(t);
    p.normal = unit_vector(p.pos - center);
    p.mat = mat;
    return true;
};

float Sphere::area() const {
    return 4 * pi * radius;
}

float Sphere::sample_pdf(const intersection& ref, const Vec3& w) const {
    // TODO: ����
    return 0.0f;
}
intersection Sphere::sample(const intersection& ref) const {
    // TODO: ����
    intersection isect;
    return isect;
}


// *** �~�� ***
Disk::Disk(Vec3 c, float r, std::shared_ptr<Material> m)
    : center(c), radius(r), mat(m) {};

bool Disk::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // ���C��xz���ʂ̌����_���~�����ɂ��邩����
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
    p.normal = unit_vector(Vec3(0.0f,-r.get_dir().get_y(),0.0f));
    p.mat = mat;
    return true;
};

float Disk::area() const {
    return 2 * pi * radius;
}

float Disk::sample_pdf(const intersection& ref, const Vec3& w) const {
    return w.length2() / (std::abs(dot(ref.normal, unit_vector(-w))) * area());
}

intersection Disk::sample(const intersection& ref) const {
    auto disk_sample = Random::concentric_disk_sample();
    auto x = disk_sample.get_x() * radius - center.get_x();
    auto z = disk_sample.get_y() * radius - center.get_z();
    auto y = center.get_y();
    intersection isect;
    isect.normal = Vec3(0, -1, 0);
    isect.pos = Vec3(x, y, z);
    auto wi = isect.pos - ref.pos;
    return isect;
}


// *** �~�� ***
Cylinder::Cylinder(Vec3 c, float r, float h, std::shared_ptr<Material> m)
    : center(c), radius(r), height(h), mat(m) {};

bool Cylinder::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // �������̉~�ɉ��̌����𗘗p
    if (r.get_dir().get_y() == 0) {
        return false;
    }
    auto temp = r.get_origin() - center;
    auto dir = r.get_dir();
    auto a = dir.get_x() * dir.get_x() + dir.get_z() + dir.get_z();
    auto b_half = dir.get_x() * temp.get_x() + dir.get_z() + temp.get_z();
    auto c = temp.get_x() * temp.get_x() + temp.get_z() * temp.get_z() - radius * radius;
    auto D = b_half * b_half - a * c;
    if (D < 0) return false;
    auto b = b_half * 2;
    auto d = 2 * std::sqrt(D);
    auto t = (-b - d) / (2 * a);
    if (t < t_min || t > t_max) {
        t = (-b + d) / (2 * a);
        if (t < t_min || t > t_max) {
            // �����_��y���W���~���͈͓̔�������
            auto y = r.at(t).get_y();
            auto y_min = center.get_y();
            auto y_max = y_min + height;
            if (y < y_min || y > y_max) {
                return false;
            }
        }
    }
    // �����_���̍X�V
    p.t = t;
    p.pos = r.at(t);
    p.normal = unit_vector(p.pos - Vec3(center.get_x(),p.pos.get_y(),center.get_z()));
    p.mat = mat;
    return true;
}

float Cylinder::area() const {
    return 2 * pi * radius * height;
}

float Cylinder::sample_pdf(const intersection& ref, const Vec3& w) const {
    // TODO: ����
    return 0.0f;
}
intersection Cylinder::sample(const intersection& p) const {
    // TODO: ����
    intersection isect;
    return isect;
}