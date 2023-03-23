/**
* @file Vcavity.h
* @brief �}�C�N���t�@�Z�b�g���z�N���X(v-cavity���f���p)
*/

#pragma once

#include "Vec3.h"

/**
* @brief �}�C�N���t�@�Z�b�g���z�̎��
*/
enum class NDFType {
    Beckmann = 1, /**< �x�b�N�}�����z */
    GGX      = 1  /**< GGX���z */
};


/** �}�C�N���t�@�Z�b�g���z�N���X(V-cavity���f���p) */
class Vcavity {
public:

    /**
    * @brief �R���X�g���N�^
    * @param[in]  alpha : �}�C�N���t�@�Z�b�g�\�ʑe��
    * @param[in]  type  : �}�C�N���t�@�Z�b�g���z�̎��
    */
    Vcavity(float _alpha, NDFType _type=NDFType::Beckmann);

    /**
    * @brief �}�C�N���t�@�Z�b�g���z��]������֐�
    * @param[in]  h :�n�[�t�����x�N�g��
    * @return float :�n�[�t�����ł̃}�C�N���t�@�Z�b�g���z
    */
    float D(const Vec3& h) const;

    /**
    * @brief V-cavity�V���h�E�C���O-�}�X�L���O�֐�
    * @param[in] wo :�o�˕����x�N�g��
    * @param[in] wi :���˕����x�N�g��
    * @param[in] h  :�}�C�N���t�@�Z�b�g�@��
    * @param[in] n  :�W�I���g���@��
    * @return float :�V���h�E�C���O-�}�X�L���O��
    */
    float G(const Vec3& wo, const Vec3& wi, const Vec3& h) const;

    /**
    * @brief �n�[�t�����̃T���v�����O���s���֐�
    * @param[in] wo :�o�˕����x�N�g��
    * @return float :�n�[�t�����x�N�g��
    */
    Vec3 sample_halfvector(const Vec3& wo) const;

    /**
    * @brief �n�[�t�����̃T���v�����O�m�����x��]������֐�
    * @param[in] h  :�n�[�t�����x�N�g��
    * @param[in] wo :�o�˕����x�N�g��
    * @return float :�T���v�����O�m�����x
    */
    float eval_pdf(const Vec3& h, const Vec3& wo) const;

private:
    std::shared_ptr<class NDF> dist; // NDF
};