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
    */
    Camera();

    /**
    * @brief カメラを初期化
    * @param[in] film     :フィルム
    * @param[in] _fd      :焦点距離
    * @param[in] _pos     :カメラの原点
    * @param[in] _forward :カメラの方向ベクトル
    */
    Camera(std::shared_ptr<class Film> _film, float _fd,  Vec3 _pos, Vec3 _forward);

    int get_h() const;
    int get_w() const;
    int get_c() const;
    const char* get_filename() const;

    /**
    * @brief カメラの原点からレイを生成
    * @param[in] u :方向調整パラメータ(横方向)
    * @param[in] v :方向調整パラメータ(縦方向)
    * @return Ray  :レイ
    */
    Ray generate_ray(float u, float v);

private:
    float fd;         /**< 焦点距離             */
    Vec3 pos;         /**< カメラの原点         */
    Vec3 forward;     /**< 前方ベクトル         */
    Vec3 right;       /**< 右ベクトル           */
    Vec3 up;          /**< 上ベクトル           */
    Vec3 film_corner; /**< フィルム左下端の座標 */
    std::shared_ptr<class Film> film; /**< フィルム             */
};
