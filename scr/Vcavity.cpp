#include "Vcabvty.h"
#include "Microfacet.h"
#include "Random.h"

/**
* @brief 法線選択の確率密度を評価する関数
* @param[in] h  :ハーフ方向ベクトル1
* @param[in] hd :ハーフ方向ベクトル2
* @param[in] wo :出射方向ベクトル
* @return float :サンプリング確率密度
*/
float eval_dual_normal_pdf(const Vec3& h, const Vec3& hd, const Vec3& wo) {
    auto hd_term = std::max(0.0f, dot(hd, wo));
    auto h_term = std::max(0.0f, dot(h, wo));
    if (hd_term == 0 && h_term == 0) {
        return 0.0f;
    }
    return hd_term / (hd_term + h_term);
}

Vcavity::Vcavity(float _alpha, NDFType _type) 
    : dist(nullptr)
{
    // 分布で場合分け
    if (_type == NDFType::Beckmann) {
        dist = std::make_shared<Beckmann>(_alpha);
    }
    else if (_type == NDFType::GGX) {
        dist = std::make_shared<GGX>(_alpha);
    }
}

float Vcavity::D(const Vec3& h) const{
    return dist->D(h);
}

float Vcavity::G1(const Vec3& w, const Vec3& h, const Vec3& n) const {
    auto geo_term = dot(w, n);
    auto micro_term = dot(w, h);
    if (micro_term == 0 || geo_term == 0 || micro_term / geo_term < 0) {
        return 0.0f;
    }
    auto amount = std::abs(dot(h, n)) * std::abs(geo_term) / std::abs(micro_term);
    return std::min(1.0f, 2 * amount);
}

float Vcavity::G(const Vec3& wo, const Vec3& wi, const Vec3& h, const Vec3& n) const {
    auto G1_wo = G1(wo, h, n);
    auto G1_wi = G1(wi, h, n);
    if (dot(wo, n) > 0) {
        return std::min(G1_wo, G1_wi);
    }
    return std::max(G1_wo + G1_wi - 1.0f, 0.0f);
}

Vec3 Vcavity::sample_halfvector(const Vec3& wo) const {
    auto h = dist->sample_halfvector();
    auto hd = Vec3(-h.get_x(), -h.get_y(), h.get_z()); // 二つ目の法線
    auto u = eval_dual_normal_pdf(h, hd, wo);
    if (Random::uniform_float() < u) {
        h = hd;
    }
    return h;
}

float Vcavity::eval_pdf(const Vec3& h, const Vec3& wo) const {
    auto hd = Vec3(-h.get_x(), -h.get_y(), h.get_z()); // 二つ目の法線
    auto u = eval_dual_normal_pdf(h, hd, wo);
    return (1 - u) * dist->eval_pdf(h);
}