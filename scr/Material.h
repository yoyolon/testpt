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


/** �}�e���A���̒��ۃN���X */
class Material {
public:
    virtual ~Material() {};

    /**
    * @brief �R���X�g���N�^
    * @param[in] _type :���˓����̎��
    */
    Material() {};

    /**
    * @brief ���˕����ɑ΂��ē��˕������T���v�����O����BRDF��]������֐�
    * @param[in]  wo              :���˕����x�N�g��
    * @param[in]  p               :���̕\�ʂ̌����_���
    * @param[out] brdf            :���˕����Əo�˕����ɑ΂���BRDF�̒l
    * @param[out] wi              :�o�˕����x�N�g��
    * @param[out] pdf             :���̊p�Ɋւ�����˕����T���v�����O�m�����x
    * @oaram[out] sampled_type    :�T���v�����O������BxDF�̎��
    * @oaram[in]  acceptable_type :�T���v�����O�\��BxDF�̎��
    * @return Vec3                :���˕����Əo�˕����ɑ΂���BRDF�̒l
    */
    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf,
                  BxDFType& sampled_type, BxDFType acceptable_type=BxDFType::All) const;

    /**
    * @brief BRDF��]������֐�
    * @param[in] wo              :�o�˕����x�N�g��
    * @param[in] wi              :���˕����x�N�g��
    * @oaram[in] acceptable_type :�T���v�����O�\��BxDF�̎��
    * @return Vec3               :BRDF�̒l
    */
    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p,
                BxDFType acceptable_type=BxDFType::All) const;

    /**
    * @brief ���˕����̃T���v�����O�m�����x���v�Z����֐�
    * @param[in] wo              :�o�˕����x�N�g��
    * @param[in] wi              :���˕����x�N�g��
    * @oaram[in] acceptable_type :�T���v�����O�\��BxDF�̎��
    * @return float              :�T���v�����O�m�����x
    */
    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p,
                   BxDFType acceptable_type=BxDFType::All) const;

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

private:
    std::vector<std::shared_ptr<BxDF>> bxdf_list; /**> BxDF�̏W�� */
};


/** �g�U���˃}�e���A�� */
class Diffuse : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _base :�x�[�X�J���[
    */
    Diffuse(Vec3 _base);

private:
    Vec3 base; /**> �x�[�X�J���[ */
};


/** �g�U���߃}�e���A�� */
class DiffusePlastic : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _base :�x�[�X�J���[
    * @param[in] _r    :���ˌW��
    * @param[in] _t    :���ߌW��
    */
    DiffusePlastic(Vec3 _base, Vec3 _r, Vec3 _t);

private:
    Vec3 base; /**> �x�[�X�J���[ */
    Vec3 r;    /**> ���ˌW��     */
    Vec3 t;    /**> ���ߌW��     */
};


/** ���}�e���A�� */
class Mirror : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _base :�x�[�X�J���[
    */
    Mirror(Vec3 _base);

private:
    Vec3 base; /**> �x�[�X�J���[ */
};


/** �K���X�}�e���A�� */
class Glass : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _base :�x�[�X�J���[
    * @param[in] _r    :���ˌW��
    * @param[in] _t    :���ߌW��
    * @param[in] _n    :���ܗ�
    */
    Glass(Vec3 _base, Vec3 _r, Vec3 _t, float _n, float _alpha=0.0f);

private:
    Vec3 base;   /**> �x�[�X�J���[ */
    Vec3 r;      /**> ���ˌW��     */
    Vec3 t;      /**> ���ߌW��     */
    float n;     /**> ���ܗ�       */
    float alpha; /**> �\�ʑe��     */
};


/** �����}�e���A�� */
class Metal : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _base  :�x�[�X�J���[
    * @param[in] _fr    :�������˗�
    * @param[in] _alpha :�\�ʑe��
    */
    Metal(Vec3 _base, Vec3 fr, float _alpha);

private:
    Vec3  base;  /**> �x�[�X�J���[ */
    Vec3  fr;    /**> �������˗�   */
    float alpha; /**> �\�ʑe��     */
};


/** Phong�}�e���A�� */
class Phong : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _base  :�x�[�X�J���[
    * @param[in] _kd    :�g�U���ˌW��
    * @param[in] _ks    :���ʔ��ˌW��
    * @param[in] _shine :����x
    */
    Phong(Vec3 _base, Vec3 _kd, Vec3 _ks, float _shine);

private:
    Vec3  base;  /**> �x�[�X�J���[ */
    Vec3  kd;    /**> �g�U���ˌW�� */
    Vec3  ks;    /**> ���ʔ��ˌW�� */
    float shine; /**> ����x       */
};