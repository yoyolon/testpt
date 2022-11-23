#pragma once

#include "Ray.h"

enum class IsectType {
	None = 1, Material = 2, Light = 4
};

// *** 交差点情報 ***
// TODO: 面光源への対応
struct intersection {
	Vec3 pos;    // 交差点
	Vec3 normal; // 法線
	float t;     // レイのパラメータ
	std::shared_ptr<class Material> mat; // マテリアル
	std::shared_ptr<class Light> light; // 光源
	IsectType type; // 交差点の種類(光源orマテリアル)
};


// *** 形状モデルクラス ***
class Shape {
public:
	virtual ~Shape() {};
	// レイとシェイプの交差判定
	virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;
	// 面積
	virtual float area() const = 0;
};