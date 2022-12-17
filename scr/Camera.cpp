#include "Camera.h"
#include <iostream>

Camera::Camera(float h, float _aspect, float _fd, Vec3 _pos, Vec3 _forward)
    : film_h(h), aspect(_aspect), fd(_fd), pos(_pos), forward_vector(_forward)
{
    forward_vector *= -1;
    right_vector = unit_vector(cross(Vec3(0.0, 1.0, 0.0), forward_vector));
    up_vector = unit_vector(cross(right_vector, forward_vector));
    film_w = film_h * aspect;
    right_vector *= film_w;
    up_vector *= film_h;
    film_corner = -0.5 * right_vector -0.5 * up_vector - fd * forward_vector + pos;
}

Ray Camera::generate_ray(float u, float v) {
    auto dir = film_corner + u * right_vector + v * up_vector - pos;
    return Ray(pos, dir);
}