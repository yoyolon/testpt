#include "Shape.h"

float Shape::sample_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // ref����W�I���g���ւ̃��C
    intersection isect;                    // �W�I���g���̌����_
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.0f;
    }
    auto d = ref.pos - isect.pos;
    return d.length2() / (std::abs(dot(isect.normal, -w)) * area());
}