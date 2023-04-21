/**
* @file  Material.h
* @brief �}�e���A��
* @note: �V�F�[�f�B���O�K��
*        1.�@����z�����̕����Ƃ����V�F�[�f�B���O���W�n�ōs��
*        2.z���ƃx�N�g�����Ȃ��p��theta�Ƃ���D
*        3.����/�o�˕����͕��̕\�ʂ��痣�������𐳂Ƃ���
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
    * @brief �o�˕���������˕������T���v�����O����BRDF��]������֐�
    * @param[in]  wo              :�o�˕����x�N�g��(���[�J�����W)
    * @param[in]  p               :���̕\�ʂ̌����_���
    * @param[out] brdf            :���˕����Əo�˕����ɑ΂���BRDF�̒l
    * @param[out] wi              :���˕����x�N�g��(���[�J�����W)
    * @param[out] pdf             :���̊p�Ɋւ�����˕����T���v�����O�m�����x
    * @oaram[out] sampled_type    :�T���v�����O����BxDF�̎��
    * @oaram[in]  acceptable_type :�T���v�����O�\��BxDF�̎��
    * @return Vec3                :���˕����Əo�˕����ɑ΂���BRDF�̒l
    */
    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf,
                  BxDFType& sampled_type, BxDFType acceptable_type=BxDFType::All) const;

    /**
    * @brief BRDF��]������֐�
    * @param[in] wo              :�o�˕����x�N�g��(���[�J�����W)
    * @param[in] wi              :���˕����x�N�g��(���[�J�����W)
    * @oaram[in] acceptable_type :�T���v�����O�\��BxDF�̎��
    * @return Vec3               :BRDF�̒l
    */
    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p,
                BxDFType acceptable_type=BxDFType::All) const;

    /**
    * @brief ���˕����̃T���v�����O�m�����x���v�Z����֐�
    * @param[in] wo              :�o�˕����x�N�g��(���[�J�����W)
    * @param[in] wi              :���˕����x�N�g��(���[�J�����W)
    * @oaram[in] acceptable_type :�T���v�����O�\��BxDF�̎��
    * @return float              :�T���v�����O�m�����x
    */
    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p,
                   BxDFType acceptable_type=BxDFType::All) const;

    /**
    * @brief BxDF�̏W�����擾
    * @return std::vector<std::shared_ptr<BxDF>> : BxDF�̏W��
    */
    std::vector<std::shared_ptr<BxDF>> get_BSDF() const { return bxdf_list; }

protected:
    /**
    * @brief �ގ���BxDF��ǉ�����֐�
    * @param[in] bxdf: �U��������\��BxDF
    */
    void add(std::shared_ptr<BxDF> bxdf) { bxdf_list.push_back(bxdf); }

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


/** �����}�e���A��(v-cavity) */
class VcavityMetal : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _base  :�x�[�X�J���[
    * @param[in] _fr    :�������˗�
    * @param[in] _alpha :�\�ʑe��
    */
    VcavityMetal(Vec3 _base, Vec3 fr, float _alpha);

private:
    Vec3  base;  /**> �x�[�X�J���[ */
    Vec3  fr;    /**> �������˗�   */
    float alpha; /**> �\�ʑe��     */
};

/** �v���X�`�b�N�}�e���A�� */
class Plastic : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _base  :�x�[�X�J���[
    * @param[in] _kd    :�g�U���ˌW��
    * @param[in] _ks    :���ʔ��ˌW��
    * @param[in] _alpha :�\�ʑe��
    */
    Plastic(Vec3 _base, Vec3 _kd, Vec3 _ks, float _alpha);

private:
    Vec3  base;  /**> �x�[�X�J���[ */
    Vec3  kd;    /**> �g�U���ˌW�� */
    Vec3  ks;    /**> ���ʔ��ˌW�� */
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


/** �����}�e���A�� */
class Thinfilm : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _base  :�x�[�X�J���[
    * @param[in] _thickness       :�����̖���
    * @param[in] _n_inside        :�����}���̋��ܗ�
    * @param[in] _n_film          :�����̋��ܗ�
    * @param[in] _alpha           :�\�ʑe��
    * @param[in] _is_transmission :���ߕ��̂Ȃ�true
    */
    Thinfilm(Vec3 _base, float _thickness, float _n_inside, float _n_film, 
             float _alpha=0.0f, bool is_transmission=false);

private:
    Vec3 base;       /**> �x�[�X�J���[     */
    float thickness; /**> ����             */
    float n_inside;  /**> �����}���̋��ܗ� */
    float n_film;    /**> �����̋��ܗ�     */
    float alpha;     /**> �\�ʑe��         */
};