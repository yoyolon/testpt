/**
* @file  Shape.h
* @brief �O�����`�󃂃f���̒��ۃN���X�ƌ����_�\����
* @note �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
*/

#pragma once

#include "Vec3.h"

/** �}�C�N���t�@�Z�b�g���z�N���X */
class MicrofacetDistribution {
public:
    virtual ~MicrofacetDistribution();

    /**
    * @brief NFD(�}�C�N���t�@�Z�b�g���z)��]��
    * @param[in]  h :�n�[�u�x�N�g��
    * @return float :�n�[�u�x�N�g���ƃ}�C�N���t�@�Z�b�g�@������v���Ă��銄��
    */
    virtual float D(const Vec3& h) const = 0;

    /**
    * @brief ���C�ƃI�u�W�F�N�g�̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @param[out] p     :�����_���
    * @return bool      :��������̌���
    */
    virtual float lambda(const Vec3& w) const = 0;
    float G1(const Vec3& w) const {
        return 1 / (1 + lambda(w));
    }
    float G(const Vec3& wi, const Vec3& wo) const {
        return 1 / (1 + lambda(wi) + lambda(wo));
    }
    virtual Vec3 sample_halfvector() const = 0;
    virtual float eval_pdf(const Vec3& wi, const Vec3& h) const = 0;
};


/** Beckmann���z�N���X */
class BeckmannDistribution : public MicrofacetDistribution {
public:
    BeckmannDistribution(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& h) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& wi, const Vec3& h) const override;

private: 
    float alpha;
};


/** Trowbridge-Reitz(GGX)���z�N���X */
class GGXDistribution : public MicrofacetDistribution {
public:
    GGXDistribution(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& h) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& wi, const Vec3& h) const override;

private:
    float alpha;
};