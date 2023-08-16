/**
* @file  Film.h
* @brief カメラのフィルム
*/

#pragma once

#include <string>

/** フィルムクラス */
class Film {
public:
	/**
	* @brief フィルムを初期化
	* @param[in] _w        :フィルム幅
	* @param[in] _h        :フィルムの高さ
	* @param[in] _c        :フィルムのチェンネル数
	* @param[in] _filename :出力画像のファイル名
	*/
	Film(int _w, int _h, int _c, const std::string& _filename);

	int get_w() const { return w; }
	int get_h() const { return h; }
	int get_c() const { return c; }
	float get_aspect() const { return aspect; }
	const char* get_filename() const { return filename.c_str(); }

private:
	int w;         /**< 幅           */
	int h;         /**< 高さ         */
	int c;         /**< チャンネル数 */
	float aspect;  /**< アスペクト比 */
	std::string filename; /**< 出力ファイル名 */
};
