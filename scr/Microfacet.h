/**
* @file  Microfacrt.h
* @brief マイクロファセット分布(Smithモデル用)
* @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
*/

#pragma once

#include "Math.h"

/** マイクロファセット分布クラス(Smithモデル用) */
class NDF {
public:
    virtual ~NDF() {};

    /**
    * @brief マイクロファセット分布を評価する関数
    * @param[in]  h :ハーフ方向ベクトル
    * @return float :ハーフ方向でのマイクロファセット分布の評価値
    */
    virtual float D(const Vec3& h) const = 0;

    /**
    * @brief Smithマスキング関数のヘルパー関数
    * @param[in] w  :方向ベクトル
    * @return float :評価値(Smithマスキング関数で使用)
    */
    virtual float lambda(const Vec3& w) const = 0;

    /**
    * @brief Smithマスキング関数
    * @param[in] w  :方向ベクトル
    * @return float :マスキング量
    */
    float G1(const Vec3& w) const {
        return 1 / (1 + lambda(w));
    }

    /**
    * @brief Smithシャドウイング-マスキング関数
    * @param[in] wo :出射方向ベクトル
    * @param[in] wi :入射方向ベクトル
    * @return float :シャドウイング-マスキング量
    */
    float G(const Vec3& wo, const Vec3& wi) const {
        return 1 / (1 + lambda(wo) + lambda(wi));
    }

    /**
    * @brief ハーフ方向のサンプリングを行う関数
    * @param[in] wo :出射方向ベクトル
    * @return float :ハーフ方向ベクトル
    */
    virtual Vec3 sample_halfvector(const Vec3& wo) const = 0;

    /**
    * @brief ハーフ方向のサンプリングPDF(確率密度)を評価する関数
    * @param[in] h  :ハーフ方向ベクトル
    * @param[in] wo :出射方向ベクトル
    * @return float :サンプリング確率密度
    */
    virtual float eval_pdf(const Vec3& h, const Vec3& wo) const = 0;
};


/** Beckmann分布クラス */
class Beckmann : public NDF {
public:
    Beckmann(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& w) const override;
    Vec3 sample_halfvector(const Vec3& wo) const override;
    float eval_pdf(const Vec3& h, const Vec3& wo) const override;

private: 
    /**
    * @brief Beckmann分布からハーフベクトルをサンプリング
    * @param[in] alpha :分布のパラメータ
    * @return Vec3     :サンプリング値
    * @note: 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
    */
    Vec3 beckmann_sample(float alpha) const;

    float alpha; /**< 分布のスロープパラメータ(表面粗さ) */
};


/** Trowbridge-Reitz(GGX)分布クラス */
class GGX : public NDF {
public:
    GGX(float alpha, bool is_vsible_sampling=true);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& w) const override;
    Vec3 sample_halfvector(const Vec3& wo) const override;
    float eval_pdf(const Vec3& h, const Vec3& wo) const override;

private:
    /**
    * @brief Trowbridge-Reitz(GGX)分布からハーフベクトルをサンプリング
    * @param[in] alpha :分布のパラメータ
    * @return Vec3     :サンプリング値
    * @note: 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
    */
    Vec3 ggx_sample(float alpha) const;

    /**
    * @brief Trowbridge-Reitz(GGX)分布の可視法線分布からハーフベクトルをサンプリング
    * @param[in] wo    :出射方向
    * @param[in] alpha :分布のパラメータ
    * @return Vec3     :サンプリング値
    * @note: 参考: [Heitz 2018](https://jcgt.org/published/0007/04/01/)
    */
    Vec3 visible_ggx_sample(const Vec3& wo, float alpha) const;

    float alpha; /**< 分布のスロープパラメータ(表面粗さ) */
    bool is_vsible_sampling;  /**< 可視法線分布からサンプリングするならtrue */
};