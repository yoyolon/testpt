#include "Random.h"

// �����̏�����
void Random::init() {
	//std::random_device rd;
	//mt.seed(rd()); 
	mt.seed(1); // �V�[�h���Œ�
}

// ��l����[0,1]�𐶐�
float Random::uniform_float() {
	return uniform_float(0.0f, 1.0f);
}

// ��l����[min,max]�𐶐�
float Random::uniform_float(float min, float max) {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(mt);
}

// ��l����[min,max]�𐶐�
int Random::uniform_int(int min, int max) {
	std::uniform_int_distribution<> dist(min, max);
	return dist(mt);
}

// ��l�ȒP�ʋ��T���v��
Vec3 Random::uniform_sphere_sample() {
	float theta = 2 * pi * Random::uniform_float();
	float phi = 2 * pi * Random::uniform_float();
	float x = std::cos(phi) * std::cos(theta);
	float y = std::cos(phi) * std::sin(theta);
	float z = std::sin(phi);
	return Vec3(x, y, z);
}

// ��l�Ȕ����T���v��
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

// �]���ɏ]���������T���v��
Vec3 Random::cosine_hemisphere_sample() {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto z = std::sqrt(std::max(1.0f - v, 0.0f));
	auto phi = 2 * pi * u;
	auto x = std::cos(phi) * std::sqrt(v);
	auto y = std::sin(phi) * std::sqrt(v);
	return Vec3(x, y, z);
}

// GGX�̏d�_�I�T���v�����O[WMLT07]
Vec3 Random::ggx_sample(float alpha) {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto tan2_theta = alpha * alpha * u / (1.0f - u); // atan2�͒x���̂ŉ��
	auto cos2theta = 1 / (1 + tan2_theta);
	auto sin2theta = 1 - cos2theta;
	auto sin_theta = std::sqrt(std::max(sin2theta, 0.0f));
	auto phi = 2 * pi * v;
	auto z = std::sqrt(std::max(cos2theta, 0.0f));
	auto x = std::cos(phi) * sin_theta;
	auto y = std::sin(phi) * sin_theta;
	return Vec3(x, y, z);
}

// �x�b�N�}�����z�̏d�_�I�T���v�����O[WMLT07]
Vec3 Random::beckmann_sample(float alpha) {
	auto u = Random::uniform_float();
	auto v = Random::uniform_float();
	auto logs = std::log(1.0f - u);
	if (std::isinf(logs)) logs = 0.0f;
	auto tan2_theta = -alpha * alpha * logs; // atan2�͒x���̂ŉ��
	auto cos2theta = 1 / (1 + tan2_theta);
	auto sin2theta = 1 - cos2theta;
	auto sin_theta = std::sqrt(std::max(sin2theta, 0.0f));
	auto phi = 2 * pi * v;
	auto z = std::sqrt(std::max(cos2theta, 0.0f));
	auto x = std::cos(phi) * sin_theta;
	auto y = std::sin(phi) * sin_theta;
	return Vec3(x, y, z);
}

// ���d�d�_�I�T���v�����O�̏d�݌v�Z
// �Q�l: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Importance_Sampling
float Random::balance_heuristic(int n1, float pdf1, int n2, float pdf2) {
	return (n1 * pdf1) / (n1 * pdf1 + n2 * pdf2);
}
float Random::power_heuristic(int n1, float pdf1, int n2, float pdf2, int beta) {
	float e1 = std::pow(n1 * pdf1, beta);
	float e2 = std::pow(n2 * pdf2, beta);
	return e1 / (e1 + e2);

}