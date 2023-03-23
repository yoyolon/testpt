/**
* @file  Shape.h
* @brief �O�����`�󃂃f���̒��ۃN���X�ƌ����_�\����
*/

#pragma once

#include "Ray.h"

/** �����_�̎�� */
enum class IsectType {
    None     = 1 << 0,  /**< �Ȃ�     */
    Material = 1 << 1,  /**< ���̕\�� */
    Light    = 1 << 2   /**< ����     */
};

/** �����_��� */
struct intersection {
    Vec3 pos;                            /**< ���W             */
    Vec3 normal;                         /**< �@��             */
    float t=0.0f;                        /**< ���C�̃p�����[�^ */
    bool is_front=true;                  /**< �����_�̗��\     */
    IsectType type=IsectType::None;      /**< �����_�̎��     */
    std::shared_ptr<class Material> mat; /**< �ގ��̎��       */
    std::shared_ptr<class Light> light;  /**< �����̎��       */
};


/**
* @brief ���̕\�ʂ̕\���𔻒肷��֐�
* @param[in] r :�V�F�C�v�ւ̓��˃��C
* @param[in] n :�V�F�C�v�̖@��
* @return bool :�\�Ȃ�true��Ԃ�
*/
inline bool is_front(const Ray& r, const Vec3  n) {
    return dot(n, -r.get_dir()) > 0; // ���̕\�ʂ��痣�������𐳂ɂ��邽�߂�-1����Z
}


/** �O�����`�󃂃f�����ۃN���X */
class Shape {
public:
    virtual ~Shape() {};

    /**
    * @brief ���C�ƃV�F�C�v�̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @param[out] p     :�����_���
    * @return bool      :��������̌���
    */
    virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;

    /**
    * @brief �V�F�C�v�̕\�ʐς��v�Z����֐�
    * @return float :�V�F�C�v�̕\�ʐ�
    */
    virtual float area() const = 0;

    /**
    * @brief �W�I���g���T���v�����O�̗��̊p�Ɋւ���PDF(�m�����x)��]������֐�
    * @param[in] ref :�T���v�����O���̌����_���
    * @param[in] w   :�T���v�����O����(�W�I���g���Ɍ�������������)
    * @return float  :PDF
    * @detail �T���v�����O�͗��̊p�Ɋւ��čs��
    */
    virtual float eval_pdf(const intersection& ref, const Vec3& w) const;

    /**
    * @brief �W�I���g���T���v�����O���s���֐�
    * @param[in] ref       :�T���v�����O���̌����_���
    * @return intersection :�T���v�����������_���
    */
    virtual intersection sample(const intersection& ref) const = 0;
};