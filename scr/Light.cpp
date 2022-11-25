#include "Light.h"
#include "Shape.h"
#include "Random.h"
#include "Scene.h"

// *** 光源 ***
bool Light::IsVisible(const intersection& p1, const intersection& p2, const Scene& world) {
	Ray r = Ray(p1.pos, unit_vector(p2.pos - p1.pos));
	intersection isect;
	return !world.intersect(r, 0.001f, inf, isect);
}

// *** 面光源 ***
AreaLight::AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape, LightType _type) 
	: Light(_type), intensity(_intensity), shape(_shape)
{
	area = shape->area();
}

Vec3 AreaLight::emitte() const {
	return intensity;
}

Vec3 AreaLight::power() const {
	return intensity * area * 4 * pi;
}

bool AreaLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
	return shape->intersect(r, t_min, t_max, p);
}

float AreaLight::sample_pdf(const Vec3& wi, const intersection& p) const {
	return 1 / area; // 面光源を一様サンプリング
}

Vec3 AreaLight::sample_Li(Vec3& wo, float& pdf) {
	// NOTE: 実装手順
	// wiをローカル座標系に変換
	// 面光源の点をランダムにサンプル
	// pdfを計算
	// 交差点とサンプル点の可視判定(TODO: 可視じゃなかったらどうする?)
	// wiをグローバル座標系に変換
	return Vec3(0.0f, 0.0f, 0.0f);
}