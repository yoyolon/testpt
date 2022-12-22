/**
* @file  ONB.h
* @brief ���K�������(ONB)�N���X
* @note  ���K��������z�����̕�����������Ƃ���
*/

#pragma once

#include "Vec3.h"

/** ���K�������N���X */
class ONB {
public:
    /**
    * @brief �f�t�H���g�R���X�g���N�^
    */
    ONB() {}

    /**
    * @brief �@���x�N�g��(������x�N�g��)����ONB���\�z
    * @param[in]  Vec3 :������̎��x�N�g��
    */
    ONB(const Vec3& _n) : n(_n) { build_ONB(n); }

    /**
    * @brief ���x�N�g�����擾����֐�
    * @return Vec3 :s���̊��x�N�g��
    */
    Vec3 get_s() const { return s; }

    /**
    * @brief ���x�N�g�����擾����֐�
    * @return Vec3 :t���̊��x�N�g��
    */
    Vec3 get_t() const { return t; }

    /**
    * @brief ���x�N�g�����擾����֐�
    * @return Vec3 :n���̊��x�N�g��
    */
    Vec3 get_n() const { return n; }

    /**
    * @brief �@�����琳�K�������(ONB)���\�z����֐�
    * @param[in]  Vec3 :���[���h���W�n�̖@���x�N�g��
    * @note �Q�l: Shirly. "Realistic Ray Tracing". 2007.
    */
    void build_ONB(const Vec3& normal);

    /**
    * @brief ���[���h���W�n�̃x�N�g����ONB���W�n�̃x�N�g���ɕϊ�����֐�
    * @param[in]  Vec3 :���[���h���W�̃x�N�g��
    * @return Vec3     :ONB���W�n�̃x�N�g��
    */
    Vec3 to_local(const Vec3& a) const;

    /**
    * @brief ONB���W�n�̃x�N�g�������[���h���W�n�̃x�N�g���ɕϊ�����֐�
    * @param[in]  Vec3 :ONB���W�̃x�N�g��
    * @return Vec3     :���[���h���W�n�̃x�N�g��
    */
    Vec3 to_world(const Vec3& a) const;

private:
    Vec3 s, t, n; /**< ���K�����x�N�g�� */
};