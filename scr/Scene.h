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
    Scene() : envmap(nullptr) {}

    /**
    * @brief �R���X�g���N�^
    * @param[in]  map : ���}�b�v
    * @param[in]  w   : ���}�b�v�̕�
    * @param[in]  h   : ���}�b�v�̍���
    * @param[in]  c   : ���}�b�v�̃`�����l����
    */
    Scene(float* map, int w, int h, int c) {
        envmap = map;
        w_envmap = w;
        h_envmap = h;
        c_envmap = c;
    }

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
        isect.type = IsectType::Light;
        // �����Ƃ̌�������
        for (const auto& light : light_list) {
            if (light->intersect(r, t_min, t_first, isect)) {
                is_isect = true;
                t_first = isect.t;
                isect.light = light;
                p = isect;
            }
        }
        return is_isect;
    }

    /**
    * @brief ���}�b�v�̃T���v�����O
    * @param[in]  r :���C
    * @return Vec3  :���C�ɉ��������}�b�v�̕��ˋP�x
    * @note: TODO: IBL�������Ŏ���������폜
    */
    Vec3 sample_envmap(const Ray& r) const {
        if (envmap == nullptr) {
            return Vec3(0.0f, 0.0f, 0.0f);
        }
        Vec3 w = unit_vector(r.get_dir());
        float u = std::atan2(w.get_z(), w.get_x()) + pi;
        u *= invpi * 0.5;
        float v = std::acos(std::clamp(w.get_y(), -1.0f, 1.0f)) * invpi;
        // ���}�b�v������ˋP�x���T���v�����O
        int x = std::clamp((int)(w_envmap * u), 0, w_envmap-1);
        int y = std::clamp((int)(h_envmap * v), 0, h_envmap-1);
        int index = y * w_envmap * 3 + x * 3;
        float R = envmap[index++];
        float G = envmap[index++];
        float B = envmap[index];
        return Vec3(R, G, B);
    }

private:
    std::vector<std::shared_ptr<Shape>> shape_list; /**< �V�[�����̃V�F�C�v */
    std::vector<std::shared_ptr<Light>> light_list; /**< �V�[�����̌���         */
    // TODO: IBL�������Ŏ���������폜
    float* envmap;    /**< ���}�b�v               */
    int w_envmap = 0; /**< ���}�b�v�̍���         */
    int h_envmap = 0; /**< ���}�b�v�̕�           */
    int c_envmap = 0; /**< ���}�b�v�̃`�����l���� */
};