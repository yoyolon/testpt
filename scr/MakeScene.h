/**
* @file  MakeScene.h
* @brief �V�[���f�[�^�̎���
*/

#pragma once

/**
* @brief �V���v���ȋ��̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
*/
void make_scene_simple(class Scene& world, class Camera& cam);

/**
* @brief �~���̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
*/
void make_scene_cylinder(class Scene& world, class Camera& cam);


/**
* @brief �d�_�I�T���v�����O���ؗp�V�[��
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
* @note ���f���o�T: https://pbrt.org/scenes-v3
*       ���̃\�t�g�E�F�A�ɏ�L���f���͕t�����Ă���܂���.
*/
void make_scene_MIS(class Scene& world, class Camera& cam);


/**
* @brief �R�[�l���{�b�N�X�̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
* @note �Q�l: http://www.graphics.cornell.edu/online/box/data.html
*/
void make_scene_cornell_box(class Scene& world, class Camera& cam);


/**
* @brief �R�[�l���{�b�N�X�Ƌ��̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
* @note �Q�l: http://www.graphics.cornell.edu/online/box/data.html
*/
void make_scene_box_with_sphere(class Scene& world, class Camera& cam);


/**
* @brief �ԕr���f���̃V�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
* @note ���f���o�T: https://polyhaven.com/a/ceramic_vase_01
*       ���̃\�t�g�E�F�A�ɏ�L���f���͕t�����Ă���܂���.
*/
void make_scene_vase(class Scene& world, class Camera& cam);


/**
* @brief ���������܂ރV�[���𐶐�����֐�
* @param[out] world :�V�[���f�[�^
* @param[out] cam   :�J�����f�[�^
*/
void make_scene_thinfilm(class Scene& world, class Camera& cam);