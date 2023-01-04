/**
* @file  Material.h
* @brief �}�e���A��
* @note: �V�F�[�f�B���O�K��
*        1.�@����z�����̕����Ƃ����V�F�[�f�B���O���W�n�ōs��
*        2.����/�o�˕����͕��̕\�ʂ��痣�������𐳂Ƃ���
*        3.z���ƃx�N�g�����Ȃ��p��theta�Ƃ���D
*        4.����/�o�˕����͐��K������Ă���
*/
#pragma once

#include "BxDF.h"


struct intersection;

enum class MaterialType {
    Diffuse  = 1, /**< �g�U���� */
    Specular = 2, /**< ���ʔ��� */
    Glossy   = 4, /**< ���򔽎� */
};


/** �}�e���A���̒��ۃN���X */
class Material {
public:
    virtual ~Material() {};

    /**
    * @brief �R���X�g���N�^
    * @param[in] _type :���˓����̎��
    */
    Material(MaterialType _type) : type(_type) {};

    /**
    * @brief ���˕����ɑ΂��ē��˕������T���v�����O����BRDF��]������֐�
    * @param[in]  wo   :���˕����x�N�g��(���K��)
    * @param[in]  p    :���̕\�ʂ̌����_���
    * @param[out] brdf :���˕����Əo�˕����ɑ΂���BRDF�̒l
    * @param[out] wi   :�o�˕����x�N�g��(���K��)
    * @param[out] pdf  :���̊p�Ɋւ�����˕����T���v�����O�m�����x
    * @return Vec3     :���˕����Əo�˕����ɑ΂���BRDF�̒l
    */
    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const;

    /**
    * @brief BRDF��]������֐�
    * @param[in] wo :�o�˕����x�N�g��
    * @param[in] wi :���˕����x�N�g��
    * @return Vec3  :BRDF�̒l
    */
    Vec3 eval_f(const Vec3& wo, const Vec3& wi) const;

    /**
    * @brief ���˕����̃T���v�����O�m�����x���v�Z����֐�
    * @param[in] wo :�o�˕����x�N�g��
    * @param[in] wi :���˕����x�N�g��
    * @return float :�T���v�����O�m�����x
    */
    float eval_pdf(const Vec3& wo, const Vec3& wi) const;

    /**
    * @brief �ގ���BxDF��ǉ�����֐�
    * @param[in] bxdf: �U��������\��BxDF
    */
    void add(std::shared_ptr<BxDF> bxdf) { bxdf_list.push_back(bxdf); }

    /**
    * @brief BxDF�̏W�����擾
    * @return std::vector<std::shared_ptr<BxDF>> : BxDF�̏W��
    */
    std::vector<std::shared_ptr<BxDF>> get_BSDF() const { return bxdf_list; }

    /**
    * @brief �ގ��̔��˓������擾����֐�
    * @return MaterialType :�ގ��̔��˓���
    */
    MaterialType get_type() const { return type; }

private:
    MaterialType type; /**> ���˓��� */
    std::vector<std::shared_ptr<BxDF>> bxdf_list; /**> BxDF�̏W�� */
};


/** �g�U���˃}�e���A�� */
class Diffuse : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _albedo :���ˌW��
    */
    Diffuse(Vec3 _albedo);

private:
    Vec3 albedo; /**> ���ˌW�� */
};


/** ���}�e���A�� */
class Mirror : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _albedo :���ˌW��
    */
    Mirror(Vec3 _albedo);

private:
    Vec3 albedo; /**> ���ˌW�� */
};

//
//
//// *** Phong���˃��f���N���X ***
//class Phong : public Material {
//public:
//    /**
//    * @brief �R���X�g���N�^
//    * @param[in] _albedo :���ˌW��
//    * @param[in] _Kd     :�g�U���ˌW��
//    * @param[in] _Ks     :���ʔ��ˌW��
//    * @param[in] _shin   :����x
//    */
//    Phong(Vec3 _albedo, Vec3 _Kd, Vec3 _Ks, float _shin);
//
//    Vec3 f(const Vec3& wo, const Vec3& wi) const override;
//
//    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;
//
//    float eval_pdf(const Vec3& wo, const Vec3& wi) const override;
//
//private:
//    Vec3 albedo; /**> ���ˌW��     */
//    Vec3 Kd;     /**> �g�U���ˌW�� */
//    Vec3 Ks;     /**> ���ʔ��ˌW�� */
//    float shin;  /**> ����x       */
//};
//
//