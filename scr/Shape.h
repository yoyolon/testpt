/**
* @file  Shape.h
* @brief 三次元形状モデルと交差点構造体
*/

#pragma once

#include <vector>
#include "Math.h"

class Material;
class Light;
class Ray;


/** 交差点の種類 */
enum class IsectType {
    None     = 1 << 0,  /**< なし     */
    Material = 1 << 1,  /**< 物体表面 */
    Light    = 1 << 2   /**< 光源     */
};

/** 交差点情報 */
struct intersection {
    Vec3 pos;                              /**< 座標             */
    Vec3 normal;                           /**< 法線             */
    float t=0.f;                           /**< レイのパラメータ */
    bool is_front=true;                    /**< 交差点の裏表     */
    IsectType type=IsectType::None;        /**< 交差点の種類     */
    std::shared_ptr<Material> mat=nullptr; /**< 材質の種類       */
    std::shared_ptr<Light> light=nullptr;  /**< 光源の種類       */
};


/** 三次元形状モデル抽象クラス */
class Shape {
public:
    /**
    * @brief シェイプをマテリアルで初期化
    * @param[in] m :シェイプに設定するマテリアル
    */
    Shape(std::shared_ptr<Material> m);

    virtual ~Shape() {};

    void set_mat(const std::shared_ptr<Material> m) { mat = m; }

    /**
    * @brief レイとシェイプの交差判定を行う関数
    * @param[in]  r     :入射レイ
    * @param[in]  t_min :入射レイのパラメータ制限
    * @param[in]  t_max :入射レイのパラメータ制限
    * @param[out] p     :交差点情報
    * @return bool      :交差判定の結果
    */
    virtual bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const = 0;

    /**
    * @brief シェイプの表面積を計算する関数
    * @return float :シェイプの表面積
    */
    virtual float area() const = 0;

    /**
    * @brief シェイプ上の点をサンプリングした場合の立体角に関する確率密度を評価する関数
    * @param[in] ref :サンプリング元の交差点情報
    * @param[in] w   :サンプリング方向(ジオメトリに向かう方向が正)
    * @return float  :立体角に関する確率密度
    * @detail サンプリングは立体角に関して行う
    */
    float eval_pdf(const intersection& ref, const Vec3& w) const;

    /**
    * @brief シェイプ上の点サンプリングする関数
    * @param[in] ref       :サンプリング元の交差点情報
    * @return intersection :サンプルした交差点情報
    */
    virtual intersection sample(const intersection& ref) const = 0;

protected:
    std::shared_ptr<Material> mat; /**< マテリアル */
};


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

private:
    Vec3 center;                   /**< 中心座標   */
    float radius;                  /**< 半径       */
};


/** 三角形クラス */
class Triangle : public Shape {
public:
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
};


/** 三角形メッシュクラス */
class TriangleMesh : public Shape {
public:
    /**
    * @brief 頂点配列とインデックス配列から三角形メッシュシェイプを初期化
    * @param[in] vertices :三角形の頂点配列
    * @param[in] indices  :三角形のインデックス配列
    * @param[in] m        :マテリアル
    */
    TriangleMesh(std::vector<Vec3> vertices, std::vector<Vec3> indices, 
                 std::shared_ptr<Material> m);

    /**
    * @brief .objファイルから三角形メッシュシェイプを初期化
    * @param[in] filename  :.objファイルのパス
    * @param[in] m         :マテリアル
    * @param[in] is_smooth :trueならスムーズシェーディングを適用する
    */
    TriangleMesh(std::string filename, std::shared_ptr<Material> m, bool is_smooth=true);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    std::vector<Triangle> Triangles; /**< 三角形配列     */
};