#include "Material.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** マテリアル ***

Vec3 Material::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    // 一つのBxDFから方向をサンプリングして寄与はすべてのBxDFを考慮する
    auto f = Vec3::zero;
    auto num_bxdfs = bxdf_list.size();
    if (num_bxdfs == 0) {
        return f;
    }
    // BxDFをランダムに一つ選択
    auto bxdf_index = Random::uniform_int(0, num_bxdfs - 1);
    const auto& bxdf_main = bxdf_list[bxdf_index]; // サンプリングするBxDF
    // BxDFで方向をサンプリング
    f = bxdf_main->sample_f(wo, p, wi, pdf);
    // すべてのBxDFを考慮してBSDFとpdfを計算
    f = eval_f(wo, wi);
    pdf = eval_pdf(wo, wi);
    return f;
}

Vec3 Material::eval_f(const Vec3& wo, const Vec3& wi) const {
    // すべてのBxDFの総和を計算
    // TODO: 反射と透過の区別(内積を利用)
    //       あるいはBxDFを半球でなく全球範囲で定義する
    auto f = Vec3::zero;
    auto num_bxdfs = bxdf_list.size();
    if (num_bxdfs == 0) {
        return f;
    }
    for (const auto& bxdf : bxdf_list) {
        f += bxdf->eval_f(wo, wi);
    }
    return f;
}

float Material::eval_pdf(const Vec3& wo, const Vec3& wi) const {
    // すべてのBxDFのpdfの平均を計算
    auto num_bxdfs = bxdf_list.size();
    if (num_bxdfs == 0) {
        return 0.0f;
    }
    auto pdf = 0.0f;
    for (const auto& bxdf : bxdf_list) {
        pdf += bxdf->eval_pdf(wo, wi);
    }
    return pdf / num_bxdfs;
}


// *** 拡散反射マテリアル ***
Diffuse::Diffuse(Vec3 _albedo) 
    : Material(MaterialType::Diffuse), 
      albedo(_albedo) 
{
    // ランバートBRDFを追加
    // TODO: オレンナイヤルモデルを実装したら粗さを考慮
    add(std::make_shared<LambertianReflection>(albedo));
}


// *** 鏡マテリアル ***
Mirror::Mirror(Vec3 _albedo)
    : Material(MaterialType::Specular),
    albedo(_albedo)
{
    auto fres = std::make_shared<FresnelConstant>(Vec3::one);
    add(std::make_shared<SpecularReflection>(albedo, fres));
}


// *** 金属マテリアル ***
Metal::Metal(Vec3 _albedo, Vec3 _fr, float _alpha)
    : Material(MaterialType::Glossy),
      albedo(_albedo),
      fr(_fr),
      alpha(_alpha)
{
    auto fres = std::make_shared<FresnelSchlick>(fr);
    if (alpha == 0) {
        set_type(MaterialType::Specular);
        add(std::make_shared<SpecularReflection>(albedo, fres));
    }
    else {
        auto dist = std::make_shared<GGX>(alpha);
        add(std::make_shared<MicrofacetReflection>(albedo, dist, fres));
    }
}


// *** Phongマテリアル ***
Phong::Phong(Vec3 _albedo, Vec3 _kd, Vec3 _ks, float _shine)
    : Material(MaterialType::Glossy),
      albedo(_albedo),
      kd(_kd),
      ks(_ks),
      shine(_shine)
{
    if (!is_zero(kd)) {
        add(std::make_shared<LambertianReflection>(albedo * kd));
    }
    if (!is_zero(ks)) {
        add(std::make_shared<PhongReflection>(albedo * ks, shine));
    }
}