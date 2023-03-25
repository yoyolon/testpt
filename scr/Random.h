/**
* @file  Random.h
* @brief ���������ƃT���v�����O
*/

#pragma once

#include "utility.h"
#include "Math.h"

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
    * @return Vec2 :�T���v�����O�l(x,y)���W
    */
    static Vec2 uniform_disk_sample();

    /**
    * @brief �P�ʉ~����̈�l�ȃT���v�����O(�c�݂�������)
    * @return Vec2 :�T���v�����O�l(x,y)���W
    * @note: �Q�l: https://psgraphics.blogspot.com/2011/01/
    */
    static Vec2 concentric_disk_sample();

    /**
    * @brief �O�p�`����̈�l�ȃT���v�����O(�d�S���W)
    * @return Vec3 :�T���v�����O�l(�d�S���W)
    */
    static Vec2 uniform_triangle_sample();

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
    * @brief ���K��Phong���z����U���������T���v�����O
    * @param[in] float :���z�̃p�����[�^
    * @return Vec3     :�T���v�����O�l
    * @note: [E.Lafortune and Y.Willems 1994]����Ɏ���
    */
    static Vec3 phong_sample(float shine);

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


/** 1D�敪�֐� */
class Piecewise1D {
    // �Q�l: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Sampling_Random_Variables
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] _f :���U�����ꂽ�֐��z��
    * @param[in] _n :�z��̗v�f��
    */
    Piecewise1D(const float* data, int n);

    int get_n() const { return n; }
    float get_f(int index) const { return f[index]; }
    float get_integral_f() const { return integral_f; }

    /**
    * @brief �t�֐��@��f(x)����x���T���v�����Ă���PDF(�m�����x)��]������֐�
    * @param[out] pdf   :�T���v�����OPDF
    * @param[out] index :�T���v�����O�l�̔z��C���f�b�N�X
    * @return float     :�T���v������x�̒l(f(x)�łȂ�x��Ԃ��̂Œ���)
    */
    float sample(float& pdf, int& index) const;

private:
    std::vector<float> f;   /**< 1D�敪�֐��̔z��      */
    int n;                  /**< �z��̗v�f��          */
    std::vector<float> cdf; /**< CDF(�ݐϕ��z�֐�)     */
    float integral_f;       /**< f���`��Őϕ������l */
};


/** 2D�敪�֐� */
class Piecewise2D {
    // �Q�l: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations
public:
    /**
    * @brief �R���X�g���N�^
    * @param[in] data :���U�����ꂽ�֐��z��
    * @param[in] _nu :u�����̗v�f��
    * @param[in] _nv :v�����̗v�f��
    */
    Piecewise2D(const float* data, int _nu, int _nv);

    int get_nu() const { return nu; }
    int get_nv() const { return nv; }

    /**
    * @brief �t�֐��@��f(u, v)����(u, v)���T���v�����Ă���PDF(�m�����x)��]������֐�
    * @param[out] pdf :�T���v�����OPDF
    * @return float   :�T���v������(u,v)�̒l(f(u,v)�łȂ�(u,v)��Ԃ��̂Œ���)
    */
    Vec2 sample(float& pdf) const;

    /**
    * @brief (u, v)���T���v�����O����PDF(�m�����x)��]������֐�
    * @param[out] uv:(u,v)���W
    * @return float   :�T���v������(u,v)�̒l(f(u,v)�łȂ�(u,v)��Ԃ��̂Œ���)
    */
    float eval_pdf(const Vec2& uv) const;

private:
    int nu; /**< u�����̗v�f�� */
    int nv; /**< v�����̗v�f�� */
    std::vector<std::unique_ptr<Piecewise1D>> conditional_pdf; /**< �����t���m�����x(p[u|v]) */
    std::unique_ptr<Piecewise1D> merginal_pdf; /**< ���ӊm�����x(p[v]) */
};