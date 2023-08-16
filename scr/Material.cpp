#include "Material.h"
#include "Fresnel.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"


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
    // NOTE: bxdf_listに許容不可能なBxDFが含まれている場合があるのでforループで確認
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
    bool is_reflect = wo.get_z() * wi.get_z() > 0; // 出射方向が反射方向ならtrue
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
    auto pdf = 0.f;
    int num_acceptable_bxdfs = 0;
    // 許容可能なBxDFの要素数をカウント
    for (const auto& bxdf : bxdf_list) {
        if (bxdf->is_same_type(acceptable_type)) {
            num_acceptable_bxdfs++;
            pdf += bxdf->eval_pdf(wo, wi, p);
        }
    }
    if (num_acceptable_bxdfs == 0) {
        return 0.f;
    }
    return pdf / num_acceptable_bxdfs;
}

bool Material::is_perfect_specular() const {
    bool is_specular = true;
    for (const auto& bxdf : bxdf_list) {
        if (!bxdf->is_specular()) {
            return false;
        }
    }
    return true;
}


// *** 拡散反射マテリアル ***
Diffuse::Diffuse(Vec3 _base) 
    :  
      base(_base) 
{
    // ランバートBRDFを追加
    // TODO: オレンナイヤルモデルを実装したら粗さを考慮
    add(std::make_shared<LambertianReflection>(base));
}


// *** 鏡マテリアル ***
Mirror::Mirror(Vec3 _base)
    : 
    base(_base)
{
    auto fres = std::make_shared<FresnelConstant>(Vec3::one);
    //auto fres = std::make_shared<FresnelLUT>("asset/LUT.csv"); // LUTテスト
    add(std::make_shared<SpecularReflection>(base, fres));
}


// *** ガラスマテリアル ***
Glass::Glass(Vec3 _base, Vec3 _r, Vec3 _t, float _n, float _alpha, 
    bool is_efficient_sampling)
    : 
      base(_base),
      r(_r),
      t(_t),
      n(_n),
      alpha(_alpha)
{   
    if (alpha != 0) {
        auto dist = std::make_shared<Beckmann>(alpha);
        if (!is_zero(r)) {
            add(std::make_shared<MicrofacetReflection>(base, dist, n));
        }
        if (!is_zero(t)) {
            add(std::make_shared<MicrofacetTransmission>(base, dist, n));
        }

    }
    else { // 粗さが0の場合完全鏡面
        if (is_efficient_sampling) {
            // フレネル式に基づく効率的なサンプリング
            //add(std::make_shared<SpecularFresnel>(base * r, base * t, n));
        }
        else {
            if (!is_zero(r)) {
                add(std::make_shared<SpecularReflection>(base * r, n));
            }
            if (!is_zero(t)) {
                add(std::make_shared<SpecularTransmission>(base * t, n));
            }
        }
    }
}


// *** 金属マテリアル ***
Metal::Metal(Vec3 _base, Vec3 _fr, float _alpha)
    : 
      base(_base),
      fr(_fr),
      alpha(_alpha)
{
    auto fres = std::make_shared<FresnelSchlick>(fr);
    //auto fres = std::make_shared<FresnelConstant>(fr);
    if (alpha == 0) {
        add(std::make_shared<SpecularReflection>(base, fres));
    }
    else {
        auto dist = std::make_shared<GGX>(alpha);
        //auto dist = std::make_shared<Beckmann>(alpha);
        add(std::make_shared<MicrofacetReflection>(base, dist, fres));
    }
}


// *** プラスチックマテリアル ***
Plastic::Plastic(Vec3 _base, Vec3 _kd, Vec3 _ks, float _alpha)
    : 
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
    : 
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


// *** Thinfilmマテリアル ***
Thinfilm::Thinfilm(Vec3 _base, float _thickness, float _n_inside, float _n_film, float _alpha, bool is_transmission)
    : base(_base),
     thickness(_thickness),
     n_inside(_n_inside),
     n_film(_n_film),
     alpha(_alpha)
{
    auto fres = std::make_shared<FresnelThinfilm>(thickness, n_inside, n_film);
    if (alpha == 0) {
        add(std::make_shared<SpecularReflection>(base, fres));
        if (is_transmission) {
            add(std::make_shared<SpecularTransmission>(base, n_inside, 1.0f, fres));
        }
    }
    else {
        auto dist = std::make_shared<Beckmann>(alpha);
        add(std::make_shared<MicrofacetReflection>(base, dist, fres));
        if (is_transmission) {
            add(std::make_shared<MicrofacetTransmission>(base, dist, n_inside, 1.0f, fres));
        }
    }
}