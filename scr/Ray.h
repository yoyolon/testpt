/**
* @file  Ray.h
* @brief ���C
*/

#pragma once

#include "Math.h"

/** ���C�N���X */
class Ray {
public:
    /**
    * @brief ���_�ƕ����x�N�g�����烌�C��������
    * @param[in] o :���C�̌��_
    * @param[in] d :���C�̕����x�N�g��
    */
    Ray(Vec3 o, Vec3 d) : origin(o), dir(d) {};

    /**
    * @brief ���C�̌��_���擾����֐�
    * @return Vec3 :���C�̌��_
    */
    Vec3 get_origin() const { return origin; }

    /**
    * @brief ���C�̕������擾����֐�
    * @return Vec3 :���C�̕����x�N�g��
    */
    Vec3 get_dir() const { return dir; }

    /**
    * @brief �p�����[�^t�ɂ����郌�C�̍��W���v�Z����֐�
    * @param[in] t :���C�̃p�����[�^
    * @return Vec3 :���C�̍��W
    */
    inline Vec3 at(float t) const { return origin + t * dir; }

private:
    Vec3 origin; /**< ���C�̌��_ */
    Vec3 dir;    /**< ���C�̕��� */
};