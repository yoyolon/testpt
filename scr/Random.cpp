#include "Random.h"

// 乱数の初期化
void Random::init() {
	//std::random_device rd;
	//mt.seed(rd()); 
	mt.seed(1); // シードを固定
}

// 一様乱数[0,1]を生成
float Random::uniform_float() {
	return uniform_float(0.0f, 1.0f);
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

// 全球からの一様な方向サンプリング
Vec3 Random::uniform_sphere_sample() {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto z = 1 - 2 * u;
	auto r = std::sqrt(std::max(1.0f - z*z, 0.0f));
	auto phi = 2 * pi * v;
	auto x = std::cos(phi) * r;
	auto y = std::sin(phi) * r;
	return Vec3(x, y, z);
}

// 半球からの一様な方向サンプリング
Vec3 Random::uniform_hemisphere_sample() {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto z = u;
	auto r = std::sqrt(std::max(1.0f - z*z, 0.0f));
	auto phi = 2 * pi * v;
	auto x = std::cos(phi) * r;
	auto y = std::sin(phi) * r;
	return Vec3(x, y, z);
}

// 半球からの余弦に従ったサンプリング
Vec3 Random::cosine_hemisphere_sample() {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto z = std::sqrt(std::max(1.0f - v, 0.0f));
	auto phi = 2 * pi * u;
	auto x = std::cos(phi) * std::sqrt(v);
	auto y = std::sin(phi) * std::sqrt(v);
	return Vec3(x, y, z);
}

// GGX分布のハーフベクトル重点的サンプリング
Vec3 Random::ggx_sample(float alpha) {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto tan2_theta = alpha * alpha * u / (1.0f - u); // atan2は遅いので回避
	auto cos2theta = 1 / (1 + tan2_theta);
	auto sin2theta = 1 - cos2theta;
	auto sin_theta = std::sqrt(std::max(sin2theta, 0.0f));
	auto phi = 2 * pi * v;
	auto z = std::sqrt(std::max(cos2theta, 0.0f));
	auto x = std::cos(phi) * sin_theta;
	auto y = std::sin(phi) * sin_theta;
	return Vec3(x, y, z);
}

// ベックマン分布のハーフベクトル重点的サンプリング
Vec3 Random::beckmann_sample(float alpha) {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto logs = std::log(1.0f - u);
	if (std::isinf(logs)) logs = 0.0f;
	auto tan2_theta = -alpha * alpha * logs; // atan2は遅いので回避
	auto cos2theta = 1 / (1 + tan2_theta);
	auto sin2theta = 1 - cos2theta;
	auto sin_theta = std::sqrt(std::max(sin2theta, 0.0f));
	auto phi = 2 * pi * v;
	auto z = std::sqrt(std::max(cos2theta, 0.0f));
	auto x = std::cos(phi) * sin_theta;
	auto y = std::sin(phi) * sin_theta;
	return Vec3(x, y, z);
}

// 多重重点的サンプリングの重み計算
// 参考: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Importance_Sampling
float Random::balance_heuristic(int n1, float pdf1, int n2, float pdf2) {
	return (n1 * pdf1) / (n1 * pdf1 + n2 * pdf2);
}
float Random::power_heuristic(int n1, float pdf1, int n2, float pdf2, float beta) {
	float e1 = std::powf(n1 * pdf1, beta);
	float e2 = std::powf(n2 * pdf2, beta);
	return e1 / (e1 + e2);

}