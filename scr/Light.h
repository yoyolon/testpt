/**
* @file  Light.h
* @brief 光源
* @note  MaterialはShapeに"所有される"がAreaLightはShapeを"所有する"
*/

#pragma once

#include "Ray.h"

struct intersection;
class Piecewise2D;
class Scene;
class Shape;

enum class LightType {
    None     = 1 << 0,  /**< なし   */
    Parallel = 1 << 1,  /**< 平行光線 */
    Area     = 1 << 2,  /**< 面光源 */
    IBL      = 1 << 3   /**< IBL    */
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
    * @brief 光源の放射輝度を評価する関数
    * @param[in] wi :放射輝度が沿う方向ベクトル
    * @return Vec3  :放射輝度の評価値
    */
    virtual Vec3 evel_light(const Vec3& wi) const = 0;

    /**
    * @brief 光源の放射エネルギーを評価する関数
    * @return Vec3 :放射エネルギーの評価値
    */
    virtual Vec3 power() const = 0;

    /**
    * @brief 光源からの入射方向をサンプルして放射輝度を評価する関数
    * @param[in]  ref :サンプリング元の交差点情報
    * @param[out] wi  :ワールド座標系での光源への入射方向(光源へ向かう方向が正)
    * @param[out] pdf :サンプリング確率密度(立体角測度)
    * @return Vec3    :放射輝度の評価値
    * @note: 視線方向から追跡するため光源へ向かう方向が正になる
    */
    virtual Vec3 sample_light(const intersection& ref, Vec3& wi, float& pdf) const = 0;

    /**
    * @brief 入射方向から光源サンプリングの確率密度を評価する関数
    * @param[in] ref  :サンプリング元の交差点情報
    * @param[in] wi   :ワールド座標系での光源への入射方向(光源へ向かう方向が正)
    * @return float   :サンプリング確率密度の評価値
    * @note 光源に到達しない場合は確率密度はゼロになる
    */
    virtual float eval_pdf(const intersection& ref, const Vec3& wi) const = 0;

    /**
    * @brief レイと光源の交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果(交差するならtrue)
    */
    virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;

    /**
    * @brief 二つの交差点の可視判定を行う関数
    * @param[in]  p1    :交差点1
    * @param[in]  p2    :交差点2
    * @param[in]  world :シーン
    * @return bool      :可視判定の結果(可視ならtrue)
    */
    bool is_visible(const intersection& p1, const intersection& p2, const Scene& world);


    /**
    * @brief 光源分布がデルタ分布か判定
    * @return bool :デルタ分布ならtrue
    */
    bool is_delta_light() const { return type == LightType::Parallel; }

    LightType get_type() const { return type; }


private:
    const LightType type;
};


// *** 平行光線 ***
class ParallelLight : public Light {
public:
    /**
    * @brief 面光源の初期化
    * @param[in] _intensity :光源の放射輝度
    */
    ParallelLight(const Vec3& _intensity, const Vec3& _wi_light);

    Vec3 evel_light(const Vec3& wi) const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& ref, Vec3& wi, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& wi) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    Vec3 intensity; /**< 光源の放射輝度  */
    Vec3 wi_light;  /**< ワールド座標系での光源の入射方向 */
};


// *** 面光源 ***
class AreaLight : public Light {
public:
    /**
    * @brief 面光源の初期化
    * @param[in] _intensity :光源の放射輝度
    * @param[in] _shape     :光源のシェイプ
    */
    AreaLight(const Vec3& _intensity, std::shared_ptr<Shape> _shape);

    Vec3 evel_light(const Vec3& wi) const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& ref, Vec3& wi, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& wi) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    Vec3 intensity;               /**< 光源の放射輝度   */
    std::shared_ptr<Shape> shape; /**< 面光源のシェイプ */
    float area;                   /**< 光源の面積       */
};


// *** 環境光源(IBL) ***
class EnvironmentLight : public Light {
public:
    /**
    * @brief 面光源の初期化
    * @param[in] filename :環境マップのパス
    * @param[in] rotation :環境マップのz軸に関する回転角(度数法)
    */
    EnvironmentLight(const std::string& filename, float rotation=0);

    Vec3 evel_light(const Vec3& wi) const override;

    Vec3 power() const override;

    Vec3 sample_light(const intersection& ref, Vec3& wi, float& pdf) const override;

    float eval_pdf(const intersection& ref, const Vec3& wi) const override;

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

private:
    /**
    * @brief uv座標から環境マップの放射輝度を評価する関数
    * @param[in] uv :uv座標
    * @return Vec3  :環境マップの放射輝度の評価値
    */
    Vec3 evel_light_uv(const Vec2& uv) const;

    /**
    * @brief 配列インデックスから環境マップの放射輝度を評価する関数
    * @param[in] x :行のインデックス
    * @param[in] y :列のインデックス
    * @return Vec3 :環境マップの放射輝度の評価値
    */
    Vec3 evel_envmap(int x, int y) const;

    /**
    * @brief 環境マップを回転する関数関数
    * @param[in] deg :回転角(度数法)
    */
    void rotate_envmap(float deg);


    float* envmap;    /**< 環境マップ   */
    int nw;           /**< 幅           */
    int nh;           /**< 高さ         */
    int nc;           /**< チャンネル数 */
    float luminance;  /**< 明るさ       */
    std::unique_ptr<Piecewise2D> dist;  /**< 環境マップの輝度分布 */
};