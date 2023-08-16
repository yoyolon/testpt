/**
* @file  MakeScene.h
* @brief シーンデータの実装
* @note  ワールド座標系は手前方向をz軸正とする右手系(x軸正:右，y軸正:上，z軸正:手前)
*/

#pragma once


class Camera;
class Scene;


/**
* @brief シンプルな球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
void make_scene_simple(Scene& world, Camera& cam);

/**
* @brief シンプルな球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
void make_scene_simple2(Scene& world, Camera& cam);


/**
* @brief 重点的サンプリング検証用シーン
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note モデル出典: https://pbrt.org/scenes-v3
*       このソフトウェアに上記モデルは付属しておりません.
*/
void make_scene_MIS(Scene& world, Camera& cam);


/**
* @brief コーネルボックスのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note 参考: http://www.graphics.cornell.edu/online/box/data.html
*/
void make_scene_cornell_box(Scene& world, Camera& cam);


/**
* @brief コーネルボックスと球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note 参考: http://www.graphics.cornell.edu/online/box/data.html
*/
void make_scene_box_with_sphere(Scene& world, Camera& cam);


/**
* @brief 花瓶モデルのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note モデル出典: https://polyhaven.com/a/ceramic_vase_01
*       このソフトウェアに上記モデルは付属しておりません.
*/
void make_scene_vase(Scene& world, Camera& cam);


/**
* @brief 薄膜干渉を含むシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
void make_scene_thinfilm(Scene& world, Camera& cam);