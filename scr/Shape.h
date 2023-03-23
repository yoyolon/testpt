/**
* @file  Shape.h
* @brief 三次元形状モデルと交差点構造体
*/

#pragma once

#include <vector>
#include "Ray.h"

class Material;


/** 交差点の種類 */
enum class IsectType {
    None     = 1 << 0,  /**< なし     */
    Material = 1 << 1,  /**< 物体表面 */
    Light    = 1 << 2   /**< 光源     */
};

/** 交差点情報 */
struct intersection {
    Vec3 pos;                            /**< 座標             */
    Vec3 normal;                         /**< 法線             */
    float t=0.0f;                        /**< レイのパラメータ */
    bool is_front=true;                  /**< 交差点の裏表     */
    IsectType type=IsectType::None;      /**< 交差点の種類     */
    std::shared_ptr<class Material> mat; /**< 材質の種類       */
    std::shared_ptr<class Light> light;  /**< 光源の種類       */
};


/**
* @brief 物体表面の表裏を判定する関数
* @param[in] r :シェイプへの入射レイ
* @param[in] n :シェイプの法線
* @return bool :表ならtrueを返す
*/
inline bool is_front(const Ray& r, const Vec3  n) {
    return dot(n, -r.get_dir()) > 0; // 物体表面から離れる方向を正にするために-1を乗算
}


/** 三次元形状モデル抽象クラス */
class Shape {
public:
    virtual ~Shape() {};

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
    * @brief ジオメトリサンプリングの立体角に関するPDF(確率密度)を評価する関数
    * @param[in] ref :サンプリング元の交差点情報
    * @param[in] w   :サンプリング方向(ジオメトリに向かう方向が正)
    * @return float  :PDF
    * @detail サンプリングは立体角に関して行う
    */
    virtual float eval_pdf(const intersection& ref, const Vec3& w) const;

    /**
    * @brief ジオメトリサンプリングを行う関数
    * @param[in] ref       :サンプリング元の交差点情報
    * @return intersection :サンプルした交差点情報
    */
    virtual intersection sample(const intersection& ref) const = 0;
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
    Disk(Vec3 c, float r, std::shared_ptr<Material> m, bool is_flip = false);

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
    TriangleMesh(std::string filename, std::shared_ptr<Material> m, bool is_smooth = true);

    bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;

    float area() const override;

    intersection sample(const intersection& ref) const override;

private:
    std::vector<Triangle> Triangles; /**< 三角形配列     */
    std::shared_ptr<Material> mat;   /**< マテリアル     */
};