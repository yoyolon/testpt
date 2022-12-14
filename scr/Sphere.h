/**
* @file  Spehre.h
* @brief ���E�~�ՁE�~���N���X
*/

#pragma once

#include "Material.h"
#include "Shape.h"

/** ���N���X */
class Sphere : public Shape {
public:
    /**
    * @brief ���S���W�Ɣ��a���狅�I�u�W�F�N�g��������
    * @param[in] c :���S���W
    * @param[in] r :���a
    * @param[in] m :�}�e���A��
    */
    Sphere(Vec3 c, float r, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    float sample_pdf(const intersection& ref, const Vec3& w) const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 center;                   /**< ���S���W   */
    float radius;                  /**< ���a       */
    std::shared_ptr<Material> mat; /**< �}�e���A�� */
};


/** �~�ՃN���X */
class Disk : public Shape {
public:
    /**
    * @brief ���S���W�Ɣ��a����~�ՃI�u�W�F�N�g��������
    * @param[in] c :���S���W
    * @param[in] r :���a
    * @param[in] m :�}�e���A��
    */
    Disk(Vec3 c, float r, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    float sample_pdf(const intersection& ref, const Vec3& w) const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 center;                   /**< ���S���W   */
    float radius;                  /**< ���a       */
    std::shared_ptr<Material> mat; /**< �}�e���A�� */
};


/** �~���N���X */
class Cylinder : public Shape {
public:
    /**
    * @brief ���S���W�Ɣ��a�ƍ�������~���I�u�W�F�N�g��������
    * @param[in] c :���S���W
    * @param[in] r :���a
    * @param[in] h :����
    * @param[in] m :�}�e���A��
    */
    Cylinder(Vec3 c, float r, float h, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    float sample_pdf(const intersection& ref, const Vec3& w) const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 center;                   /**< ���S���W   */
    float radius;                  /**< ���a       */
    float height;                  /**< ����       */
    std::shared_ptr<Material> mat; /**< �}�e���A�� */
};