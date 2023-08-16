/**
* @file  utility.h
* @brief 便利な定数や関数などを実装
*/

#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

class Vec3;

// *** 定数 ***

 // 無限
constexpr float inf = std::numeric_limits<float>::infinity();

// 円周率
constexpr float pi = 3.14159265359f;

// 円周率の半分
constexpr float pi_half = pi / 2;

// 円周率の逆数
constexpr float invpi = 1.0f / pi;

// マシンイプシロン
constexpr float epsilon = std::numeric_limits<float>::epsilon();

// 交差判定用マシンイプシロン(自己交差が生じないように経験的に決定)
constexpr float eps_isect = 0.01f;


// *** 関数 ***

/**
* @brief sRGBのガンマ補正のヘルパー関数
* @param[in]  c  :ガンマ補正前の色の要素
* @return Vfloat :ガンマ補正後の色の要素
*/
float gamma_correction_element(float c);


/**
* @brief sRGBのガンマ補正を行う関数
* @param[in]  color :ガンマ補正前の色
* @return Vec3      :ガンマ補正後の色
*/
Vec3 gamma_correction(const Vec3& color);


/**
* @brief sRGBの逆ガンマ補正のヘルパー関数
* @param[in]  c  :逆ガンマ補正前の色の要素
* @return Vfloat :逆ガンマ補正後の色の要素
*/
float inv_gamma_correction_element(float c);


/**
* @brief sRGBの逆ガンマ補正を行う関数
* @param[in]  color :逆ガンマ補正前の色
* @return Vec3      :逆ガンマ補正後の色
*/
Vec3 inv_gamma_correction(const Vec3& color);


/**
* @brief 輝度から無効な値(NaNやinf)を除外する関数
* @param[in]  color :輝度
* @return Vec3      :有効値に変換後の値
*/
Vec3 exclude_invalid(const Vec3& color);


/**
* @brief 文字列を指定した文字で分割する関数
* @param[in]  line      :分割する文字列
* @param[in]  delimiter :区切り文字
* @return std::vector<std::string> 分割後の文字列の配列
*/
std::vector<std::string> split_string(const std::string& line, char delimiter = ' ');