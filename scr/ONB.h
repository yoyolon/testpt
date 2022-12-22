/**
* @file  ONB.h
* @brief 正規直交基底(ONB)クラス
* @note  正規直交基底はz軸正の方向を上向きとする
*/

#pragma once

#include "Vec3.h"

/** 正規直交基底クラス */
class ONB {
public:
    /**
    * @brief デフォルトコンストラクタ
    */
    ONB() {}

    /**
    * @brief 法線ベクトル(上向きベクトル)からONBを構築
    * @param[in]  Vec3 :上向きの軸ベクトル
    */
    ONB(const Vec3& _n) : n(_n) { build_ONB(n); }

    /**
    * @brief 基底ベクトルを取得する関数
    * @return Vec3 :s軸の基底ベクトル
    */
    Vec3 get_s() const { return s; }

    /**
    * @brief 基底ベクトルを取得する関数
    * @return Vec3 :t軸の基底ベクトル
    */
    Vec3 get_t() const { return t; }

    /**
    * @brief 基底ベクトルを取得する関数
    * @return Vec3 :n軸の基底ベクトル
    */
    Vec3 get_n() const { return n; }

    /**
    * @brief 法線から正規直交基底(ONB)を構築する関数
    * @param[in]  Vec3 :ワールド座標系の法線ベクトル
    * @note 参考: Shirly. "Realistic Ray Tracing". 2007.
    */
    void build_ONB(const Vec3& normal);

    /**
    * @brief ワールド座標系のベクトルをONB座標系のベクトルに変換する関数
    * @param[in]  Vec3 :ワールド座標のベクトル
    * @return Vec3     :ONB座標系のベクトル
    */
    Vec3 to_local(const Vec3& a) const;

    /**
    * @brief ONB座標系のベクトルをワールド座標系のベクトルに変換する関数
    * @param[in]  Vec3 :ONB座標のベクトル
    * @return Vec3     :ワールド座標系のベクトル
    */
    Vec3 to_world(const Vec3& a) const;

private:
    Vec3 s, t, n; /**< 正規直交ベクトル */
};