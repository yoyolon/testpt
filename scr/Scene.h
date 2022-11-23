#pragma once

#include "Shape.h"
#include "Light.h"

// *** シーンクラス ***
class Scene {
public:
	Scene() : envmap(nullptr) {}
	Scene(std::shared_ptr<Shape> _object) : envmap(nullptr) { add(_object); }
	Scene(const char* filename) { 
		envmap = nullptr;
		// TODO: ファイル読み込みをmain関数で実行
		//envmap = stbi_loadf(filename, &w_envmap, &h_envmap, &c_envmap, 0);
	}

	// シーンにオブジェクト/ライトを追加
	void add(std::shared_ptr<Shape> object) { object_list.push_back(object); }
	void add(std::shared_ptr<Light> light) { light_list.push_back(light); }
	void clear() { object_list.clear(); }

	// レイとシーンの交差判定
	// TODO: isectを光源対応
	// TODO: VBHによる高速化
	bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
		intersection isect;
		bool is_isect = false;
		auto t_first = t_max;
		isect.type = IsectType::None;
		// オブジェクトとの交差判定
		for (const auto& object : object_list) {
			if (object->intersect(r, t_min, t_first, isect)) {
				is_isect = true;
				t_first = isect.t;
				isect.type = IsectType::Material;
				p = isect;
			}
		}
		// 光源との交差判定
		for (const auto& light : light_list) {
			if (light->intersect(r, t_min, t_first, isect)) {
				is_isect = true;
				t_first = isect.t;
				isect.light = light;
				isect.type = IsectType::Light;
				p = isect;
			}
		}
		return is_isect;
	}

	// 背景からサンプリング
	Vec3 sample_envmap(const Ray& r) const {
		if (envmap == nullptr) return Vec3(0.0f,0.0f,0.0f);
		Vec3 w = unit_vector(r.get_dir());
		float u = std::atan2(w.get_z(), w.get_x()) + pi;
		u *= invpi * 0.5;
		float v = std::acos(std::clamp(w.get_y(), -1.0f, 1.0f)) * invpi;
		// テクスチャからサンプル
		int x = std::clamp((int)(w_envmap * u), 0, w_envmap-1);
		int y = std::clamp((int)(h_envmap * v), 0, h_envmap-1);
		int index = y * w_envmap * 3 + x * 3;
		float R = envmap[index++];
		float G = envmap[index++];
		float B = envmap[index];
		return Vec3(R, G, B);
	}

private:
	std::vector<std::shared_ptr<Shape>> object_list; // オブジェクト
	std::vector<std::shared_ptr<Light>> light_list; // シーン中の光源
	float* envmap;
	int w_envmap, h_envmap, c_envmap;
};