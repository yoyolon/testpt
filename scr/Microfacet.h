/**
* @file  Shape.h
* @brief 三次元形状モデルの抽象クラスと交差点構造体
* @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
*/

#pragma once

#include "Vec3.h"

/** マイクロファセット分布クラス */
class MicrofacetDistribution {
public:
    virtual ~MicrofacetDistribution();

    /**
    * @brief NFD(マイクロファセット分布)を評価
    * @param[in]  h :ハーブベクトル
    * @return float :ハーブベクトルとマイクロファセット法線が一致している割合
    */
    virtual float D(const Vec3& h) const = 0;

    /**
    * @brief レイとオブジェクトの交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果
    */
    virtual float lambda(const Vec3& w) const = 0;
    float G1(const Vec3& w) const {
        return 1 / (1 + lambda(w));
    }
    float G(const Vec3& wi, const Vec3& wo) const {
        return 1 / (1 + lambda(wi) + lambda(wo));
    }
    virtual Vec3 sample_halfvector() const = 0;
    virtual float eval_pdf(const Vec3& wi, const Vec3& h) const = 0;
};


/** Beckmann分布クラス */
class BeckmannDistribution : public MicrofacetDistribution {
public:
    BeckmannDistribution(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& h) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& wi, const Vec3& h) const override;

private: 
    float alpha;
};


/** Trowbridge-Reitz(GGX)分布クラス */
class GGXDistribution : public MicrofacetDistribution {
public:
    GGXDistribution(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& h) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& wi, const Vec3& h) const override;

private:
    float alpha;
};