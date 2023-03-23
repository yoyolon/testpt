/**
* @file  Film.h
* @brief カメラのフィルムクラス
*/

#pragma once

/** カメラのフィルム情報を保持するクラス */
class Film {
public:
	Film(int _w, int _h, int _c, const char* filename);

	int get_w() const { return w; }
	int get_h() const { return h; }
	int get_c() const { return c; }
	float get_aspect() const { return aspect; }
	const char* get_filename() const { return filename; }

private:
	int w;         /**< 幅           */
	int h;         /**< 高さ         */
	int c;         /**< チャンネル数 */
	float aspect;  /**< アスペクト比 */
	const char* filename;  /**< 出力ファイル名 */
};
