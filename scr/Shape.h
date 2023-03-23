/**
* @file  Shape.h
* @brief 三次元形状モデルの抽象クラスと交差点構造体
*/

#pragma once

#include "Ray.h"

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