/**
* @file  Triangle.h
* @brief 三角形クラスと三角形メッシュクラス
* @note  MaterialはShapeに"所有される"がLightはShapeを"所有する"
*/

#pragma once

#include "Ray.h"

struct intersection;

enum class LightType {
    Area = 1, IBL = 2
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
    * @return Vec3 :光源の放射輝度
    */
    virtual Vec3 emitte() const = 0;

    /**
    * @brief 光源の放射エネルギーを計算する関数
    * @return Vec3 :光源の放射エネルギー
    */
    virtual Vec3 power() const = 0;

    /**
    * @brief 光源からの入射方向をサンプルして放射輝度を返す関数
    * @param[in]  ref :サンプリング元の交差点情報
    * @param[out] wo  :光源からの入射方向
    * @param[out] pdf :立体角に関するサンプリング確率密度
    * @return Vec3    :光源からの入射方向
    * @note: 実際は光源からの入射方向は反転する(視線方向から追跡するため)
    */
    virtual Vec3 sample_light(const intersection& ref, Vec3& w, float& pdf) = 0;

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
    bool is_visible(const intersection& p1, const intersection& p2, const class Scene& world);

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
    AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape);

    Vec3 emitte() const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& p, Vec3& wo, float& pdf) override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    Vec3 intensity;                     /**< 光源の放射輝度     */
    std::shared_ptr<class Shape> shape; /**< 面光源のジオメトリ */
    float area;                         /**< 光源の面積         */
};