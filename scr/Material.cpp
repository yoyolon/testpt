#include "Material.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** �}�e���A�� ***

Vec3 Material::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
    // ���BxDF����������T���v�����O���Ċ�^�͂��ׂĂ�BxDF���l������
    auto f = Vec3::zero;
    auto num_bxdfs = bxdf_list.size();
    if (num_bxdfs == 0) {
        return f;
    }
    // BxDF�������_���Ɉ�I��
    auto bxdf_index = Random::uniform_int(0, num_bxdfs - 1);
    const auto& bxdf_main = bxdf_list[bxdf_index]; // �T���v�����O����BxDF
    // BxDF�ŕ������T���v�����O
    f = bxdf_main->sample_f(wo, p, wi, pdf);
    // ���ׂĂ�BxDF���l������BSDF��pdf���v�Z
    f = eval_f(wo, wi);
    pdf = eval_pdf(wo, wi);
    return f;
}

Vec3 Material::eval_f(const Vec3& wo, const Vec3& wi) const {
    // ���ׂĂ�BxDF�̑��a���v�Z
    // TODO: ���˂Ɠ��߂̋��(���ς𗘗p)
    //       ���邢��BxDF�𔼋��łȂ��S���͈͂Œ�`����
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
    // ���ׂĂ�BxDF��pdf�̕��ς��v�Z
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


// *** �g�U���˃}�e���A�� ***
Diffuse::Diffuse(Vec3 _albedo) 
    : Material(MaterialType::Diffuse), 
      albedo(_albedo) 
{
    // �����o�[�gBRDF��ǉ�
    // TODO: �I�����i�C�������f��������������e�����l��
    add(std::make_shared<LambertianReflection>(albedo));
}


// *** ���}�e���A�� ***
Mirror::Mirror(Vec3 _albedo)
    : Material(MaterialType::Specular),
    albedo(_albedo)
{
    auto fres = std::make_shared<FresnelConstant>(Vec3::one);
    add(std::make_shared<SpecularReflection>(albedo, fres));
}


// *** �����}�e���A�� ***
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


// *** Phong�}�e���A�� ***
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