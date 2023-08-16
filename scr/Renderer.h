/**
* @file  Renderer.h
* @brief �����_�����O�����̎���
* @note  memo
*/

#pragma once

#include "Math.h"

struct intersection;
class Camera;
class ONB;
class Ray;
class Scene;
class Shape;

// ���ڌ��̃T���v�����O�헪
enum class Sampling {
    UNIFORM = 1 << 0,  /**< ��l�T���v�����O             */
    BSDF    = 1 << 1,  /**< BSDF�ɂ��d�_�I�T���v�����O */
    LIGHT   = 1 << 2,  /**< �����ɂ��d�_�I�T���v�����O */
    MIS     = 1 << 3,  /**< ���d�d�_�I�T���v�����O       */
};

/** �����_���[�N���X */
class Renderer {
public:
    /**
    * @brief �����_���[��������
    * @param[in] _spp : 1�s�N�Z��������̃T���v����(samples per pixel)
    * @param[in] strategy  :�}�e���A��
    */
    Renderer(int _spp=4, Sampling _strategy=Sampling::UNIFORM);

    /**
    * @brief ���ڌ�����l�ɑI�񂾓��˕�������T���v�����O����֐�
    * @pram[in] r             :�ǐՃ��C
    * @pram[in] isect         :�I�u�W�F�N�g�̌����_���
    * @pram[in] world         :�V�[��
    * @pram[in] shading_coord :�V�F�[�f�B���O���W�n
    * @return Vec3 :���ڌ��̓��˕��ˋP�x
    * @note �X�y�L�������C�ł͎��s����Ȃ�
    */
    Vec3 explict_uniform(const Ray& r, const intersection& isect, const Scene& world,
        const ONB& shading_coord) const;

    /**
    * @brief ���ڌ���BSDF�ɉ��������˕�������T���v�����O����֐�
    * @pram[in] r             :�ǐՃ��C
    * @pram[in] isect         :�I�u�W�F�N�g�̌����_���
    * @pram[in] world         :�V�[��
    * @pram[in] shading_coord :�V�F�[�f�B���O���W�n
    * @return Vec3 :���ڌ��̏d�ݕt�����˕��ˋP�x
    * @note �X�y�L�������C�ł͎��s����Ȃ�
    */
    Vec3 explict_bsdf(const Ray& r, const intersection& isect, const Scene& world,
        const ONB& shading_coord) const;

    /**
    * @brief ���ڌ�����̌�������T���v�����O
    * @pram[in] r             :�ǐՃ��C
    * @pram[in] isect         :�I�u�W�F�N�g�̌����_���
    * @pram[in] world         :�V�[��
    * @pram[in] shading_coord :�V�F�[�f�B���O���W�n
    * @return Vec3 :���ڌ��̏d�ݕt�����˕��ˋP�x
    * @note �X�y�L�������C�ł͎��s����Ȃ�
    */
    Vec3 explict_one_light(const Ray& r, const intersection& isect, const Scene& world,
        const ONB& shading_coord) const;

    /**
    * @brief �����I�ɒ��ڌ����T���v�����O
    * @pram[in] r             :�ǐՃ��C
    * @pram[in] isect         :�I�u�W�F�N�g�̌����_���
    * @pram[in] world         :�V�[��
    * @pram[in] shading_coord :�V�F�[�f�B���O���W�n
    * @return Vec3 :�����̏d�ݕt�����ˋP�x
    */
    Vec3 explicit_direct_light_sampling(const Ray& r, const intersection& isect,
        const Scene& world, const ONB& shading_coord) const;

    /**
    * @brief �m���I���C�g���[�V���O�����s����֐�
    * @param[in]  r_in      :�J������������̃��C
    * @param[in]  max_depth :���C�̍ő�o�E���X��
    * @param[in]  world     :�����_�����O����V�[���̃f�[�^
    * @return Vec3          :���C�ɉ��������ˋP�x
    * @note ���V�A�����[���b�g�ɂ��ł��؂���������Ă��Ȃ��̂�max_depth�͏����߂ɂ��Ă���
    */
    Vec3 L_raytracing(const Ray& r_in, int max_depth, const Scene& world) const;

    /**
    * @brief �i�C�[�u�ȃp�X�g���[�V���O�����s����֐�
    * @param[in]  r_in      :�J������������̃��C
    * @param[in]  max_depth :���C�̍ő�o�E���X��
    * @param[in]  world     :�����_�����O����V�[���̃f�[�^
    * @return Vec3          :���C�ɉ��������ˋP�x
    */
    Vec3 L_naive_pathtracing(const Ray& r_in, int max_depth, const Scene& world) const;

    /**
    * @brief �p�X�g���[�V���O�����s����֐�
    * @param[in]  r_in      :�J������������̃��C
    * @param[in]  max_depth :���C�̍ő�o�E���X��
    * @param[in]  world     :�����_�����O����V�[���̃f�[�^
    * @return Vec3          :���C�ɉ��������ˋP�x
    */
    Vec3 L_pathtracing(const Ray& r_in, int max_depth, const Scene& world) const;

    /**
    * @brief �V�[�����̃V�F�C�v�̖@������������֐�
    * @param[in]  r         :�ǐՂ��郌�C
    * @param[in]  world     :�����_�����O����V�[���̃f�[�^
    * @return Vec3          :�@���̉���
    * @note z�����̕������J�����֌����������Ƃ���
    */
    Vec3 L_normal(const Ray& r, const Scene& world) const;

    /**
    * @brief �w�肵���f�[�^����V�[���������_�����O����֐�
    * @param[out] world    :�V�[���f�[�^
    * @param[out] cam      :�J�����f�[�^
    */
    void render(const Scene& world, const Camera& cam) const;


private:
    int spp;           /**< 1�s�N�Z��������̃T���v���� */
    Sampling strategy; /**< �����̃T���v�����O�헪      */
};