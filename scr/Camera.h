/**
* @file  Camera.h
* @brief �J����
*/

#pragma once

#include "Ray.h"

class Film;

/** �J�����N���X */
class Camera {
public:

    /**
    * @brief �J������������
    */
    Camera();

    /**
    * @brief �J������������
    * @param[in] film     :�t�B����
    * @param[in] _fd      :�œ_����
    * @param[in] _pos     :�J�����̌��_
    * @param[in] _forward :�J�����̕����x�N�g��
    */
    Camera(std::shared_ptr<Film> _film, float _fd,  Vec3 _pos, Vec3 _forward);

    int get_h() const;
    int get_w() const;
    int get_c() const;
    const char* get_filename() const;
    Vec3 get_forward() const;

    /**
    * @brief �J�����̌��_���烌�C�𐶐�
    * @param[in] u :���������p�����[�^(������)
    * @param[in] v :���������p�����[�^(�c����)
    * @return Ray  :���C
    */
    Ray generate_ray(float u, float v) const;

private:
    float fd;         /**< �œ_����             */
    Vec3 pos;         /**< �J�����̌��_         */
    Vec3 forward;     /**< �O���x�N�g��         */
    Vec3 right;       /**< �E�x�N�g��           */
    Vec3 up;          /**< ��x�N�g��           */
    Vec3 film_corner; /**< �t�B���������[�̍��W */
    std::shared_ptr<Film> film; /**< �t�B���� */
};
