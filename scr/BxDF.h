/**
* @file  BxDF.h
* @brief BRDFとBTDFの実装
* @note  クラス設計参考: https://pbr-book.org/3ed-2018/Reflection_Models/Basic_Interface
*        ***シェーディング規則***
*        1.法線をz軸正の方向としたシェーディング座標系で行う
*        2.入出射方向は物体表面から離れる方向を正
*        3.入出射方向は正規化されている
*/
#pragma once

#include "Math.h"

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

/**
* @brief 入出射方向が同一半球内に存在するか判定
* @param[in]  wo :出射方向ベクトル
* @param[in]  wi :入射方向ベクトル
* @return float :同一半球内にあるならtrue
*/
inline bool is_same_hemisphere(const Vec3& wo, const Vec3& wi) { 
    return wo.get_z() * wi.get_z() > 0; 
}

/**
* @brief 材質の反射特性が含まれているか判定
* @param a : チェックされる材質特性
* @param b : 
* @return aにbが含まれるならtrue
*/
inline bool is_include_type(BxDFType a, BxDFType b) { return BxDFType((uint8_t)a & (uint8_t)b) == b; }

/**
* @brief スペキュラ材質特性であるか確認
* @param t : チェックされる材質
* @return スペキュラ材質ならtrue
*/
inline bool is_spacular_type(BxDFType t) { return is_include_type(t, BxDFType::Specular); }


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
    virtual Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const { return Vec3::zero; }

    /**
    * @brief 散乱方向のサンプリング確率密度を評価する関数
    * @param[in] wo :出射方向ベクトル
    * @param[in] wi :入射方向ベクトル
    * @return float :サンプリング確率密度
    */
    virtual float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const = 0;

    /**
    * @brief 材質の反射特性を取得する関数
    * @return MaterialType :材質の反射特性
    */
    BxDFType get_type() const { return type; }

    /**
    * @brief 材質の反射特性がtと一致するか判定
    * @param t :チェックする反射特性
    * @return 一致するならtrue
    */
    bool is_same_type(BxDFType t) const { return BxDFType((uint8_t)type & (uint8_t)t) == type; }

    /**
    * @brief 材質の反射特性にtが含まれているか判定
    * @param t :チェックする反射特性
    * @return 含まれるならtrue
    */
    bool is_include_type(BxDFType t) const { return BxDFType((uint8_t)type & (uint8_t)t) == t; }

    /**
    * @brief 完全鏡面であるか判定する関数
    * @return bool :完全鏡面ならtrue
    * @note enum classは型キャストが必要
    */
    bool is_specular() const { return is_include_type(BxDFType::Specular); }

    /**
    * @brief 反射物体であるか判定する関数
    * @return bool :反射物体ならtrue
    * @note enum classは型キャストが必要
    */
    bool is_reflection() const { return is_include_type(BxDFType::Reflection); }

    /**
    * @brief 透過物体であるか判定する関数
    * @return bool :透過物体ならtrue
    * @note enum classは型キャストが必要
    */
    bool is_transmission() const { return is_include_type(BxDFType::Transmission); }

private:
    BxDFType type; /**> BxDFの種類 */
};


/** Lambert反射 */
class LambertianReflection : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    */
    LambertianReflection(const Vec3& _scale);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

private:
    Vec3 scale; /**> スケールファクター */
};


/** Lambert透過 */
class LambertianTransmission : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    */
    LambertianTransmission(const Vec3& _scale);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

private:
    Vec3 scale; /**> スケールファクター */
};


/** 完全鏡面反射 */
class SpecularReflection : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    * @param[in] _fres  :フレネル式
    */
    SpecularReflection(Vec3 _scale, std::shared_ptr<class Fresnel> _fres);

    /**
    * @brief 誘電体用コンストラクタ
    * @param[in] _scale :スケールファクター
    * @param[in] _n     :屈折率
    */
    SpecularReflection(Vec3 _scale, float _ni, float _no=1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> スケールファクター */
    std::shared_ptr<class Fresnel> fres; /**> フレネル項 */
};


/** 完全鏡面透過 */
class SpecularTransmission : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    */
    SpecularTransmission(Vec3 _scale, float _n_inside, float _n_outside=1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> スケールファクター */
    float n_inside;  /**> 内側媒質の屈折率 */
    float n_outside; /**> 外側媒質の屈折率 */
    std::shared_ptr<class Fresnel> fres; /**> フレネル項 */
};


// *** Phong鏡面反射 ***
class PhongReflection : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    * @param[in] _shine :光沢度
    */
    PhongReflection(Vec3 _scale, float _shine);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale;  /**> 反射係数 */
    float shine; /**> 光沢度 */
};


/** v-cavityマイクロファセット反射 */
class VcavityReflection : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    * @param[in] _dist  :マイクロファセット分布
    * @param[in] _fres  :フレネル式
    */
    VcavityReflection(Vec3 _scale, std::shared_ptr<class Vcavity> _dist,
                      std::shared_ptr<class Fresnel> _fres);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale; /**> スケールファクター */
    std::shared_ptr<class Fresnel> fres; /**> フレネル項 */
    std::shared_ptr<class Vcavity> dist; /**> マイクロファセット分布(v-cavity用) */
};


/** マイクロファセット反射 */
/** @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class MicrofacetReflection : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    * @param[in] _dist  :マイクロファセット分布
    * @param[in] _fres  :フレネル式
    */
    MicrofacetReflection(Vec3 _scale, std::shared_ptr<class NDF> _dist, 
                         std::shared_ptr<class Fresnel> _fres);

    /**
    * @brief 誘電体用コンストラクタ
    * @param[in] _scale :スケールファクター
    * @param[in] _n     :屈折率
    */
    MicrofacetReflection(Vec3 _scale, std::shared_ptr<class NDF> _dist, 
                         float _ni, float _no = 1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale; /**> スケールファクター */
    std::shared_ptr<class Fresnel> fres; /**> フレネル項 */
    std::shared_ptr<class NDF> dist; /**> マイクロファセット分布 */
};


/** マイクロファセット透過 */
/** @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class MicrofacetTransmission : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    * @param[in] _dist  :マイクロファセット分布
    * @param[in] _fres  :フレネル式
    */
    MicrofacetTransmission(Vec3 _scale, std::shared_ptr<class NDF> _dist, 
                           float _n_inside, float _n_outside = 1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;


private:
    Vec3 scale; /**> スケールファクター */
    float n_inside;  /**> 内側媒質の屈折率 */
    float n_outside; /**> 外側媒質の屈折率 */
    std::shared_ptr<class NDF> dist; /**> マイクロファセット分布 */
    std::shared_ptr<class Fresnel> fres; /**> フレネル項 */
};