/**
* @file  Shape.h
* @brief 三次元形状モデルの抽象クラスと交差点構造体
*/

#pragma once

#include "Ray.h"

/** 交差点の種類 */
enum class IsectType {
    None     = 1,
    Material = 2,
    Light    = 4
};

/** 交差点情報 */
struct intersection {
    Vec3 pos;                            /**< 座標             */
    Vec3 normal;                         /**< 法線             */
    float t;                             /**< レイのパラメータ */
    IsectType type;                      /**< 交差点の種類     */
    std::shared_ptr<class Material> mat; /**< 材質の種類       */
    std::shared_ptr<class Light> light;  /**< 光源の種類       */
};


/** 三次元形状モデル抽象クラス */
class Shape {
public:
    virtual ~Shape() {};

    /**
    * @brief レイとオブジェクトの交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果
    */
    virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;

    /**
    * @brief オブジェクトの表面積を計算する関数
    * @return float :オブジェクトの表面積
    */
    virtual float area() const = 0;

    /**
    * @brief ジオメトリサンプリングの確率密度を計算する関数
    * @param[in] p  :サンプリング元の交差点情報
    * @param[in] w  :サンプリング方向
    * @return float :確率密度
    * @detail サンプリングは立体角に関して行う
    */
    virtual float sample_pdf(const intersection& ref, const Vec3& w) const;

    /**
    * @brief ジオメトリサンプリングの確率密度を計算する関数
    * @param[in] ref       :サンプリング元の交差点情報
    * @return intersection :サンプルした交差点情報
    */
    virtual intersection sample(const intersection& ref) const = 0;
};