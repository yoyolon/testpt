#include "Camera.h"
#include "Film.h"
#include <iostream>

Camera::Camera()
    : fd(1.0f), 
      forward(Vec3::zero), 
      right(Vec3::zero),
      up(Vec3::zero),
      film_corner(Vec3::zero),
      film(nullptr)
{}

Camera::Camera(std::shared_ptr<Film> _film, float _fd, Vec3 _pos, Vec3 _forward)
    : film(_film), fd(_fd), pos(_pos), forward(_forward)
{
    forward *= -1;
    right = unit_vector(cross(Vec3(0.0, 1.0, 0.0), forward));
    up = unit_vector(cross(right, forward));
    //right = unit_vector(cross(forward, Vec3(0.0, 1.0, 0.0)));
    //up = unit_vector(cross(forward, right));
    // NOTE: フィルムの高さは2.0で固定
    auto film_h = 2.0f;
    auto film_w = film_h * film->get_aspect();
    //auto film_w = 2.0f;
    //auto film_h = film_w * film->get_aspect();
    right *= film_w;
    up *= film_h;
    film_corner = -0.5 * right -0.5 * up - fd * forward + pos;
}

int Camera::get_h() const { return film->get_h(); }

int Camera::get_w() const { return film->get_w(); }

int Camera::get_c() const { return film->get_c(); }

const char* Camera::get_filename() const { return film->get_filename(); }

Vec3 Camera::get_forward() const { return forward; }


Ray Camera::generate_ray(float u, float v) const {
    auto dir = film_corner + u * right + v * up - pos;
    return Ray(pos, dir);
}