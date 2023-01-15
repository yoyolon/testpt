/**
* @file  MakeScene.h
* @brief シーンデータの実装
*/

#pragma once

/**
* @brief シンプルな球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
void make_scene_simple(class Scene& world, class Camera& cam);

/**
* @brief 円柱のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
void make_scene_cylinder(class Scene& world, class Camera& cam);


/**
* @brief 重点的サンプリング検証用シーン
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note モデル出典: https://pbrt.org/scenes-v3
*       このソフトウェアに上記モデルは付属しておりません.
*/
void make_scene_MIS(class Scene& world, class Camera& cam);


/**
* @brief コーネルボックスのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note 参考: http://www.graphics.cornell.edu/online/box/data.html
*/
void make_scene_cornell_box(class Scene& world, class Camera& cam);


/**
* @brief コーネルボックスと球のシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note 参考: http://www.graphics.cornell.edu/online/box/data.html
*/
void make_scene_box_with_sphere(class Scene& world, class Camera& cam);


/**
* @brief 花瓶モデルのシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
* @note モデル出典: https://polyhaven.com/a/ceramic_vase_01
*       このソフトウェアに上記モデルは付属しておりません.
*/
void make_scene_vase(class Scene& world, class Camera& cam);


/**
* @brief 薄膜干渉を含むシーンを生成する関数
* @param[out] world :シーンデータ
* @param[out] cam   :カメラデータ
*/
void make_scene_thinfilm(class Scene& world, class Camera& cam);