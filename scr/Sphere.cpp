#include "Sphere.h"
#include "Random.h"

// *** 球 ***
Sphere::Sphere(Vec3 c, float r, std::shared_ptr<Material> m)
    : center(c), radius(r), mat(m) {};

bool Sphere::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // 二次方程式の判別式D/4 = (b/2)^2 - a*cを利用(bは偶数)
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
    // 交差点情報の更新
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
    // TODO: 実装
    return 0.0f;
}
intersection Sphere::sample(const intersection& ref) const {
    // TODO: 実装
    intersection isect;
    return isect;
}


// *** 円盤 ***
Disk::Disk(Vec3 c, float r, std::shared_ptr<Material> m)
    : center(c), radius(r), mat(m) {};

bool Disk::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // レイとxz平面の交差点が円内部にあるか判定
    if (r.get_dir().get_y() == 0) {
        return false;
    }
    auto t = (center.get_y() - r.get_origin().get_y()) / r.get_dir().get_y();
    if (t < t_min || t > t_max) {
        return false;
    }
    auto pos = r.at(t); // xz平面との交差点
    // 円との交差判定
    auto dx = pos.get_x() - center.get_x();
    auto dz = pos.get_z() - center.get_z();
    auto dist2 = dx * dx + dz * dz;
    if (dist2 > radius * radius) {
        return false;
    }
    // 交差点情報の更新
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


// *** 円柱 ***
Cylinder::Cylinder(Vec3 c, float r, float h, std::shared_ptr<Material> m)
    : center(c), radius(r), height(h), mat(m) {};

bool Cylinder::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // 二次方程式の判別式D/4 = (b/2)^2 - a*cを利用(bは偶数)
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
     // 交差点のy座標が円柱の範囲内か判定
    auto y = r.at(t).get_y();
    auto y_min = center.get_y();
    auto y_max = y_min + height;
    if (y <= y_min || y >= y_max) {
        return false;
        t = (-b + d) / (2 * a); // 円柱の内側
        auto y = r.at(t).get_y();
        if (y <= y_min || y >= y_max) {
            return false;
        }
    }
     // 交差点情報の更新
    p.t = t;
    p.pos = r.at(t);
    auto diff = p.pos - center;
    p.normal = unit_vector(Vec3(diff.get_x(), 0.0f, diff.get_z()));
    p.mat = mat;
    return true;
}

float Cylinder::area() const {
    return 2 * pi * radius * height;
}

float Cylinder::sample_pdf(const intersection& ref, const Vec3& w) const {
    // TODO: 実装
    return 0.0f;
}
intersection Cylinder::sample(const intersection& p) const {
    // TODO: 実装
    intersection isect;
    return isect;
}