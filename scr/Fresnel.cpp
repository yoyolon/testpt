#include "Fresnel.h"
#include "Shape.h"
#include <complex>
#include <fstream>
#include <iostream>

/**
* @brief s偏光のフレネル反射係数を計算する関数
* @param[in] ni   :媒質iの屈折率
* @param[in] nj   :媒質jの屈折率
* @param[in] cosi :媒質iへの入射角余弦
* @param[in] cosj :媒質jへの入射角余弦
* @return float   :s偏光のフレネル反射係数
*/
inline float fresnel_rs(float ni, float nj, float cosi, float cosj) {
    return (ni * cosi - nj * cosj) / (ni * cosi + nj * cosj);
}

/**
* @brief s偏光のフレネル透過係数を計算する関数
* @param[in] ni   :媒質iの屈折率
* @param[in] nj   :媒質jの屈折率
* @param[in] cosi :媒質iへの入射角余弦
* @param[in] cosj :媒質jへの入射角余弦
* @return float   :s偏光のフレネル透過係数
*/
inline float fresnel_ts(float ni, float nj, float cosi, float cosj) {
    return (ni * cosi + ni * cosi) / (ni * cosi + nj * cosj);
}

/**
* @brief p偏光のフレネル反射係数を計算する関数
* @param[in] ni   :媒質iの屈折率
* @param[in] nj   :媒質jの屈折率
* @param[in] cosi :媒質iへの入射角余弦
* @param[in] cosj :媒質jへの入射角余弦
* @return float   :s偏光のフレネル反射係数
*/
inline float fresnel_rp(float ni, float nj, float cosi, float cosj) {
    return (nj * cosi - ni * cosj) / (ni * cosj + nj * cosi);
}

/**
* @brief p偏光のフレネル透過係数を計算する関数
* @param[in] ni   :媒質iの屈折率
* @param[in] nj   :媒質jの屈折率
* @param[in] cosi :媒質iへの入射角余弦
* @param[in] cosj :媒質jへの入射角余弦
* @return float   :s偏光のフレネル透過係数
*/
inline float fresnel_tp(float ni, float nj, float cosi, float cosj) {
    return (ni * cosi + ni * cosi) / (ni * cosj + nj * cosi);
}


/**
* @brief 干渉を考慮したフレネル反射係数を計算する関数
* @param[in] r            :フレネル反射係数
* @param[in] R            :フレネル反射係数
* @param[in] phi          :位相差
* @return complex<float>  :合成反射係数
* @note: [H.Hirayama et al. 2001]を基に実装
*/
static std::complex<float> composit_r(float r, float R, float phi) {
    std::complex<float>i(0.f, 1.0f);
    return (r + R * exp(i * 2.0f * phi)) / (1.0f + r * R * exp(i * 2.0f * phi));
}

/**
* @brief 干渉を考慮したフレネル透過係数を計算する関数
* @param[in] r            :フレネル反射係数
* @param[in] R            :フレネル反射係数
* @param[in] t            :フレネル透過係数
* @param[in] T            :フレネル透過係数
* @param[in] phi          :位相差
* @return complex<float>  :合成透過係数
* @note: [H.Hirayama et al. 2001]を基に実装
*/
static std::complex<float> composit_t(float r, float R, float t, float T, float phi) {
    std::complex<float>i(0.f, 1.0f);
    return (t * T * exp(i * phi)) / (1.0f + r * R * exp(i * 2.0f * phi));
}

