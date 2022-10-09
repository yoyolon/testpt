#include "Random.h"

// 乱数の初期化
void Random::init() {
	//std::random_device rd; // シード用乱数を生成
	//mt.seed(rd()); 
	mt.seed(1);
}

// 一様乱数[0,1]を生成
float Random::uniform_float() {
	return uniform_float(1.0f, 0.0f);
}

// 一様乱数[min,max]を生成
float Random::uniform_float(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(mt);
}

// 一様乱数[min,max]を生成
int Random::uniform_int(int min, int max) {
	std::uniform_int_distribution<> dist(min, max);
	return dist(mt);
}

// 一様な単位球サンプル
Vec3 Random::uniform_sphere_sample() {
	float theta = 2 * pi * Random::uniform_float();
	float phi = 2 * pi * Random::uniform_float();
	float x = std::cos(phi) * std::cos(theta);
	float y = std::cos(phi) * std::sin(theta);
	float z = std::sin(phi);
	return Vec3(x, y, z);
}

// 一様な半球サンプル
Vec3 Random::uniform_hemisphere_sample() {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto r = std::sqrt(std::max(1.0f - u*u, 0.0f));
	auto phi = 2 * pi * v;
	auto x = std::cos(phi) * r;
	auto y = std::sin(phi) * r;
	auto z = u;
	return Vec3(x, y, z);
}

// 余弦に従った半球サンプル
Vec3 Random::cosine_hemisphere_sample() {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto z = std::sqrt(std::max(1.0f - v, 0.0f));
	auto phi = 2 * pi * u;
	auto x = std::cos(phi) * std::sqrt(v);
	auto y = std::sin(phi) * std::sqrt(v);
	return Vec3(x, y, z);
}

// GGXの重点的サンプリング
Vec3 Random::GGX_sample(float roughness) {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto phi = 2 * pi * u;
	auto theta = std::atan2(roughness * std::sqrt(v), std::sqrt(1-v));
	auto x = std::cos(phi) * std::sin(theta);
	auto y = std::sin(phi) * std::sin(theta);
	auto z = std::cos(theta);
	return Vec3(x, y, z);
}