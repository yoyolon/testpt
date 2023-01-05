#include "Material.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** マテリアル ***

Vec3 Material::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf,
                        BxDFType& sampled_type, BxDFType acceptable_type) const {
    // 一つのBxDFから方向をサンプリングして寄与はすべてのBxDFを考慮する
    int num_acceptable_bxdfs = 0;
    // 許容可能なBxDFの要素数をカウント
    for (const auto& bxdf : bxdf_list) {
        if (bxdf->is_same_type(acceptable_type))
            num_acceptable_bxdfs++;
    }
    if (num_acceptable_bxdfs == 0) {
        return Vec3::zero;
    }
    // 許容可能なBxDFからランダムにサンプリング
    // note: bxdf_listに許容不可能なBxDFが含まれている場合があるのでforループで確認
    auto bxdf_count = Random::uniform_int(0, num_acceptable_bxdfs - 1); // 許容可能なBxDFの順番
    auto bxdf_list_index = 0; // bxdf_list上でのサンプリングBxDFのインデックス
    for (const auto& bxdf : bxdf_list) {
        if (bxdf->is_same_type(acceptable_type)) {
            if (bxdf_count == 0) break;
            bxdf_count--;
        }
        bxdf_list_index++;
    }
    const auto& sampled_bxdf = bxdf_list[bxdf_list_index];
    sampled_bxdf->sample_f(wo, p, wi, pdf);
    sampled_type = sampled_bxdf->get_type();
    // すべての許容可能なBxDFを考慮してBSDFとpdfを計算
    auto f = eval_f(wo, wi, acceptable_type);
    pdf = eval_pdf(wo, wi, acceptable_type);
    return f;
}

Vec3 Material::eval_f(const Vec3& wo, const Vec3& wi, BxDFType acceptable_type) const {
    // すべてのBxDFの総和を計算
    // TODO: 反射と透過の区別(内積を利用)
    //       あるいはBxDFを半球でなく全球範囲で定義する
    auto f = Vec3::zero;
    bool is_reflect = wo.get_z() * wi.get_z() > 0; // 方向が反射はどうか判定
    for (const auto& bxdf : bxdf_list) {
        if (bxdf->is_same_type(acceptable_type)) {
            if ((is_reflect && bxdf->is_reflection()) ||
                (!is_reflect && bxdf->is_transmission())) {
                f += bxdf->eval_f(wo, wi);
            }
        }
    }
    return f;
}

float Material::eval_pdf(const Vec3& wo, const Vec3& wi, BxDFType acceptable_type) const {
    auto pdf = 0.0f;
    int num_acceptable_bxdfs = 0;
    // 許容可能なBxDFの要素数をカウント
    for (const auto& bxdf : bxdf_list) {
        if (bxdf->is_same_type(acceptable_type)) {
            num_acceptable_bxdfs++;
            pdf += bxdf->eval_pdf(wo, wi);
        }
    }
    if (num_acceptable_bxdfs == 0) {
        return 0.0f;
    }
    return pdf / num_acceptable_bxdfs;
}


// *** 拡散反射マテリアル ***
Diffuse::Diffuse(Vec3 _base) 
    : Material(MaterialType::Diffuse), 
      base(_base) 
{
    // ランバートBRDFを追加
    // TODO: オレンナイヤルモデルを実装したら粗さを考慮
    add(std::make_shared<LambertianReflection>(base));
}


// *** 拡散透過マテリアル ***
DiffusePlastic::DiffusePlastic(Vec3 _base, Vec3 _r, Vec3 _t)
    : Material(MaterialType::Diffuse),
      base(_base),
      r(_r),
      t(_t)
{
    // ランバートBRDFを追加
    if (!is_zero(r)) {
        add(std::make_shared<LambertianReflection>(base * r));
    }
    if (!is_zero(t)) {
        add(std::make_shared<LambertianTransmission>(base * t));
    }
}

// *** 鏡マテリアル ***
Mirror::Mirror(Vec3 _base)
    : Material(MaterialType::Specular),
    base(_base)
{
    auto fres = std::make_shared<FresnelConstant>(Vec3::one);
    add(std::make_shared<SpecularReflection>(base, fres));
}


// *** 金属マテリアル ***
Metal::Metal(Vec3 _base, Vec3 _fr, float _alpha)
    : Material(MaterialType::Glossy),
      base(_base),
      fr(_fr),
      alpha(_alpha)
{
    auto fres = std::make_shared<FresnelSchlick>(fr);
    if (alpha == 0) {
        set_type(MaterialType::Specular);
        add(std::make_shared<SpecularReflection>(base, fres));
    }
    else {
        auto dist = std::make_shared<GGX>(alpha);
        add(std::make_shared<MicrofacetReflection>(base, dist, fres));
    }
}


// *** Phongマテリアル ***
Phong::Phong(Vec3 _base, Vec3 _kd, Vec3 _ks, float _shine)
    : Material(MaterialType::Glossy),
      base(_base),
      kd(_kd),
      ks(_ks),
      shine(_shine)
{
    if (!is_zero(kd)) {
        add(std::make_shared<LambertianReflection>(base * kd));
    }
    if (!is_zero(ks)) {
        add(std::make_shared<PhongReflection>(base * ks, shine));
    }
}