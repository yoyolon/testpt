#include "Random.h"

// �����̏�����
void Random::init() {
	//std::random_device rd; // �V�[�h�p�����𐶐�
	//mt.seed(rd()); 
	mt.seed(1);
}

// ��l����[0,1]�𐶐�
float Random::uniform_float() {
	return uniform_float(1.0f, 0.0f);
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

// GGX�̏d�_�I�T���v�����O
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