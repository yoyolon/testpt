#pragma once

#include <limits>
#include <memory>
#include <vector>
#include <cmath>
#include <random>

// *** ’è” ***
constexpr float inf = std::numeric_limits<float>::infinity();
constexpr float pi = 3.14159265359f;
constexpr float invpi = 1.0f / pi;
constexpr float epsilon = std::numeric_limits<float>::epsilon();

// *** ŠÖ” ***
inline float radian2digree(float radian) { return 180.0f * radian / pi; }
inline float digree2radian(float degree) { return pi * degree / 180.0f; }