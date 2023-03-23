/**
* @file  Film.h
* @brief �J�����̃t�B�����N���X
*/

#pragma once

/** �J�����̃t�B��������ێ�����N���X */
class Film {
public:
	Film(int _w, int _h, int _c, const char* filename);

	int get_w() const { return w; }
	int get_h() const { return h; }
	int get_c() const { return c; }
	float get_aspect() const { return aspect; }
	const char* get_filename() const { return filename; }

private:
	int w;         /**< ��           */
	int h;         /**< ����         */
	int c;         /**< �`�����l���� */
	float aspect;  /**< �A�X�y�N�g�� */
	const char* filename;  /**< �o�̓t�@�C���� */
};
