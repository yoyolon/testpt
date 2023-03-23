/**
* @file  Spehre.h
* @brief 球・円盤・円柱クラス
*/

#pragma once

#include "Shape.h"

class Material;

/** 球クラス */
class Sphere : public Shape {
public:
    /**
    * @brief 中心座標と半径から球シェイプを初期化
    * @param[in] c :中心座標
    * @param[in] r :半径
    * @param[in] m :マテリアル
    */
    Sphere(Vec3 c, float r, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

    /**
    * @brief ジオメトリサンプリングの確率密度を計算する関数(オーバーライド)
    * @param[in] p  :サンプリング元の交差点情報
    * @param[in] w  :サンプリング方向(光源に向かう方向が正)
    * @return float :確率密度
    * @note 球の可視領域を考慮して半球方向から一様サンプリング
    */
    float eval_pdf(const intersection& ref, const Vec3& w) const override;

private:
    Vec3 center;                   /**< 中心座標   */
    float radius;                  /**< 半径       */
    std::shared_ptr<Material> mat; /**< マテリアル */
};


/** 円盤クラス */
class Disk : public Shape {
public:
    /**
    * @brief 中心座標と半径から円盤シェイプを初期化
    * @param[in] c       :中心座標
    * @param[in] r       :半径
    * @param[in] m       :マテリアル
    * @param[in] is_flip :法線を反転するかどうか
    */
    Disk(Vec3 c, float r, std::shared_ptr<Material> m, bool is_flip=false);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 center;                   /**< 中心座標     */
    float radius;                  /**< 半径         */
    std::shared_ptr<Material> mat; /**< マテリアル   */
    bool is_flip_normal;           /**< 法線の反転   */
    const static Vec3 normal;      /**< 法線(下向き) */
};


/** 円柱クラス */
class Cylinder : public Shape {
public:
    /**
    * @brief 中心座標と半径と高さから円柱シェイプを初期化
    * @param[in] c :中心座標
    * @param[in] r :半径
    * @param[in] h :高さ
    * @param[in] m :マテリアル
    */
    Cylinder(Vec3 c, float r, float h, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 center;                   /**< 中心座標   */
    float radius;                  /**< 半径       */
    float height;                  /**< 高さ       */
    std::shared_ptr<Material> mat; /**< マテリアル */
};