/**
* @file  Ray.h
* @brief レイ
*/

#pragma once

#include "Math.h"

/** レイクラス */
class Ray {
public:
    /**
    * @brief 原点と方向ベクトルからレイを初期化
    * @param[in] o :レイの原点
    * @param[in] d :レイの方向ベクトル
    */
    Ray(Vec3 o, Vec3 d) : origin(o), dir(d) {};

    /**
    * @brief レイの原点を取得する関数
    * @return Vec3 :レイの原点
    */
    Vec3 get_origin() const { return origin; }

    /**
    * @brief レイの方向を取得する関数
    * @return Vec3 :レイの方向ベクトル
    */
    Vec3 get_dir() const { return dir; }

    /**
    * @brief パラメータtにおけるレイの座標を計算する関数
    * @param[in] t :レイのパラメータ
    * @return Vec3 :レイの座標
    */
    inline Vec3 at(float t) const { return origin + t * dir; }

private:
    Vec3 origin; /**< レイの原点 */
    Vec3 dir;    /**< レイの方向 */
};