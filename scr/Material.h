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

#include "Ray.h"


struct intersection;

enum class MaterialType {
    Diffuse  = 1, /**< �g�U���� */
    Specular = 2, /**< ���ʔ��� */
    Glossy   = 4, /**< ���򔽎� */
    Emitter  = 8  /**< ����     */
};

/**
* @brief ���ˊp�]�����v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :���ˊp�]��
*/
inline float get_cos(const Vec3& w) { return w.get_z(); }

/**
* @brief ���ˊp�]���̓����v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :���ˊp�]���̓��
*/
inline float get_cos2(const Vec3& w) { return w.get_z() * w.get_z(); }

/**
* @brief ���ˊp�����̓����v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :���ˊp�����̓��
*/
inline float get_sin2(const Vec3& w) { return std::max(0.0f, 1.0f - get_cos2(w)); }

/**
* @brief ���ˊp�������v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :���ˊp����
*/
inline float get_sin(const Vec3& w) { return std::sqrt(get_sin2(w)); }

/**
* @brief ���ˊp���ڂ��v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :���ˊp����
*/
inline float get_tan(const Vec3& w) { return get_sin(w) / get_cos(w); }

/**
* @brief ���ˊp���ڂ̓����v�Z����֐�
* @param[in]  w :�����x�N�g��
* @return float :���ˊp���ڂ̓��
*/
inline float get_tan2(const Vec3& w) { return get_sin2(w) / get_cos2(w); }


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
    * @brief ���˕����ɑ΂��Ĕ��˕������T���v�����O����BRDF��]������֐�
    * @param[in]  wi   :���˕����x�N�g��(���K��)
    * @param[in]  p    :���̕\�ʂ̌����_���
    * @param[out] brdf :���˕����Əo�˕����ɑ΂���BRDF�̒l
    * @param[out] wo   :�o�˕����x�N�g��(���K��)
    * @param[out] pdf  :���̊p�Ɋւ���T���v�����O�m�����x
    * @return bool     :���ˍގ��Ȃ�True
    * @note: ���ۂ͓��˕������T���v�����O���邪BRDF�������ɂ�蔽�˕����̃T���v�����O�Ƃ��Ď�������
    */
    virtual bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const = 0;

    /**
    * @brief ���˕����Ɣ��˕����ɑ΂���BRDF��]������֐�
    * @param[in]  wi :���˕����x�N�g��
    * @param[out] wo :�o�˕����x�N�g��
    * @return Vec3   :BRDF�̒l
    */
    virtual Vec3 f(const Vec3& wi, const Vec3& wo) const { return Vec3::zero; }

    /**
    * @brief ���Ȕ�����]������֐�
    * @return Vec3 : �����ɂ����ˋP�x
    */
    virtual Vec3 emitte() const { return Vec3::zero; }

    /**
    * @brief �o�˕����̃T���v�����O�m�����x���v�Z����֐�
    * @param[in] wi :���˕����x�N�g��
    * @param[in] wo :�o�˕����x�N�g��
    * @return float :�o�˕����̃T���v�����O�m�����x
    */
    virtual float sample_pdf(const Vec3& wi, const Vec3& wo) const = 0;

    /**
    * @brief �ގ��̔��˓������擾����֐�
    * @return MaterialType :�ގ��̔��˓���
    */
    MaterialType get_type() const { return type; }

private:
    MaterialType type; /**> ���˓��� */
};


/** �g�U���˃N���X */
class Diffuse : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _albedo :���ˌW��
    */
    Diffuse(Vec3 _albedo);

    Vec3 f(const Vec3& wi, const Vec3& wo) const override;

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
    Vec3 albedo; /**> ���ˌW�� */
};


/** ���ʔ��˃N���X */
class Mirror : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _albedo :���ˌW��
    */
    Mirror(Vec3 _albedo);

    Vec3 f(const Vec3& wi, const Vec3& wo) const override;

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
    Vec3 albedo; /**> ���ˌW�� */
};


// *** Phong���˃��f���N���X ***
class Phong : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _albedo :���ˌW��
    * @param[in] _Kd     :�g�U���ˌW��
    * @param[in] _Ks     :���ʔ��ˌW��
    * @param[in] _shin   :����x
    */
    Phong(Vec3 _albedo, Vec3 _Kd, Vec3 _Ks, float _shin);

    Vec3 f(const Vec3& wi, const Vec3& wo) const override;

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
    Vec3 albedo; /**> ���ˌW��     */
    Vec3 Kd;     /**> �g�U���ˌW�� */
    Vec3 Ks;     /**> ���ʔ��ˌW�� */
    float shin;  /**> ����x       */
};


/** �}�C�N���t�@�Z�b�g���˃��f���N���X */
/** @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class Microfacet : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _albedo       :���ˌW��
    * @param[in] _distribution :�}�C�N���t�@�Z�b�g���z
    * @param[in] _fresnel      :�t���l���̎�
    */
    Microfacet(Vec3 _albedo, std::shared_ptr<class MicrofacetDistribution> _distribution, 
               std::shared_ptr<class Fresnel> _fresnel);

    Vec3 f(const Vec3& wi, const Vec3& wo) const override;

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
    Vec3 albedo; /**> ���ˌW��     */
    std::shared_ptr<class Fresnel> fresnel; /**> �t���l���� */
    std::shared_ptr<class MicrofacetDistribution> distribution; /**> �}�C�N���t�@�Z�b�g���z */
};


/** �����N���X */
class Emitter : public Material {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _intensity :���ˋP�x
    */
    Emitter(Vec3 _intensity);

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

    Vec3 emitte() const;

private:
    Vec3 intensity; /**> ���ˋP�x */
};