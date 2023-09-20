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
class Fresnel;
class NDF;

/**
* @brief BxDF�̃t���O(�ގ�����)
* @note �g�p��: if(frag & Specular)��frag�̍ގ����������S���ʂ��ǂ�������ł���
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
inline float get_sin2(const Vec3& w) { return std::max(0.f, 1.0f - get_cos2(w)); }

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
* @brief ����/�o�˕��������ꔼ�����ɑ��݂��邩����
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
* @param b : �`�F�b�N����ގ�����
* @return a��b���܂܂��Ȃ�true
*/
inline bool is_include_type(BxDFType a, BxDFType b) { return BxDFType((uint8_t)a & (uint8_t)b) == b; }

/**
* @brief �X�y�L�����ގ������ł��邩�m�F
* @param t : �`�F�b�N�����ގ�����
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
    */
    BxDF(BxDFType _type) : type(_type) {};

    /**
    * @brief �o�˕����ɑ΂��ē��˕������T���v�����O����BxDF��]������֐�
    * @param[in]  wo   :���˕����x�N�g��(���[�J�����W)
    * @param[in]  p    :���̕\�ʂ̌����_���
    * @param[out] wi   :�o�˕����x�N�g��(���[�J�����W)
    * @param[out] pdf  :���˕����T���v�����O�m�����x(���̊p���x)
    * @return Vec3     :BxDF�̕]���l
    */
    virtual Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const = 0;

    /**
    * @brief BxDF��]������֐�
    * @param[in] wo :�o�˕����x�N�g��(���[�J�����W)
    * @param[in] wi :���˕����x�N�g��(���[�J�����W)
    * @param[in] p  :���̕\�ʂ̌����_���
    * @return Vec3  :BxDF�̕]���l
    */
    virtual Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const { return Vec3::zero; }

    /**
    * @brief ���˕����̃T���v�����O�m�����x��]������֐�
    * @param[in] wo :�o�˕����x�N�g��(���[�J�����W)
    * @param[in] wi :���˕����x�N�g��(���[�J�����W)
    * @param[in] p  :���̕\�ʂ̌����_���
    * @return float :�T���v�����O�m�����x(���̊p���x)
    */
    virtual float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const = 0;

    /**
    * @brief �ގ��������擾����֐�
    * @return MaterialType :�ގ�����
    */
    BxDFType get_type() const { return type; }

    /**
    * @brief �ގ�������t�ƈ�v���邩����
    * @param t :�`�F�b�N����ގ�����
    * @return ��v����Ȃ�true
    */
    bool is_same_type(BxDFType t) const { return BxDFType((uint8_t)type & (uint8_t)t) == type; }

    /**
    * @brief �ގ�������t���܂܂�Ă��邩����
    * @param t :�`�F�b�N����ގ�����
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


/** ���S���ʔ��� */
class SpecularReflection : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _fres  :�t���l����
    */
    SpecularReflection(Vec3 _scale, std::shared_ptr<Fresnel> _fres);

    /**
    * @brief �U�d�̗p�R���X�g���N�^
    * @param[in] _scale :�X�P�[���t�@�N�^�[
    * @param[in] _n_inside    :�����}���̋��ܗ�
    * @param[in] _n_outside   :�O���}���̋��ܗ�(�ʏ�͋�C)
    */
    SpecularReflection(Vec3 _scale, float _n_inside, float _n_outside=1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    std::shared_ptr<Fresnel> fres; /**> �t���l���� */
};


/** ���S���ʓ��� */
class SpecularTransmission : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale     :�X�P�[���t�@�N�^�[
    * @param[in] _n_inside  :�����}���̋��ܗ�
    * @param[in] _n_outside :�O���}���̋��ܗ�(�ʏ�͋�C)
    * @param[in] _fres      :�t���l����
    */
    SpecularTransmission(Vec3 _scale, float _n_inside, float _n_outside=1.0f, 
                         std::shared_ptr<Fresnel> _fres=nullptr);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    float n_inside;  /**> �����}���̋��ܗ� */
    float n_outside; /**> �O���}���̋��ܗ� */
    std::shared_ptr<Fresnel> fres; /**> �t���l���� */
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
    /**
    * @brief ���K��Phong���z���烌�C�̕������T���v�����O
    * @param[in] alpha :���z�̃p�����[�^
    * @return Vec3     :���C�̕���
    * @note: [E.Lafortune and Y.Willems 1994]����Ɏ���
    */
    Vec3 phong_sample(float shine) const;

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
    * @param[in] _fres  :�t���l����
    */
    MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist, std::shared_ptr<Fresnel> _fres,
                         bool _is_multiple_scattering=false);

    /**
    * @brief �U�d�̗p�R���X�g���N�^
    * @param[in] _scale     :�X�P�[���t�@�N�^�[
    * @param[in] _n_inside  :�����}���̋��ܗ�
    * @param[in] _n_outside :�O���}���̋��ܗ�(�ʏ�͋�C)
    */
    MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist, 
                         float _n_inside, float _n_outside=1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    /**
    * @brief ���d�U���e�[�u�����v�Z����֐�
    */
    void create_multiple_scattering_table();
    /**
    * @brief �����e�J��������̏d�݂��v�Z����֐�(���d�U���e�[�u���̌v�Z�ɗ��p)
    * @param[in] theta      :�p�̗]��
    * @param[in] phi        :���ʊp
    * @param[in] dist_alpha :�}�C�N���t�@�Z�b�gNDF
    * @note: VNDF�T���v�����O�Ɋ�Â��d�݂��v�Z���Ă���̂Œ���(TODO: �C��)
    */
    float weight(float cos_theta, float phi, const std::shared_ptr<NDF>& dist_alpha) const;

    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    std::shared_ptr<Fresnel> fres; /**> �t���l���� */
    std::shared_ptr<NDF> dist; /**> �}�C�N���t�@�Z�b�g���z */
    bool is_multiple_scattering; /**> ���d�U���̍l������Ȃ�true */
    float E_ave; /**> ���σA���x�h */
    Vec3 F_ave;  /**> ���σt���l��*/
    float E[1000] = { 0.f }; /**> Directional�A���x�h */
    int table_size = sizeof(E) / sizeof(float);
};


/** �}�C�N���t�@�Z�b�g���� */
/** @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class MicrofacetTransmission : public BxDF {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _scale     :�X�P�[���t�@�N�^�[
    * @param[in] _dist      :�}�C�N���t�@�Z�b�g���z
    * @param[in] _n_inside  :�����}���̋��ܗ�
    * @param[in] _n_outside :�O���}���̋��ܗ�(�ʏ�͋�C)
    * @param[in] _fres      :�t���l����
    */
    MicrofacetTransmission(Vec3 _scale, std::shared_ptr<NDF> _dist, float _n_inside, 
                           float _n_outside=1.0f, std::shared_ptr<Fresnel> _fres=nullptr);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> �X�P�[���t�@�N�^�[ */
    float n_inside;  /**> �����}���̋��ܗ� */
    float n_outside; /**> �O���}���̋��ܗ� */
    std::shared_ptr<NDF> dist; /**> �}�C�N���t�@�Z�b�g���z */
    std::shared_ptr<Fresnel> fres; /**> �t���l���� */
};