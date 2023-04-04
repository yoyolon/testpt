#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define __STDC_LIB_EXT1__

#include "external/stb_image_write.h"
#include "external/stb_image.h"
#include "Light.h"
#include "ONB.h"
#include "Random.h"
#include "Scene.h"
#include "Shape.h"

// *** 光源 ***
bool Light::is_visible(const intersection& p1, const intersection& p2, const Scene& world) {
    Ray r = Ray(p1.pos, unit_vector(p2.pos - p1.pos));
    intersection isect;
    return !world.intersect(r, eps_isect, inf, isect);
}


// *** 面光源 ***
AreaLight::AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape)
    : Light(LightType::Area), intensity(_intensity), shape(_shape)
{
    area = shape->area();
}

Vec3 AreaLight::evel_light(const Vec3& w) const {
    return intensity;
}

Vec3 AreaLight::power() const {
    return intensity * area * 4 * pi;
}

Vec3 AreaLight::sample_light(const intersection& ref, Vec3& wi, float& pdf) const {
    auto isect = shape->sample(ref);
    wi = unit_vector(isect.pos - ref.pos);
    pdf = shape->eval_pdf(ref, wi);
    return evel_light(wi);
}

float AreaLight::eval_pdf(const intersection& ref, const Vec3& w) const {
    return shape->eval_pdf(ref, w);
}

bool AreaLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    return shape->intersect(r, t_min, t_max, p);
}


EnvironmentLight::EnvironmentLight(const std::string& filename, float rotation) 
    : Light(LightType::IBL), nw(1), nh(1), nc(3), luminance(0.f)
{
    // 環境マップの生成
    envmap = stbi_loadf(filename.c_str(), &nw, &nh, &nc, 0);
    if (envmap != nullptr) {
        // 環境マップの回転
        rotate_envmap(rotation);
        
        // 輝度マップを生成
        auto luminance_map = std::make_unique<float[]>(nw * nh);
        for (int h = 0; h < nh; h++) {
            float sin_theta = std::sin(pi * (h + 0.5f) / nh);
            for (int w = 0; w < nw; w++) {
                int index = h * nw * 3 + w * 3;
                // RGBから輝度を計算
                float R = envmap[index++];
                float G = envmap[index++];
                float B = envmap[index];
                float luminance_element = 0.2126f * R + 0.7152f * G + 0.0722f * B;
                luminance += luminance_element;
                luminance_map [h * nw + w] = luminance_element * sin_theta;
            }
        }
        luminance /= (nw * nh);
        dist = std::make_unique<Piecewise2D>(luminance_map.get(), nw, nh);
    }
    else {
        std::cerr << "failed to load" << filename << '\n';
    }
    //// サンプリングテスト
    //std::vector<uint8_t> img(nw * nh * nc, 0);  // 画像データ
    //int nsample = 1000000;
    //for (int i = 0; i < nsample; i++) {
    //    float pdf;
    //    Vec2 uv = dist->sample(pdf);
    //    int w = std::clamp(int(uv[0] * nw), 0, nw - 1);
    //    int h = std::clamp(int(uv[1] * nh), 0, nh - 1);
    //    int index = h * nw * 3 + w * 3;
    //    img[index] = std::clamp(img[index]+10, 0, 255);
    //}
    //stbi_write_png("b.png", nw, nh, 3, img.data(), nw * nc * sizeof(uint8_t));
}

Vec3 EnvironmentLight::evel_light(const Vec3& w) const {
    if (envmap == nullptr) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }
    // 方向からuv座標を計算
    Vec3 dir = unit_vector(w);
    float u = std::atan2(dir.get_z(), dir.get_x()) + pi; // +piは右手座標を考慮
    u *= invpi * 0.5;
    float v = std::acos(std::clamp(dir.get_y(), -1.0f, 1.0f)) * invpi;
    // 環境マップから放射輝度をサンプリング
    return evel_light_uv(Vec2(u, v));
}

Vec3 EnvironmentLight::power() const {
    float radius = 100;
    return pi * radius * radius * luminance;
}