/**
* @brief 干渉を考慮した反射率を計算する関数
* @param[in] cos0 :入射角余弦
* @param[in] d    :薄膜の膜厚
* @param[in] n0   :入射媒質の屈折率
* @param[in] n1   :薄膜の屈折率
* @param[in] n2   :出射媒質の屈折率
* @return Vec3    :薄膜干渉を考慮した反射率
*/
Vec3 irid_reflectance(float cos0, float d, float n0, float n1, float n2) {
    // 透過角計算
    float sin0 = std::sqrt(std::max(0.f, 1.0f - cos0 * cos0));
    float sin1 = n0 / n1 * sin0;
    if (sin1 >= 1.0) return  Vec3(1.0f, 1.0f, 1.0f); // 全反射
    float cos1 = std::sqrt(std::max(0.f, 1.0f - sin1 * sin1));
    float sin2 = n0 / n2 * sin0;
    if (sin2 >= 1.0) return Vec3(1.0f, 1.0f, 1.0f);  // 全反射
    float cos2 = std::sqrt(std::max(0.f, 1 - sin2 * sin2));
    // フレネル反射係数計算
    float r01s = fresnel_rs(n0, n1, cos0, cos1);
    float r12s = fresnel_rs(n1, n2, cos1, cos2);
    float r01p = fresnel_rp(n0, n1, cos0, cos1);
    float r12p = fresnel_rp(n1, n2, cos1, cos2);
    // 反射率計算
    float lambda[3] = { 640.0, 540.0, 450.0 };
    float rgb[3] = { 0.0 };
    std::complex<float>r_s[3];
    std::complex<float>r_p[3];
    for (int i = 0; i < 3; i++) {
        float phi = 2 * pi / lambda[i] * n1 * d * cos1;
        r_s[i] = composit_r(r01s, r12s, phi); // 合成反射係数(s波)
        r_p[i] = composit_r(r01p, r12p, phi); // 合成反射係数(p波)
        rgb[i] = (norm(r_s[i]) + norm(r_p[i])) / 2;
    }
    return Vec3(rgb);
}


// *** 一定反射率 ***

FresnelConstant::FresnelConstant(Vec3 _F0)
    : F0(_F0) {}

Vec3 FresnelConstant::eval(float cos_theta, const intersection& p) const {
    return F0;
}


// *** Schlickフレネル ***

FresnelSchlick::FresnelSchlick(Vec3 _F0)
    : F0(_F0) {}

Vec3 FresnelSchlick::eval(float cos_theta, const intersection& p) const {
    return F0 + (Vec3(1.0f,1.0f,1.0f) - F0) * (float)std::pow(1.0f - cos_theta, 5);
}


// *** 誘電体フレネル ***

FresnelDielectric::FresnelDielectric(float _n_inside, float _n_outside) 
    : n_inside(_n_inside), n_outside(_n_outside) {}

Vec3 FresnelDielectric::eval(float cos_theta, const intersection& p) const {
    bool is_inside = !p.is_front; // 入射レイが媒質の内側にあるか判定
    auto n_coming = is_inside ? n_inside : n_outside;
    auto n_going  = is_inside ? n_outside : n_inside;
    cos_theta = std::abs(cos_theta);
    float sin_theta = std::sqrt(std::max(0.f, 1.0f - cos_theta * cos_theta));
    float sin_refract = n_coming / n_going * sin_theta;
    if (sin_refract >= 1.0f) { // 全反射
        return Vec3::one;
    }
    float cos_refract = std::sqrt(std::max(0.f, 1.0f - sin_refract * sin_refract));
    float Rs = fresnel_rs(n_coming, n_going, cos_theta, cos_refract);
    float Rp = fresnel_rp(n_coming, n_going, cos_theta, cos_refract);
    float R =  (Rs * Rs + Rp * Rp) / 2;
    return Vec3(R, R, R);
}


// *** 薄膜干渉フレネル ***

FresnelThinfilm::FresnelThinfilm(float _thickness, float _n_inside, float _n_film, float _n_outside)
    : thickness(_thickness), n_inside(_n_inside), n_film(_n_film), n_outside(_n_outside) {}

Vec3 FresnelThinfilm::eval(float cos_theta, const intersection& p) const {
    // 入射レイが媒質の内側にあるか判定
    bool is_inside = !p.is_front;
    auto n_coming = is_inside ? n_inside : n_outside;
    auto n_going = is_inside ? n_outside : n_inside;
    cos_theta = std::abs(cos_theta);
    return irid_reflectance(cos_theta, thickness, n_coming, n_film, n_going);
}


// *** 反射率テーブルによるフレネル ***
FresnelLUT::FresnelLUT(std::string filename) 
{
    load_LUT(filename);
}

void FresnelLUT::load_LUT(std::string filename) {
    // Read csv file
    std::ifstream ifs(filename, std::ios::in);
    if (!ifs) {
        std::cerr << "Failed to load " << filename << '\n';
        exit(1);
    }
    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> row = split_string(line, ',');
        Vec3 rgb(std::stof(row[0]), std::stof(row[1]), std::stof(row[2]));
        table.push_back(rgb);
    }
}

Vec3 FresnelLUT::eval(float cos_theta, const intersection& p) const {
    auto theta = std::acos(cos_theta);
    int index = std::clamp(int(to_degree(theta)), 0, 89);
    return table[index];
}