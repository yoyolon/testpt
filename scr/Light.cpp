#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define __STDC_LIB_EXT1__

#include "external/stb_image_write.h"
#include "external/stb_image.h"
#include "Light.h"
#include "Shape.h"
#include "Random.h"
#include "Scene.h"

// *** ���� ***
bool Light::is_visible(const intersection& p1, const intersection& p2, const Scene& world) {
    Ray r = Ray(p1.pos, unit_vector(p2.pos - p1.pos));
    intersection isect;
    return !world.intersect(r, eps_isect, inf, isect);
}


// *** �ʌ��� ***
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


EnvironmentLight::EnvironmentLight(const std::string& filename, float rotation) 
    : Light(LightType::IBL), nw(1), nh(1), nc(3), brightness(0.f)
{
    // ���}�b�v�̐���
    envmap = stbi_loadf("asset/envmap.hdr", &nw, &nh, &nc, 0);
    if (envmap != nullptr) {
        // ���}�b�v�̉�]
        rotate_envmap(rotation);
        // �P�x�}�b�v�𐶐�
        auto brightmap = std::make_unique<float[]>(nw * nh);
        for (int h = 0; h < nh; h++) {
            float sin_theta = std::sin(pi * (h + 0.5f) / nh);
            for (int w = 0; w < nw; w++) {
                int index = h * nw * 3 + w * 3;
                // sRGB����XYZ��Y�������v�Z
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
    //// �T���v�����O�e�X�g
    //std::vector<uint8_t> img(nw * nh * nc, 0);  // �摜�f�[�^
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
    // ��������uv���W���v�Z
    Vec3 dir = unit_vector(w);
    float u = std::atan2(dir.get_z(), dir.get_x()) + pi;
    u *= invpi * 0.5;
    float v = std::acos(std::clamp(dir.get_y(), -1.0f, 1.0f)) * invpi;
    // ���}�b�v������ˋP�x���T���v�����O
    return evel_light_uv(Vec2(u, v));
}

Vec3 EnvironmentLight::power() const {
    float radius = 100;
    return pi * radius * radius * brightness;
}

Vec3 EnvironmentLight::sample_light(const intersection& ref, Vec3& wo, float& pdf) const {
    Vec2 uv = dist->sample(pdf);
    //Vec2 uv(Random::uniform_float(), Random::uniform_float());
    float phi = 2 * pi * uv[0];
    float theta = pi * uv[1];
    float sin_theta = std::sin(theta);
    // Note: x��y��-1����Z����Ə�肭����(���ƂŌ����ǋy)
    // Note: sin_theta������?
    wo = Vec3(-sin_theta * std::cos(phi), -sin_theta * std::sin(phi), std::cos(theta));
    //wo = Vec3(sin_theta * std::cos(phi), sin_theta * std::sin(phi), std::cos(theta));
    // ���Ε������T���v�����O�����ꍇ�̏���)
    //if (dot(ref.normal, -wo) < 0) {
    //    pdf = 0;
    //    return Vec3::zero;
    //}
    pdf /= 2 * pi * pi * sin_theta;
    //pdf = 1.0f / (2 * pi * pi * sin_theta);
    return evel_light_uv(uv);
}

float EnvironmentLight::eval_pdf(const intersection& ref, const Vec3& w) const {
    float theta = std::acos(w.get_z());
    float phi = std::atan2(w.get_y(), w.get_x());
    if (phi < 0) phi += 2 * pi;
    float sin_theta = std::sin(theta);
    if (sin_theta == 0) return 0;
    return dist->eval_pdf(Vec2(phi * invpi, theta * invpi)) / (2 * pi * pi * sin_theta);
    //return 1.0f / (2 * pi * pi * sin_theta);
}

bool EnvironmentLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // ���Ɍ��������I�u�W�F�N�g������Ȃ�������Ȃ�
    if (std::isinf(t_max) == false) {
        return false;
    }
    // �����_���̍X�V
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
    // ���}�b�v������ˋP�x���T���v�����O
    int index_u = std::clamp((int)(nw * uv[0]), 0, nw - 1);
    int index_v = std::clamp((int)(nh * uv[1]), 0, nh - 1);
    int index = index_v * nw * 3 + index_u * 3;
    float R = envmap[index++];
    float G = envmap[index++];
    float B = envmap[index];
    return Vec3(R, G, B);
}

void EnvironmentLight::rotate_envmap(float deg) {
    // TODO: for���[�v�łȂ��ꊇ�ŉ�]������
    int num_rotation = int(nw * deg / 360) % nw;
    for (int r = 0; r < num_rotation; r++) {
        // ���}�b�v����](1�s���Ƃɏz��)
        for (int h = 0; h < nh; h++) {
            int index_begin = h * nw * 3;
            int index_end   = (h+1) * nw * 3 - 3;
            float R_first = envmap[index_begin];   // �z�pR�o�b�t�@
            float G_first = envmap[index_begin+1]; // �z�pG�o�b�t�@
            float B_first = envmap[index_begin+2]; // �z�pB�o�b�t�@
            // �s�Ɋւ��ďz��
            for (int w = 0; w < nw; w++) {
                int index = index_begin + w * 3;
                if (index >= index_end) {
                    envmap[index]     = R_first;
                    envmap[index + 1] = G_first;
                    envmap[index + 2] = B_first;
                    break;
                }
                // �z��̗v�f���V�t�g
                envmap[index]     = envmap[index + 3];
                envmap[index + 1] = envmap[index + 4];
                envmap[index + 2] = envmap[index + 5];
            }
        }
    }
}