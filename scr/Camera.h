#pragma once

#include "Ray.h"
#include <iostream>

// *** カメラクラス ***
class Camera {
	float filmH;
	float filmW;
	float aspect;
	float focal_length; // 焦点距離
	Vec3 camPos;
	Vec3 camDir;
	Vec3 camRight;
	Vec3 camUp;
	Vec3 filmCorner;

public:
	Camera(float h=2.0, float aspe=1.0, float fd=1.0, 
		   Vec3 pos=Vec3(0.0, 0.0, 0.0), Vec3 dir=Vec3(0.0, 0.0, 1.0))
		: filmH(h), aspect(aspe), focal_length(fd), camPos(pos), camDir(dir)
	{
		filmW = filmH * aspect;
		camRight = unit_vector(cross(Vec3(0.0, 1.0, 0.0), camDir));
		camUp = unit_vector(cross(camRight, camDir));
		camRight *= filmW;
		camUp *= filmH;
		filmCorner = camPos - 0.5 * camRight - 0.5 * camUp - fd * camDir;
	}

	// レイの生成
	Ray generate_Ray(float u, float v) {
		auto Raydir = filmCorner + u * camRight + v * camUp - camPos;
		return Ray(camPos, Raydir);
	}
	
};
