#include "Scene.h"
#include "Light.h"
#include "Material.h"
#include "Math.h"
#include "Ray.h"
#include "Shape.h"


bool Scene::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    intersection isect;
    bool is_isect = false;
    auto t_first = t_max;
    isect.type = IsectType::None;
    // シェイプとの交差判定
    for (const auto& object : shape_list) {
        if (object->intersect(r, t_min, t_first, isect)) {
            is_isect = true;
            t_first = isect.t;
            isect.type = IsectType::Material;
        }
    }
    // 光源との交差判定
    for (const auto& light : light_list) {
        if (light->intersect(r, t_min, t_first, isect)) {
            is_isect = true;
            t_first = isect.t;
            isect.light = light;
            isect.type = IsectType::Light;
        }
    }
    p = isect;
    return is_isect;
}


bool Scene::intersect_object(const Ray& r, float t_min, float t_max) const {
    intersection isect;
    bool is_isect = false;
    auto t_first = t_max;
    isect.type = IsectType::None;
    // シェイプとの交差判定
    for (const auto& object : shape_list) {
        if (object->intersect(r, t_min, t_first, isect)) {
            is_isect = true;
            t_first = isect.t;
            isect.type = IsectType::Material;
        }
    }
    return is_isect;
}


bool Scene::intersect_light(const Ray& r, float t_min, float t_max, intersection& p) const {
    intersection isect;
    bool is_isect = false;
    auto t_first = t_max;
    isect.type = IsectType::None;
    // 光源との交差判定
    for (const auto& light : light_list) {
        if (light->intersect(r, t_min, t_first, isect)) {
            is_isect = true;
            t_first = isect.t;
            isect.light = light;
            isect.type = IsectType::Light;
            p = isect;
        }
    }
    return is_isect;
}