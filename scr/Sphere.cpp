#include "Sphere.h"
#include "Random.h"
#include "ONB.h"

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
    p.is_front = is_front(r, p.normal);
    p.mat = mat;
    return true;
};

float Sphere::area() const {
    return 4 * pi * radius * radius;
}

intersection Sphere::sample(const intersection& ref) const {
    // 球から一様サンプリング(没)
    //intersection isect;
    //isect.normal = Random::uniform_sphere_sample();
    //isect.pos = radius * isect.normal + center;
    //return isect;
    
    // 球の可視領域を考慮してサンプリング
    // ローカルな半球方向からサンプリング
    auto z = unit_vector(ref.pos - center);
    auto sampling_coord = ONB(z);
    auto sampling_local_pos = Random::uniform_hemisphere_sample();
    // グローバルな半球に変換
    intersection isect;
    isect.normal = sampling_coord.to_world(sampling_local_pos);
    isect.pos = radius * isect.normal + center;
    return isect;
}

float Sphere::eval_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // refからジオメトリへのレイ
    intersection isect;                    // ジオメトリの交差点
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.0f;
    }
    // 面の裏側をサンプルした場合pdfをゼロにする
    if (dot(isect.normal, -w) < 0) {
        return 0.0f;
    }
    auto d = ref.pos - isect.pos;
    return d.length2() / (std::abs(dot(isect.normal, -w)) * area());
}

// *** 円盤 ***
const Vec3 Disk::normal = Vec3(0.0f, -1.0f, 0.0f);

Disk::Disk(Vec3 c, float r, std::shared_ptr<Material> m, bool is_flip)
    : center(c), radius(r), mat(m), is_flip_normal(is_flip) {};

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
    p.normal = normal;
    if (is_flip_normal) p.normal *= -1; // 法線の反転
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
    if (is_flip_normal) isect.normal *= -1; // 法線の反転
    isect.pos = Vec3(x, y, z);
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
    p.is_front = is_front(r, p.normal);
    p.mat = mat;
    return true;
}

float Cylinder::area() const {
    return 2 * pi * radius * height;
}

intersection Cylinder::sample(const intersection& p) const {
    // NOTE: y軸正の方向が上向き
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