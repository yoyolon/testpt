/**
* @file Scene.h
* @brief �V�[�����̃V�F�C�v���Ǘ�
*/

#pragma once

#include "Shape.h"
#include "Light.h"

// *** �V�[���N���X ***
class Scene {
public:
    /**
    * @brief �f�t�H���g�R���X�g���N�^
    */
    Scene() {}

    /**
    * @brief �V�[���ɃV�F�C�v��ǉ�
    * @param[in]  object :�V�F�C�v
    */
    void add(std::shared_ptr<Shape> object) { shape_list.push_back(object); }

    /**
    * @brief �V�[���Ɍ�����ǉ�
    * @param[in]  light :����
    */
    void add(std::shared_ptr<Light> light) { light_list.push_back(light); }

    /**
    * @brief �V�[������S�ẴV�F�C�v�ƌ���������
    */
    void clear() { shape_list.clear(); }

    /**
    * @brief �V�[���̃V�F�C�v���擾
    * @return std::vector<std::shared_ptr<Shape>> :�V�[�����̃V�F�C�v�̏W��
    */
    std::vector<std::shared_ptr<Shape>> get_shape() const { return shape_list; }

    /**
    * @brief �V�[���̌������擾
    * @return std::vector<std::shared_ptr<Light>> :�V�[�����̌����̏W��
    */
    std::vector<std::shared_ptr<Light>> get_light() const { return light_list; }

    /**
    * @brief ���C�ƃI�u�W�F�N�g(�V�F�C�v�ƌ���)�̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @param[out] p     :�����_���
    * @return bool      :��������̌���
    * @note TODO: BVH�̎���
    */
    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
        intersection isect;
        bool is_isect = false;
        auto t_first = t_max;
        isect.type = IsectType::None;
        // �V�F�C�v�Ƃ̌�������
        for (const auto& object : shape_list) {
            if (object->intersect(r, t_min, t_first, isect)) {
                is_isect = true;
                t_first = isect.t;
                isect.type = IsectType::Material;
                p = isect;
            }
        }
        // �����Ƃ̌�������
        for (const auto& light : light_list) {
            if (light->intersect(r, t_min, t_first, isect)) {
                is_isect = true;
                t_first = isect.t;
                isect.light = light;
                isect.type = IsectType::Light;
                p = isect;
            }
        }
        return is_isect;
    }

    /**
    * @brief ���C�ƃV�F�C�v�̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @return bool      :��������̌���
    */
    bool intersect_object(const Ray& r, float t_min, float t_max) const {
        intersection isect;
        bool is_isect = false;
        auto t_first = t_max;
        isect.type = IsectType::None;
        // �V�F�C�v�Ƃ̌�������
        for (const auto& object : shape_list) {
            if (object->intersect(r, t_min, t_first, isect)) {
                is_isect = true;
                t_first = isect.t;
                isect.type = IsectType::Material;
            }
        }
        return is_isect;
    }

    /**
    * @brief ���C�ƌ����̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @param[out] p     :�����_���
    * @return bool      :��������̌���
    * @note TODO: BVH�̎���
    */
    bool intersect_light(const Ray& r, float t_min, float t_max, intersection& p) const {
        intersection isect;
        bool is_isect = false;
        auto t_first = t_max;
        isect.type = IsectType::None;
        // �����Ƃ̌�������
        for (const auto& light : light_list) {
            if (light->intersect(r, t_min, t_first, isect)) {
                is_isect = true;
                t_first = isect.t;
                isect.light = light;
                isect.type = IsectType::Light;
                p = isect;
            }
        }
        return is_isect;
    }

private:
    std::vector<std::shared_ptr<Shape>> shape_list; /**< �V�[�����̃V�F�C�v */
    std::vector<std::shared_ptr<Light>> light_list; /**< �V�[�����̌���         */
};