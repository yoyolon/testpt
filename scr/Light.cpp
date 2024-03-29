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


// *** 平行光線 ***

ParallelLight::ParallelLight(const Vec3& _intensity, const Vec3& _wi_light)
    : Light(LightType::Parallel), intensity(_intensity), wi_light(_wi_light)
{
    wi_light = unit_vector(wi_light); // 方向を正規化する
}

Vec3 ParallelLight::evel_light(const Vec3& w) const {
    return Vec3::zero; // w=wi_light以外では放射輝度がゼロ(デルタ分布)のため
}

Vec3 ParallelLight::power() const {
    float radius = 100; // シーン半径(仮)
    return pi * radius * radius * intensity;
}

Vec3 ParallelLight::sample_light(const intersection& ref, Vec3& wi, float& pdf) const {
    wi = wi_light; // 物体表面から離れる(光源に向かう)方向が正
    pdf = 1.0f; // デルタ関数のサンプリングなのでPDFは1
    return intensity;
}

float ParallelLight::eval_pdf(const intersection& ref, const Vec3& wi) const {
    return 0.f; // w=wi_light以外では放射輝度がゼロ(デルタ分布)のため
}

bool ParallelLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // 平行光線とレイの方向が一致する場合
    Vec3 diff = unit_vector(r.get_dir()) - wi_light;
    if (diff.length() > 0.001) {
        return false;
    }
    // 他に交差したオブジェクトがあるなら交差しない(無限遠光源のため)
    if (std::isinf(t_max) == false) {
        return false;
    }
    // 光源の交差点情報の更新
    p.t = t_max;
    p.pos = Vec3::zero;
    p.normal = Vec3::zero;
    p.is_front = true;
    return true;
}


// *** 面光源 ***

AreaLight::AreaLight(const Vec3& _intensity, std::shared_ptr<class Shape> _shape)
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
    auto isect = shape->sample(ref); // 光源のサンプリング交差点
    wi = unit_vector(isect.pos - ref.pos); // 光源に向かう方向が正
    pdf = shape->eval_pdf(ref, wi);
    return evel_light(wi);
}

float AreaLight::eval_pdf(const intersection& ref, const Vec3& wi) const {
    return shape->eval_pdf(ref, wi);
}

bool AreaLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    return shape->intersect(r, t_min, t_max, p);
}


// *** 環境光源(IBL) ***

EnvironmentLight::EnvironmentLight(const std::string& filename, float rotation) 
    : Light(LightType::IBL), nw(1), nh(1), nc(3), luminance(0.f), envmap(nullptr)
{
    // 環境マップの読み込み
    float* map = stbi_loadf(filename.c_str(), &nw, &nh, &nc, 0);
    if (map != nullptr) {
        // 環境マップの生成
        envmap = std::make_unique<float[]>(nw * nh * nc);
        for (int index = 0; index < nw * nh * nc; index++) {
            envmap[index] = map[index]; // コピー
        }
        stbi_image_free(map); // メモリ開放

        // 環境マップの回転
        rotate_envmap(rotation);
        
        // サンプリング分布の生成
        auto luminance_map = std::make_unique<float[]>(nw * nh); // 輝度分布
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
        std::cerr << "Failed to load" << filename << '\n';
        exit(1);
    }
    // サンプリングテスト
    bool is_sampling_visualization = false;
    if (is_sampling_visualization) {
        std::vector<uint8_t> img(nw * nh * nc, 0);  // 画像データ
        int nsample = 1e6;
        for (int i = 0; i < nsample; i++) {
            float pdf;
            Vec2 uv = dist->sample(pdf);
            int w = std::clamp(int(uv[0] * nw), 0, nw - 1);
            int h = std::clamp(int(uv[1] * nh), 0, nh - 1);
            int index = h * nw * 3 + w * 3;
            img[index] = std::clamp(img[index]+10, 0, 255);
        }
        stbi_write_png("envmap_sampling.png", nw, nh, 3, img.data(), nw * nc * sizeof(uint8_t));
    }
}

