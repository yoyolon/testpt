/**
* @file  Microfacrt.h
* @brief �}�C�N���t�@�Z�b�g���z(Smith���f���p)
* @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
*/

#pragma once

#include "Math.h"

/** �}�C�N���t�@�Z�b�g���z�N���X(Smith���f���p) */
class NDF {
public:
    virtual ~NDF() {};

    /**
    * @brief �}�C�N���t�@�Z�b�g���z��]������֐�
    * @param[in]  h :�n�[�t�����x�N�g��
    * @return float :�n�[�t�����ł̃}�C�N���t�@�Z�b�g���z�̕]���l
    */
    virtual float D(const Vec3& h) const = 0;

    /**
    * @brief Smith�}�X�L���O�֐��̃w���p�[�֐�
    * @param[in] w  :�����x�N�g��
    * @return float :�]���l(Smith�}�X�L���O�֐��Ŏg�p)
    */
    virtual float lambda(const Vec3& w) const = 0;

    /**
    * @brief Smith�}�X�L���O�֐�
    * @param[in] w  :�����x�N�g��
    * @return float :�}�X�L���O��
    */
    float G1(const Vec3& w) const {
        return 1 / (1 + lambda(w));
    }

    /**
    * @brief Smith�V���h�E�C���O-�}�X�L���O�֐�
    * @param[in] wo :�o�˕����x�N�g��
    * @param[in] wi :���˕����x�N�g��
    * @return float :�V���h�E�C���O-�}�X�L���O��
    */
    float G(const Vec3& wo, const Vec3& wi) const {
        return 1 / (1 + lambda(wo) + lambda(wi));
    }

    /**
    * @brief �n�[�t�����̃T���v�����O���s���֐�
    * @param[in] wo :�o�˕����x�N�g��
    * @return float :�n�[�t�����x�N�g��
    */
    virtual Vec3 sample_halfvector(const Vec3& wo) const = 0;

    /**
    * @brief �n�[�t�����̃T���v�����OPDF(�m�����x)��]������֐�
    * @param[in] h  :�n�[�t�����x�N�g��
    * @param[in] wo :�o�˕����x�N�g��
    * @return float :�T���v�����O�m�����x
    */
    virtual float eval_pdf(const Vec3& h, const Vec3& wo) const = 0;
};


/** Beckmann���z�N���X */
class Beckmann : public NDF {
public:
    Beckmann(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& w) const override;
    Vec3 sample_halfvector(const Vec3& wo) const override;
    float eval_pdf(const Vec3& h, const Vec3& wo) const override;

private: 
    /**
    * @brief Beckmann���z����n�[�t�x�N�g�����T���v�����O
    * @param[in] alpha :���z�̃p�����[�^
    * @return Vec3     :�T���v�����O�l
    * @note: �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
    */
    Vec3 beckmann_sample(float alpha) const;

    float alpha; /**< ���z�̃X���[�v�p�����[�^(�\�ʑe��) */
};


/** Trowbridge-Reitz(GGX)���z�N���X */
class GGX : public NDF {
public:
    GGX(float alpha, bool is_vsible_sampling=true);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& w) const override;
    Vec3 sample_halfvector(const Vec3& wo) const override;
    float eval_pdf(const Vec3& h, const Vec3& wo) const override;

private:
    /**
    * @brief Trowbridge-Reitz(GGX)���z����n�[�t�x�N�g�����T���v�����O
    * @param[in] alpha :���z�̃p�����[�^
    * @return Vec3     :�T���v�����O�l
    * @note: �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
    */
    Vec3 ggx_sample(float alpha) const;

    /**
    * @brief Trowbridge-Reitz(GGX)���z�̉��@�����z����n�[�t�x�N�g�����T���v�����O
    * @param[in] wo    :�o�˕���
    * @param[in] alpha :���z�̃p�����[�^
    * @return Vec3     :�T���v�����O�l
    * @note: �Q�l: [Heitz 2018](https://jcgt.org/published/0007/04/01/)
    */
    Vec3 visible_ggx_sample(const Vec3& wo, float alpha) const;

    float alpha; /**< ���z�̃X���[�v�p�����[�^(�\�ʑe��) */
    bool is_vsible_sampling;  /**< ���@�����z����T���v�����O����Ȃ�true */
};