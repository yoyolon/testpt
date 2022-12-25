/**
* @file  Spehre.h
* @brief 球・円盤・円柱クラス
*/

#pragma once

#include "Material.h"
#include "Shape.h"

/** 球クラス */
class Sphere : public Shape {
public:
    /**
    * @brief 中心座標と半径から球オブジェクトを初期化
    * @param[in] c :中心座標
    * @param[in] r :半径
    * @param[in] m :マテリアル
    */
    Sphere(Vec3 c, float r, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 center;                   /**< 中心座標   */
    float radius;                  /**< 半径       */
    std::shared_ptr<Material> mat; /**< マテリアル */
};


/** 円盤クラス */
class Disk : public Shape {
public:
    /**
    * @brief 中心座標と半径から円盤オブジェクトを初期化
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
    * @brief 中心座標と半径と高さから円柱オブジェクトを初期化
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