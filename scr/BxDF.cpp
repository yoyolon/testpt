#include "BxDF.h"
#include "Fresnel.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"


// *** Lambert反射 ***

LambertianReflection::LambertianReflection(const Vec3& _scale)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Diffuse)),
    scale(_scale)
{}

float LambertianReflection::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
}

Vec3 LambertianReflection::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return scale * invpi;
}

Vec3 LambertianReflection::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    wi = Random::cosine_hemisphere_sample();
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}


// *** 完全鏡面反射 ***

SpecularReflection::SpecularReflection(Vec3 _scale, std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Specular)),
    scale(_scale),
    fres(_fres)
{}

SpecularReflection::SpecularReflection(Vec3 _scale, float _n_inside, float _n_outside)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Specular)),
    scale(_scale)
{
    fres = std::make_shared<FresnelDielectric>(_n_inside, _n_outside);
}

float SpecularReflection::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return 0.f; // デルタ関数のため
}

Vec3 SpecularReflection::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return Vec3::zero; // デルタ関数のため
}

Vec3 SpecularReflection::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    // 正反射方向を明示的に重点的サンプリングするのでevalメソッドは使わない
    wi = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向
    pdf = 1.0f;
    auto cos_term = get_cos(wi); // 完全鏡面なのでcos(wi)とcos(wos)は等しい
    auto F = fres->eval(cos_term, p);
    auto brdf = F / std::abs(cos_term);
    return scale * brdf;
}


// *** 完全鏡面透過 ***

SpecularTransmission::SpecularTransmission(Vec3 _scale, float _n_inside, float _n_outside,
    std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Transmission | (uint8_t)BxDFType::Specular)),
    scale(_scale),
    n_inside(_n_inside),
    n_outside(_n_outside),
    fres(_fres)
{
    if (fres == nullptr) {
        fres = std::make_shared<FresnelDielectric>(n_inside, n_outside);
    }
}

float SpecularTransmission::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return 0.f; // デルタ関数のため
}

Vec3 SpecularTransmission::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    return Vec3::zero; // デルタ関数のため
}

Vec3 SpecularTransmission::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    // 透過方向を明示的に重点的サンプリングする
    // note: woを入射方向としたBTDFを計算してwiが入射方向になるように変換
    auto eta = p.is_front ? n_outside / n_inside : n_inside / n_outside; // 相対屈折
    wi = refract(wo, Vec3(0.f, 0.f, 1.0f), eta); // 屈折方向
    // 全反射の場合(note: refract()は全反射の場合ゼロベクトルを返すが修正すべき)
    if (is_zero(wi)) {
        pdf = 0.f;
        return Vec3::zero;
    }
    pdf = 1.0f;
    auto cos_term = get_cos(wo);
    auto F = Vec3::one - fres->eval(cos_term, p); // フレネル透過率
    // note: eta^2の乗算は屈折率の違いによるエネルギー密度の変化を考慮
    auto btdf = eta * eta * F / std::abs(cos_term);
    return scale * btdf;
}


// *** Phong鏡面反射 ***

PhongReflection::PhongReflection(Vec3 _scale, float _shine)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)),
    scale(_scale),
    shine(_shine)
{}

float PhongReflection::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    auto specular = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向
    if (!is_same_hemisphere(specular, wi)) {
        return 0.f;
    }
    auto lobe = std::pow(dot(specular, wi), shine); // Phongローブ
    return (shine + 1.0f) / 2 * invpi * std::max(lobe, epsilon);
}

Vec3 PhongReflection::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    auto specular = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // 正反射方向
    if (!is_same_hemisphere(specular, wi)) {
        return Vec3::zero;
    }
    auto lobe = std::pow(dot(specular, wi), shine); // Phongローブ
    auto brdf = (shine + 2.0f) / (2 * pi) * lobe;
    return scale * brdf;
}

Vec3 PhongReflection::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    wi = phong_sample(shine);
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}

Vec3 PhongReflection::phong_sample(float shine) const {
    auto u = Random::uniform_float();
    auto v = Random::uniform_float();
    auto z = std::pow(u, 1 / (shine + 1.0f));
    auto r = std::sqrt(std::max(1.0f - z * z, 0.f));
    auto phi = 2 * pi * v;
    auto x = std::cos(phi) * r;
    auto y = std::sin(phi) * r;
    return Vec3(x, y, z);
}

// *** マイクロファセットBRDF ***

MicrofacetReflection::MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist,
    std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)),
    scale(_scale),
    dist(_dist),
    fres(_fres)
{}

MicrofacetReflection::MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist,
    float _n_inside, float _n_outside)
    : BxDF(BxDFType((uint8_t)BxDFType::Reflection | (uint8_t)BxDFType::Glossy)),
    scale(_scale),
    dist(_dist)
{
    fres = std::make_shared<FresnelDielectric>(_n_inside, _n_outside);
}