EnvironmentLight::EnvironmentLight(const Vec3& intensity) 
    : Light(LightType::IBL), nw(360), nh(360), nc(3), luminance(0.f), envmap(nullptr)
{
    // 環境マップの初期化
    envmap = std::make_unique<float[]>(nw * nh * nc); // メモリの確保
    for (int h = 0; h < nh; h++) {
        for (int w = 0; w < nw; w++) {
            int index = h * nw * 3 + w * 3;
            // 放射輝度の設定
            envmap[index]   = intensity.get_x();
            envmap[index+1] = intensity.get_y();
            envmap[index+2] = intensity.get_z();
        }
    }
    // 輝度とサンプリング分布の生成
    if (envmap != nullptr) {
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
                luminance_map[h * nw + w] = luminance_element * sin_theta;
            }
        }
        luminance /= (nw * nh);
        dist = std::make_unique<Piecewise2D>(luminance_map.get(), nw, nh);
    }
}

Vec3 EnvironmentLight::evel_light(const Vec3& w) const {
    if (envmap == nullptr) {
        return Vec3(0.f, 0.f, 0.f);
    }
    // 方向からuv座標を計算
    Vec3 dir = unit_vector(w);
    float u = std::atan2(dir.get_z(), dir.get_x()) + pi; // piの加算は右手座標系を考慮
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
        return Vec3(0.f, 0.f, 0.f);
    }
    // uv座標をサンプリング
    float sample_pdf;
    Vec2 uv = dist->sample(sample_pdf);
    if (sample_pdf == 0) return Vec3::zero;
    // uv座標から方向を計算
    float phi = 2 * pi * uv[0] + pi; // piの加算は右手座標系を考慮
    float theta = pi * uv[1];
    float sin_theta = std::sin(theta);
    if (sin_theta == 0) {
        pdf = 0.f;
        return Vec3::zero;
    }
    wi = Vec3(sin_theta * std::cos(phi), std::cos(theta), sin_theta * std::sin(phi));
    // サンプリング確率密度と放射輝度を評価
    pdf = sample_pdf / (2 * pi * pi * sin_theta);
    return evel_light_uv(uv);
}

float EnvironmentLight::eval_pdf(const intersection& ref, const Vec3& w) const {
    if (envmap == nullptr) {
        return 0.f;
    }
    float theta = std::acos(w.get_y());
    float phi = std::atan2f(w.get_z(), w.get_x()) + pi; // piの加算は右手座標系を考慮
    if (phi < 0) phi += 2 * pi;
    float sin_theta = std::sin(theta);
    if (sin_theta == 0) return 0;
    return dist->eval_pdf(Vec2(0.5f * phi * invpi, theta * invpi)) / (2 * pi * pi * sin_theta);
}

bool EnvironmentLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // 他に交差したオブジェクトがあるなら交差しない(無限遠光源のため)
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
        return Vec3(0.f, 0.f, 0.f);
    }
    // 環境マップから放射輝度をサンプリング
    int index_u = std::clamp((int)(nw * uv[0]), 0, nw - 1);
    int index_v = std::clamp((int)(nh * uv[1]), 0, nh - 1);
    float t_u = nw * uv[0] - (float)index_u;
    float t_v = nh * uv[1] - (float)index_v;
    // 放射輝度をバイリニア補間(NOTE: この手法は正しくないかも)
    return   (1-t_u) * (1-t_v) * evel_envmap(index_u,   index_v  )
           + (1-t_u) * (t_v)   * evel_envmap(index_u,   index_v+1)
           + (t_u)   * (1-t_v) * evel_envmap(index_u+1, index_v  )
           + (t_u)   * (t_v)   * evel_envmap(index_u+1, index_v+1);
}

Vec3 EnvironmentLight::evel_envmap(int x, int y) const {
    if (envmap == nullptr) {
        return Vec3(0.f, 0.f, 0.f);
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
    std::memcpy(envmap_copy, envmap.get(), sizeof(float) * nh * nw * 3);
    // 環境マップを回転
    int offset = int(nw * deg / 360) % (nw); // 列ピクセルのオフセット
    for (int h = 0; h < nh; h++) {
        int index_begin = h * nw * 3 + offset * 3; // 回転後の先頭のインデックス 
        // 行に関して循環
        for (int w = 0; w < nw; w++) {
            int index = h * nw * 3 + w * 3;
            int index_offset = h * nw * 3 + (index_begin + w * 3) % (nw * 3);
            // 配列の要素をシフト
            envmap[index++] = envmap_copy[index_offset++];
            envmap[index++] = envmap_copy[index_offset++];
            envmap[index]   = envmap_copy[index_offset];
        }
    }
    delete[] envmap_copy;
}