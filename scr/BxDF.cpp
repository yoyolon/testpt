#include "BxDF.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** ランバート反射 ***
LambertianReflection::LambertianReflection(const Vec3& _scale)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Diffuse)),
      scale(_scale)
{}

float LambertianReflection::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
}

Vec3 LambertianReflection::eval_f(const Vec3& wo, const Vec3& wi) const {
    return scale * invpi;
}

Vec3 LambertianReflection::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    wi = Random::cosine_hemisphere_sample();
    pdf = eval_pdf(wo, wi);
    auto brdf = eval_f(wo, wi);
    return brdf;
}



// *** 完全鏡面反射 ***
SpecularReflection::SpecularReflection(Vec3 _scale) 
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Specular)), 
      scale(_scale) 
{}

float SpecularReflection::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    return 1.0f;
}

Vec3 SpecularReflection::eval_f(const Vec3& wo, const Vec3& wi) const {
    return scale / dot(Vec3(0, 0, 1), unit_vector(wi));
}

Vec3 SpecularReflection::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    wi = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向(reflectより速い)
    pdf = eval_pdf(wo, wi);
    auto brdf = eval_f(wo, wi);
    return brdf;
}