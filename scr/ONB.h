#pragma once

#include "Vec3.h"

// *** 正規直交基底(ONB) ***
class ONB {
public:
	// コンストラクタ
	ONB() {}
	ONB(const Vec3& _s, const Vec3& _t, const Vec3& _n) : s(_s), t(_t), n(_n) {}

	// ゲッタ
	Vec3 get_s() const { return s; }
	Vec3 get_t() const { return t; }
	Vec3 get_n() const { return n; }

	// 法線からONBを構築
	void build_ONB(const Vec3& normal);

	// 座標変換
	Vec3 to_local(const Vec3& a) const;
	Vec3 to_world(const Vec3& a) const;

private:
	Vec3 s, t, n; // 正規直交ベクトル
};