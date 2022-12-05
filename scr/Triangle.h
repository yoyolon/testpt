#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Material.h"
#include "Ray.h"
#include "Shape.h"

// *** �O�p�|���S�� ***
class Triangle : public Shape {
public:
	Triangle();
	Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<Material> m);
	Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 n0, Vec3 n1, Vec3 n2, std::shared_ptr<Material> m);

	void move(Vec3 pos);
	bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;
	float area() const override;

private:
	Vec3 V0, V1, V2; // ���_
	Vec3 N0, N1, N2; // �e���_�̖@��
	std::shared_ptr<Material> mat;

};


// *** �O�p���b�V�� ***
class TriangleMesh : public Shape {
public:
	TriangleMesh();
	TriangleMesh(std::vector<Vec3> vertices, std::vector<Vec3>indices, std::shared_ptr<Material> m, Vec3 p);
	TriangleMesh(std::string filename, std::shared_ptr<Material> m, Vec3 p, bool smooth=true);

	void move(Vec3 pos);
	bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const override;
	float area() const override;

private:
	std::vector<Triangle> Triangles; // �O�p�`�z��
	std::shared_ptr<Material> mat;
	Vec3 pos; // �ʒu
};