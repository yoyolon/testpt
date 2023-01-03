//#include "Material.h"
//#include "Microfacet.h"
//#include "Shape.h"
//#include "Random.h"
//#include "Fresnel.h"
//
//// *** ÉâÉìÉoÅ[ÉgîΩéÀ ***
//Diffuse::Diffuse(Vec3 _albedo) : Material(MaterialType::Diffuse), albedo(_albedo) {}
//
//float Diffuse::sample_pdf(const Vec3& wo, const Vec3& wi) const {
//    return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
//}
//
//Vec3 Diffuse::f(const Vec3& wo, const Vec3& wi) const {
//    return albedo * invpi;
//}
//
//Vec3 Diffuse::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
//    wi = Random::cosine_hemisphere_sample();
//    pdf = sample_pdf(wo, wi);
//    auto brdf = f(wo, wi);
//    return brdf;
//}