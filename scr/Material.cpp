#include "Material.h"
#include "Microfacet.h"
#include "Shape.h"
#include "Random.h"
#include "Fresnel.h"

// *** �}�e���A�� ***

Vec3 Material::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf,
                        BxDFType& sampled_type, BxDFType acceptable_type) const {
    // ���e�\��BxDF�̗v�f�����J�E���g
    int num_acceptable_bxdfs = 0;
    for (const auto& bxdf : bxdf_list) {
        if (bxdf->is_same_type(acceptable_type))
            num_acceptable_bxdfs++;
    }
    if (num_acceptable_bxdfs == 0) {
        return Vec3::zero;
    }
    // ���e�\��BxDF���烉���_���ɃT���v�����O
    // note: bxdf_list�ɋ��e�s�\��BxDF���܂܂�Ă���ꍇ������̂�for���[�v�Ŋm�F
    auto bxdf_count = Random::uniform_int(0, num_acceptable_bxdfs - 1); // ���e�\��BxDF�̏���
    auto bxdf_list_index = 0; // bxdf_list��ł̃T���v�����OBxDF�̃C���f�b�N�X
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
    // ���ׂĂ̋��e�\��BxDF���l������BSDF��pdf���v�Z
    auto f = eval_f(wo, wi, p, acceptable_type);
    pdf = eval_pdf(wo, wi, p, acceptable_type);
    // �T���v�����OBxDF���X�y�L�����̏ꍇeval��0��^�ƂȂ�̂ŕ␳
    // TODO: �����܂ŋ~�Ϗ��u���ƂŏC��
    if (sampled_bxdf->is_specular()) {
        f += sampled_f;
        pdf += sampled_pdf / num_acceptable_bxdfs;
    }
    return f;
}

Vec3 Material::eval_f(const Vec3& wo, const Vec3& wi, const intersection& p, 
                      BxDFType acceptable_type) const {
    // ���ׂĂ�BxDF�̑��a���v�Z
    auto f = Vec3::zero;
    bool is_reflect = wo.get_z() * wi.get_z() > 0; // ���������˂͂ǂ�������
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
    // ���e�\��BxDF�̗v�f�����J�E���g
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


// *** �g�U���˃}�e���A�� ***
Diffuse::Diffuse(Vec3 _base) 
    : Material(), 
      base(_base) 
{
    // �����o�[�gBRDF��ǉ�
    // TODO: �I�����i�C�������f��������������e�����l��
    add(std::make_shared<LambertianReflection>(base));
}


// *** �g�U���߃}�e���A�� ***
// TODO: [�v����]�g�U���߂��ĉ��H�H�H
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


// *** ���}�e���A�� ***
Mirror::Mirror(Vec3 _base)
    : Material(),
    base(_base)
{
    auto fres = std::make_shared<FresnelConstant>(Vec3::one);
    add(std::make_shared<SpecularReflection>(base, fres));
}


// *** �K���X�}�e���A�� ***
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
    else { // �e����0�̏ꍇ���S����
        if (!is_zero(r)) {
            add(std::make_shared<SpecularReflection>(base * r, n));
        }
        if (!is_zero(t)) {
            add(std::make_shared<SpecularTransmission>(base * t, n));
        }
    }
}


// *** �����}�e���A�� ***
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


// *** �v���X�`�b�N�}�e���A�� ***
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


// *** Phong�}�e���A�� ***
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