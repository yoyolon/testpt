/**
* @file Scene.h
* @brief シーン中のオブジェクトを管理
*/

#pragma once

#include "Shape.h"
#include "Light.h"

// *** シーンクラス ***
class Scene {
public:
    /**
    * @brief デフォルトコンストラクタ
    */
    Scene() : envmap(nullptr) {}

    /**
    * @brief デフォルトコンストラクタ
    * @param[in]  map : 環境マップ
    * @param[in]  w   : 環境マップの幅
    * @param[in]  h   : 環境マップの高さ
    * @param[in]  c   : 環境マップのチャンネル数
    */
    Scene(float* map, int w, int h, int c) {
        envmap = map;
        w_envmap = w;
        h_envmap = h;
        c_envmap = c;
    }

    /**
    * @brief シーンにオブジェクトを追加
    * @param[in]  object :オブジェクト
    */
    void add(std::shared_ptr<Shape> object) { object_list.push_back(object); }

    /**
    * @brief シーンに光源を追加
    * @param[in]  light :光源
    */
    void add(std::shared_ptr<Light> light) { light_list.push_back(light); }

    /**
    * @brief シーンからオブジェクトと光源を除去
    */
    void clear() { object_list.clear(); }

    /**
    * @brief シーンのオブジェクトを取得
    * @return std::vector<std::shared_ptr<Shape>> :シーンのオブジェクト
    */
    std::vector<std::shared_ptr<Shape>> get_shape() const { return object_list; }

    /**
    * @brief シーンの光源を取得
    * @return std::vector<std::shared_ptr<Light>> :シーンの光源
    */
    std::vector<std::shared_ptr<Light>> get_light() const { return light_list; }

    /**
    * @brief レイとオブジェクト(光源も含む)の交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果
    * @note TODO: BVHの実装
    */
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

    /**
    * @brief レイとオブジェクトの交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果
    */
    bool intersect_object(const Ray& r, float t_min, float t_max, intersection& p) const {
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
        return is_isect;
    }

    /**
    * @brief 環境マップのサンプリング
    * @param[in]  r :レイ
    * @return Vec3  :レイに沿った環境マップの放射輝度
    */
    Vec3 sample_envmap(const Ray& r) const {
        if (envmap == nullptr) {
            return Vec3(0.0f, 0.0f, 0.0f);
        }
        Vec3 w = unit_vector(r.get_dir());
        float u = std::atan2(w.get_z(), w.get_x()) + pi;
        u *= invpi * 0.5;
        float v = std::acos(std::clamp(w.get_y(), -1.0f, 1.0f)) * invpi;
        // 環境マップから放射輝度をサンプリング
        int x = std::clamp((int)(w_envmap * u), 0, w_envmap-1);
        int y = std::clamp((int)(h_envmap * v), 0, h_envmap-1);
        int index = y * w_envmap * 3 + x * 3;
        float R = envmap[index++];
        float G = envmap[index++];
        float B = envmap[index];
        return Vec3(R, G, B);
    }

private:
    std::vector<std::shared_ptr<Shape>> object_list; /**< シーン中のオブジェクト */
    std::vector<std::shared_ptr<Light>> light_list;  /**< シーン中の光源         */
    float* envmap;    /**< 環境マップ               */
    int w_envmap = 0; /**< 環境マップの高さ         */
    int h_envmap = 0; /**< 環境マップの幅           */
    int c_envmap = 0; /**< 環境マップのチャンネル数 */
};