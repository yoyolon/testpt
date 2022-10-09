#pragma once

#include "utility.h"
#include "Vec3.h"

// *** —”¶¬ƒNƒ‰ƒX ***
class Random {
public:
	static void init();
	static float uniform_float();
	static float uniform_float(float min, float max);
	static int uniform_int(int min, int max);
	static Vec3 uniform_sphere_sample();
	static Vec3 uniform_hemisphere_sample();
	static Vec3 cosine_hemisphere_sample();
	static Vec3 GGX_sample(float roughness);
};

static std::mt19937 mt; // —”¶¬Ší