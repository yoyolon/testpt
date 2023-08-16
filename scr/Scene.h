/**
* @file Scene.h
* @brief シーンを管理するクラス
*/

#pragma once

#include <memory>
#include <vector>
#include "Math.h"

struct intersection;
class Light;
class Material;
class Ray;
class Shape;

// *** シーンクラス ***
class Scene {
public:
    /**
    * @brief コンストラクタ
    */
    Scene() : bg_color(Vec3::zero) {}

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
    * @brief シーン中のシェイプが空か判定
    * @return bool :空ならtrue
    */
    bool is_empty_shape_list() const { return shape_list.empty(); }

    /**
    * @brief シーンから全てのシェイプと光源を除去
    */
    void clear() { 
        shape_list.clear();
        light_list.clear();
    }

    /**
    * @brief シーンの全シェイプを取得
    * @return std::vector<std::shared_ptr<Shape>> :シーン中のシェイプの集合
    */
    std::vector<std::shared_ptr<Shape>> get_shape() const { return shape_list; }

    /**
    * @brief シーンの全光源を取得
    * @return std::vector<std::shared_ptr<Light>> :シーン中の光源の集合
    */
    std::vector<std::shared_ptr<Light>> get_light() const { return light_list; }

    Vec3 get_bg_color() const { return bg_color; }
    void set_bg_color(Vec3 color) { bg_color = color; }

    /**
    * @brief レイとオブジェクト(シェイプと光源)の交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果
    * @note TODO: BVHの実装
    */
    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const;

    /**
    * @brief レイとシェイプの交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @return bool      :交差判定の結果
    * @note TODO: BVHの実装
    */
    bool intersect_object(const Ray& r, float t_min, float t_max) const;

    /**
    * @brief レイと光源の交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果
    * @note TODO: BVHの実装
    */
    bool intersect_light(const Ray& r, float t_min, float t_max, intersection& p) const;


private:
    std::vector<std::shared_ptr<Shape>> shape_list; /**< シーン中のシェイプ */
    std::vector<std::shared_ptr<Light>> light_list; /**< シーン中の光源     */
    Vec3 bg_color; /**< 背景色 */
};