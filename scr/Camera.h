/**
* @file  Camera.h
* @brief カメラクラス
*/

#pragma once

#include "Ray.h"
#include <iostream>

/** カメラクラス */
class Camera {
public:

    /**
    * @brief カメラを初期化
    * @param[in] h        :フィルムの高さ
    * @param[in] _aspect  :フィルムのアスペクト比
    * @param[in] _fd      :焦点距離
    * @param[in] _pos     :カメラの原点
    * @param[in] _forward :カメラの方向ベクトル
    */
    Camera(float h=2.0, float _aspect=1.0, float _fd=1.0, 
           Vec3 _pos=Vec3(0.0, 0.0, 0.0), Vec3 _forward=Vec3(0.0, 0.0, 1.0));

    /**
    * @brief カメラの原点からレイを生成
    * @param[in] u :方向調整パラメータ(横方向)
    * @param[in] v :方向調整パラメータ(縦方向)
    * @return Ray  :レイ
    */
    Ray generate_ray(float u, float v);

private:
    float film_w;        /**< フィルム幅           */
    float film_h;        /**< フィルム高さ         */
    float aspect;        /**< アスペクト比         */
    float fd;            /**< 焦点距離             */
    Vec3 pos;            /**< カメラの原点         */
    Vec3 forward_vector; /**< 前方ベクトル         */
    Vec3 right_vector;   /**< 右ベクトル           */
    Vec3 up_vector;      /**< 上ベクトル           */
    Vec3 film_corner;    /**< フィルム左下端の座標 */
};
