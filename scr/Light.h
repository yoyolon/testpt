/**
* @file  Triangle.h
* @brief �O�p�`�N���X�ƎO�p�`���b�V���N���X
* @note  Material��Shape��"���L�����"��Light��Shape��"���L����"
*/

#pragma once

#include "Ray.h"

struct intersection;

enum class LightType {
    Area = 1, IBL = 2
};

/** �������ۃN���X */
class Light {
public:
    virtual ~Light() {};

    /**
    * @brief �����̏�����
    * @param[in] type :�����̎��
    */
    Light(LightType _type) : type(_type) {};

    /**
    * @brief �����̕��ˋP�x���v�Z����֐�
    * @return Vec3 :�����̕��ˋP�x
    */
    virtual Vec3 emitte() const = 0;

    /**
    * @brief �����̕��˃G�l���M�[���v�Z����֐�
    * @return Vec3 :�����̕��˃G�l���M�[
    */
    virtual Vec3 power() const = 0;

    /**
    * @brief ��������̓��˕������T���v�����ĕ��ˋP�x��Ԃ��֐�
    * @param[in]  ref :�T���v�����O���̌����_���
    * @param[out] wo  :��������̓��˕���
    * @param[out] pdf :���̊p�Ɋւ���T���v�����O�m�����x
    * @return Vec3    :��������̓��˕���
    * @note: ���ۂ͌�������̓��˕����͔��]����(������������ǐՂ��邽��)
    */
    virtual Vec3 sample_light(const intersection& ref, Vec3& w, float& pdf) = 0;

    /**
    * @brief ���C�ƌ����̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @param[out] p     :�����_���
    * @return bool      :��������̌���
    */
    virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;

    /**
    * @brief ��_�Ԃ̉�������s���֐�
    * @param[in]  p1    :�����_1
    * @param[in]  p2    :�����_2
    * @param[in]  world :�V�[��
    * @return bool      :������̌���
    */
    bool is_visible(const intersection& p1, const intersection& p2, const class Scene& world);

private:
    const LightType type;
};


// *** �ʌ��� ***
class AreaLight : public Light {
public:
    /**
    * @brief �ʌ����̏�����
    * @param[in] _intensity :�����̕��ˋP�x
    * @param[in] _shape     :�����̃W�I���g��
    */
    AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape);

    Vec3 emitte() const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& p, Vec3& wo, float& pdf) override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    Vec3 intensity;                     /**< �����̕��ˋP�x     */
    std::shared_ptr<class Shape> shape; /**< �ʌ����̃W�I���g�� */
    float area;                         /**< �����̖ʐ�         */
};