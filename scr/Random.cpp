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
	auto r1 = Random::uniform_float();
	auto r2 = Random::uniform_float();
	auto r = std::sqrt(std::max(1.0f - r1*r1, 0.0f));
	auto phi = 2 * pi * r2;
	auto x = std::cos(phi) * r;
	auto y = std::sin(phi) * r;
	auto z = r1;
	return Vec3(x, y, z);
}

// �]���ɏ]���������T���v��
Vec3 Random::cosine_hemisphere_sample() {
	auto r1 = Random::uniform_float();
	auto r2 = Random::uniform_float();
	auto z = std::sqrt(std::max(1.0f - r2, 0.0f));
	auto phi = 2 * pi * r1;
	auto x = std::cos(phi) * std::sqrt(r2);
	auto y = std::sin(phi) * std::sqrt(r2);
	return Vec3(x, y, z);
}
