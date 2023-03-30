#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <vector>

class Vec3;

// *** �萔 ***

 // ����
constexpr float inf = std::numeric_limits<float>::infinity();

// �~����
constexpr float pi = 3.14159265359f;

// �~�����̋t��
constexpr float invpi = 1.0f / pi;

// �}�V���C�v�V����
constexpr float epsilon = std::numeric_limits<float>::epsilon();

// ��������p�}�V���C�v�V����(���Ȍ����������Ȃ��悤�Ɍo���I�Ɍ���)
constexpr float eps_isect = 0.01f;



// *** �֐� ***
/**
* @brief �K���}�␳�̃w���p�[�֐�
* @param[in]  c  :�K���}�␳�O�̐F�̗v�f
* @return Vfloat :�K���}�␳��̐F�̗v�f
*/
float gamma_correction_element(float c);

/**
* @brief �K���}�␳
* @param[in]  color :�K���}�␳�O�̐F
* @return Vec3      :�K���}�␳��̐F
*/
Vec3 gamma_correction(const Vec3& color);