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

enum class MaterialType {
    Diffuse  = 1, /**< 拡散反射 */
    Specular = 2, /**< 鏡面反射 */
    Glossy   = 4, /**< 光沢反射 */
};


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
    * @brief 反射方向に対して入射方向をサンプリングしてBRDFを評価する関数
    * @param[in]  wo   :入射方向ベクトル(正規化)
    * @param[in]  p    :物体表面の交差点情報
    * @param[out] brdf :入射方向と出射方向に対するBRDFの値
    * @param[out] wi   :出射方向ベクトル(正規化)
    * @param[out] pdf  :立体角に関する入射方向サンプリング確率密度
    * @return Vec3     :入射方向と出射方向に対するBRDFの値
    */
    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const;

    /**
    * @brief BRDFを評価する関数
    * @param[in] wo :出射方向ベクトル
    * @param[in] wi :入射方向ベクトル
    * @return Vec3  :BRDFの値
    */
    Vec3 eval_f(const Vec3& wo, const Vec3& wi) const;

    /**
    * @brief 入射方向のサンプリング確率密度を計算する関数
    * @param[in] wo :出射方向ベクトル
    * @param[in] wi :入射方向ベクトル
    * @return float :サンプリング確率密度
    */
    float eval_pdf(const Vec3& wo, const Vec3& wi) const;

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

    /**
    * @brief 材質の反射特性を取得する関数
    * @return MaterialType :材質の反射特性
    */
    MaterialType get_type() const { return type; }

private:
    MaterialType type; /**> 反射特性 */
    std::vector<std::shared_ptr<BxDF>> bxdf_list; /**> BxDFの集合 */
};


/** 拡散反射マテリアル */
class Diffuse : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _albedo :反射係数
    */
    Diffuse(Vec3 _albedo);

private:
    Vec3 albedo; /**> 反射係数 */
};


/** 鏡マテリアル */
class Mirror : public Material {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _albedo :反射係数
    */
    Mirror(Vec3 _albedo);

private:
    Vec3 albedo; /**> 反射係数 */
};

//
//
//// *** Phong反射モデルクラス ***
//class Phong : public Material {
//public:
//    /**
//    * @brief コンストラクタ
//    * @param[in] _albedo :反射係数
//    * @param[in] _Kd     :拡散反射係数
//    * @param[in] _Ks     :鏡面反射係数
//    * @param[in] _shin   :光沢度
//    */
//    Phong(Vec3 _albedo, Vec3 _Kd, Vec3 _Ks, float _shin);
//
//    Vec3 f(const Vec3& wo, const Vec3& wi) const override;
//
//    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;
//
//    float eval_pdf(const Vec3& wo, const Vec3& wi) const override;
//
//private:
//    Vec3 albedo; /**> 反射係数     */
//    Vec3 Kd;     /**> 拡散反射係数 */
//    Vec3 Ks;     /**> 鏡面反射係数 */
//    float shin;  /**> 光沢度       */
//};
//
//