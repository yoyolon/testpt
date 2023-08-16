/**
* @file  Film.h
* @brief �J�����̃t�B����
*/

#pragma once

#include <string>

/** �t�B�����N���X */
class Film {
public:
	/**
	* @brief �t�B������������
	* @param[in] _w        :�t�B������
	* @param[in] _h        :�t�B�����̍���
	* @param[in] _c        :�t�B�����̃`�F���l����
	* @param[in] _filename :�o�͉摜�̃t�@�C����
	*/
	Film(int _w, int _h, int _c, const std::string& _filename);

	int get_w() const { return w; }
	int get_h() const { return h; }
	int get_c() const { return c; }
	float get_aspect() const { return aspect; }
	const char* get_filename() const { return filename.c_str(); }

private:
	int w;         /**< ��           */
	int h;         /**< ����         */
	int c;         /**< �`�����l���� */
	float aspect;  /**< �A�X�y�N�g�� */
	std::string filename; /**< �o�̓t�@�C���� */
};
