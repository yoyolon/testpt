/**
* @file  Random.h
* @brief 乱数生成とサンプリング
*/

#pragma once

#include "utility.h"
#include "Vec3.h"

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
    * @return Vec3 :サンプリング値
    */
    static Vec3 uniform_disk_sample();

    /**
    * @brief 単位円からの一様なサンプリング(歪みが小さい)
    * @return Vec3 :サンプリング値
    * @note: 参考: https://psgraphics.blogspot.com/2011/01/
    */
    static Vec3 concentric_disk_sample();

    /**
    * @brief 三角形からの一様なサンプリング(重心座標)
    * @return Vec3 :サンプリング値
    */
    static Vec3 uniform_triangle_sample();

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