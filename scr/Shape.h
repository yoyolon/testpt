/**
* @file  Shape.h
* @brief �O�����`�󃂃f���ƌ����_�\����
*/

#pragma once

#include <vector>
#include "Ray.h"

class Material;


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
    Disk(Vec3 c, float r, std::shared_ptr<Material> m, bool is_flip = false);

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


/** �O�p�`�N���X */
class Triangle : public Shape {
public:
    Triangle();

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
    std::shared_ptr<Material> mat; /**< �}�e���A�� */

};


/** �O�p�`���b�V���N���X */
class TriangleMesh : public Shape {
public:
    TriangleMesh();

    /**
    * @brief ���_�z��ƃC���f�b�N�X�z�񂩂�O�p�`���b�V���V�F�C�v��������
    * @param[in] vertices :�O�p�`�̒��_�z��
    * @param[in] indices  :�O�p�`�̃C���f�b�N�X�z��
    * @param[in] m        :�}�e���A��
    * @param[in] p        :�ʒu�I�t�Z�b�g
    */
    TriangleMesh(std::vector<Vec3> vertices, std::vector<Vec3> indices, std::shared_ptr<Material> m);

    /**
    * @brief obj�t�@�C������O�p�`���b�V���V�F�C�v��������
    * @param[in] filenames :�O�p�`�̒��_�z��
    * @param[in] m         :�}�e���A��
    * @param[in] p         :�ʒu�I�t�Z�b�g
    * @param[in] is_smooth :�X���[�Y�V�F�[�f�B���O�̐ݒ�
    */
    TriangleMesh(std::string filename, std::shared_ptr<Material> m, bool is_smooth = true);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    std::vector<Triangle> Triangles; /**< �O�p�`�z��     */
    std::shared_ptr<Material> mat;   /**< �}�e���A��     */
};