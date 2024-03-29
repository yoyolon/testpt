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
class Fresnel;
class NDF;

/**
* @brief BxDFのフラグ(材質特性)
* @note 使用例: if(frag & Specular)でfragの材質特性が完全鏡面かどうか判定できる
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
inline float get_sin2(const Vec3& w) { return std::max(0.f, 1.0f - get_cos2(w)); }

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
* @brief 入射/出射方向が同一半球内に存在するか判定
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
* @param b : チェックする材質特性
* @return aにbが含まれるならtrue
*/
inline bool is_include_type(BxDFType a, BxDFType b) { return BxDFType((uint8_t)a & (uint8_t)b) == b; }

/**
* @brief スペキュラ材質特性であるか確認
* @param t : チェックされる材質特性
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
    */
    BxDF(BxDFType _type) : type(_type) {};

    /**
    * @brief 出射方向に対して入射方向をサンプリングしてBxDFを評価する関数
    * @param[in]  wo   :入射方向ベクトル(ローカル座標)
    * @param[in]  p    :物体表面の交差点情報
    * @param[out] wi   :出射方向ベクトル(ローカル座標)
    * @param[out] pdf  :入射方向サンプリング確率密度(立体角測度)
    * @return Vec3     :BxDFの評価値
    */
    virtual Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const = 0;

    /**
    * @brief BxDFを評価する関数
    * @param[in] wo :出射方向ベクトル(ローカル座標)
    * @param[in] wi :入射方向ベクトル(ローカル座標)
    * @param[in] p  :物体表面の交差点情報
    * @return Vec3  :BxDFの評価値
    */
    virtual Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const { return Vec3::zero; }

    /**
    * @brief 入射方向のサンプリング確率密度を評価する関数
    * @param[in] wo :出射方向ベクトル(ローカル座標)
    * @param[in] wi :入射方向ベクトル(ローカル座標)
    * @param[in] p  :物体表面の交差点情報
    * @return float :サンプリング確率密度(立体角測度)
    */
    virtual float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const = 0;

    /**
    * @brief 材質特性を取得する関数
    * @return MaterialType :材質特性
    */
    BxDFType get_type() const { return type; }

    /**
    * @brief 材質特性がtと一致するか判定
    * @param t :チェックする材質特性
    * @return 一致するならtrue
    */
    bool is_same_type(BxDFType t) const { return BxDFType((uint8_t)type & (uint8_t)t) == type; }

    /**
    * @brief 材質特性にtが含まれているか判定
    * @param t :チェックする材質特性
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


/** 完全鏡面反射 */
class SpecularReflection : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale :スケールファクター
    * @param[in] _fres  :フレネル式
    */
    SpecularReflection(Vec3 _scale, std::shared_ptr<Fresnel> _fres);

    /**
    * @brief 誘電体用コンストラクタ
    * @param[in] _scale :スケールファクター
    * @param[in] _n_inside    :内側媒質の屈折率
    * @param[in] _n_outside   :外側媒質の屈折率(通常は空気)
    */
    SpecularReflection(Vec3 _scale, float _n_inside, float _n_outside=1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> スケールファクター */
    std::shared_ptr<Fresnel> fres; /**> フレネル式 */
};


/** 完全鏡面透過 */
class SpecularTransmission : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale     :スケールファクター
    * @param[in] _n_inside  :内側媒質の屈折率
    * @param[in] _n_outside :外側媒質の屈折率(通常は空気)
    * @param[in] _fres      :フレネル式
    */
    SpecularTransmission(Vec3 _scale, float _n_inside, float _n_outside=1.0f, 
                         std::shared_ptr<Fresnel> _fres=nullptr);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> スケールファクター */
    float n_inside;  /**> 内側媒質の屈折率 */
    float n_outside; /**> 外側媒質の屈折率 */
    std::shared_ptr<Fresnel> fres; /**> フレネル項 */
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
    /**
    * @brief 正規化Phong分布からレイの方向をサンプリング
    * @param[in] alpha :分布のパラメータ
    * @return Vec3     :レイの方向
    * @note: [E.Lafortune and Y.Willems 1994]を基に実装
    */
    Vec3 phong_sample(float shine) const;

    Vec3 scale;  /**> 反射係数 */
    float shine; /**> 光沢度 */
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
    MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist, std::shared_ptr<Fresnel> _fres,
                         bool _is_multiple_scattering=false);

    /**
    * @brief 誘電体用コンストラクタ
    * @param[in] _scale     :スケールファクター
    * @param[in] _n_inside  :内側媒質の屈折率
    * @param[in] _n_outside :外側媒質の屈折率(通常は空気)
    */
    MicrofacetReflection(Vec3 _scale, std::shared_ptr<NDF> _dist, 
                         float _n_inside, float _n_outside=1.0f);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    /**
    * @brief 多重散乱テーブルを計算する関数
    */
    void create_multiple_scattering_table();
    /**
    * @brief モンテカルロ推定の重みを計算する関数(多重散乱テーブルの計算に利用)
    * @param[in] theta      :仰角の余弦
    * @param[in] phi        :方位角
    * @param[in] dist_alpha :マイクロファセットNDF
    * @note: VNDFサンプリングに基づき重みを計算しているので注意(TODO: 修正)
    */
    float weight(float cos_theta, float phi, const std::shared_ptr<NDF>& dist_alpha) const;

    Vec3 scale; /**> スケールファクター */
    std::shared_ptr<Fresnel> fres; /**> フレネル式 */
    std::shared_ptr<NDF> dist; /**> マイクロファセット分布 */
    bool is_multiple_scattering; /**> 多重散乱の考慮するならtrue */
    float E_ave; /**> 平均アルベド */
    Vec3 F_ave;  /**> 平均フレネル*/
    float E[1000] = { 0.f }; /**> Directionalアルベド */
    int table_size = sizeof(E) / sizeof(float);
};


/** マイクロファセット透過 */
/** @note 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models */
class MicrofacetTransmission : public BxDF {
public:
    /**
    * @brief コンストラクタ
    * @param[in] _scale     :スケールファクター
    * @param[in] _dist      :マイクロファセット分布
    * @param[in] _n_inside  :内側媒質の屈折率
    * @param[in] _n_outside :外側媒質の屈折率(通常は空気)
    * @param[in] _fres      :フレネル式
    */
    MicrofacetTransmission(Vec3 _scale, std::shared_ptr<NDF> _dist, float _n_inside, 
                           float _n_outside=1.0f, std::shared_ptr<Fresnel> _fres=nullptr);

    float eval_pdf(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 eval_f(const Vec3& wo, const Vec3& wi, const intersection& p) const override;

    Vec3 sample_f(const Vec3& wo, const intersection& p, Vec3& wi, float& pdf) const override;

private:
    Vec3 scale; /**> スケールファクター */
    float n_inside;  /**> 内側媒質の屈折率 */
    float n_outside; /**> 外側媒質の屈折率 */
    std::shared_ptr<NDF> dist; /**> マイクロファセット分布 */
    std::shared_ptr<Fresnel> fres; /**> フレネル項 */
};