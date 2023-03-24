/**
* @file  Light.h
* @brief �����N���X
* @note  Material��Shape��"���L�����"��Light��Shape��"���L����"
*/

#pragma once

#include "Ray.h"

struct intersection;
class Piecewise2D;
class Scene;
class Shape;

enum class LightType {
    None = 1 << 0,  /**< �Ȃ�   */
    Area = 1 << 1,  /**< �ʌ��� */
    IBL  = 1 << 2   /**< IBL    */
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
    * @param[in] w :���ˋP�x����������
    * @return Vec3 :�����̕��ˋP�x
    */
    virtual Vec3 evel_light(const Vec3& w) const = 0;

    /**
    * @brief �����̕��˃G�l���M�[���v�Z����֐�
    * @return Vec3 :�����̕��˃G�l���M�[
    */
    virtual Vec3 power() const = 0;

    /**
    * @brief ��������̓��˕������T���v�����ĕ��ˋP�x��Ԃ��֐�
    * @param[in]  ref :�T���v�����O���̌����_���
    * @param[out] w   :��������̓��˕���(���K��)
    * @param[out] pdf :���̊p�Ɋւ���T���v�����O�m�����x
    * @return Vec3    :��������̓��˕���
    * @note: ���ۂ͌�������̓��˕����͔��]����(������������ǐՂ��邽��)
    */
    virtual Vec3 sample_light(const intersection& ref, Vec3& w, float& pdf) const = 0;

    /**
    * @brief ���˕�����������T���v�����O�̊m�����x��Ԃ��֐�
    * @param[in]  ref :�T���v�����O���̌����_���
    * @param[in] w    :�����ւ̓��˕���(���K��)
    * @return float   :�m�����x
    * @note �����ɓ��B���Ȃ��ꍇ�͊m�����x�̓[���ɂȂ�
    */
    virtual float eval_pdf(const intersection& ref, const Vec3& w) const = 0;

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
    bool is_visible(const intersection& p1, const intersection& p2, const Scene& world);

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
    AreaLight(Vec3 _intensity, std::shared_ptr<Shape> _shape);

    Vec3 evel_light(const Vec3& w) const override;

    Vec3 power() const override;


    Vec3 sample_light(const intersection& ref, Vec3& wo, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& w) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    Vec3 intensity;               /**< �����̕��ˋP�x     */
    std::shared_ptr<Shape> shape; /**< �ʌ����̃W�I���g�� */
    float area;                   /**< �����̖ʐ�         */
};


// *** ������(IBL) ***
class EnvironmentLight : public Light {
public:
    /**
    * @brief �ʌ����̏�����
    * @param[in] _intensity :�����̕��ˋP�x
    * @param[in] _shape     :�����̃W�I���g��
    */
    EnvironmentLight(std:: string filename);

    Vec3 evel_light(const Vec3& w) const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& ref, Vec3& wo, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& w) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    float* envmap;    /**< ���}�b�v   */
    int nw;           /**< ��           */
    int nh;           /**< ����         */
    int nc;           /**< �`�����l���� */
    float brightness; /**< ���邳       */
    std::unique_ptr<Piecewise2D> dist;  /**< ���}�b�v�̋P�x���z */
};