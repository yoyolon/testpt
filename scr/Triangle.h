/**
* @file  Triangle.h
* @brief 三角形クラスと三角形メッシュクラス
*/

#pragma once

#include <vector>
#include "Shape.h"

class Material;

/** 三角形クラス */
class Triangle : public Shape {
public:
    Triangle();

    /**
    * @brief 頂点から三角形シェイプを初期化
    * @param[in] v0 :三角形の頂点
    * @param[in] v1 :三角形の頂点
    * @param[in] v2 :三角形の頂点
    * @param[in] m  :マテリアル
    */
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<Material> m);

    /**
    * @brief 頂点とその法線から三角形シェイプを初期化
    * @param[in] v0 :三角形の頂点
    * @param[in] v1 :三角形の頂点
    * @param[in] v2 :三角形の頂点
    * @param[in] n0 :頂点v0の法線
    * @param[in] n1 :頂点v1の法線
    * @param[in] n2 :頂点v2の法線
    * @param[in] m  :マテリアル
    */
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 n0, Vec3 n1, Vec3 n2, std::shared_ptr<Material> m);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    Vec3 V0, V1, V2;               /**< 頂点       */
    Vec3 N0, N1, N2;               /**< 法線       */
    std::shared_ptr<Material> mat; /**< マテリアル */

};


/** 三角形メッシュクラス */
class TriangleMesh : public Shape {
public:
    TriangleMesh();

    /**
    * @brief 頂点配列とインデックス配列から三角形メッシュシェイプを初期化
    * @param[in] vertices :三角形の頂点配列
    * @param[in] indices  :三角形のインデックス配列
    * @param[in] m        :マテリアル
    * @param[in] p        :位置オフセット
    */
    TriangleMesh(std::vector<Vec3> vertices, std::vector<Vec3> indices, std::shared_ptr<Material> m);

    /**
    * @brief objファイルから三角形メッシュシェイプを初期化
    * @param[in] filenames :三角形の頂点配列
    * @param[in] m         :マテリアル
    * @param[in] p         :位置オフセット
    * @param[in] is_smooth :スムーズシェーディングの設定 
    */
    TriangleMesh(std::string filename, std::shared_ptr<Material> m, bool is_smooth=true);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    std::vector<Triangle> Triangles; /**< 三角形配列     */
    std::shared_ptr<Material> mat;   /**< マテリアル     */
};