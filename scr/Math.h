/**
* @file  Math.h
* @brief 数学(線形代数)クラス
*/

#pragma once

#include <cmath>
#include <iostream>
#include "utility.h"

// *** 関数 ***
inline float to_degree(float radian) { return 180.0f * radian / pi; }
inline float to_radian(float degree) { return pi * degree / 180.0f; }
inline float lerp(float a, float b, float t) { return (1 - t) * a + t * b; }


/** 三次元ベクトルクラス */
class Vec3 {
public:
    /**
    * @brief 各要素でベクトルを初期化
    * @param[in] x :x成分
    * @param[in] y :y成分
    * @param[in] z :z成分
    */
    Vec3(float x=0.0f, float y=0.0f, float z=0.0f) : e{x, y, z} {};

    /**
    * @brief 配列でベクトルを初期化
    * @param[in] e :各要素を格納した配列s
    */
    Vec3(float* _e) : e{_e[0], _e[1], _e[2]} {};

    // 配列アクセス演算子
    float operator[](int i) const { return e[i]; }
    float& operator[](int i) { return e[i]; }

    // 単項演算子
    Vec3 operator+() const { return *this; }
    Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }

    // 複合代入演算子
    Vec3& operator+=(const Vec3 & a) {
        e[0] += a.e[0];
        e[1] += a.e[1];
        e[2] += a.e[2];
        return *this;
    }
    Vec3& operator-=(const Vec3& a) {
        e[0] -= a.e[0];
        e[1] -= a.e[1];
        e[2] -= a.e[2];
        return *this;
    }
    Vec3& operator*=(float t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }
    Vec3& operator/=(float t) {
        if (t == 0) return *this; // ゼロ除算の処理(暫定)
        float invt = 1.0f / t;
        return *this *= invt;
    }

    // 二項演算子
    friend inline Vec3 operator+(const Vec3& a, const Vec3& b) {
        return Vec3(a.e[0] + b.e[0], a.e[1] + b.e[1], a.e[2] + b.e[2]);
    }
    friend inline Vec3 operator-(const Vec3& a, const Vec3& b) {
        return Vec3(a.e[0] - b.e[0], a.e[1] - b.e[1], a.e[2] - b.e[2]);
    }
    friend inline Vec3 operator*(const Vec3& a, const Vec3& b) {
        return Vec3(a.e[0] * b.e[0], a.e[1] * b.e[1], a.e[2] * b.e[2]);
    }
    friend inline Vec3 operator*(const Vec3& a, float t) {
        return Vec3(a.e[0] * t, a.e[1] * t, a.e[2] * t);
    }
    friend inline Vec3 operator*(float t, const Vec3& a) {
        return Vec3(a.e[0] * t, a.e[1] * t, a.e[2] * t);
    }
    friend inline Vec3 operator/(const Vec3& a, float t) {
        if (t == 0) return a; // ゼロ除算の処理(暫定)
        float invt = 1.0f / t;
        return Vec3(a.e[0] * invt, a.e[1] * invt, a.e[2] * invt);
    }

    /**
    * @brief x成分を取得する関数
    * @return float :ベクトルのx成分
    */
    float get_x() const { return e[0]; }

    /**
    * @brief y成分を取得する関数
    * @return float :ベクトルのy成分
    */
    float get_y() const { return e[1]; }

    /**
    * @brief z成分を取得する関数
    * @return float :ベクトルのz成分
    */
    float get_z() const { return e[2]; }

    /**
    * @brief ベクトルのL2ノルムを計算する関数
    * @return float :ベクトルのL2ノルム
    */
    float length2() const { return e[0] * e[0] + e[1] * e[1] + e[2] * e[2]; }

    /**
    * @brief ベクトルのノルムを計算する関数
    * @return float :ベクトルのノルム
    */
    float length() const { return std::sqrt(length2()); }

    /**
    * @brief ベクトルの平均値を計算する関数
    * @return float :ベクトルの各成分の平均値
    */
    float average() const { return (e[0] + e[1] + e[2]) / 3; }

    /**
     * @brief 2つのベクトルの内積を計算する関数
     * @param[in]  a :ベクトル1
     * @param[in]  b :ベクトル2
     * @return float :ベクトルの内積
     */
    friend inline float dot(Vec3 a, Vec3 b) { 
        return a.e[0] * b.e[0] + a.e[1] * b.e[1] + a.e[2] * b.e[2]; 
    }

     /**
     * @brief 2つのベクトルの外積を計算する関数
     * @param[in]  a :ベクトル1
     * @param[in]  b :ベクトル2
     * @return float :ベクトルの外積
     */
    friend inline Vec3 cross(Vec3 a, Vec3 b) {
        return Vec3(a.e[1] * b.e[2] - a.e[2] * b.e[1],
                    a.e[2] * b.e[0] - a.e[0] * b.e[2],
                    a.e[0] * b.e[1] - a.e[1] * b.e[0]);
    }

