/**
* @file  Shape.h
* @brief �O�����`�󃂃f���̒��ۃN���X�ƌ����_�\����
*/

#pragma once

#include "Ray.h"

/** �����_�̎�� */
enum class IsectType {
    None     = 1,
    Material = 2,
    Light    = 4
};

/** �����_��� */
struct intersection {
    Vec3 pos;                            /**< ���W             */
    Vec3 normal;                         /**< �@��             */
    float t;                             /**< ���C�̃p�����[�^ */
    IsectType type;                      /**< �����_�̎��     */
    std::shared_ptr<class Material> mat; /**< �ގ��̎��       */
    std::shared_ptr<class Light> light;  /**< �����̎��       */
};


/** �O�����`�󃂃f�����ۃN���X */
class Shape {
public:
    virtual ~Shape() {};

    /**
    * @brief ���C�ƃI�u�W�F�N�g�̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @param[out] p     :�����_���
    * @return bool      :��������̌���
    */
    virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;

    /**
    * @brief �I�u�W�F�N�g�̕\�ʐς��v�Z����֐�
    * @return float :�I�u�W�F�N�g�̕\�ʐ�
    */
    virtual float area() const = 0;

    /**
    * @brief �W�I���g���T���v�����O�̊m�����x���v�Z����֐�
    * @param[in] p  :�T���v�����O���̌����_���
    * @param[in] w  :�T���v�����O����
    * @return float :�m�����x
    * @detail �T���v�����O�͗��̊p�Ɋւ��čs��
    */
    virtual float sample_pdf(const intersection& ref, const Vec3& w) const;

    /**
    * @brief �W�I���g���T���v�����O�̊m�����x���v�Z����֐�
    * @param[in] ref       :�T���v�����O���̌����_���
    * @return intersection :�T���v�����������_���
    */
    virtual intersection sample(const intersection& ref) const = 0;
};