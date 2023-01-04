/**
* @file  BxDF.h
* @brief BRDF��BTDF�̎���
* @note  �N���X�݌v�Q�l: https://pbr-book.org/3ed-2018/Reflection_Models/Basic_Interface
*        ***�V�F�[�f�B���O�K��***
*        1.�@����z�����̕����Ƃ����V�F�[�f�B���O���W�n�ōs��
*        2.���o�˕����͕��̕\�ʂ��痣�������𐳂Ƃ���
*        3.z���ƃx�N�g�����Ȃ��p��theta�Ƃ���D
*        4.����/�o�˕����͐��K������Ă���
*/
#pragma once

#include "Vec3.h"


struct intersection;

/**
* @brief BxDF�̃t���O
* @note �g�p��: if(frag & Specular)�ōގ������ʂ��ǂ�������ł���
*/
enum class BxDFType : uint8_t {
    Reflection   = 1 << 0,  /**< ����   */
    Transmission = 1 << 1,  /**< ����   */
    Specular     = 1 << 2,  /**< ����   */
    Diffuse      = 1 << 3,  /**< �g�U�� */
    Glossy       = 1 << 4,  /**< ����� */
    All          = Reflection | Transmission | Specular | Diffuse | Glossy
};

/**
* @brief �����x�N�g���Ɩ@���̂Ȃ��p�̗]�����v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :�]��
*/
inline float get_cos(const Vec3& w) { return w.get_z(); }

/**
* @brief �����x�N�g���Ɩ@���̂Ȃ��p�̗]���̓����v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :�]���̓��
*/
inline float get_cos2(const Vec3& w) { return w.get_z() * w.get_z(); }

/**
* @brief �����x�N�g���Ɩ@���̂Ȃ��p�̐����̓����v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :�����̓��
*/
inline float get_sin2(const Vec3& w) { return std::max(0.0f, 1.0f - get_cos2(w)); }

/**
* @brief �����x�N�g���Ɩ@���̂Ȃ��p�̐������v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :����
*/
inline float get_sin(const Vec3& w) { return std::sqrt(get_sin2(w)); }

/**
* @brief �����x�N�g���Ɩ@���̂Ȃ��p�̐��ڂ��v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :����
*/
inline float get_tan(const Vec3& w) { return get_sin(w) / get_cos(w); }

/**
* @brief �����x�N�g���Ɩ@���̂Ȃ��p�̐��ڂ̓����v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :���ڂ̓��
*/
inline float get_tan2(const Vec3& w) { return get_sin2(w) / get_cos2(w); }

/**
* @brief ���o�˕��������ꔼ�����ɑ��݂��邩����
* @param[in]  wo :�o�˕����x�N�g��
* @param[in]  wi :���˕����x�N�g��
* @return float :���ꔼ�����ɂ���Ȃ�true
*/
inline bool is_same_hemisphere(const Vec3& wo, const Vec3& wi) { 
    return wo.get_z() * wi.get_z() > 0; 
}


/** BxDF�̒��ۃN���X */
class BxDF {
public:
    virtual ~BxDF() {};

    /**
    * @brief �R���X�g���N�^
    * @param[in] _type  :���˓����̎��
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    */
    BxDF(BxDFType _type) : type(_type) {};

    /**
    * @brief �o�˕����ɑ΂��ē��˕������T���v�����O����BxDF��]������֐�
    * @param[in]  wo   :���˕����x�N�g��(���K��)
    * @param[in]  p    :���̕\�ʂ̌����_���
    * @param[out] brdf :���˕����Əo�˕����ɑ΂���BRDF�̒l
    * @param[out] wi   :�o�˕����x�N�g��(���K��)
    * @param[out] pdf  :���̊p�Ɋւ�����˕����T���v�����O�m�����x
    * @return Vec3     :���˕����Əo�˕����ɑ΂���BRDF�̒l
    */
    virtual Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const = 0;

    /**
    * @brief BxDF��]������֐�
    * @param[in] wo :�o�˕����x�N�g��
    * @param[in] wi :���˕����x�N�g��
    * @return Vec3  :BxDF�̒l
    */
    virtual Vec3 eval_f(const Vec3& wo, const Vec3& wi) const { return Vec3::zero; }

    /**
    * @brief �U�������̃T���v�����O�m�����x��]������֐�
    * @param[in] wo :�o�˕����x�N�g��
    * @param[in] wi :���˕����x�N�g��
    * @return float :�T���v�����O�m�����x
    */
    virtual float eval_pdf(const Vec3& wo, const Vec3& wi) const = 0;

    /**
    * @brief �ގ��̔��˓������擾����֐�
    * @return MaterialType :�ގ��̔��˓���
    */
    BxDFType get_type() const { return type; }

    /**
    * @brief ���S���ʂł��邩���肷��֐�
    * @return bool :���S���ʂȂ�true
    * @note enum class�͌^�L���X�g���K�v
    */
    bool is_specular() const { return ((uint8_t)type & (uint8_t)BxDFType::Specular); }

    /**
    * @brief ���˕��̂ł��邩���肷��֐�
    * @return bool :���˕��̂Ȃ�true
    * @note enum class�͌^�L���X�g���K�v
    */
    bool is_reflection() const { return ((uint8_t)type & (uint8_t)BxDFType::Reflection); }

    /**
    * @brief ���ߕ��̂ł��邩���肷��֐�
    * @return bool :���ߕ��̂Ȃ�true
    * @note enum class�͌^�L���X�g���K�v
    */
    bool is_transmission() const { return ((uint8_t)type & (uint8_t)BxDFType::Transmission); }

private:
    BxDFType type; /**> BxDF�̎�� */
};


/** Lambert���� */
class LambertianReflection : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    */
    LambertianReflection(const Vec3& _scale);

    float eval_pdf(const Vec3& wo, const Vec3& wi) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi) const override;

private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
};


/** ���S���ʔ��� */
class SpecularReflection : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    */
    SpecularReflection(Vec3 _scale, std::shared_ptr<class Fresnel> _fres);

    float eval_pdf(const Vec3& wo, const Vec3& wi) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    std::shared_ptr<class Fresnel> fres; /**> �t���l���� */
};



// *** Phong���ʔ��� ***
class PhongReflection : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _shine :����x
    */
    PhongReflection(Vec3 _scale, float _shine);

    float eval_pdf(const Vec3& wo, const Vec3& wi) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale;  /**> ���ˌW�� */
    float shine; /**> ����x */
};



/** �}�C�N���t�@�Z�b�g���� */
/** @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class MicrofacetReflection : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _dist  :�}�C�N���t�@�Z�b�g���z
    * @param[in] _fres  :�t���l���̎�
    */
    MicrofacetReflection(Vec3 _scale, 
                         std::shared_ptr<class NDF> _dist, 
                         std::shared_ptr<class Fresnel> _fres);

    float eval_pdf(const Vec3& wo, const Vec3& wi) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    std::shared_ptr<class Fresnel> fres; /**> �t���l���� */
    std::shared_ptr<class NDF> dist; /**> �}�C�N���t�@�Z�b�g���z */
};