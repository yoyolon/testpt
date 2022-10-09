#pragma once

#include "Shape.h"

// *** シーンクラス ***
class scene : Shape {
public:
	scene() {}
	scene(std::shared_ptr<Shape> _object) { add(_object); }

	// シーンにオブジェクトを追加
	void add(std::shared_ptr<Shape> object) { object_list.push_back(object); }
	void clear() { object_list.clear(); }

	// レイとシーンの交差判定
	// TODO: VBHによる高速化
	bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
		intersection isect;
		bool is_isect = false;
		auto t_first = t_max;
		for (const auto& object : object_list) {
			if (object->intersect(r, t_min, t_first, isect)) {
				is_isect = true;
				t_first = isect.t;
				p = isect;
			}
		}
		return is_isect;
	}

private:
	std::vector<std::shared_ptr<Shape>> object_list; // シーン中のオブジェクト
};