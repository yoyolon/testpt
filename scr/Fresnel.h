/**
* @file  Fresnel.h
* @brief フレネル式
*/

#pragma once

#include "Math.h"

/** フレネル式の抽象クラス */
class Fresnel {
public:
    virtual ~Fresnel() {};

    /**
    * @brief フレネル反射率を評価する関数
    * @param[in] cos_theta :入射角余弦
    * @return Vec          :フレネル反射率
    * @note: 入射角は法線ベクトルと入射方向ベクトルがなす角
    */
    virtual Vec3 eval(float cos_theta, const struct intersection& p) const = 0;
};

/** 
* 一定反射率クラス
* @note: フレネル式に従っていないので注意
*/
class FresnelConstant : public Fresnel {
public:
    /**
    * @brief コンストラクタ
    * @param[in]  _F0 :
    */
    FresnelConstant(Vec3 _F0);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    Vec3 F0; /**< 反射率 */
};

/** Schlickによるフレネル式の近似クラス */
class FresnelSchlick : public Fresnel {
public:
    /**
    * @brief コンストラクタ
    * @param[in]  _F0 :垂直入射でのフレネル反射率
    */
    FresnelSchlick(Vec3 _F0);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    Vec3 F0; /**< 垂直入射でのフレネル反射率 */
};


/** 誘電体フレネル式クラス */
class FresnelDielectric : public Fresnel {
public:
    /**
    * @brief コンストラクタ
    * @param[in]  _n_inside  :内側媒質の屈折率
    * @param[in]  _n_outside :外側媒質の屈折率
    */
    FresnelDielectric(float _n_inside, float _n_outisde=1.0f);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    float n_inside, n_outside; /**< 屈折率 */
};


/** 誘電体単層薄膜干渉フレネル式クラス */
class FresnelThinfilm : public Fresnel {
public:
    /**
    * @brief コンストラクタ
    * @param[in]  _thickness :薄膜の膜厚
    * @param[in]  _n_inside  :内側媒質の屈折率
    * @param[in]  _n_film    :薄膜媒質の屈折率
    * @param[in]  _n_outside :外側媒質の屈折率
    */
    FresnelThinfilm(float _thickness, float _n_inside, float _n_film, float _n_outside=1.0f);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    float thickness; /**< 薄膜の膜厚 */
    float n_inside, n_film, n_outside; /**< 屈折率 */
};


/** 反射率テーブル(1度間隔)によるフレネル式クラス */
class FresnelLUT : public Fresnel {
public:
    /**
    * @brief コンストラクタ
    * @param[in]  _filename :反射率テーブル(csv形式)のパス
    */
    FresnelLUT(std::string filename);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    /**
    * @brief 反射率テーブル(csv形式)を読み込む関数
    * @param[in]  _filename :反射率テーブルのパス
    */
    void load_LUT(std::string filename);

    std::vector<Vec3> table; /**< フレネル反射率テーブル */
};