private:
    float e[3]; /**< ベクトルの成分 */

public:
    // 定数
    static const Vec3 zero;  /**< ゼロベクトル      */
    static const Vec3 one;   /**< 要素が1のベクトル */
    static const Vec3 red;   /**< 赤色 */
    static const Vec3 green; /**< 緑色 */
    static const Vec3 blue;  /**< 青色 */
};

inline std::ostream& operator<<(std::ostream& s, const Vec3& a) {
    return s << '(' << a.get_x() << ' ' << a.get_y() << ' ' << a.get_z() << ')';
}

/**
* @brief 零ベクトルの判定
* @param[in]  a :ベクトル
* @return bool  :判定結果
*/
inline bool is_zero(const Vec3& a) { return a.length2() > 0 ? false : true; }

/**
* @brief 単位ベクトルを計算する関数
* @param[in]  a :ベクトル
* @return Vec3  :単位ベクトル
*/
inline Vec3 unit_vector(const Vec3& a) { return a / a.length(); }

/**
* @brief 各成分を指定した数値でクランプする関数
* @param[in]  a   :クランプされるベクトル
* @param[in]  min :最小値
* @param[in]  max :最大値
* @return Vec3  :クランプされたベクトル
*/
inline Vec3 clamp(const Vec3& a, float min=0.0f, float max=1.0f) {
    auto x = std::clamp(a.get_x(), min, max);
    auto y = std::clamp(a.get_y(), min, max);
    auto z = std::clamp(a.get_z(), min, max);
    return Vec3(x, y, z); 
}

/**
* @brief 入射ベクトルの正反射方向ベクトルを計算する関数
* @param[in]  w :入射ベクトル
* @param[in]  n :法線ベクトル
* @return Vec3  :正反射方向ベクトル
* @note 物体表面から離れる方向を正とする
*/
inline Vec3 reflect(const Vec3& w, const Vec3& n) {
    return -w + 2 * dot(w, n) * n;
}

/**
* @brief 入射ベクトルの屈折方向ベクトルを計算する関数(全反射ならゼロベクトルを返す)
* @param[in]  w   :入射ベクトル
* @param[in]  n   :法線ベクトル
* @param[in]  eta :相対屈折率(n_theta / n_refract)
* @return Vec3    :スネルの法則に従った屈折方向ベクトル
* @note 物体表面から離れる方向を正とする
*/
inline Vec3 refract(const Vec3& w, const Vec3& n, float eta) {
    auto cos_theta    = dot(w, n);
    auto sin2_theta   = std::max(0.0f, 1.0f - cos_theta * cos_theta);
    auto sin2_refract = eta * eta * sin2_theta;
    // 全反射判定
    if (sin2_refract >= 1.0f) {
        return Vec3::zero;
    }
    auto cos_refract = std::sqrt(1.0f - sin2_refract);
    return eta * -w + (eta * cos_theta - cos_refract) * n;
}


/** 二次元ベクトルクラス */
class Vec2 {
public:
    /**
    * @brief 各要素でベクトルを初期化
    * @param[in] x :x成分
    * @param[in] y :y成分
    */
    Vec2(float x = 0.0f, float y = 0.0f) : e{ x, y} {};

    /**
    * @brief 配列でベクトルを初期化
    * @param[in] e :各要素を格納した配列s
    */
    Vec2(float* _e) : e{ _e[0], _e[1] } {};

    // 配列アクセス演算子
    float operator[](int i) const { return e[i]; }
    float& operator[](int i) { return e[i]; }

