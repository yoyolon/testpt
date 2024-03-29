#include "Random.h"
#include <algorithm>

/** 乱数生成クラス */
void Random::init() {
    //std::random_device rd;
    //mt.seed(rd()); 
    mt.seed(0); // シードを固定
}

float Random::uniform_float() {
    return uniform_float(0.f, 1.0f);
}

float Random::uniform_float(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(mt);
}

int Random::uniform_int(int min, int max) {
    std::uniform_int_distribution<> dist(min, max);
    return dist(mt);
}

Vec2 Random::uniform_disk_sample() {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto r = std::sqrt(u);
    auto phi = 2 * pi * v;
    auto x = std::cos(phi) * r;
    auto y = std::sin(phi) * r;
    return Vec2(x, y);
}

Vec2 Random::concentric_disk_sample() {
    float r, phi;
    auto u = 2 * Random::uniform_float() -1.0f;
    auto v = 2 * Random::uniform_float() -1.0f;
    if (u == 0 && v == 0) {
        return Vec2::zero;
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
    return Vec2(x, y);
}

Vec2 Random::uniform_triangle_sample() {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto sqrt_u = std::sqrt(u);
    auto x = 1 - sqrt_u;
    auto y = v * sqrt_u;
    return Vec2(x, y);
}

Vec3 Random::uniform_sphere_sample() {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto z = 1 - 2 * u;
    auto r = std::sqrt(std::max(1.0f - z*z, 0.f));
    auto phi = 2 * pi * v;
    auto x = std::cos(phi) * r;
    auto y = std::sin(phi) * r;
    return Vec3(x, y, z);
}

Vec3 Random::uniform_hemisphere_sample() {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto z = u;
    auto r = std::sqrt(std::max(1.0f - z*z, 0.f));
    auto phi = 2 * pi * v;
    auto x = std::cos(phi) * r;
    auto y = std::sin(phi) * r;
    return Vec3(x, y, z);
}

Vec3 Random::cosine_hemisphere_sample() {
    auto d = Random::concentric_disk_sample();
    auto x = d.get_x();
    auto y = d.get_y();
    auto z = std::sqrt(std::max(1.0f - x*x - y*y, 0.f));
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


/** 1D区分関数 */
Piecewise1D::Piecewise1D(const float* data, int _n)
    : f(data, data+_n), n(_n), cdf(_n + 1)
{
    // CDFを計算
    cdf[0] = 0;
    for (int i = 1; i < n + 1; i++) {
        cdf[i] = cdf[i-1] + f[i-1] / n;
    }
    integral_f = cdf[n];
    for (int i = 1; i < n + 1; i++) {
        cdf[i] /= integral_f;
    }
    std::sort(cdf.begin(), cdf.end());
}

float Piecewise1D::sample(float& pdf, int& index) const {
    // cdf[index] <= u < cdf[index+1]となるインデックスを探索
    auto u = Random::uniform_float();
    auto iter = std::lower_bound(cdf.begin(), cdf.end(), u); // 二分探索
    index = std::distance(cdf.begin(), iter) - 1; // cdf[index] <= uとするために-1する
    pdf = f[index] / integral_f;
    auto t = (u - cdf[index]) / (cdf[index + 1] - cdf[index]);
    return (index + t) / n;
}


/** 2D区分関数 */
Piecewise2D::Piecewise2D(const float* data, int _nu, int _nv)
    : nu(_nu), nv(_nv)
{
    // 条件付き確率密度関数p(u|v)を計算
    for (int v = 0; v < nv; v++) {
        conditional_pdf.push_back(std::make_unique<Piecewise1D>(&data[v * nu], nu));
    }
    // 周辺密度関数p(v)を計算
    std::vector<float> merginal_f;
    for (int v = 0; v < nv; v++) {
        merginal_f.push_back(conditional_pdf[v]->get_integral_f());
    }
    merginal_pdf = std::make_unique<Piecewise1D>(&merginal_f[0], nv);
}

Vec2 Piecewise2D::sample(float& pdf) const {
    float pdf_u = 0, pdf_v = 0;
    int index_u, index_v;
    float v = merginal_pdf->sample(pdf_v, index_v);
    float u = conditional_pdf[index_v]->sample(pdf_u, index_u);
    pdf = pdf_u * pdf_v;
    return Vec2(u, v);
}

float Piecewise2D::eval_pdf(const Vec2& uv) const {
    int index_u = std::clamp(int(uv[0] * nu), 0, nu - 1);
    int index_v = std::clamp(int(uv[1] * nv), 0, nv - 1);
    // p(u, v) = f(u,v) / \int \int f(u,v) dudv
    return conditional_pdf[index_v]->get_f(index_u) / merginal_pdf->get_integral_f();
}