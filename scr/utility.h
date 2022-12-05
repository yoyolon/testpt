#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <vector>

// *** ’è” ***
constexpr float inf = std::numeric_limits<float>::infinity();
constexpr float pi = 3.14159265359f;
constexpr float invpi = 1.0f / pi;
constexpr float epsilon = std::numeric_limits<float>::epsilon();

// *** ŠÖ” ***
inline float to_degree(float radian) { return 180.0f * radian / pi; }
inline float to_radian(float degree) { return pi * degree / 180.0f; }