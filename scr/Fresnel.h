/**
* @file  Fresnel.h
* @brief �t���l����
*/

#pragma once

#include "Vec3.h"

/** �t���l�����̒��ۃN���X */
class Fresnel {
public:
    virtual ~Fresnel() {};

    /**
    * @brief ���ˊp�ɂ��t���l�����˗����v�Z����֐�
    * @param[in] cos_theta :���ˊp�]��
    * @return Vec          :�t���l�����˗�
    * @note: ���ˊp�͖@���x�N�g���Ɠ��˕����x�N�g�����Ȃ��p
    */
    virtual Vec3 eval(float cos_theta, const struct intersection& p) const = 0;
};

/** 
* ��蔽�˗��N���X
* @note: �t���l���̎��ɏ]���Ă��Ȃ��̂Œ���
*/
class FresnelConstant : public Fresnel {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in]  _F0 :
    */
    FresnelConstant(Vec3 _F0);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    Vec3 F0; /**< ���˗� */
};

/** Schlick�t���l���N���X */
class FresnelSchlick : public Fresnel {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in]  _F0 :�������˂ł̃t���l�����˗�
    */
    FresnelSchlick(Vec3 _F0);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    Vec3 F0; /**< �������˂ł̃t���l�����˗� */
};


/** �U�d�̃t���l���N���X */
class FresnelDielectric : public Fresnel {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in]  _n_inside  :�����}���̋��ܗ�
    * @param[in]  _n_outside :�O���}���̋��ܗ�
    */
    FresnelDielectric(float _n_inside, float _n_outisde=1.0f);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    float n_inside, n_outside; /**< ���ܗ� */
};


/** �U�d�̒P�w�������t���l���N���X */
class FresnelThinfilm : public Fresnel {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in]  _d :�����̖���
    * @param[in] _ni :���˕����}���̋��ܗ�
    * @param[in] _nf :�����̋��ܗ�
    * @param[in] _no :�o�˕����}���̋��ܗ�
    */
    FresnelThinfilm(float _d, float _ni, float _nf, float _no);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    float d;          /**< ����   */
    float ni, nf, no; /**< ���ܗ� */
};


/** LUT�t���l���N���X */
class FresnelLUT : public Fresnel {
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in]  _filename :���˗��e�[�u���̃p�X
    */
    FresnelLUT(std::string filename);

    /**
    * @brief �R���X�g���N�^
    * @param[in]  _filename :���˗��e�[�u���̃p�X
    */
    void load_LUT(std::string filename);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    std::vector<Vec3> table; /**< �t���l�����˗��e�[�u�� */
};
