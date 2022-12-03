#pragma once

#include "Ray.h"
#include <iostream>

// *** カメラクラス ***
class Camera {
public:
	Camera(float h=2.0, float _aspect=1.0, float _fd=1.0, 
		   Vec3 _pos=Vec3(0.0, 0.0, 0.0), Vec3 _forward=Vec3(0.0, 0.0, 1.0))
		: film_h(h), aspect(_aspect), fd(_fd), pos(_pos), forward_vector(_forward)
	{
		film_w = film_h * aspect;
		right_vector = unit_vector(cross(Vec3(0.0, 1.0, 0.0), forward_vector));
		up_vector = unit_vector(cross(right_vector, forward_vector));
		right_vector *= film_w;
		up_vector *= film_h;
		film_corner = pos - 0.5 * right_vector - 0.5 * up_vector - fd * forward_vector;
	}

	// レイの生成
	Ray generate_ray(float u, float v) {
		auto dir = film_corner + u * right_vector + v * up_vector - pos;
		return Ray(pos, dir);
	}

private:
	float film_h;
	float film_w;
	float aspect;
	float fd; // 焦点距離
	Vec3 pos;
	Vec3 forward_vector;
	Vec3 right_vector;
	Vec3 up_vector;
	Vec3 film_corner;
};
