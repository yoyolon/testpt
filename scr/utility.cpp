#include "utility.h"
#include "Math.h"


float gamma_correction_element(float c) {
    // ƒKƒ“ƒ}•â³
    if (c < 0.0031308f) {
        return 12.92f * c;
    }
    return 1.055f * std::powf(c, 1 / 2.4f) - 0.055f;
}

Vec3 gamma_correction(const Vec3& color) {
    // ƒKƒ“ƒ}•â³
    auto r = gamma_correction_element(color.get_x());
    auto g = gamma_correction_element(color.get_y());
    auto b = gamma_correction_element(color.get_z());
    return Vec3(r, g, b);
}