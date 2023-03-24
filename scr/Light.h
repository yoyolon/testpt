/**
* @file  Light.h
* @brief 光源クラス
* @note  MaterialはShapeに"所有される"がLightはShapeを"所有する"
*/

#pragma once

#include "Ray.h"

struct intersection;
class Piecewise2D;
class Scene;
class Shape;

enum class LightType {
    None = 1 << 0,  /**< なし   */
    Area = 1 << 1,  /**< 面光源 */
    IBL  = 1 << 2   /**< IBL    */
};

/** 光源抽象クラス */
class Light {
public:
    virtual ~Light() {};

    /**
    * @brief 光源の初期化
    * @param[in] type :光源の種類
    */
    Light(LightType _type) : type(_type) {};

    /**
    * @brief 光源の放射輝度を計算する関数
    * @param[in] w :放射輝度が沿う方向
    * @return Vec3 :光源の放射輝度
    */
    virtual Vec3 evel_light(const Vec3& w) const = 0;

    /**
    * @brief 光源の放射エネルギーを計算する関数
    * @return Vec3 :光源の放射エネルギー
    */
    virtual Vec3 power() const = 0;

    /**
    * @brief 光源からの入射方向をサンプルして放射輝度を返す関数
    * @param[in]  ref :サンプリング元の交差点情報
    * @param[out] w   :光源からの入射方向(正規化)
    * @param[out] pdf :立体角に関するサンプリング確率密度
    * @return Vec3    :光源からの入射方向
    * @note: 実際は光源からの入射方向は反転する(視線方向から追跡するため)
    */
    virtual Vec3 sample_light(const intersection& ref, Vec3& w, float& pdf) const = 0;

    /**
    * @brief 入射方向から光源サンプリングの確率密度を返す関数
    * @param[in]  ref :サンプリング元の交差点情報
    * @param[in] w    :光源への入射方向(正規化)
    * @return float   :確率密度
    * @note 光源に到達しない場合は確率密度はゼロになる
    */
    virtual float eval_pdf(const intersection& ref, const Vec3& w) const = 0;

    /**
    * @brief レイと光源の交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果
    */
    virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;

    /**
    * @brief 二点間の可視判定を行う関数
    * @param[in]  p1    :交差点1
    * @param[in]  p2    :交差点2
    * @param[in]  world :シーン
    * @return bool      :可視判定の結果
    */
    bool is_visible(const intersection& p1, const intersection& p2, const Scene& world);

private:
    const LightType type;
};


// *** 面光源 ***
class AreaLight : public Light {
public:
    /**
    * @brief 面光源の初期化
    * @param[in] _intensity :光源の放射輝度
    * @param[in] _shape     :光源のジオメトリ
    */
    AreaLight(Vec3 _intensity, std::shared_ptr<Shape> _shape);

    Vec3 evel_light(const Vec3& w) const override;

    Vec3 power() const override;


    Vec3 sample_light(const intersection& ref, Vec3& wo, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& w) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    Vec3 intensity;               /**< 光源の放射輝度     */
    std::shared_ptr<Shape> shape; /**< 面光源のジオメトリ */
    float area;                   /**< 光源の面積         */
};


// *** 環境光源(IBL) ***
class EnvironmentLight : public Light {
public:
    /**
    * @brief 面光源の初期化
    * @param[in] _intensity :光源の放射輝度
    * @param[in] _shape     :光源のジオメトリ
    */
    EnvironmentLight(std:: string filename);

    Vec3 evel_light(const Vec3& w) const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& ref, Vec3& wo, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& w) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    float* envmap;    /**< 環境マップ   */
    int nw;           /**< 幅           */
    int nh;           /**< 高さ         */
    int nc;           /**< チャンネル数 */
    float brightness; /**< 明るさ       */
    std::unique_ptr<Piecewise2D> dist;  /**< 環境マップの輝度分布 */
};