#include "Vcavity.h"
#include "BxDF.h"
#include "Microfacet.h"
#include "Random.h"


Vcavity::Vcavity(float _alpha, NDFType _type) 
    : dist(nullptr)
{
    // •ª•z‚Åê‡•ª‚¯
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

float Vcavity::G(const Vec3& wo, const Vec3& wi, const Vec3& h) const {
    auto n_dot_h = std::abs(get_cos(h));
    auto n_dot_wo = std::abs(get_cos(wo));
    auto n_dot_wi = std::abs(get_cos(wi));
    auto wo_dot_h = std::abs(dot(wo, h));
    return std::min(1.0f, std::min(2.0f * n_dot_h * n_dot_wo / wo_dot_h, 
                                   2.0f * n_dot_h * n_dot_wo / wo_dot_h));
}

Vec3 Vcavity::sample_halfvector(const Vec3& wo) const {
    auto h = dist->sample_halfvector();
    return h;
}

float Vcavity::eval_pdf(const Vec3& h, const Vec3& wo) const {
    return dist->eval_pdf(h);
}