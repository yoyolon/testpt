#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define __STDC_LIB_EXT1__

#include "external/stb_image_write.h"
#include "external/stb_image.h"
#include "Light.h"
#include "Shape.h"
#include "Random.h"
#include "Scene.h"

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

Vec3 AreaLight::sample_light(const intersection& ref, Vec3& w, float& pdf) const {
    auto isect = shape->sample(ref);
    w = unit_vector(isect.pos - ref.pos);
    pdf = shape->eval_pdf(ref, w);
    return evel_light(w);
}

float AreaLight::eval_pdf(const intersection& ref, const Vec3& w) const {
    return shape->eval_pdf(ref, w);
}

bool AreaLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    return shape->intersect(r, t_min, t_max, p);
}


// *** 環境光源(IBL) ***
EnvironmentLight::EnvironmentLight(std::string filename) 
    : Light(LightType::IBL), nw(1), nh(1), nc(3), brightness(0.f)
{
    envmap = stbi_loadf("asset/envmap.hdr", &nw, &nh, &nc, 0);
    if (envmap != nullptr) {
        // 輝度マップを生成した
        auto brightmap = std::make_unique<float[]>(nw * nh);
        for (int h = 0; h < nh; h++) {
            float sin_theta = std::sin(pi * float(h + 0.5f) / nh);
            for (int w = 0; w < nw; w++) {
                int index = h * nw * 3 + w * 3;
                // sRGBからXYZのY成分を計算
                float R = envmap[index++];
                float G = envmap[index++];
                float B = envmap[index];
                float brightness_element = 0.2126f * R + 0.7152f * G + 0.0722f * B;
                brightness += brightness_element;
                brightmap[h * nw + w] = brightness_element * sin_theta;
            }
        }
        brightness /= (nw * nh);
        dist = std::make_unique<Piecewise2D>(brightmap.get(), nw, nh);
    }
}

Vec3 EnvironmentLight::evel_light(const Vec3& w) const {
    if (envmap == nullptr) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }
    // 方向からuv座標を計算
    Vec3 dir = unit_vector(w);
    float u = std::atan2(dir.get_z(), dir.get_x()) + pi;
    u *= invpi * 0.5;
    float v = std::acos(std::clamp(dir.get_y(), -1.0f, 1.0f)) * invpi;
    // 環境マップから放射輝度をサンプリング
    int x = std::clamp((int)(nw * u), 0, nw - 1);
    int y = std::clamp((int)(nh * v), 0, nh - 1);
    int index = y * nw * 3 + x * 3;
    float R = envmap[index++];
    float G = envmap[index++];
    float B = envmap[index];
    return Vec3(R, G, B);
}

Vec3 EnvironmentLight::power() const {
    float radius = 100;
    return pi * radius * radius * brightness;
}

Vec3 EnvironmentLight::sample_light(const intersection& ref, Vec3& wo, float& pdf) const {
    Vec2 uv = dist->sample(pdf);
    float phi = 2 * pi * uv[0];
    float theta = uv[1] * pi;
    wo = Vec3(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta));
    pdf /= 2 * pi * pi * std::sin(theta);
    return evel_light(wo);
}

float EnvironmentLight::eval_pdf(const intersection& ref, const Vec3& w) const {
    float theta = std::acos(w.get_z());
    float phi = std::atan2(w.get_y(), w.get_x());
    if (phi < 0) phi = phi + 2 * pi;
    float sin_theta = std::sin(theta);
    if (sin_theta == 0) return 0;
    return dist->eval_pdf(Vec2(phi * invpi, theta * invpi)) / (2 * pi * pi * sin_theta);
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