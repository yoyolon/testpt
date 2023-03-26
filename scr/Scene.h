/**
* @file Scene.h
* @brief シーン中のシェイプを管理
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
    Scene() {}

    /**
    * @brief シーンにシェイプを追加
    * @param[in]  object :シェイプ
    */
    void add(std::shared_ptr<Shape> object) { shape_list.push_back(object); }

    /**
    * @brief シーンに光源を追加
    * @param[in]  light :光源
    */
    void add(std::shared_ptr<Light> light) { light_list.push_back(light); }

    /**
    * @brief シーンから全てのシェイプと光源を除去
    */
    void clear() { shape_list.clear(); }

    /**
    * @brief シーンのシェイプを取得
    * @return std::vector<std::shared_ptr<Shape>> :シーン中のシェイプの集合
    */
    std::vector<std::shared_ptr<Shape>> get_shape() const { return shape_list; }

    /**
    * @brief シーンの光源を取得
    * @return std::vector<std::shared_ptr<Light>> :シーン中の光源の集合
    */
    std::vector<std::shared_ptr<Light>> get_light() const { return light_list; }

    /**
    * @brief レイとオブジェクト(シェイプと光源)の交差判定を行う関数
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
        // シェイプとの交差判定
        for (const auto& object : shape_list) {
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
    * @brief レイとシェイプの交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @return bool      :交差判定の結果
    */
    bool intersect_object(const Ray& r, float t_min, float t_max) const {
        intersection isect;
        bool is_isect = false;
        auto t_first = t_max;
        isect.type = IsectType::None;
        // シェイプとの交差判定
        for (const auto& object : shape_list) {
            if (object->intersect(r, t_min, t_first, isect)) {
                is_isect = true;
                t_first = isect.t;
                isect.type = IsectType::Material;
            }
        }
        return is_isect;
    }

    /**
    * @brief レイと光源の交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果
    * @note TODO: BVHの実装
    */
    bool intersect_light(const Ray& r, float t_min, float t_max, intersection& p) const {
        intersection isect;
        bool is_isect = false;
        auto t_first = t_max;
        isect.type = IsectType::None;
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

private:
    std::vector<std::shared_ptr<Shape>> shape_list; /**< シーン中のシェイプ */
    std::vector<std::shared_ptr<Light>> light_list; /**< シーン中の光源         */
};