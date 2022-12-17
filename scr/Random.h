/**
* @file  Random.h
* @brief ���������ƃT���v�����O
*/

#pragma once

#include "utility.h"
#include "Vec3.h"

/** ���������N���X */
class Random {
public:
    /**
    * @brief �����̏���������֐�
    */
    static void init();

    /**
    * @brief ��l����[0, 1]�𐶐�����֐�
    * @return float :�T���v�����O�l
    */
    static float uniform_float();

    /**
    * @brief ��l����[min, max]�𐶐�����֐�
    * @param[in] min :�ŏ��l
    * @param[in] max :�ő�l
    * @return float  :�T���v�����O�l
    */
    static float uniform_float(float min, float max);

    /**
    * @brief ������l����[min, max]�𐶐�����֐�
    * @param[in] min :�ŏ��l
    * @param[in] max :�ő�l
    * @return int    :�T���v�����O�l
    */
    static int uniform_int(int min, int max);

    /**
    * @brief �P�ʉ~����̈�l�ȃT���v�����O
    * @return Vec3 :�T���v�����O�l
    */
    static Vec3 uniform_disk_sample();

    /**
    * @brief �P�ʉ~����̈�l�ȃT���v�����O(�c�݂�������)
    * @return Vec3 :�T���v�����O�l
    * @note: �Q�l: https://psgraphics.blogspot.com/2011/01/
    */
    static Vec3 concentric_disk_sample();

    /**
    * @brief �O�p�`����̈�l�ȃT���v�����O(�d�S���W)
    * @return Vec3 :�T���v�����O�l
    */
    static Vec3 uniform_triangle_sample();

    /**
    * @brief �P�ʋ�����̈�l�ȕ����x�N�g�����T���v�����O
    * @return Vec3 :�T���v�����O�l
    */
    static Vec3 uniform_sphere_sample();

    /**
    * @brief �P�ʔ�������̈�l�ȕ����x�N�g�����T���v�����O
    * @return Vec3 :�T���v�����O�l
    */
    static Vec3 uniform_hemisphere_sample();

    /**
    * @brief �P�ʔ�������̃R�T�C���d�݂̕����T���v�����O
    * @return Vec3 :�T���v�����O�l
    * @note: �R�T�C���͖@���ƃT���v�����O�����̂Ȃ��p
    * @note: Malley�̎�@�𗘗p
    */
    static Vec3 cosine_hemisphere_sample();

    /**
    * @brief Trowbridge-Reitz(GGX)���z����n�[�u�x�N�g�����T���v�����O
    * @param[in] float :���z�̃p�����[�^
    * @return Vec3     :�T���v�����O�l
    * @note: �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
    */
    static Vec3 ggx_sample(float alpha);

    /**
    * @brief Beckmann���z����n�[�u�x�N�g�����T���v�����O
    * @param[in] float :���z�̃p�����[�^
    * @return Vec3     :�T���v�����O�l
    * @note: �Q�l: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
    */
    static Vec3 beckmann_sample(float alpha);

    /**
    * @brief ���d�d�_�I�T���v�����O�̏d�݂��v�Z����֐�
    * @param[in] n1   :1�ڂ̊֐��̃T���v����
    * @param[in] pdf1 :1�ڂ̊֐��̊m�����x
    * @param[in] n2   :2�ڂ̊֐��̃T���v����
    * @param[in] pdf2 :2�ڂ̊֐��̊m�����x
    * @return float   :�d��
    * @note: �Q�l: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Importance_Sampling
    */
    static float balance_heuristic(int n1, float pdf1, int n2, float pdf2);

    /**
    * @brief ���d�d�_�I�T���v�����O�̏d�݂��v�Z����֐�
    * @param[in] n1   :1�ڂ̊֐��̃T���v����
    * @param[in] pdf1 :1�ڂ̊֐��̊m�����x
    * @param[in] n2   :2�ڂ̊֐��̃T���v����
    * @param[in] pdf2 :2�ڂ̊֐��̊m�����x
    * @return float   :�d��
    * @note: �Q�l: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Importance_Sampling
    */
    static float power_heuristic(int n1, float pdf1, int n2, float pdf2, float beta=2.0f);
};

static std::mt19937 mt; /**< ���������� */