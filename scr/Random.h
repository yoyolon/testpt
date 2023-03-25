/**
* @file  Random.h
* @brief 乱数生成とサンプリング
*/

#pragma once

#include "utility.h"
#include "Math.h"

/** 乱数生成クラス */
class Random {
public:
    /**
    * @brief 乱数の初期化する関数
    */
    static void init();

    /**
    * @brief 一様乱数[0, 1]を生成する関数
    * @return float :サンプリング値
    */
    static float uniform_float();

    /**
    * @brief 一様乱数[min, max]を生成する関数
    * @param[in] min :最小値
    * @param[in] max :最大値
    * @return float  :サンプリング値
    */
    static float uniform_float(float min, float max);

    /**
    * @brief 整数一様乱数[min, max]を生成する関数
    * @param[in] min :最小値
    * @param[in] max :最大値
    * @return int    :サンプリング値
    */
    static int uniform_int(int min, int max);

    /**
    * @brief 単位円からの一様なサンプリング
    * @return Vec2 :サンプリング値(x,y)座標
    */
    static Vec2 uniform_disk_sample();

    /**
    * @brief 単位円からの一様なサンプリング(歪みが小さい)
    * @return Vec2 :サンプリング値(x,y)座標
    * @note: 参考: https://psgraphics.blogspot.com/2011/01/
    */
    static Vec2 concentric_disk_sample();

    /**
    * @brief 三角形からの一様なサンプリング(重心座標)
    * @return Vec3 :サンプリング値(重心座標)
    */
    static Vec2 uniform_triangle_sample();

    /**
    * @brief 単位球からの一様な方向ベクトルをサンプリング
    * @return Vec3 :サンプリング値
    */
    static Vec3 uniform_sphere_sample();

    /**
    * @brief 単位半球からの一様な方向ベクトルをサンプリング
    * @return Vec3 :サンプリング値
    */
    static Vec3 uniform_hemisphere_sample();

    /**
    * @brief 単位半球からのコサイン重みの方向サンプリング
    * @return Vec3 :サンプリング値
    * @note: コサインは法線とサンプリング方向のなす角
    * @note: Malleyの手法を利用
    */
    static Vec3 cosine_hemisphere_sample();

    /**
    * @brief 正規化Phong分布から散乱方向をサンプリング
    * @param[in] float :分布のパラメータ
    * @return Vec3     :サンプリング値
    * @note: [E.Lafortune and Y.Willems 1994]を基に実装
    */
    static Vec3 phong_sample(float shine);

    /**
    * @brief Trowbridge-Reitz(GGX)分布からハーブベクトルをサンプリング
    * @param[in] float :分布のパラメータ
    * @return Vec3     :サンプリング値
    * @note: 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
    */
    static Vec3 ggx_sample(float alpha);

    /**
    * @brief Beckmann分布からハーブベクトルをサンプリング
    * @param[in] float :分布のパラメータ
    * @return Vec3     :サンプリング値
    * @note: 参考: https://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
    */
    static Vec3 beckmann_sample(float alpha);

    /**
    * @brief 多重重点的サンプリングの重みを計算する関数
    * @param[in] n1   :1つ目の関数のサンプル数
    * @param[in] pdf1 :1つ目の関数の確率密度
    * @param[in] n2   :2つ目の関数のサンプル数
    * @param[in] pdf2 :2つ目の関数の確率密度
    * @return float   :重み
    * @note: 参考: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Importance_Sampling
    */
    static float balance_heuristic(int n1, float pdf1, int n2, float pdf2);

    /**
    * @brief 多重重点的サンプリングの重みを計算する関数
    * @param[in] n1   :1つ目の関数のサンプル数
    * @param[in] pdf1 :1つ目の関数の確率密度
    * @param[in] n2   :2つ目の関数のサンプル数
    * @param[in] pdf2 :2つ目の関数の確率密度
    * @return float   :重み
    * @note: 参考: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Importance_Sampling
    */
    static float power_heuristic(int n1, float pdf1, int n2, float pdf2, float beta=2.0f);
};

static std::mt19937 mt; /**< 乱数生成器 */


/** 1D区分関数 */
class Piecewise1D {
    // 参考: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/Sampling_Random_Variables
public:
    /**
    * @brief コンストラクタ
    * @param[in] _f :離散化かれた関数配列
    * @param[in] _n :配列の要素数
    */
    Piecewise1D(const float* data, int n);

    int get_n() const { return n; }
    float get_f(int index) const { return f[index]; }
    float get_integral_f() const { return integral_f; }

    /**
    * @brief 逆関数法でf(x)からxをサンプルしてそのPDF(確率密度)を評価する関数
    * @param[out] pdf   :サンプリングPDF
    * @param[out] index :サンプリング値の配列インデックス
    * @return float     :サンプルしたxの値(f(x)でなくxを返すので注意)
    */
    float sample(float& pdf, int& index) const;

private:
    std::vector<float> f;   /**< 1D区分関数の配列      */
    int n;                  /**< 配列の要素数          */
    std::vector<float> cdf; /**< CDF(累積分布関数)     */
    float integral_f;       /**< fを定義域で積分した値 */
};


/** 2D区分関数 */
class Piecewise2D {
    // 参考: https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations
public:
    /**
    * @brief コンストラクタ
    * @param[in] data :離散化かれた関数配列
    * @param[in] _nu :u方向の要素数
    * @param[in] _nv :v方向の要素数
    */
    Piecewise2D(const float* data, int _nu, int _nv);

    int get_nu() const { return nu; }
    int get_nv() const { return nv; }

    /**
    * @brief 逆関数法でf(u, v)から(u, v)をサンプルしてそのPDF(確率密度)を評価する関数
    * @param[out] pdf :サンプリングPDF
    * @return float   :サンプルした(u,v)の値(f(u,v)でなく(u,v)を返すので注意)
    */
    Vec2 sample(float& pdf) const;

    /**
    * @brief (u, v)をサンプリングするPDF(確率密度)を評価する関数
    * @param[out] uv:(u,v)座標
    * @return float   :サンプルした(u,v)の値(f(u,v)でなく(u,v)を返すので注意)
    */
    float eval_pdf(const Vec2& uv) const;

private:
    int nu; /**< u方向の要素数 */
    int nv; /**< v方向の要素数 */
    std::vector<std::unique_ptr<Piecewise1D>> conditional_pdf; /**< 条件付き確率密度(p[u|v]) */
    std::unique_ptr<Piecewise1D> merginal_pdf; /**< 周辺確率密度(p[v]) */
};