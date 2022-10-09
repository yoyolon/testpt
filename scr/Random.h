#pragma once

#include "utility.h"
#include "Vec3.h"

// *** 乱数生成クラス ***
class Random {
public:
	static void init();
	static float uniform_float();
	static float uniform_float(float min, float max);
	static int uniform_int(int min, int max);
	static Vec3 uniform_sphere_sample();
	static Vec3 uniform_hemisphere_sample();
	static Vec3 cosine_hemisphere_sample();
	static Vec3 GGX_sample(float alpha);
	static Vec3 Beckmann_sample(float alpha);
};

static std::mt19937 mt; // 乱数生成器