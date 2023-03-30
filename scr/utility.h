#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <vector>

class Vec3;

// *** 定数 ***

 // 無限
constexpr float inf = std::numeric_limits<float>::infinity();

// 円周率
constexpr float pi = 3.14159265359f;

// 円周率の逆数
constexpr float invpi = 1.0f / pi;

// マシンイプシロン
constexpr float epsilon = std::numeric_limits<float>::epsilon();

// 交差判定用マシンイプシロン(自己交差が生じないように経験的に決定)
constexpr float eps_isect = 0.01f;



// *** 関数 ***
/**
* @brief ガンマ補正のヘルパー関数
* @param[in]  c  :ガンマ補正前の色の要素
* @return Vfloat :ガンマ補正後の色の要素
*/
float gamma_correction_element(float c);

/**
* @brief ガンマ補正
* @param[in]  color :ガンマ補正前の色
* @return Vec3      :ガンマ補正後の色
*/
Vec3 gamma_correction(const Vec3& color);