/**
* @file  BxDF.h
* @brief BRDFとBTDFの実装
* @note  クラス設計参考: https://pbr-book.org/3ed-2018/Reflection_Models/Basic_Interface
*        ***シェーディング規則***
*        1.法線をz軸正の方向としたシェーディング座標系で行う
*        2.入出射方向は物体表面から離れる方向を正とする
*        3.z軸とベクトルがなす角をthetaとする．
*        4.入射/出射方向は正規化されている
*/
#pragma once

#include "Vec3.h"


struct intersection;

/**
* @brief BxDFのフラグ
* @note 使用例: if(frag & Specular)で材質が鏡面かどうか判定できる
*/
enum class BxDFType : uint8_t {
    Reflection   = 1 << 0,  /**< 反射   */
    Transmission = 1 << 1,  /**< 透過   */
    Specular     = 1 << 2,  /**< 鏡面   */
    Diffuse      = 1 << 3,  /**< 拡散面 */
    Glossy       = 1 << 4,  /**< 光沢面 */
    All          = Reflection | Transmission | Specular | Diffuse | Glossy
};

/**
* @brief 方向ベクトルと法線のなす角の余弦を計算する関数
* @param[in]  w :方向ベクトル
* @return float :余弦
*/
inline float get_cos(const Vec3& w) { return w.get_z(); }

/**
* @brief 方向ベクトルと法線のなす角の余弦の二乗を計算する関数
* @param[in]  w :方向ベクトル
* @return float :余弦の二乗
*/
inline float get_cos2(const Vec3& w) { return w.get_z() * w.get_z(); }

/**
* @brief 方向ベクトルと法線のなす角の正弦の二乗を計算する関数
* @param[in]  w :方向ベクトル
* @return float :正弦の二乗
*/
inline float get_sin2(const Vec3& w) { return std::max(0.0f, 1.0f - get_cos2(w)); }

/**
* @brief 方向ベクトルと法線のなす角の正弦を計算する関数
* @param[in]  w :方向ベクトル
* @return float :正弦
*/
inline float get_sin(const Vec3& w) { return std::sqrt(get_sin2(w)); }

/**
* @brief 方向ベクトルと法線のなす角の正接を計算する関数
* @param[in]  w :方向ベクトル
* @return float :正接
*/
inline float get_tan(const Vec3& w) { return get_sin(w) / get_cos(w); }

/**
* @brief 方向ベクトルと法線のなす角の正接の二乗を計算する関数
* @param[in]  w :方向ベクトル
* @return float :正接の二乗
*/
inline float get_tan2(const Vec3& w) { return get_sin2(w) / get_cos2(w); }


/** BxDFの抽象クラス */
class BxDF {
public:
    virtual ~BxDF() {};

    /**
    * @brief コンストラクタ
    * @param[in] _type  :反射特性の種類
    * @param[in] _scale :スケールファクター
    */
    BxDF(BxDFType _type) : type(_type) {};

    /**
    * @brief 出射方向に対して入射方向をサンプリングしてBxDFを評価する関数
    * @param[in]  wo   :入射方向ベクトル(正規化)
    * @param[in]  p    :物体表面の交差点情報
    * @param[out] brdf :入射方向と出射方向に対するBRDFの値
    * @param[out] wi   :出射方向ベクトル(正規化)
    * @param[out] pdf  :立体角に関する入射方向サンプリング確率密度
    * @return Vec3     :入射方向と出射方向に対するBRDFの値
    */
    virtual Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const = 0;

    /**
    * @brief BxDFを評価する関数
    * @param[in] wo :出射方向ベクトル
    * @param[in] wi :入射方向ベクトル
    * @return Vec3  :BxDFの値
    */
    virtual Vec3 eval_f(const Vec3& wo, const Vec3& wi) const { return Vec3::zero; }

    /**
    * @brief 散乱方向のサンプリング確率密度を評価する関数
    * @param[in] wo :出射方向ベクトル
    * @param[in] wi :入射方向ベクトル
    * @return float :サンプリング確率密度
    */
    virtual float eval_pdf(const Vec3& wo, const Vec3& wi) const = 0;

    /**
    * @brief 材質の反射特性を取得する関数
    * @return MaterialType :材質の反射特性
    */
    BxDFType get_type() const { return type; }

    /**
    * @brief 完全鏡面であるか判定
    * @return bool :完全鏡面ならtrue
    * @note enum classは型キャストが必要
    */
    bool is_specular() const { return ((uint8_t)type & (uint8_t)BxDFType::Specular); }

    /**
    * @brief 反射物体であるか判定
    * @return bool :反射物体ならtrue
    * @note enum classは型キャストが必要
    */
    bool is_reflection() const { return ((uint8_t)type & (uint8_t)BxDFType::Reflection); }

    /**
    * @brief 透過物体であるか判定
    * @return bool :透過物体ならtrue
    * @note enum classは型キャストが必要
    */
    bool is_transmission() const { return ((uint8_t)type & (uint8_t)BxDFType::Transmission); }

private:
    BxDFType type; /**> BxDFの種類 */
};


/** ランバート反射クラス */
class LambertianReflection : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    */
    LambertianReflection(const Vec3& _scale);

    float eval_pdf(const Vec3& wo, const Vec3& wi) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi) const override;

private:
    Vec3 scale; /**> スケールファクター */
};


/** 完全鏡面反射クラス */
class SpecularReflection : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    */
    SpecularReflection(Vec3 _scale);

    float eval_pdf(const Vec3& wo, const Vec3& wi) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> スケールファクター */
};


/** マイクロファセット反射モデルクラス */
/** @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class MicrofacetReflection : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _albedo       :反射係数
    * @param[in] _distribution :マイクロファセット分布
    * @param[in] _fresnel      :フレネルの式
    */
    MicrofacetReflection(Vec3 _albedo, std::shared_ptr<class MicrofacetDistribution> _distribution, 
               std::shared_ptr<class Fresnel> _fresnel);

    float eval_pdf(const Vec3& wo, const Vec3& wi) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 albedo; /**> 反射係数     */
    std::shared_ptr<class Fresnel> fresnel; /**> フレネル項 */
    std::shared_ptr<class MicrofacetDistribution> distribution; /**> マイクロファセット分布 */
};