Vec3 EnvironmentLight::sample_light(const intersection& ref, Vec3& wi, float& pdf) const {
    if (envmap == nullptr) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }
    // uv座標をサンプリング
    float sample_pdf;
    Vec2 uv = dist->sample(sample_pdf);
    if (sample_pdf == 0) return Vec3::zero;

    // uv座標から方向を計算
    float phi = 2 * pi * uv[0] + pi; // +piは右手座標を考慮
    float theta = pi * uv[1];
    float sin_theta = std::sin(theta);
    wi = Vec3(sin_theta * std::cos(phi), std::cos(theta), sin_theta * std::sin(phi));
    // 方向に関するサンプリングPDFを計算し放射輝度を返す
    if (sin_theta == 0) {
        pdf = 0.0f;
        return Vec3::zero;
    }
    pdf = sample_pdf / (2 * pi * pi * sin_theta);
    return evel_light_uv(uv);
}

float EnvironmentLight::eval_pdf(const intersection& ref, const Vec3& w) const {
    if (envmap == nullptr) {
        return 0.0f;
    }
    float theta = std::acos(w.get_y());
    float phi = std::atan2f(w.get_z(), w.get_x()) + pi; // +piは右手座標を考慮
    if (phi < 0) phi += 2 * pi;
    float sin_theta = std::sin(theta);
    if (sin_theta == 0) return 0;
    return dist->eval_pdf(Vec2(0.5 * phi * invpi, theta * invpi)) / (2 * pi * pi * sin_theta);
}

bool EnvironmentLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // 他に交差したオブジェクトがあるなら交差しない
    if (std::isinf(t_max) == false) {
        return false;
    }
    // 交差点情報の更新
    p.t = t_max;
    p.pos = Vec3::zero;
    p.normal = Vec3::zero;
    p.is_front = true;
    return true;
}

Vec3 EnvironmentLight::evel_light_uv(const Vec2& uv) const {
    if (envmap == nullptr) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }
    // 環境マップから放射輝度をサンプリング
    int index_u = std::clamp((int)(nw * uv[0]), 0, nw - 1);
    int index_v = std::clamp((int)(nh * uv[1]), 0, nh - 1);
    float t_u = nw * uv[0] - (float)index_u;
    float t_v = nh * uv[1] - (float)index_v;
    // バイリニア補間
    return   (1-t_u) * (1-t_v) * evel_envmap(index_u,   index_v  )
           + (1-t_u) * (t_v)   * evel_envmap(index_u,   index_v+1)
           + (t_u)   * (1-t_v) * evel_envmap(index_u+1, index_v  )
           + (t_u)   * (t_v)   * evel_envmap(index_u+1, index_v+1);
}

Vec3 EnvironmentLight::evel_envmap(int x, int y) const {
    if (envmap == nullptr) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }
    // 環境マップから放射輝度をサンプリング
    x = std::clamp(x, 0, nw - 1);
    y = std::clamp(y, 0, nh - 1);
    int index = y * nw * 3 + x * 3;
    float R = envmap[index++];
    float G = envmap[index++];
    float B = envmap[index];
    return Vec3(R, G, B);
}

void EnvironmentLight::rotate_envmap(float deg) {
    // 環境マップをコピー
    float* envmap_copy = new float[nh * nw * 3];
    std::memcpy(envmap_copy, envmap, sizeof(float) * nh * nw * 3);
    // 環境マップを回転
    int offset = int(nw * deg / 360) % (nw); // 列ピクセルのオフセット
    for (int h = 0; h < nh; h++) {
        int index_begin = h * nw * 3 + offset * 3; // 回転後の先頭のインデックス 
        // 行に関して循環
        for (int w = 0; w < nw; w++) {
            int index = h * nw * 3 + w * 3;
            int index_offset = h * nw * 3 + (index_begin + w * 3) % (nw * 3);
            // 配列の要素をシフト
            envmap[index]     = envmap_copy[index_offset];
            envmap[index + 1] = envmap_copy[index_offset + 1];
            envmap[index + 2] = envmap_copy[index_offset + 2];
        }
    }
    delete[] envmap_copy;
}