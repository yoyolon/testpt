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

#include "Ray.h"


struct intersection;

enum class MaterialType {
    Diffuse  = 1, /**< 拡散反射 */
    Specular = 2, /**< 鏡面反射 */
    Glossy   = 4, /**< 光沢反射 */
    Emitter  = 8  /**< 発光     */
};

/**
* @brief 入射角余弦を計算する関数
* @param[in]  w :方向ベクトル
* @return float :入射角余弦
*/
inline float get_cos(const Vec3& w) { return w.get_z(); }

/**
* @brief 入射角余弦の二乗を計算する関数
* @param[in]  w :方向ベクトル
* @return float :入射角余弦の二乗
*/
inline float get_cos2(const Vec3& w) { return w.get_z() * w.get_z(); }

/**
* @brief 入射角正弦の二乗を計算する関数
* @param[in]  w :方向ベクトル
* @return float :入射角正弦の二乗
*/
inline float get_sin2(const Vec3& w) { return std::max(0.0f, 1.0f - get_cos2(w)); }

/**
* @brief 入射角正弦を計算する関数
* @param[in]  w :方向ベクトル
* @return float :入射角正弦
*/
inline float get_sin(const Vec3& w) { return std::sqrt(get_sin2(w)); }

/**
* @brief 入射角正接を計算する関数
* @param[in]  w :方向ベクトル
* @return float :入射角正接
*/
inline float get_tan(const Vec3& w) { return get_sin(w) / get_cos(w); }

/**
* @brief 入射角正接の二乗を計算する関数
* @param[in]  w :方向ベクトル
* @return float :入射角正接の二乗
*/
inline float get_tan2(const Vec3& w) { return get_sin2(w) / get_cos2(w); }


/** マテリアルの抽象クラス */
class Material {
public:
    virtual ~Material() {};

    /**
    * @brief コンストラクタ
    * @param[in] _type :反射特性の種類
    */
    Material(MaterialType _type) : type(_type) {};

    /**
    * @brief 入射方向に対して反射方向をサンプリングしてBRDFを評価する関数
    * @param[in]  wi   :入射方向ベクトル(正規化)
    * @param[in]  p    :物体表面の交差点情報
    * @param[out] brdf :入射方向と出射方向に対するBRDFの値
    * @param[out] wo   :出射方向ベクトル(正規化)
    * @param[out] pdf  :立体角に関するサンプリング確率密度
    * @return bool     :反射材質ならTrue
    * @note: 実際は入射方向をサンプリングするがBRDF相反性により反射方向のサンプリングとして実装した
    */
    virtual bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const = 0;

    /**
    * @brief 入射方向と反射方向に対してBRDFを評価する関数
    * @param[in]  wi :入射方向ベクトル
    * @param[out] wo :出射方向ベクトル
    * @return Vec3   :BRDFの値
    */
    virtual Vec3 f(const Vec3& wi, const Vec3& wo) const { return Vec3::zero; }

    /**
    * @brief 自己発光を評価する関数
    * @return Vec3 : 発光による放射輝度
    */
    virtual Vec3 emitte() const { return Vec3::zero; }

    /**
    * @brief 出射方向のサンプリング確率密度を計算する関数
    * @param[in] wi :入射方向ベクトル
    * @param[in] wo :出射方向ベクトル
    * @return float :出射方向のサンプリング確率密度
    */
    virtual float sample_pdf(const Vec3& wi, const Vec3& wo) const = 0;

    /**
    * @brief 材質の反射特性を取得する関数
    * @return MaterialType :材質の反射特性
    */
    MaterialType get_type() const { return type; }

private:
    MaterialType type; /**> 反射特性 */
};


/** 拡散反射クラス */
class Diffuse : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _albedo :反射係数
    */
    Diffuse(Vec3 _albedo);

    Vec3 f(const Vec3& wi, const Vec3& wo) const override;

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
    Vec3 albedo; /**> 反射係数 */
};


/** 鏡面反射クラス */
class Mirror : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _albedo :反射係数
    */
    Mirror(Vec3 _albedo);

    Vec3 f(const Vec3& wi, const Vec3& wo) const override;

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
    Vec3 albedo; /**> 反射係数 */
};


// *** Phong反射モデルクラス ***
class Phong : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _albedo :反射係数
    * @param[in] _Kd     :拡散反射係数
    * @param[in] _Ks     :鏡面反射係数
    * @param[in] _shin   :光沢度
    */
    Phong(Vec3 _albedo, Vec3 _Kd, Vec3 _Ks, float _shin);

    Vec3 f(const Vec3& wi, const Vec3& wo) const override;

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
    Vec3 albedo; /**> 反射係数     */
    Vec3 Kd;     /**> 拡散反射係数 */
    Vec3 Ks;     /**> 鏡面反射係数 */
    float shin;  /**> 光沢度       */
};


/** マイクロファセット反射モデルクラス */
/** @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class Microfacet : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _albedo       :反射係数
    * @param[in] _distribution :マイクロファセット分布
    * @param[in] _fresnel      :フレネルの式
    */
    Microfacet(Vec3 _albedo, std::shared_ptr<class MicrofacetDistribution> _distribution, 
               std::shared_ptr<class Fresnel> _fresnel);

    Vec3 f(const Vec3& wi, const Vec3& wo) const override;

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

private:
    Vec3 albedo; /**> 反射係数     */
    std::shared_ptr<class Fresnel> fresnel; /**> フレネル項 */
    std::shared_ptr<class MicrofacetDistribution> distribution; /**> マイクロファセット分布 */
};


/** 発光クラス */
class Emitter : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _intensity :放射輝度
    */
    Emitter(Vec3 _intensity);

    bool sample_f(const Vec3& wi, const intersection& p, Vec3& brdf, Vec3& wo, float& pdf) const override;

    float sample_pdf(const Vec3& wi, const Vec3& wo) const override;

    Vec3 emitte() const;

private:
    Vec3 intensity; /**> 放射輝度 */
};