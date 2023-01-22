/**
* @file  Fresnel.h
* @brief フレネル式
*/

#pragma once

#include "Vec3.h"

/** フレネル式の抽象クラス */
class Fresnel {
public:
    virtual ~Fresnel() {};

    /**
    * @brief 入射角によるフレネル反射率を計算する関数
    * @param[in] cos_theta :入射角余弦
    * @return Vec          :フレネル反射率
    * @note: 入射角は法線ベクトルと入射方向ベクトルがなす角
    */
    virtual Vec3 eval(float cos_theta, const struct intersection& p) const = 0;
};

/** 
* 一定反射率クラス
* @note: フレネルの式に従っていないので注意
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

/** Schlickフレネルクラス */
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


/** 誘電体フレネルクラス */
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


/** 誘電体単層薄膜干渉フレネルクラス */
class FresnelThinfilm : public Fresnel {
public:
    /**
    * @brief コンストラクタ
    * @param[in]  _d :薄膜の膜厚
    * @param[in] _ni :入射方向媒質の屈折率
    * @param[in] _nf :薄膜の屈折率
    * @param[in] _no :出射方向媒質の屈折率
    */
    FresnelThinfilm(float _d, float _ni, float _nf, float _no);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    float d;          /**< 膜厚   */
    float ni, nf, no; /**< 屈折率 */
};


/** LUTフレネルクラス */
class FresnelLUT : public Fresnel {
public:
    /**
    * @brief コンストラクタ
    * @param[in]  _filename :反射率テーブルのパス
    */
    FresnelLUT(std::string filename);

    /**
    * @brief コンストラクタ
    * @param[in]  _filename :反射率テーブルのパス
    */
    void load_LUT(std::string filename);

    Vec3 eval(float cos_theta, const struct intersection& p) const override;

private:
    std::vector<Vec3> table; /**< フレネル反射率テーブル */
};
