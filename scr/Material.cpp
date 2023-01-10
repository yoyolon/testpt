#include "Material.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** マテリアル ***

Vec3 Material::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf,
                        BxDFType& sampled_type, BxDFType acceptable_type) const {
    // 許容可能なBxDFの要素数をカウント
    int num_acceptable_bxdfs = 0;
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
    float sampled_pdf;
    auto sampled_f = sampled_bxdf->sample_f(wo, p, wi, sampled_pdf);
    sampled_type = sampled_bxdf->get_type();
    // すべての許容可能なBxDFを考慮してBSDFとpdfを計算
    auto f = eval_f(wo, wi, p, acceptable_type);
    pdf = eval_pdf(wo, wi, p, acceptable_type);
    // サンプリングBxDFがスペキュラの場合evalで0寄与となるので補正
    // TODO: あくまで救済処置あとで修正
    if (sampled_bxdf->is_specular()) {
        f += sampled_f;
        pdf += sampled_pdf / num_acceptable_bxdfs;
    }
    return f;
}

Vec3 Material::eval_f(const Vec3& wo, const Vec3& wi, const intersection& p, 
                      BxDFType acceptable_type) const {
    // すべてのBxDFの総和を計算
    auto f = Vec3::zero;
    bool is_reflect = wo.get_z() * wi.get_z() > 0; // 方向が反射はどうか判定
    for (const auto& bxdf : bxdf_list) {
        if (bxdf->is_same_type(acceptable_type)) {
            if ((is_reflect && bxdf->is_reflection()) ||
                (!is_reflect && bxdf->is_transmission())) {
                f += bxdf->eval_f(wo, wi, p);
            }
        }
    }
    return f;
}

float Material::eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p,
                         BxDFType acceptable_type) const {
    auto pdf = 0.0f;
    int num_acceptable_bxdfs = 0;
    // 許容可能なBxDFの要素数をカウント
    for (const auto& bxdf : bxdf_list) {
        if (bxdf->is_same_type(acceptable_type)) {
            num_acceptable_bxdfs++;
            pdf += bxdf->eval_pdf(wo, wi, p);
        }
    }
    if (num_acceptable_bxdfs == 0) {
        return 0.0f;
    }
    return pdf / num_acceptable_bxdfs;
}


// *** 拡散反射マテリアル ***
Diffuse::Diffuse(Vec3 _base) 
    : Material(), 
      base(_base) 
{
    // ランバートBRDFを追加
    // TODO: オレンナイヤルモデルを実装したら粗さを考慮
    add(std::make_shared<LambertianReflection>(base));
}


// *** 拡散透過マテリアル ***
// TODO: [要検証]拡散透過って何？？？
DiffusePlastic::DiffusePlastic(Vec3 _base, Vec3 _r, Vec3 _t)
    : Material(),
      base(_base),
      r(_r),
      t(_t)
{
    if (!is_zero(r)) {
        add(std::make_shared<LambertianReflection>(base * r));
    }
    if (!is_zero(t)) {
        add(std::make_shared<LambertianTransmission>(base * t));
    }
}


// *** 鏡マテリアル ***
Mirror::Mirror(Vec3 _base)
    : Material(),
    base(_base)
{
    auto fres = std::make_shared<FresnelConstant>(Vec3::one);
    add(std::make_shared<SpecularReflection>(base, fres));
}


// *** ガラスマテリアル ***
Glass::Glass(Vec3 _base, Vec3 _r, Vec3 _t, float _n, float _alpha)
    : Material(),
      base(_base),
      r(_r),
      t(_t),
      n(_n),
      alpha(_alpha)
{   
    if (alpha != 0) {
        auto dist = std::make_shared<GGX>(alpha);
        if (!is_zero(r)) {
            add(std::make_shared<MicrofacetReflection>(base, dist, n));
        }
        if (!is_zero(t)) {
            add(std::make_shared<MicrofacetTransmission>(base, dist, n));
        }

    }
    else { // 粗さが0の場合完全鏡面
        if (!is_zero(r)) {
            add(std::make_shared<SpecularReflection>(base * r, n));
        }
        if (!is_zero(t)) {
            add(std::make_shared<SpecularTransmission>(base * t, n));
        }
    }
}


// *** 金属マテリアル ***
Metal::Metal(Vec3 _base, Vec3 _fr, float _alpha)
    : Material(),
      base(_base),
      fr(_fr),
      alpha(_alpha)
{
    auto fres = std::make_shared<FresnelSchlick>(fr);
    if (alpha == 0) {
        add(std::make_shared<SpecularReflection>(base, fres));
    }
    else {
        auto dist = std::make_shared<GGX>(alpha);
        add(std::make_shared<MicrofacetReflection>(base, dist, fres));
    }
}


// *** プラスチックマテリアル ***
Plastic::Plastic(Vec3 _base, Vec3 _kd, Vec3 _ks, float _alpha)
    : Material(),
    base(_base),
    kd(_kd),
    ks(_ks),
    alpha(_alpha)
{
    auto fres = std::make_shared<FresnelSchlick>(ks);
    if (alpha == 0) {
        add(std::make_shared<SpecularReflection>(base, fres));
    }
    else {
        auto dist = std::make_shared<GGX>(alpha);
        add(std::make_shared<MicrofacetReflection>(base, dist, fres));
    }
    add(std::make_shared<LambertianReflection>(base * kd));
}


// *** Phongマテリアル ***
Phong::Phong(Vec3 _base, Vec3 _kd, Vec3 _ks, float _shine)
    : Material(),
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