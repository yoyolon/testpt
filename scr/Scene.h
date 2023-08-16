/**
* @file Scene.h
* @brief �V�[�����Ǘ�����N���X
*/

#pragma once

#include <memory>
#include <vector>
#include "Math.h"

struct intersection;
class Light;
class Material;
class Ray;
class Shape;

// *** �V�[���N���X ***
class Scene {
public:
    /**
    * @brief �R���X�g���N�^
    */
    Scene() : bg_color(Vec3::zero) {}

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
    * @brief �V�[�����̃V�F�C�v���󂩔���
    * @return bool :��Ȃ�true
    */
    bool is_empty_shape_list() const { return shape_list.empty(); }

    /**
    * @brief �V�[������S�ẴV�F�C�v�ƌ���������
    */
    void clear() { 
        shape_list.clear();
        light_list.clear();
    }

    /**
    * @brief �V�[���̑S�V�F�C�v���擾
    * @return std::vector<std::shared_ptr<Shape>> :�V�[�����̃V�F�C�v�̏W��
    */
    std::vector<std::shared_ptr<Shape>> get_shape() const { return shape_list; }

    /**
    * @brief �V�[���̑S�������擾
    * @return std::vector<std::shared_ptr<Light>> :�V�[�����̌����̏W��
    */
    std::vector<std::shared_ptr<Light>> get_light() const { return light_list; }

    Vec3 get_bg_color() const { return bg_color; }
    void set_bg_color(Vec3 color) { bg_color = color; }

    /**
    * @brief ���C�ƃI�u�W�F�N�g(�V�F�C�v�ƌ���)�̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @param[out] p     :�����_���
    * @return bool      :��������̌���
    * @note TODO: BVH�̎���
    */
    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const;

    /**
    * @brief ���C�ƃV�F�C�v�̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @return bool      :��������̌���
    * @note TODO: BVH�̎���
    */
    bool intersect_object(const Ray& r, float t_min, float t_max) const;

    /**
    * @brief ���C�ƌ����̌���������s���֐�
    * @param[in]  r     :���˃��C
    * @param[in]  t_min :���˃��C�̃p�����[�^����
    * @param[in]  t_max :���˃��C�̃p�����[�^����
    * @param[out] p     :�����_���
    * @return bool      :��������̌���
    * @note TODO: BVH�̎���
    */
    bool intersect_light(const Ray& r, float t_min, float t_max, intersection& p) const;


private:
    std::vector<std::shared_ptr<Shape>> shape_list; /**< �V�[�����̃V�F�C�v */
    std::vector<std::shared_ptr<Light>> light_list; /**< �V�[�����̌���     */
    Vec3 bg_color; /**< �w�i�F */
};