    // 単項演算子
    Vec2 operator+() const { return *this; }
    Vec2 operator-() const { return Vec2(-e[0], -e[1]); }

    // 複合代入演算子
    Vec2& operator+=(const Vec2& a) {
        e[0] += a.e[0];
        e[1] += a.e[1];
        return *this;
    }
    Vec2& operator-=(const Vec2& a) {
        e[0] -= a.e[0];
        e[1] -= a.e[1];
        return *this;
    }
    Vec2& operator*=(float t) {
        e[0] *= t;
        e[1] *= t;
        return *this;
    }
    Vec2& operator/=(float t) {
        if (t == 0) return *this; // ゼロ除算の処理(暫定)
        float invt = 1.0f / t;
        return *this *= invt;
    }

    // 二項演算子
    friend inline Vec2 operator+(const Vec2& a, const Vec2& b) {
        return Vec2(a.e[0] + b.e[0], a.e[1] + b.e[1]);
    }
    friend inline Vec2 operator-(const Vec2& a, const Vec2& b) {
        return Vec2(a.e[0] - b.e[0], a.e[1] - b.e[1]);
    }
    friend inline Vec2 operator*(const Vec2& a, const Vec2& b) {
        return Vec2(a.e[0] * b.e[0], a.e[1] * b.e[1]);
    }
    friend inline Vec2 operator*(const Vec2& a, float t) {
        return Vec2(a.e[0] * t, a.e[1] * t);
    }
    friend inline Vec2 operator*(float t, const Vec2& a) {
        return Vec2(a.e[0] * t, a.e[1] * t);
    }
    friend inline Vec2 operator/(const Vec2& a, float t) {
        if (t == 0) return a; // ゼロ除算の処理(暫定)
        float invt = 1.0f / t;
        return Vec2(a.e[0] * invt, a.e[1] * invt);
    }

    /**
    * @brief x成分を取得する関数
    * @return float :ベクトルのx成分
    */
    float get_x() const { return e[0]; }

    /**
    * @brief y成分を取得する関数
    * @return float :ベクトルのy成分
    */
    float get_y() const { return e[1]; }

    /**
    * @brief ベクトルのL2ノルムを計算する関数
    * @return float :ベクトルのL2ノルム
    */
    float length2() const { return e[0] * e[0] + e[1] * e[1]; }

    /**
    * @brief ベクトルのノルムを計算する関数
    * @return float :ベクトルのノルム
    */
    float length() const { return std::sqrt(length2()); }

    /**
    * @brief ベクトルの平均値を計算する関数
    * @return float :ベクトルの各成分の平均値
    */
    float average() const { return (e[0] + e[1]) / 2; }

    /**
     * @brief 2つのベクトルの内積を計算する関数
     * @param[in]  a :ベクトル1
     * @param[in]  b :ベクトル2
     * @return float :ベクトルの内積
     */
    friend inline float dot(Vec2 a, Vec2 b) {
        return a.e[0] * b.e[0] + a.e[1] * b.e[1];
    }

private:
    float e[2]; /**< ベクトルの成分 */

public:
    // 定数
    static const Vec2 zero;  /**< ゼロベクトル      */
    static const Vec2 one;   /**< 要素が1のベクトル */
};


inline std::ostream& operator<<(std::ostream& s, const Vec2& a) {
    return s << '(' << a.get_x() << ' ' << a.get_y() << ')';
}

/**
* @brief 零ベクトルの判定
* @param[in]  a :ベクトル
* @return bool  :判定結果
*/
inline bool is_zero(const Vec2& a) { return a.length2() > 0 ? false : true; }

/**
* @brief 単位ベクトルを計算する関数
* @param[in]  a :ベクトル
* @return Vec2  :単位ベクトル
*/
inline Vec2 unit_vector(const Vec2& a) { return a / a.length(); }

/**
* @brief 各成分を指定した数値でクランプする関数
* @param[in]  a   :クランプされるベクトル
* @param[in]  min :最小値
* @param[in]  max :最大値
* @return Vec2  :クランプされたベクトル
*/
inline Vec2 clamp(const Vec2& a, float min = 0.0f, float max = 1.0f) {
    auto x = std::clamp(a.get_x(), min, max);
    auto y = std::clamp(a.get_y(), min, max);
    return Vec2(x, y);
}