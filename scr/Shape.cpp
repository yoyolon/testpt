#include "Shape.h"

float Shape::eval_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // refからジオメトリへ向かうレイ
    intersection isect;                    // ジオメトリの交差点
    // refと交差しない場合はPDFはゼロ
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.0f;
    }
    // ジオメトリの裏面をサンプルした場合はPDFはゼロ
    if (dot(isect.normal, -w) < 0) {
        return 0.0f;
    }
    // ジオメトリを一様サンプリングした場合の立体角に関するPDFを返す
    auto d = ref.pos - isect.pos;
    return d.length2() / (std::abs(dot(isect.normal, -w)) * area()); // 測度変換
}