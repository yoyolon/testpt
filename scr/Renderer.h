/**
* @file  Renderer.h
* @brief レンダリング処理の実装
* @note  memo
*/

#pragma once

#include "Math.h"

struct intersection;
class Camera;
class ONB;
class Ray;
class Scene;
class Shape;

// 直接光のサンプリング戦略
enum class Sampling {
    UNIFORM = 1 << 0,  /**< 一様サンプリング             */
    BSDF    = 1 << 1,  /**< BSDFによる重点的サンプリング */
    LIGHT   = 1 << 2,  /**< 光源による重点的サンプリング */
    MIS     = 1 << 3,  /**< 多重重点的サンプリング       */
};

/** レンダラークラス */
class Renderer {
public:
    /**
    * @brief レンダラーを初期化
    * @param[in] _spp : 1ピクセルあたりのサンプル数(samples per pixel)
    * @param[in] strategy  :マテリアル
    */
    Renderer(int _spp=4, Sampling _strategy=Sampling::UNIFORM);

    /**
    * @brief 直接光を一様に選んだ入射方向からサンプリングする関数
    * @pram[in] r             :追跡レイ
    * @pram[in] isect         :オブジェクトの交差点情報
    * @pram[in] world         :シーン
    * @pram[in] shading_coord :シェーディング座標系
    * @return Vec3 :直接光の入射放射輝度
    * @note スペキュラレイでは実行されない
    */
    Vec3 explict_uniform(const Ray& r, const intersection& isect, const Scene& world,
        const ONB& shading_coord) const;

    /**
    * @brief 直接光をBSDFに沿った入射方向からサンプリングする関数
    * @pram[in] r             :追跡レイ
    * @pram[in] isect         :オブジェクトの交差点情報
    * @pram[in] world         :シーン
    * @pram[in] shading_coord :シェーディング座標系
    * @return Vec3 :直接光の重み付き入射放射輝度
    * @note スペキュラレイでは実行されない
    */
    Vec3 explict_bsdf(const Ray& r, const intersection& isect, const Scene& world,
        const ONB& shading_coord) const;

    /**
    * @brief 直接光を一つの光源からサンプリング
    * @pram[in] r             :追跡レイ
    * @pram[in] isect         :オブジェクトの交差点情報
    * @pram[in] world         :シーン
    * @pram[in] shading_coord :シェーディング座標系
    * @return Vec3 :直接光の重み付き入射放射輝度
    * @note スペキュラレイでは実行されない
    */
    Vec3 explict_one_light(const Ray& r, const intersection& isect, const Scene& world,
        const ONB& shading_coord) const;

    /**
    * @brief 明示的に直接光をサンプリング
    * @pram[in] r             :追跡レイ
    * @pram[in] isect         :オブジェクトの交差点情報
    * @pram[in] world         :シーン
    * @pram[in] shading_coord :シェーディング座標系
    * @return Vec3 :光源の重み付き放射輝度
    */
    Vec3 explicit_direct_light_sampling(const Ray& r, const intersection& isect,
        const Scene& world, const ONB& shading_coord) const;

    /**
    * @brief 確率的レイトレーシングを実行する関数
    * @param[in]  r_in      :カメラ方向からのレイ
    * @param[in]  max_depth :レイの最大バウンス回数
    * @param[in]  world     :レンダリングするシーンのデータ
    * @return Vec3          :レイに沿った放射輝度
    * @note ロシアンルーレットによる打ち切りを実装していないのでmax_depthは小さめにしておく
    */
    Vec3 L_raytracing(const Ray& r_in, int max_depth, const Scene& world) const;

    /**
    * @brief ナイーブなパストレーシングを実行する関数
    * @param[in]  r_in      :カメラ方向からのレイ
    * @param[in]  max_depth :レイの最大バウンス回数
    * @param[in]  world     :レンダリングするシーンのデータ
    * @return Vec3          :レイに沿った放射輝度
    */
    Vec3 L_naive_pathtracing(const Ray& r_in, int max_depth, const Scene& world) const;

    /**
    * @brief パストレーシングを実行する関数
    * @param[in]  r_in      :カメラ方向からのレイ
    * @param[in]  max_depth :レイの最大バウンス回数
    * @param[in]  world     :レンダリングするシーンのデータ
    * @return Vec3          :レイに沿った放射輝度
    */
    Vec3 L_pathtracing(const Ray& r_in, int max_depth, const Scene& world) const;

    /**
    * @brief シーン中のシェイプの法線を可視化する関数
    * @param[in]  r         :追跡するレイ
    * @param[in]  world     :レンダリングするシーンのデータ
    * @return Vec3          :法線の可視化
    * @note z軸正の方向をカメラへ向かう方向とする
    */
    Vec3 L_normal(const Ray& r, const Scene& world) const;

    /**
    * @brief 指定したデータからシーンをレンダリングする関数
    * @param[out] world    :シーンデータ
    * @param[out] cam      :カメラデータ
    */
    void render(const Scene& world, const Camera& cam) const;


private:
    int spp;           /**< 1ピクセルあたりのサンプル数 */
    Sampling strategy; /**< 光源のサンプリング戦略      */
};