/**
* @file  Shape.h
* @brief �O�����`�󃂃f���ƌ����_�\����
*/

#pragma once

#include <vector>
#include "Math.h"

class Material;
class Light;
class Ray;


/** �����_�̎�� */
enum class IsectType {
    None     = 1 << 0,  /**< �Ȃ�     */
    Material = 1 << 1,  /**< ���̕\�� */
    Light    = 1 << 2   /**< ����     */
};

/** �����_��� */
struct intersection {
    Vec3 pos;                              /**< ���W             */
    Vec3 normal;                           /**< �@��             */
    float t=0.f;                           /**< ���C�̃p�����[�^ */
    bool is_front=true;                    /**< �����_�̗��\     */
    IsectType type=IsectType::None;        /**< �����_�̎��     */
    std::shared_ptr<Material> mat=nullptr; /**< �ގ��̎��       */
    std::shared_ptr<Light> light=nullptr;  /**< �����̎��       */
};


/** �O�����`�󃂃f�����ۃN���X */
class Shape {
public:
    /**
    * @brief �V�F�C�v���}�e���A���ŏ�����
    * @param[in] m :�V�F�C�v�ɐݒ肷��}�e���A��
    */
    Shape(std::shared_ptr<Material> m);

    virtual ~Shape() {};

    void set_mat(const std::shared_ptr<Material> m) { mat = m; }

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
    * @brief �V�F�C�v��̓_���T���v�����O�����ꍇ�̗��̊p�Ɋւ���m�����x��]������֐�
    * @param[in] ref :�T���v�����O���̌����_���
    * @param[in] w   :�T���v�����O����(�W�I���g���Ɍ�������������)
    * @return float  :���̊p�Ɋւ���m�����x
    * @detail �T���v�����O�͗��̊p�Ɋւ��čs��
    */
    float eval_pdf(const intersection& ref, const Vec3& w) const;

    /**
    * @brief �V�F�C�v��̓_�T���v�����O����֐�
    * @param[in] ref       :�T���v�����O���̌����_���
    * @return intersection :�T���v�����������_���
    */
    virtual intersection sample(const intersection& ref) const = 0;

protected:
    std::shared_ptr<Material> mat; /**< �}�e���A�� */
};


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

private:
    Vec3 center;                   /**< ���S���W   */
    float radius;                  /**< ���a       */
};


/** �O�p�`�N���X */
class Triangle : public Shape {
public:
    /**
    * @brief ���_����O�p�`�V�F�C�v��������
    * @param[in] v0 :�O�p�`�̒��_
    * @param[in] v1 :�O�p�`�̒��_
    * @param[in] v2 :�O�p�`�̒��_
    * @param[in] m  :�}�e���A��
    */
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<Material> m);

    /**
    * @brief ���_�Ƃ��̖@������O�p�`�V�F�C�v��������
    * @param[in] v0 :�O�p�`�̒��_
    * @param[in] v1 :�O�p�`�̒��_
    * @param[in] v2 :�O�p�`�̒��_
    * @param[in] n0 :���_v0�̖@��
    * @param[in] n1 :���_v1�̖@��
    * @param[in] n2 :���_v2�̖@��
    * @param[in] m  :�}�e���A��
    */
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 n0, Vec3 n1, Vec3 n2, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 V0, V1, V2;               /**< ���_       */
    Vec3 N0, N1, N2;               /**< �@��       */
};


/** �O�p�`���b�V���N���X */
class TriangleMesh : public Shape {
public:
    /**
    * @brief ���_�z��ƃC���f�b�N�X�z�񂩂�O�p�`���b�V���V�F�C�v��������
    * @param[in] vertices :�O�p�`�̒��_�z��
    * @param[in] indices  :�O�p�`�̃C���f�b�N�X�z��
    * @param[in] m        :�}�e���A��
    */
    TriangleMesh(std::vector<Vec3> vertices, std::vector<Vec3> indices, 
                 std::shared_ptr<Material> m);

    /**
    * @brief .obj�t�@�C������O�p�`���b�V���V�F�C�v��������
    * @param[in] filename  :.obj�t�@�C���̃p�X
    * @param[in] m         :�}�e���A��
    * @param[in] is_smooth :true�Ȃ�X���[�Y�V�F�[�f�B���O��K�p����
    */
    TriangleMesh(std::string filename, std::shared_ptr<Material> m, bool is_smooth=true);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    std::vector<Triangle> Triangles; /**< �O�p�`�z��     */
};