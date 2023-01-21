/**
* @file  Shape.h
* @brief 三次元形状モデルの抽象クラスと交差点構造体
* @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
*/

#pragma once

#include "Vec3.h"

/** マイクロファセット分布クラス */
class NDF {
public:
    virtual ~NDF();

    /**
    * @brief NFD(マイクロファセット分布)を評価
    * @param[in]  h :ハーブベクトル
    * @return float :ハーブベクトルとマイクロファセット法線が一致している割合
    */
    virtual float D(const Vec3& h) const = 0;

    /**
    * @brief Smithラムダ関数
    * @param[in] w  :入出射方向
    * @return float :評価値(Smithマスキング関数で使用)
    */
    virtual float lambda(const Vec3& w) const = 0;

    /**
    * @brief Smithマスキング関数
    * @param[in] w  :出射方向
    * @return float :マスキング量
    */
    float G1(const Vec3& w) const {
        return 1 / (1 + lambda(w));
    }

    /**
    * @brief Smithシャドウイング-マスキング関数
    * @param[in] wo  :出射方向
    * @param[in] wi  :入射方向
    * @return float :シャドウイング-マスキング量
    */
    float G(const Vec3& wo, const Vec3& wi) const {
        return 1 / (1 + lambda(wo) + lambda(wi));
    }

    virtual Vec3 sample_halfvector() const = 0;

    virtual float eval_pdf(const Vec3& h) const = 0;
};


/** Beckmann分布クラス */
class Beckmann : public NDF {
public:
    Beckmann(float alpha);
    float D(const Vec3& h) const override;
    float lambda(const Vec3& h) const override;
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
    float lambda(const Vec3& h) const override;
    Vec3 sample_halfvector() const override;
    float eval_pdf(const Vec3& h) const override;

private:
    float alpha;
};