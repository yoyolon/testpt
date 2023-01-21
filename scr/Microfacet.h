/**
* @file  Microfacrt.h
* @brief �}�C�N���t�@�Z�b�g���z(Smith���f���p)
* @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
*/

#pragma once

#include "Vec3.h"

/** �}�C�N���t�@�Z�b�g���z�N���X(Smith���f���p) */
class NDF {
public:
    virtual ~NDF();

    /**
    * @brief �}�C�N���t�@�Z�b�g���z��]������֐�
    * @param[in]  h :�n�[�t�����x�N�g��
    * @return float :�n�[�t�����ł̃}�C�N���t�@�Z�b�g���z
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
    * @return float :�n�[�t�����x�N�g��
    */
    virtual Vec3 sample_halfvector() const = 0;

    /**
    * @brief �n�[�t�����̃T���v�����O�m�����x��]������֐�
    * @param[in] h  :�n�[�t�����x�N�g��
    * @return float :�T���v�����O�m�����x
    */
    virtual float eval_pdf(const Vec3& h) const = 0;
};


/** Beckmann���z�N���X */
class Beckmann : public NDF {
public:
    Beckmann(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& w) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& h) const override;

private: 
    float alpha;
};


/** Trowbridge-Reitz(GGX)���z�N���X */
class GGX : public NDF {
public:
    GGX(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& w) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& h) const override;

private:
    float alpha;
};