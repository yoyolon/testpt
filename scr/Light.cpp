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
AreaLight::AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape, int _type) 
	: Light(_type), intensity(_intensity), shape(_shape) {}

Vec3 AreaLight::emitte(const Vec3& wi, const intersection& p, float& pdf) {
	// 実装手順
	// pdfを計算
	// 交差点とサンプル点の可視判定(TODO: 可視じゃなかったらどうする?)
	// pdfを計算: pdf=1/shape.area()になる(一様サンプリングのため)
	return intensity;
}

float AreaLight::sample_pdf(const Vec3& wi, const intersection& p) const {
	// wiをローカル座標系に変換
	// 面光源の点をランダムにサンプル
	// wiをグローバル座標系に変換
}