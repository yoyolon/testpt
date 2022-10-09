#pragma once

#include "Ray.h"

// *** 交差点情報 ***
struct intersection {
	Vec3 pos;    // 交差点
	Vec3 normal; // 法線
	float t;     // レイのパラメータ
	std::shared_ptr<class Material> mat; // マテリアル
};


// *** 形状モデルクラス ***
class Shape {
public:
	virtual ~Shape() {};
	// レイとシェイプの交差判定
	virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;
};