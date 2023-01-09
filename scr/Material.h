/**
* @file  Material.h
* @brief マテリアル
* @note: シェーディング規則
*        1.法線をz軸正の方向としたシェーディング座標系で行う
*        2.入射/出射方向は物体表面から離れる方向を正とする
*        3.z軸とベクトルがなす角をthetaとする．
*        4.入射/出射方向は正規化されている
*/
#pragma once

#include "BxDF.h"


struct intersection;


/** マテリアルの抽象クラス */
class Material {
public:
    virtual ~Material() {};

    /**
    * @brief コンストラクタ
    * @param[in] _type :反射特性の種類
    */
    Material() {};

    /**
    * @brief 反射方向に対して入射方向をサンプリングしてBRDFを評価する関数
    * @param[in]  wo              :入射方向ベクトル
    * @param[in]  p               :物体表面の交差点情報
    * @param[out] brdf            :入射方向と出射方向に対するBRDFの値
    * @param[out] wi              :出射方向ベクトル
    * @param[out] pdf             :立体角に関する入射方向サンプリング確率密度
    * @oaram[out] sampled_type    :サンプリングしたなBxDFの種類
    * @oaram[in]  acceptable_type :サンプリング可能なBxDFの種類
    * @return Vec3                :入射方向と出射方向に対するBRDFの値
    */
    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf,
                  BxDFType& sampled_type, BxDFType acceptable_type=BxDFType::All) const;

    /**
    * @brief BRDFを評価する関数
    * @param[in] wo              :出射方向ベクトル
    * @param[in] wi              :入射方向ベクトル
    * @oaram[in] acceptable_type :サンプリング可能なBxDFの種類
    * @return Vec3               :BRDFの値
    */
    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p,
                BxDFType acceptable_type=BxDFType::All) const;

    /**
    * @brief 入射方向のサンプリング確率密度を計算する関数
    * @param[in] wo              :出射方向ベクトル
    * @param[in] wi              :入射方向ベクトル
    * @oaram[in] acceptable_type :サンプリング可能なBxDFの種類
    * @return float              :サンプリング確率密度
    */
    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p,
                   BxDFType acceptable_type=BxDFType::All) const;

    /**
    * @brief 材質にBxDFを追加する関数
    * @param[in] bxdf: 散乱特性を表すBxDF
    */
    void add(std::shared_ptr<BxDF> bxdf) { bxdf_list.push_back(bxdf); }

    /**
    * @brief BxDFの集合を取得
    * @return std::vector<std::shared_ptr<BxDF>> : BxDFの集合
    */
    std::vector<std::shared_ptr<BxDF>> get_BSDF() const { return bxdf_list; }

private:
    std::vector<std::shared_ptr<BxDF>> bxdf_list; /**> BxDFの集合 */
};


/** 拡散反射マテリアル */
class Diffuse : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _base :ベースカラー
    */
    Diffuse(Vec3 _base);

private:
    Vec3 base; /**> ベースカラー */
};


/** 拡散透過マテリアル */
class DiffusePlastic : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _base :ベースカラー
    * @param[in] _r    :反射係数
    * @param[in] _t    :透過係数
    */
    DiffusePlastic(Vec3 _base, Vec3 _r, Vec3 _t);

private:
    Vec3 base; /**> ベースカラー */
    Vec3 r;    /**> 反射係数     */
    Vec3 t;    /**> 透過係数     */
};


/** 鏡マテリアル */
class Mirror : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _base :ベースカラー
    */
    Mirror(Vec3 _base);

private:
    Vec3 base; /**> ベースカラー */
};


/** ガラスマテリアル */
class Glass : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _base :ベースカラー
    * @param[in] _r    :反射係数
    * @param[in] _t    :透過係数
    * @param[in] _n    :屈折率
    */
    Glass(Vec3 _base, Vec3 _r, Vec3 _t, float _n, float _alpha=0.0f);

private:
    Vec3 base;   /**> ベースカラー */
    Vec3 r;      /**> 反射係数     */
    Vec3 t;      /**> 透過係数     */
    float n;     /**> 屈折率       */
    float alpha; /**> 表面粗さ     */
};


/** 金属マテリアル */
class Metal : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _base  :ベースカラー
    * @param[in] _fr    :金属反射率
    * @param[in] _alpha :表面粗さ
    */
    Metal(Vec3 _base, Vec3 fr, float _alpha);

private:
    Vec3  base;  /**> ベースカラー */
    Vec3  fr;    /**> 金属反射率   */
    float alpha; /**> 表面粗さ     */
};


/** Phongマテリアル */
class Phong : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _base  :ベースカラー
    * @param[in] _kd    :拡散反射係数
    * @param[in] _ks    :鏡面反射係数
    * @param[in] _shine :光沢度
    */
    Phong(Vec3 _base, Vec3 _kd, Vec3 _ks, float _shine);

private:
    Vec3  base;  /**> ベースカラー */
    Vec3  kd;    /**> 拡散反射係数 */
    Vec3  ks;    /**> 鏡面反射係数 */
    float shine; /**> 光沢度       */
};