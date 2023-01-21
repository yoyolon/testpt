/**
* @file  Microfacrt.h
* @brief マイクロファセット分布(Smithモデル用)
* @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
*/

#pragma once

#include "Vec3.h"

/** マイクロファセット分布クラス(Smithモデル用) */
class NDF {
public:
    virtual ~NDF();

    /**
    * @brief マイクロファセット分布を評価する関数
    * @param[in]  h :ハーフ方向ベクトル
    * @return float :ハーフ方向でのマイクロファセット分布
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
    * @return float :ハーフ方向ベクトル
    */
    virtual Vec3 sample_halfvector() const = 0;

    /**
    * @brief ハーフ方向のサンプリング確率密度を評価する関数
    * @param[in] h  :ハーフ方向ベクトル
    * @return float :サンプリング確率密度
    */
    virtual float eval_pdf(const Vec3& h) const = 0;
};


/** Beckmann分布クラス */
class Beckmann : public NDF {
public:
    Beckmann(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& w) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& h) const override;

private: 
    float alpha;
};


/** Trowbridge-Reitz(GGX)分布クラス */
class GGX : public NDF {
public:
    GGX(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& w) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& h) const override;

private:
    float alpha;
};