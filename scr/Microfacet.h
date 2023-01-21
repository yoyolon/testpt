/**
* @file  Shape.h
* @brief �O�����`�󃂃f���̒��ۃN���X�ƌ����_�\����
* @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
*/

#pragma once

#include "Vec3.h"

/** �}�C�N���t�@�Z�b�g���z�N���X */
class NDF {
public:
    virtual ~NDF();

    /**
    * @brief NFD(�}�C�N���t�@�Z�b�g���z)��]��
    * @param[in]  h :�n�[�u�x�N�g��
    * @return float :�n�[�u�x�N�g���ƃ}�C�N���t�@�Z�b�g�@������v���Ă��銄��
    */
    virtual float D(const Vec3& h) const = 0;

    /**
    * @brief Smith�����_�֐�
    * @param[in] w  :���o�˕���
    * @return float :�]���l(Smith�}�X�L���O�֐��Ŏg�p)
    */
    virtual float lambda(const Vec3& w) const = 0;

    /**
    * @brief Smith�}�X�L���O�֐�
    * @param[in] w  :�o�˕���
    * @return float :�}�X�L���O��
    */
    float G1(const Vec3& w) const {
        return 1 / (1 + lambda(w));
    }

    /**
    * @brief Smith�V���h�E�C���O-�}�X�L���O�֐�
    * @param[in] wo  :�o�˕���
    * @param[in] wi  :���˕���
    * @return float :�V���h�E�C���O-�}�X�L���O��
    */
    float G(const Vec3& wo, const Vec3& wi) const {
        return 1 / (1 + lambda(wo) + lambda(wi));
    }

    virtual Vec3 sample_halfvector() const = 0;

    virtual float eval_pdf(const Vec3& h) const = 0;
};


/** Beckmann���z�N���X */
class Beckmann : public NDF {
public:
    Beckmann(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& h) const override;
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
    float lambda(const Vec3& h) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& h) const override;

private:
    float alpha;
};