float MicrofacetReflection::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    if (!is_same_hemisphere(wo, wi)) {
        return 0.f;
    }
    auto h = unit_vector(wo + wi);
    // ハーブベクトルの確率密度をサンプリング方向の確率密度に変換
    return dist->eval_pdf(h, wo) / (4 * dot(wo, h));
}

Vec3 MicrofacetReflection::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    if (!is_same_hemisphere(wo, wi)) {
        return Vec3::zero;
    }
    float cos_wo = std::abs(get_cos(wo));
    float cos_wi = std::abs(get_cos(wi));
    if (cos_wo == 0 || cos_wi == 0) {
        return Vec3::zero;
    }
    // ハーフ方向の取得
    auto h = unit_vector(wo + wi);
    if (is_zero(h)) {
        return Vec3::zero;
    }
    float D = dist->D(h);
    float G = dist->G(wo, wi);
    Vec3 F = fres->eval(dot(wo, h), p);
    auto brdf = (D * G * F) / (4 * cos_wo * cos_wi);
    return scale * brdf;
}

Vec3 MicrofacetReflection::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    // ハーブベクトルをサンプリングして入射方向に変換する
    Vec3 h = dist->sample_halfvector(wo);
    wi = unit_vector(reflect(wo, h)); // reflect()では正規化しないので明示的に正規化
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}


// *** マイクロファセットBTDF ***

MicrofacetTransmission::MicrofacetTransmission(Vec3 _scale, std::shared_ptr<NDF> _dist, float _n_inside,
    float _n_outside, std::shared_ptr<Fresnel> _fres)
    : BxDF(BxDFType((uint8_t)BxDFType::Transmission | (uint8_t)BxDFType::Glossy)),
    scale(_scale),
    dist(_dist),
    n_inside(_n_inside),
    n_outside(_n_outside),
    fres(_fres)
{
    if (fres == nullptr) {
        fres = std::make_shared<FresnelDielectric>(n_inside, n_outside);
    }
}

float MicrofacetTransmission::eval_pdf(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    if (is_same_hemisphere(wo, wi)) {
        return 0.f;
    }
    auto eta = p.is_front ? n_outside / n_inside : n_inside / n_outside; // 相対屈折
    auto h = unit_vector(eta * wo + wi);
    // 全反射の場合
    if (is_zero(refract(wo, h, eta))) {
        return 1.0f;
    }
    float cos_hi = std::abs(dot(wi, h));
    float cos_ho = std::abs(dot(wo, h));
    float cos_factor = eta * eta * cos_hi;
    float eta_factor = 1 / (cos_ho + eta * cos_hi);
    // ハーブベクトルの確率密度をサンプリング方向の確率密度に変換
    return dist->eval_pdf(h, wo) * cos_factor * eta_factor * eta_factor;
}

Vec3 MicrofacetTransmission::eval_f(const Vec3& wo, const Vec3& wi,
    const intersection& p) const {
    // note: woを入射方向としたBTDFを計算してwiが入射方向になるように変換
    if (is_same_hemisphere(wo, wi)) {
        return Vec3::zero;
    }
    float cos_wo = std::abs(get_cos(wo));
    float cos_wi = std::abs(get_cos(wi));
    if (cos_wo == 0 || cos_wi == 0) {
        return Vec3::zero;
    }
    auto eta = p.is_front ? n_outside / n_inside : n_inside / n_outside; // 相対屈折
    // ハーフ方向の取得
    auto h = unit_vector(eta * wo + wi);
    if (is_zero(h)) {
        return Vec3::zero;
    }
    // 全反射の場合
    if (is_zero(refract(wo, h, eta))) {
        return Vec3::zero;
    }
    float cos_hi = std::abs(dot(wi, h));
    float cos_ho = std::abs(dot(wo, h));
    float D = dist->D(h);
    float G = dist->G(wo, wi);
    Vec3 F = Vec3::one - fres->eval(cos_ho, p); // 透過率
    float cos_factor = cos_hi * cos_ho / cos_wi / cos_wo;
    float eta_factor = 1 / (cos_ho + eta * cos_hi);
    auto btdf = cos_factor * (D * G * F) * eta_factor * eta_factor;
    return scale * btdf;
}

Vec3 MicrofacetTransmission::sample_f(const Vec3& wo, const intersection& p,
    Vec3& wi, float& pdf) const {
    Vec3 h = dist->sample_halfvector(wo);
    auto eta = p.is_front ? n_outside / n_inside : n_inside / n_outside; // 相対屈折
    wi = unit_vector(refract(wo, h, eta)); // refract()では正規化しないので明示的に正規化
    // 全反射の場合
    if (is_zero(wi)) {
        pdf = 0.f;
        return Vec3::zero;
    }
    pdf = eval_pdf(wo, wi, p);
    return eval_f(wo, wi, p);
}