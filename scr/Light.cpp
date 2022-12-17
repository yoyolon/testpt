#include "Light.h"
#include "Shape.h"
#include "Random.h"
#include "Scene.h"

// *** ���� ***
bool Light::is_visible(const intersection& p1, const intersection& p2, const Scene& world) {
    Ray r = Ray(p1.pos, unit_vector(p2.pos - p1.pos));
    intersection isect;
    return !world.intersect(r, epsilon, inf, isect);
}


// *** �ʌ��� ***
AreaLight::AreaLight(Vec3 _intensity, std::shared_ptr<class Shape> _shape)
    : Light(LightType::Area), intensity(_intensity), shape(_shape)
{
    area = shape->area();
}

Vec3 AreaLight::emitte() const {
    return intensity;
}

Vec3 AreaLight::power() const {
    return intensity * area * 4 * pi;
}

Vec3 AreaLight::sample_light(const intersection& ref, Vec3& w, float& pdf) {
    auto isect = shape->sample(ref);
    auto d = ref.pos - isect.pos;
    pdf = shape->sample_pdf(isect, d);
    w = unit_vector(d);
    return emitte();
}

bool AreaLight::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    return shape->intersect(r, t_min, t_max, p);
}
