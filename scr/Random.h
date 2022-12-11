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
	static Vec3 uniform_disk_sample();
	static Vec3 concentric_disk_sample();
	static Vec3 uniform_sphere_sample();
	static Vec3 uniform_hemisphere_sample();
	static Vec3 cosine_hemisphere_sample();
	static Vec3 ggx_sample(float alpha);
	static Vec3 beckmann_sample(float alpha);
	static float balance_heuristic(int n1, float pdf1, int n2, float pdf2);
	static float power_heuristic(int n1, float pdf1, int n2, float pdf2, float beta=2.0f);
};

static std::mt19937 mt; // —”¶¬Ší