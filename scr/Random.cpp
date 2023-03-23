#include "Random.h"

void Random::init() {
    //std::random_device rd;
    //mt.seed(rd()); 
    mt.seed(1); // シードを固定
}


float Random::uniform_float() {
    return uniform_float(0.0f, 1.0f);
}


float Random::uniform_float(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(mt);
}


int Random::uniform_int(int min, int max) {
    std::uniform_int_distribution<> dist(min, max);
    return dist(mt);
}


Vec3 Random::uniform_disk_sample() {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto r = std::sqrt(u);
    auto phi = 2 * pi * v;
    auto x = std::cos(phi) * r;
    auto y = std::cos(phi) * r;
    return Vec3(x, y, 0.0f);
}


Vec3 Random::concentric_disk_sample() {
    float r, phi;
    auto u = 2 * Random::uniform_float() -1.0f;
    auto v = 2 * Random::uniform_float() -1.0f;
    if (u == 0 && v == 0) {
        return Vec3::zero;
    }
    if (u * u > v * v) {
        r = u;
        phi = (pi / 4) * (v / u);
    }
    else {
        r = v;
        phi = pi / 2 - (pi / 4) * (u / v);
    }
    auto x = std::cos(phi) * r;
    auto y = std::sin(phi) * r;
    return Vec3(x, y, 0.0f);
}


Vec3 Random::uniform_triangle_sample() {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto sqrt_u = std::sqrt(u);
    auto x = 1 - sqrt_u;
    auto y = v * sqrt_u;
    return Vec3(x, y, 0.0f);
}


Vec3 Random::uniform_sphere_sample() {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto z = 1 - 2 * u;
    auto r = std::sqrt(std::max(1.0f - z*z, 0.0f));
    auto phi = 2 * pi * v;
    auto x = std::cos(phi) * r;
    auto y = std::sin(phi) * r;
    return Vec3(x, y, z);
}


Vec3 Random::uniform_hemisphere_sample() {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto z = u;
    auto r = std::sqrt(std::max(1.0f - z*z, 0.0f));
    auto phi = 2 * pi * v;
    auto x = std::cos(phi) * r;
    auto y = std::sin(phi) * r;
    return Vec3(x, y, z);
}


Vec3 Random::cosine_hemisphere_sample() {
    auto d = Random::concentric_disk_sample();
    auto x = d.get_x();
    auto y = d.get_y();
    auto z = std::sqrt(std::max(1.0f - x*x - y*y, 0.0f));
    return Vec3(x, y, z);
}


Vec3 Random::phong_sample(float shine) {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto z = std::pow(u, 1/(shine + 1.0f));
    auto r = std::sqrt(std::max(1.0f - z*z, 0.0f));
    auto phi = 2 * pi * v;
    auto x = std::cos(phi) * r;
    auto y = std::sin(phi) * r;
    return Vec3(x, y, z);
}


Vec3 Random::ggx_sample(float alpha) {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto tan2_theta = alpha * alpha * u / (1.0f - u); // atan2は遅いので未使用
    auto cos2_theta = 1 / (1 + tan2_theta);
    auto sin2_theta = 1 - cos2_theta;
    auto sin_theta = std::sqrt(std::max(sin2_theta, 0.0f));
    auto phi = 2 * pi * v;
    auto z = std::sqrt(std::max(cos2_theta, 0.0f));
    auto x = std::cos(phi) * sin_theta;
    auto y = std::sin(phi) * sin_theta;
    return Vec3(x, y, z);
}


Vec3 Random::beckmann_sample(float alpha) {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto logs = std::log(1.0f - u);
    if (std::isinf(logs)) logs = 0.0f;
    auto tan2_theta = -alpha * alpha * logs;
    auto cos2_theta = 1 / (1 + tan2_theta);
    auto sin2_theta = 1 - cos2_theta;
    auto sin_theta = std::sqrt(std::max(sin2_theta, 0.0f));
    auto phi = 2 * pi * v;
    auto z = std::sqrt(std::max(cos2_theta, 0.0f));
    auto x = std::cos(phi) * sin_theta;
    auto y = std::sin(phi) * sin_theta;
    return Vec3(x, y, z);
}


float Random::balance_heuristic(int n1, float pdf1, int n2, float pdf2) {
    return (n1 * pdf1) / (n1 * pdf1 + n2 * pdf2);
}


float Random::power_heuristic(int n1, float pdf1, int n2, float pdf2, float beta) {
    float e1 = std::powf(n1 * pdf1, beta);
    float e2 = std::powf(n2 * pdf2, beta);
    return e1 / (e1 + e2);

}