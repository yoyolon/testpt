#include "utility.h"
#include <sstream>
#include "Math.h"


float gamma_correction_element(float c) {
    //return std::powf(c, 1 / 2.4f);
    // ガンマ補正
    if (c <= 0.0031308f) {
        return 12.92f * c;
    }
    return 1.055f * std::powf(c, 1 / 2.4f) - 0.055f;
}

Vec3 gamma_correction(const Vec3& color) {
    auto r = gamma_correction_element(color.get_x());
    auto g = gamma_correction_element(color.get_y());
    auto b = gamma_correction_element(color.get_z());
    return Vec3(r, g, b);
}

float inv_gamma_correction_element(float c) {
    //return std::powf(c, 2.4f);
    // 逆ガンマ補正
    if (c <= 0.04045f) {
        return  c / 12.92f;
    }
    return (c + 0.055f) / (std::powf(1.055f, 2.4f));
}

Vec3 inv_gamma_correction(const Vec3& color) {
    auto r = inv_gamma_correction_element(color.get_x());
    auto g = inv_gamma_correction_element(color.get_y());
    auto b = inv_gamma_correction_element(color.get_z());
    return Vec3(r, g, b);
}


Vec3 exclude_invalid(const Vec3& color) {
    float r = color.get_x();
    float g = color.get_y();
    float b = color.get_z();
    // 無効な値をゼロにする
    if (!isfinite(r)) r = 0.f;
    if (!isfinite(g)) g = 0.f;
    if (!isfinite(b)) b = 0.f;
    return Vec3(r, g, b);
}


std::vector<std::string> split_string(const std::string& line, char delimiter) {
    std::stringstream ss(line);
    std::string temp;
    std::vector<std::string> ret;
    // EOFまで入力ストリームをdelimiterで区切る
    while (std::getline(ss, temp, delimiter)) {
        if (temp == "") continue; // 空行はスキップ
        ret.push_back(temp);
    }
    return ret;
}