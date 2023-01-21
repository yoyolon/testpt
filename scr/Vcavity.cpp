#include "Vcabvty.h"
#include "Microfacet.h"

Vcavity::Vcavity(float _alpha, NDFType _type) 
    : ndf(nullptr)
{
    // •ª•z‚Åê‡•ª‚¯
    if (_type == NDFType::Beckmann) {
        ndf = std::make_shared<Beckmann>(_alpha);
    }
    else if (_type == NDFType::GGX) {
        ndf = std::make_shared<GGX>(_alpha);
    }
}

float Vcavity::D(const Vec3& h) const{
    return ndf->D(h);
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

Vec3 Vcavity::sample_halfvector() const {
    ndf->sample_halfvector();
}

float Vcavity::eval_pdf(const Vec3& h) const {
    ndf->eval_pdf(h);
}