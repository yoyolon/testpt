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


// *** �g�U���� ***
Diffuse::Diffuse(Vec3 _albedo) 
    : Material(MaterialType::Diffuse), 
      albedo(_albedo) 
{
    // �����o�[�gBRDF��ǉ�
    add(std::make_shared<LambertianReflection>(_albedo));
}

//
//// *** ���S���ʔ��� ***
//Mirror::Mirror(Vec3 _albedo) : Material(MaterialType::Specular), albedo(_albedo) {}
//
//
//float Mirror::eval_pdf(const Vec3& wo, const Vec3& wi) const {
//    return 1.0f;
//}
//
//Vec3 Mirror::f(const Vec3& wo, const Vec3& wi) const {
//    return albedo / dot(Vec3(0, 0, 1), unit_vector(wi));
//}
//
//Vec3 Mirror::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
//    wi = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // �����˕���(reflect��葬��)
//    pdf = eval_pdf(wo, wi);
//    auto brdf = f(wo, wi);
//    return brdf;
//}
//
//
//// *** Phong���f�� ***
//// TODO: ���K��/�������I�ȏd�_�I�T���v�����O�̎���
//Phong::Phong(Vec3 _albedo, Vec3 _Kd, Vec3 _Ks, float _shin) 
//    : Material(MaterialType::Glossy), albedo(_albedo), Kd(_Kd), Ks(_Ks), shin(_shin) {}
//
//float Phong::eval_pdf(const Vec3& wo, const Vec3& wi) const {
//    return std::max(std::abs(get_cos(wi)) * invpi, epsilon);
//}
//
//Vec3 Phong::f(const Vec3& wo, const Vec3& wi) const {
//    auto dir_spec = Vec3(-wo.get_x(), -wo.get_y(), wo.get_z()); // �����˕���
//    Vec3 diffuse = Kd * invpi;
//    Vec3 specular = Ks * std::pow(dot(dir_spec, wi), shin) * invpi;
//    return albedo * (diffuse + specular);
//}
//
//Vec3 Phong::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
//    wi = Random::cosine_hemisphere_sample();
//    pdf = eval_pdf(wo, wi);
//    auto brdf = f(wo, wi);
//    return brdf;
//}
//
//
//// *** �}�C�N���t�@�Z�b�g���f�� ***
///** @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
//Microfacet::Microfacet(Vec3 _albedo, std::shared_ptr<MicrofacetDistribution> _distribution, 
//                       std::shared_ptr<Fresnel> _fresnel)
//    : Material(MaterialType::Glossy), albedo(_albedo), distribution(_distribution), fresnel(_fresnel) {}
//
//float Microfacet::eval_pdf(const Vec3& wo, const Vec3& wi) const {
//    auto h = unit_vector(wo + wi);
//    return distribution->eval_pdf(wo, h) / (4 * dot(wo, h)); // �m�����x�̕ϊ�
//}
//
//Vec3 Microfacet::f(const Vec3& wo, const Vec3& wi) const {
//    if (wo.get_z() * wi.get_z() < 0) {
//        return Vec3::zero; // ���ꔼ�����ɑ��݂��Ȃ��Ȃ甽�˂��Ȃ�(�P�U������̂���)
//    }
//    auto h = unit_vector(wo + wi);
//    float cos_wo = std::abs(get_cos(wo));
//    float cos_wi = std::abs(get_cos(wi));
//    if (cos_wo == 0 || cos_wi == 0) {
//        return Vec3::zero;
//    }
//    if (is_zero(h)) {
//        return Vec3::zero;
//    }
//    float D = distribution->D(h);
//    float G = distribution->G(wo, wi);
//    Vec3 F = fresnel->evaluate(dot(wo, h));
//    return albedo * (D * G * F) / (4 * cos_wo * cos_wi);
//}
//
//Vec3 Microfacet::sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const {
//    Vec3 h = distribution->sample_halfvector();
//    wi = unit_vector(reflect(wo, h)); // reflect()�ł͐��K�����Ȃ��̂Ŗ����I�ɐ��K��
//    pdf = eval_pdf(wo, wi);
//    auto brdf = f(wo, wi);
//    return brdf;
//}