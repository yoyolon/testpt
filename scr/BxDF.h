/**
* @file  BxDF.h
* @brief BRDF��BTDF�̎���
* @note  �N���X�݌v�Q�l: https://pbr-book.org/3ed-2018/Reflection_Models/Basic_Interface
*        ***�V�F�[�f�B���O�K��***
*        1.�@����z�����̕����Ƃ����V�F�[�f�B���O���W�n�ōs��
*        2.���o�˕����͕��̕\�ʂ��痣��������
*        3.���o�˕����͐��K������Ă���
*/
#pragma once

#include "Math.h"

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

/**
* @brief �ގ��̔��˓������܂܂�Ă��邩����
* @param a : �`�F�b�N�����ގ�����
* @param b : 
* @return a��b���܂܂��Ȃ�true
*/
inline bool is_include_type(BxDFType a, BxDFType b) { return BxDFType((uint8_t)a & (uint8_t)b) == b; }

/**
* @brief �X�y�L�����ގ������ł��邩�m�F
* @param t : �`�F�b�N�����ގ�
* @return �X�y�L�����ގ��Ȃ�true
*/
inline bool is_spacular_type(BxDFType t) { return is_include_type(t, BxDFType::Specular); }


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
    virtual Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const { return Vec3::zero; }

    /**
    * @brief �U�������̃T���v�����O�m�����x��]������֐�
    * @param[in] wo :�o�˕����x�N�g��
    * @param[in] wi :���˕����x�N�g��
    * @return float :�T���v�����O�m�����x
    */
    virtual float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const = 0;

    /**
    * @brief �ގ��̔��˓������擾����֐�
    * @return MaterialType :�ގ��̔��˓���
    */
    BxDFType get_type() const { return type; }

    /**
    * @brief �ގ��̔��˓�����t�ƈ�v���邩����
    * @param t :�`�F�b�N���锽�˓���
    * @return ��v����Ȃ�true
    */
    bool is_same_type(BxDFType t) const { return BxDFType((uint8_t)type & (uint8_t)t) == type; }

    /**
    * @brief �ގ��̔��˓�����t���܂܂�Ă��邩����
    * @param t :�`�F�b�N���锽�˓���
    * @return �܂܂��Ȃ�true
    */
    bool is_include_type(BxDFType t) const { return BxDFType((uint8_t)type & (uint8_t)t) == t; }

    /**
    * @brief ���S���ʂł��邩���肷��֐�
    * @return bool :���S���ʂȂ�true
    * @note enum class�͌^�L���X�g���K�v
    */
    bool is_specular() const { return is_include_type(BxDFType::Specular); }

    /**
    * @brief ���˕��̂ł��邩���肷��֐�
    * @return bool :���˕��̂Ȃ�true
    * @note enum class�͌^�L���X�g���K�v
    */
    bool is_reflection() const { return is_include_type(BxDFType::Reflection); }

    /**
    * @brief ���ߕ��̂ł��邩���肷��֐�
    * @return bool :���ߕ��̂Ȃ�true
    * @note enum class�͌^�L���X�g���K�v
    */
    bool is_transmission() const { return is_include_type(BxDFType::Transmission); }

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

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
};


/** Lambert���� */
class LambertianTransmission : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    */
    LambertianTransmission(const Vec3& _scale);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
};


/** ���S���ʔ��� */
class SpecularReflection : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _fres  :�t���l����
    */
    SpecularReflection(Vec3 _scale, std::shared_ptr<class Fresnel> _fres);

    /**
    * @brief �U�d�̗p�R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _n     :���ܗ�
    */
    SpecularReflection(Vec3 _scale, float _ni, float _no=1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    std::shared_ptr<class Fresnel> fres; /**> �t���l���� */
};


/** ���S���ʓ��� */
class SpecularTransmission : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    */
    SpecularTransmission(Vec3 _scale, float _n_inside, float _n_outside=1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    float n_inside;  /**> �����}���̋��ܗ� */
    float n_outside; /**> �O���}���̋��ܗ� */
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

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale;  /**> ���ˌW�� */
    float shine; /**> ����x */
};


/** v-cavity�}�C�N���t�@�Z�b�g���� */
class VcavityReflection : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _dist  :�}�C�N���t�@�Z�b�g���z
    * @param[in] _fres  :�t���l����
    */
    VcavityReflection(Vec3 _scale, std::shared_ptr<class Vcavity> _dist,
                      std::shared_ptr<class Fresnel> _fres);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    std::shared_ptr<class Fresnel> fres; /**> �t���l���� */
    std::shared_ptr<class Vcavity> dist; /**> �}�C�N���t�@�Z�b�g���z(v-cavity�p) */
};


/** �}�C�N���t�@�Z�b�g���� */
/** @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class MicrofacetReflection : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _dist  :�}�C�N���t�@�Z�b�g���z
    * @param[in] _fres  :�t���l����
    */
    MicrofacetReflection(Vec3 _scale, std::shared_ptr<class NDF> _dist, 
                         std::shared_ptr<class Fresnel> _fres);

    /**
    * @brief �U�d�̗p�R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _n     :���ܗ�
    */
    MicrofacetReflection(Vec3 _scale, std::shared_ptr<class NDF> _dist, 
                         float _ni, float _no = 1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    std::shared_ptr<class Fresnel> fres; /**> �t���l���� */
    std::shared_ptr<class NDF> dist; /**> �}�C�N���t�@�Z�b�g���z */
};


/** �}�C�N���t�@�Z�b�g���� */
/** @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class MicrofacetTransmission : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _dist  :�}�C�N���t�@�Z�b�g���z
    * @param[in] _fres  :�t���l����
    */
    MicrofacetTransmission(Vec3 _scale, std::shared_ptr<class NDF> _dist, 
                           float _n_inside, float _n_outside = 1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    float n_inside;  /**> �����}���̋��ܗ� */
    float n_outside; /**> �O���}���̋��ܗ� */
    std::shared_ptr<class NDF> dist; /**> �}�C�N���t�@�Z�b�g���z */
    std::shared_ptr<class Fresnel> fres; /**> �t���l���� */
};