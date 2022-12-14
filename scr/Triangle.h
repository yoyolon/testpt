/**
* @file  Triangle.h
* @brief �O�p�`�N���X�ƎO�p�`���b�V���N���X
*/

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Material.h"
#include "Ray.h"
#include "Shape.h"

/** �O�p�`�N���X */
class Triangle : public Shape {
public:
    Triangle();

    /**
    * @brief ���_����O�p�`�I�u�W�F�N�g��������
    * @param[in] v0 :�O�p�`�̒��_
    * @param[in] v1 :�O�p�`�̒��_
    * @param[in] v2 :�O�p�`�̒��_
    * @param[in] m  :�}�e���A��
    */
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<Material> m);

    /**
    * @brief ���_�Ƃ��̖@������O�p�`�I�u�W�F�N�g��������
    * @param[in] v0 :�O�p�`�̒��_
    * @param[in] v1 :�O�p�`�̒��_
    * @param[in] v2 :�O�p�`�̒��_
    * @param[in] n0 :���_v0�̖@��
    * @param[in] n1 :���_v1�̖@��
    * @param[in] n2 :���_v2�̖@��
    * @param[in] m  :�}�e���A��
    */
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 n0, Vec3 n1, Vec3 n2, std::shared_ptr<Material> m);

    void move(Vec3 pos);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    float sample_pdf(const intersection& ref, const Vec3& w) const override;

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
    * @brief ���_�z��ƃC���f�b�N�X�z�񂩂�O�p�`���b�V���I�u�W�F�N�g��������
    * @param[in] vertices :�O�p�`�̒��_�z��
    * @param[in] indices  :�O�p�`�̃C���f�b�N�X�z��
    * @param[in] m        :�}�e���A��
    * @param[in] p        :�ʒu�I�t�Z�b�g
    */
    TriangleMesh(std::vector<Vec3> vertices, std::vector<Vec3>indices, std::shared_ptr<Material> m, Vec3 p);

    /**
    * @brief obj�t�@�C������O�p�`���b�V���I�u�W�F�N�g��������
    * @param[in] filenames :�O�p�`�̒��_�z��
    * @param[in] m         :�}�e���A��
    * @param[in] p         :�ʒu�I�t�Z�b�g
    * @param[in] is_smooth :�X���[�Y�V�F�[�f�B���O�̐ݒ� 
    */
    TriangleMesh(std::string filename, std::shared_ptr<Material> m, Vec3 p, bool is_smooth=true);

    void move(Vec3 pos);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    float sample_pdf(const intersection& ref, const Vec3& w) const override;

    intersection sample(const intersection& ref) const override;

private:
    std::vector<Triangle> Triangles; /**< �O�p�`�z��     */
    std::shared_ptr<Material> mat;   /**< �}�e���A��     */
    Vec3 pos;                        /**< �ʒu�I�t�Z�b�g */
};