/**
* @file  Light.h
* @brief ����
* @note  Material��Shape��"���L�����"��AreaLight��Shape��"���L����"
*/

#pragma once

#include "Ray.h"

struct intersection;
class Piecewise2D;
class Scene;
class Shape;

enum class LightType {
    None     = 1 << 0,  /**< �Ȃ�   */
    Parallel = 1 << 1,  /**< ���s���� */
    Area     = 1 << 2,  /**< �ʌ��� */
    IBL      = 1 << 3   /**< IBL    */
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
    * @brief �����̕��ˋP�x��]������֐�
    * @param[in] wi :���ˋP�x�����������x�N�g��
    * @return Vec3  :���ˋP�x�̕]���l
    */
    virtual Vec3 evel_light(const Vec3& wi) const = 0;

    /**
    * @brief �����̕��˃G�l���M�[��]������֐�
    * @return Vec3 :���˃G�l���M�[�̕]���l
    */
    virtual Vec3 power() const = 0;

    /**
    * @brief ��������̓��˕������T���v�����ĕ��ˋP�x��]������֐�
    * @param[in]  ref :�T���v�����O���̌����_���
    * @param[out] wi  :���[���h���W�n�ł̌����ւ̓��˕���(�����֌�������������)
    * @param[out] pdf :�T���v�����O�m�����x(���̊p���x)
    * @return Vec3    :���ˋP�x�̕]���l
    * @note: ������������ǐՂ��邽�ߌ����֌��������������ɂȂ�
    */
    virtual Vec3 sample_light(const intersection& ref, Vec3& wi, float& pdf) const = 0;

    /**
    * @brief ���˕�����������T���v�����O�̊m�����x��]������֐�
    * @param[in] ref  :�T���v�����O���̌����_���
    * @param[in] wi   :���[���h���W�n�ł̌����ւ̓��˕���(�����֌�������������)
    * @return float   :�T���v�����O�m�����x�̕]���l
    * @note �����ɓ��B���Ȃ��ꍇ�͊m�����x�̓[���ɂȂ�
    */
    virtual float eval_pdf(const intersection& ref, const Vec3& wi) const = 0;

    /**
    * @brief ���C�ƌ����̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @param[out] p     :�����_���
    * @return bool      :��������̌���(��������Ȃ�true)
    */
    virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;

    /**
    * @brief ��̌����_�̉�������s���֐�
    * @param[in]  p1    :�����_1
    * @param[in]  p2    :�����_2
    * @param[in]  world :�V�[��
    * @return bool      :������̌���(���Ȃ�true)
    */
    bool is_visible(const intersection& p1, const intersection& p2, const Scene& world);


    /**
    * @brief �������z���f���^���z������
    * @return bool :�f���^���z�Ȃ�true
    */
    bool is_delta_light() const { return type == LightType::Parallel; }

    LightType get_type() const { return type; }


private:
    const LightType type;
};


// *** ���s���� ***
class ParallelLight : public Light {
public:
    /**
    * @brief �ʌ����̏�����
    * @param[in] _intensity :�����̕��ˋP�x
    */
    ParallelLight(const Vec3& _intensity, const Vec3& _wi_light);

    Vec3 evel_light(const Vec3& wi) const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& ref, Vec3& wi, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& wi) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    Vec3 intensity; /**< �����̕��ˋP�x  */
    Vec3 wi_light;  /**< ���[���h���W�n�ł̌����̓��˕��� */
};


// *** �ʌ��� ***
class AreaLight : public Light {
public:
    /**
    * @brief �ʌ����̏�����
    * @param[in] _intensity :�����̕��ˋP�x
    * @param[in] _shape     :�����̃V�F�C�v
    */
    AreaLight(const Vec3& _intensity, std::shared_ptr<Shape> _shape);

    Vec3 evel_light(const Vec3& wi) const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& ref, Vec3& wi, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& wi) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    Vec3 intensity;               /**< �����̕��ˋP�x   */
    std::shared_ptr<Shape> shape; /**< �ʌ����̃V�F�C�v */
    float area;                   /**< �����̖ʐ�       */
};


// *** ������(IBL) ***
class EnvironmentLight : public Light {
public:
    /**
    * @brief �ʌ����̏�����
    * @param[in] filename :���}�b�v�̃p�X
    * @param[in] rotation :���}�b�v��z���Ɋւ����]�p(�x���@)
    */
    EnvironmentLight(const std::string& filename, float rotation=0);

    Vec3 evel_light(const Vec3& wi) const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& ref, Vec3& wi, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& wi) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    /**
    * @brief uv���W������}�b�v�̕��ˋP�x��]������֐�
    * @param[in] uv :uv���W
    * @return Vec3  :���}�b�v�̕��ˋP�x�̕]���l
    */
    Vec3 evel_light_uv(const Vec2& uv) const;

    /**
    * @brief �z��C���f�b�N�X������}�b�v�̕��ˋP�x��]������֐�
    * @param[in] x :�s�̃C���f�b�N�X
    * @param[in] y :��̃C���f�b�N�X
    * @return Vec3 :���}�b�v�̕��ˋP�x�̕]���l
    */
    Vec3 evel_envmap(int x, int y) const;

    /**
    * @brief ���}�b�v����]����֐��֐�
    * @param[in] deg :��]�p(�x���@)
    */
    void rotate_envmap(float deg);


    float* envmap;    /**< ���}�b�v   */
    int nw;           /**< ��           */
    int nh;           /**< ����         */
    int nc;           /**< �`�����l���� */
    float luminance;  /**< ���邳       */
    std::unique_ptr<Piecewise2D> dist;  /**< ���}�b�v�̋P�x���z */
};