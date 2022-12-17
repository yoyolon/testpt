#include "Sphere.h"
#include "Random.h"

// ***  ***
Sphere::Sphere(Vec3 c, float r, std::shared_ptr<Material> m)
    : center(c), radius(r), mat(m) {};

bool Sphere::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // ņûöŪĖŧĘŪD/4 = (b/2)^2 - a*cðp(bÍô)
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
    // ð·_îņĖXV
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
    // TODO: Ā
    return 0.0f;
}
intersection Sphere::sample(const intersection& ref) const {
    // TODO: Ā
    intersection isect;
    return isect;
}


// *** ~Õ ***
Disk::Disk(Vec3 c, float r, std::shared_ptr<Material> m)
    : center(c), radius(r), mat(m) {};

bool Disk::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // CÆxz―ĘĖð·_Š~āÉ éĐŧč
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
    // ð·_îņĖXV
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


// *** ~ ***
Cylinder::Cylinder(Vec3 c, float r, float h, std::shared_ptr<Material> m)
    : center(c), radius(r), height(h), mat(m) {};

bool Cylinder::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // ņûöŪĖŧĘŪD/4 = (b/2)^2 - a*cðp(bÍô)
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
    auto t = (-b - D) / (2 * a);
    if (t < t_min || t > t_max) {
        t = (-b + D) / (2 * a);
        if (t < t_min || t > t_max) {
            return false;
        }
    }
     // ð·_ĖyĀWŠ~ĖÍÍāĐŧč
    auto y = r.at(t).get_y();
    auto y_min = center.get_y();
    auto y_max = y_min + height;
    if (y <= y_min || y >= y_max) {
        return false;
    }
     // ð·_îņĖXV
    p.t = t;
    p.pos = r.at(t);
    p.normal = unit_vector(p.pos - center);
    p.mat = mat;
    return true;
}

float Cylinder::area() const {
    return 2 * pi * radius * height;
}

float Cylinder::sample_pdf(const intersection& ref, const Vec3& w) const {
    // TODO: Ā
    return 0.0f;
}
intersection Cylinder::sample(const intersection& p) const {
    // TODO: Ā
    intersection isect;
    return isect;
}