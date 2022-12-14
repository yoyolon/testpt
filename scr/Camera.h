/**
* @file  Camera.h
* @brief �J�����N���X
*/

#pragma once

#include "Ray.h"
#include <iostream>

/** �J�����N���X */
class Camera {
public:

    /**
    * @brief �J������������
    * @param[in] h        :�t�B�����̍���
    * @param[in] _aspect  :�t�B�����̃A�X�y�N�g��
    * @param[in] _fd      :�œ_����
    * @param[in] _pos     :�J�����̌��_
    * @param[in] _forward :�J�����̕����x�N�g��
    */
    Camera(float h=2.0, float _aspect=1.0, float _fd=1.0, 
           Vec3 _pos=Vec3(0.0, 0.0, 0.0), Vec3 _forward=Vec3(0.0, 0.0, 1.0));

    /**
    * @brief �J�����̌��_���烌�C�𐶐�
    * @param[in] u :���������p�����[�^(������)
    * @param[in] v :���������p�����[�^(�c����)
    * @return Ray  :���C
    */
    Ray generate_ray(float u, float v);

private:
    float film_w;        /**< �t�B������           */
    float film_h;        /**< �t�B��������         */
    float aspect;        /**< �A�X�y�N�g��         */
    float fd;            /**< �œ_����             */
    Vec3 pos;            /**< �J�����̌��_         */
    Vec3 forward_vector; /**< �O���x�N�g��         */
    Vec3 right_vector;   /**< �E�x�N�g��           */
    Vec3 up_vector;      /**< ��x�N�g��           */
    Vec3 film_corner;    /**< �t�B���������[�̍��W */
};
