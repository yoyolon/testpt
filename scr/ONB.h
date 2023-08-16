/**
* @file  ONB.h
* @brief ���K�������(ONB)�N���X
* @note  ONB���W�n�͏������z�����Ƃ���E��n(s����:��O�Ct����:�E�Cn����:��)
*/

#pragma once

#include "Math.h"

/** ���K�������N���X */
class ONB {
public:
    /**
    * @brief �@���x�N�g��(n���x�N�g��)����ONB���\�z
    * @param[in]  Vec3 :n���x�N�g��
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
    * @brief ���[���h���W�n�̍��W��ONB���W�n�̍��W�ɕϊ�����֐�
    * @param[in]  Vec3 :���[���h���W�n�̍��W
    * @return Vec3     :ONB���W�n�̍��W
    */
    Vec3 to_local(const Vec3& a) const;

    /**
    * @brief ONB���W�n�̍��W�����[���h���W�n�̍��W�ɕϊ�����֐�
    * @param[in]  Vec3 :ONB���W�n�̍��W
    * @return Vec3     :���[���h���W�n�̍��W
    */
    Vec3 to_world(const Vec3& a) const;

private:
    /**
    * @brief �@���x�N�g��(n���x�N�g��)���琳�K�������(ONB)���\�z����֐�
    * @param[in]  Vec3 :���[���h���W�n�̖@���x�N�g��
    * @note �Q�l: Shirly. "Realistic Ray Tracing". 2007.
    */
    void build_ONB(const Vec3& normal);

    Vec3 s, t, n; /**< ���K�����x�N�g�� */
};