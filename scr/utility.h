/**
* @file  utility.h
* @brief �֗��Ȓ萔��֐��Ȃǂ�����
*/

#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

class Vec3;

// *** �萔 ***

 // ����
constexpr float inf = std::numeric_limits<float>::infinity();

// �~����
constexpr float pi = 3.14159265359f;

// �~�����̔���
constexpr float pi_half = pi / 2;

// �~�����̋t��
constexpr float invpi = 1.0f / pi;

// �}�V���C�v�V����
constexpr float epsilon = std::numeric_limits<float>::epsilon();

// ��������p�}�V���C�v�V����(���Ȍ����������Ȃ��悤�Ɍo���I�Ɍ���)
constexpr float eps_isect = 0.01f;


// *** �֐� ***

/**
* @brief sRGB�̃K���}�␳�̃w���p�[�֐�
* @param[in]  c  :�K���}�␳�O�̐F�̗v�f
* @return Vfloat :�K���}�␳��̐F�̗v�f
*/
float gamma_correction_element(float c);


/**
* @brief sRGB�̃K���}�␳���s���֐�
* @param[in]  color :�K���}�␳�O�̐F
* @return Vec3      :�K���}�␳��̐F
*/
Vec3 gamma_correction(const Vec3& color);


/**
* @brief sRGB�̋t�K���}�␳�̃w���p�[�֐�
* @param[in]  c  :�t�K���}�␳�O�̐F�̗v�f
* @return Vfloat :�t�K���}�␳��̐F�̗v�f
*/
float inv_gamma_correction_element(float c);


/**
* @brief sRGB�̋t�K���}�␳���s���֐�
* @param[in]  color :�t�K���}�␳�O�̐F
* @return Vec3      :�t�K���}�␳��̐F
*/
Vec3 inv_gamma_correction(const Vec3& color);


/**
* @brief �P�x���疳���Ȓl(NaN��inf)�����O����֐�
* @param[in]  color :�P�x
* @return Vec3      :�L���l�ɕϊ���̒l
*/
Vec3 exclude_invalid(const Vec3& color);


/**
* @brief ��������w�肵�������ŕ�������֐�
* @param[in]  line      :�������镶����
* @param[in]  delimiter :��؂蕶��
* @return std::vector<std::string> ������̕�����̔z��
*/
std::vector<std::string> split_string(const std::string& line, char delimiter = ' ');