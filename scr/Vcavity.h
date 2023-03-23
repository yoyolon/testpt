/**
* @file Vcavity.h
* @brief マイクロファセット分布クラス(v-cavityモデル用)
*/

#pragma once

#include "Vec3.h"

/**
* @brief マイクロファセット分布の種類
*/
enum class NDFType {
    Beckmann = 1, /**< ベックマン分布 */
    GGX      = 1  /**< GGX分布 */
};


/** マイクロファセット分布クラス(V-cavityモデル用) */
class Vcavity {
public:

    /**
    * @brief コンストラクタ
    * @param[in]  alpha : マイクロファセット表面粗さ
    * @param[in]  type  : マイクロファセット分布の種類
    */
    Vcavity(float _alpha, NDFType _type=NDFType::Beckmann);

    /**
    * @brief マイクロファセット分布を評価する関数
    * @param[in]  h :ハーフ方向ベクトル
    * @return float :ハーフ方向でのマイクロファセット分布
    */
    float D(const Vec3& h) const;

    /**
    * @brief V-cavityシャドウイング-マスキング関数
    * @param[in] wo :出射方向ベクトル
    * @param[in] wi :入射方向ベクトル
    * @param[in] h  :マイクロファセット法線
    * @param[in] n  :ジオメトリ法線
    * @return float :シャドウイング-マスキング量
    */
    float G(const Vec3& wo, const Vec3& wi, const Vec3& h) const;

    /**
    * @brief ハーフ方向のサンプリングを行う関数
    * @param[in] wo :出射方向ベクトル
    * @return float :ハーフ方向ベクトル
    */
    Vec3 sample_halfvector(const Vec3& wo) const;

    /**
    * @brief ハーフ方向のサンプリング確率密度を評価する関数
    * @param[in] h  :ハーフ方向ベクトル
    * @param[in] wo :出射方向ベクトル
    * @return float :サンプリング確率密度
    */
    float eval_pdf(const Vec3& h, const Vec3& wo) const;

private:
    std::shared_ptr<class NDF> dist; // NDF
};