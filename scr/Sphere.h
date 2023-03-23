/**
* @file  Spehre.h
* @brief ���E�~�ՁE�~���N���X
*/

#pragma once

#include "Shape.h"

class Material;

/** ���N���X */
class Sphere : public Shape {
public:
    /**
    * @brief ���S���W�Ɣ��a���狅�V�F�C�v��������
    * @param[in] c :���S���W
    * @param[in] r :���a
    * @param[in] m :�}�e���A��
    */
    Sphere(Vec3 c, float r, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

    /**
    * @brief �W�I���g���T���v�����O�̊m�����x���v�Z����֐�(�I�[�o�[���C�h)
    * @param[in] p  :�T���v�����O���̌����_���
    * @param[in] w  :�T���v�����O����(�����Ɍ�������������)
    * @return float :�m�����x
    * @note ���̉��̈���l�����Ĕ������������l�T���v�����O
    */
    float eval_pdf(const intersection& ref, const Vec3& w) const override;

private:
    Vec3 center;                   /**< ���S���W   */
    float radius;                  /**< ���a       */
    std::shared_ptr<Material> mat; /**< �}�e���A�� */
};


/** �~�ՃN���X */
class Disk : public Shape {
public:
    /**
    * @brief ���S���W�Ɣ��a����~�ՃV�F�C�v��������
    * @param[in] c       :���S���W
    * @param[in] r       :���a
    * @param[in] m       :�}�e���A��
    * @param[in] is_flip :�@���𔽓]���邩�ǂ���
    */
    Disk(Vec3 c, float r, std::shared_ptr<Material> m, bool is_flip=false);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 center;                   /**< ���S���W     */
    float radius;                  /**< ���a         */
    std::shared_ptr<Material> mat; /**< �}�e���A��   */
    bool is_flip_normal;           /**< �@���̔��]   */
    const static Vec3 normal;      /**< �@��(������) */
};


/** �~���N���X */
class Cylinder : public Shape {
public:
    /**
    * @brief ���S���W�Ɣ��a�ƍ�������~���V�F�C�v��������
    * @param[in] c :���S���W
    * @param[in] r :���a
    * @param[in] h :����
    * @param[in] m :�}�e���A��
    */
    Cylinder(Vec3 c, float r, float h, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 center;                   /**< ���S���W   */
    float radius;                  /**< ���a       */
    float height;                  /**< ����       */
    std::shared_ptr<Material> mat; /**< �}�e���A�� */
};