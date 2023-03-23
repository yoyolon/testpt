#include "Shape.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Material.h"
#include "ONB.h"
#include "Random.h"

/**
* @brief ��������w�肵�������ŋ�؂�֐�
* @param[in]  line      :��؂肽��������
* @param[in]  delimiter :��؂蕶��
* @return std::vector<std::string> ������z��
*/
std::vector<std::string> split_string(const std::string& line, char delimiter = ' ') {
    std::stringstream ss(line);
    std::string temp;
    std::vector<std::string> ret;
    while (std::getline(ss, temp, delimiter)) {
        ret.push_back(temp);
    }
    return ret;
}

/**
* @brief obj�`���̃|���S�����f����ǂݍ��ފ֐�
* @param[out]  vertex   :�|���S���̒��_�z��
* @param[out]  index    :�|���S���̃C���f�b�N�X�z��
* @param[in]   filename :�t�@�C����
* @return std::vector<std::string> ������z��
* @details �f�[�^�\��: https://en.wikipedia.org/wiki/Wavefront_.obj_file
* @note obj�t�@�C���ɓ�ȏ�̋󔒂�����ƃG���[
*/
void load_obj(std::vector<Vec3>& vertex, std::vector<Vec3>& index, const std::string& filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ios::in); // �ǂݍ��ݐ�p�Ńt�@�C�����J��
    if (!ifs) {
        std::cerr << "Can't open " << filename << '\n';
        exit(1);
    }
    // �t�@�C�����s���Ƃɓǂݍ���
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue; // ��s����
        std::vector<std::string> s = split_string(line);
        // ���_�̏ꍇ
        if (s[0] == "v") {
            float x = std::stof(s[1]);
            float y = std::stof(s[2]);
            float z = std::stof(s[3]);
            vertex.push_back(Vec3(x, y, z));
        }
        // �C���f�b�N�X�̏ꍇ
        else if (s[0] == "f") {
            int x = atoi(s[1].c_str());
            int y = atoi(s[2].c_str());
            int z = atoi(s[3].c_str());
            index.push_back(Vec3((float)x, (float)y, (float)z));
        }
    }
}


// *** �V�F�C�v���ۃN���X ***
float Shape::eval_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // ref����W�I���g���֌��������C
    intersection isect;                    // �W�I���g���̌����_
    // ref�ƌ������Ȃ��ꍇ��PDF�̓[��
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.0f;
    }
    // �W�I���g���̗��ʂ��T���v�������ꍇ��PDF�̓[��
    if (dot(isect.normal, -w) < 0) {
        return 0.0f;
    }
    // �W�I���g������l�T���v�����O�����ꍇ�̗��̊p�Ɋւ���PDF��Ԃ�
    auto d = ref.pos - isect.pos;
    return d.length2() / (std::abs(dot(isect.normal, -w)) * area()); // ���x�ϊ�
}


// *** �� ***
Sphere::Sphere(Vec3 c, float r, std::shared_ptr<Material> m)
    : center(c), radius(r), mat(m) {};

bool Sphere::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // �񎟕������̔��ʎ�D/4 = (b/2)^2 - a*c�𗘗p(b�͋���)
    auto temp = r.get_origin() - center;
    auto a = r.get_dir().length2();
    auto b_half = dot(r.get_dir(), temp);
    auto c = temp.length2() - radius * radius;
    auto D = b_half * b_half - a * c;
    if (D < 0) {
        return false;
    }
    auto b = b_half * 2;
    auto d = 2 * std::sqrt(D);
    auto t = (-b - d) / (2 * a);
    if (t < t_min || t > t_max) {
        t = (-b + d) / (2 * a);
        if (t < t_min || t > t_max) {
            return false;
        }
    }
    // �����_���̍X�V
    p.t = t;
    p.pos = r.at(t);
    p.normal = unit_vector(p.pos - center);
    p.is_front = is_front(r, p.normal);
    p.mat = mat;
    return true;
};

float Sphere::area() const {
    return 4 * pi * radius * radius;
}

intersection Sphere::sample(const intersection& ref) const {
    // ���̉��̈�(����)���l�����ăT���v�����O
    auto z = unit_vector(ref.pos - center);
    auto sampling_coord = ONB(z);
    auto sampling_local_pos = Random::uniform_hemisphere_sample();
    intersection isect;
    isect.normal = sampling_coord.to_world(sampling_local_pos);
    isect.pos = radius * isect.normal + center;
    return isect;

    // �������l�T���v�����O(�v)
    //intersection isect;
    //isect.normal = Random::uniform_sphere_sample();
    //isect.pos = radius * isect.normal + center;
    //return isect;
}

float Sphere::eval_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // ref����W�I���g���ւ̃��C
    intersection isect;                    // �W�I���g���̌����_
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.0f;
    }
    // �ʂ̗������T���v�������ꍇpdf���[���ɂ���
    if (dot(isect.normal, -w) < 0) {
        return 0.0f;
    }
    auto d = ref.pos - isect.pos;
    return d.length2() / (std::abs(dot(isect.normal, -w)) * area());
}


// *** �~�� ***
Disk::Disk(Vec3 c, float r, std::shared_ptr<Material> m, bool is_flip)
    : center(c), radius(r), mat(m), is_flip_normal(is_flip) {};

bool Disk::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // ���C��xz���ʂ̌����_���~�����ɂ��邩����
    if (r.get_dir().get_y() == 0) {
        return false;
    }
    auto t = (center.get_y() - r.get_origin().get_y()) / r.get_dir().get_y();
    if (t < t_min || t > t_max) {
        return false;
    }
    auto pos = r.at(t); // xz���ʂƂ̌����_
    // �~�Ƃ̌�������
    auto dx = pos.get_x() - center.get_x();
    auto dz = pos.get_z() - center.get_z();
    auto dist2 = dx * dx + dz * dz;
    if (dist2 > radius * radius) {
        return false;
    }
    // �����_���̍X�V
    p.t = t;
    p.pos = pos;
    p.normal = normal;
    if (is_flip_normal) p.normal *= -1; // �@���̔��]
    p.is_front = is_front(r, p.normal);
    p.mat = mat;
    return true;
};

float Disk::area() const {
    return pi * radius * radius;
}

intersection Disk::sample(const intersection& ref) const {
    auto disk_sample = Random::concentric_disk_sample();
    auto x = disk_sample.get_x() * radius + center.get_x();
    auto z = disk_sample.get_y() * radius + center.get_z();
    auto y = center.get_y();
    intersection isect;
    isect.normal = normal;
    if (is_flip_normal) isect.normal *= -1; // �@���̔��]
    isect.pos = Vec3(x, y, z);
    return isect;
}

const Vec3 Disk::normal = Vec3(0.0f, -1.0f, 0.0f);


// *** �~�� ***
Cylinder::Cylinder(Vec3 c, float r, float h, std::shared_ptr<Material> m)
    : center(c), radius(r), height(h), mat(m) {};

bool Cylinder::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // �񎟕������̔��ʎ�D/4 = (b/2)^2 - a*c�𗘗p(b�͋���)
    auto temp = r.get_origin() - center;
    auto dir_x = r.get_dir().get_x();
    auto dir_z = r.get_dir().get_z();
    auto temp_x = temp.get_x();
    auto temp_z = temp.get_z();
    auto a = dir_x * dir_x + dir_z * dir_z;
    auto b_half = dir_x * temp_x + dir_z * temp_z;
    auto c = temp_x * temp_x + temp_z * temp_z - radius * radius;
    auto D = b_half * b_half - a * c;
    if (D < 0) {
        return false;
    }
    auto b = b_half * 2;
    auto d = 2 * std::sqrt(D);
    auto t = (-b - d) / (2 * a);
    if (t < t_min || t > t_max) {
        t = (-b + d) / (2 * a);
        if (t < t_min || t > t_max) {
            return false;
        }
    }
    // �����_��y���W���~���͈͓̔�������
    auto y = r.at(t).get_y();
    auto y_min = center.get_y();
    auto y_max = y_min + height;
    if (y <= y_min || y >= y_max) {
        return false;
        t = (-b + d) / (2 * a); // �~���̓���
        auto y = r.at(t).get_y();
        if (y <= y_min || y >= y_max) {
            return false;
        }
    }
    // �����_���̍X�V
    p.t = t;
    p.pos = r.at(t);
    auto diff = p.pos - center;
    p.normal = unit_vector(Vec3(diff.get_x(), 0.0f, diff.get_z()));
    p.is_front = is_front(r, p.normal);
    p.mat = mat;
    return true;
}

float Cylinder::area() const {
    return 2 * pi * radius * height;
}

intersection Cylinder::sample(const intersection& p) const {
    // NOTE: y�����̕����������
    float u = Random::uniform_float();
    float v = Random::uniform_float();
    float y = center.get_y() + u * height;
    float phi = 2 * pi * v;
    float x = center.get_x() + std::sin(phi);
    float z = center.get_y() + std::cos(phi);
    intersection isect;
    isect.pos = Vec3(x, y, z);
    isect.normal = unit_vector(Vec3(x, 0, z));
    return isect;
}


// *** �O�p�`�N���X ***
Triangle::Triangle() {};

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<Material> m)
    : V0(v0), V1(v1), V2(v2), mat(m)
{
    // �ʖ@�����v�Z
    Vec3 E1 = V1 - V0;
    Vec3 E2 = V2 - V0;
    N0 = unit_vector(cross(E1, E2));
    N1 = N0;
    N2 = N0;
};

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 n0, Vec3 n1, Vec3 n2, std::shared_ptr<Material> m)
    : V0(v0), V1(v1), V2(v2), N0(n0), N1(n1), N2(n2), mat(m) {};

bool Triangle::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // �Q�l: http://www.graphics.cornell.edu/pubs/1997/MT97.html
    Vec3 T = r.get_origin() - V0;
    Vec3 E1 = V1 - V0;
    Vec3 E2 = V2 - V0;
    Vec3 D = r.get_dir();
    Vec3 P = cross(D, E2);
    Vec3 Q = cross(T, E1);
    float c = 1.0f / dot(P, E1);
    float t = c * dot(Q, E2);
    float u = c * dot(P, T);
    float v = c * dot(Q, D);
    if (t < t_min || t > t_max) {
        return false;
    }
    if (u < 0.0f || v < 0.0f || u + v > 1.0f) {
        return false;
    }
    // �����_���̍X�V
    p.t = t;
    Vec3 N_lerp = (1.0f - u - v) * N0 + u * N1 + v * N2; // �@�����
    p.normal = N_lerp;
    p.is_front = is_front(r, p.normal);
    p.pos = r.at(t);
    p.mat = mat;
    return true;
}

float Triangle::area() const {
    return 0.5f * cross(V1 - V0, V2 - V0).length();
}

intersection Triangle::sample(const intersection& ref) const {
    auto barycenter = Random::uniform_triangle_sample();
    auto s = barycenter.get_x();
    auto t = barycenter.get_y();
    auto u = 1.0f - s - t;
    intersection isect;
    isect.pos = s * V0 + t * V1 + u * V2;
    isect.normal = s * N0 + t * N1 + u * N2;
    return isect;
}


// *** �O�p�`���b�V���N���X ***
TriangleMesh::TriangleMesh() {}

TriangleMesh::TriangleMesh(std::vector<Vec3> Vertices, std::vector<Vec3> Indices, std::shared_ptr<Material> m)
    : mat(m) {
    // �e���_�C���f�b�N�X����O�p�|���S�����\��
    for (const auto& index : Indices) {
        int x = std::max((int)index.get_x(), 0);
        int y = std::max((int)index.get_y(), 0);
        int z = std::max((int)index.get_z(), 0);
        Vec3 V0 = Vertices[x];
        Vec3 V1 = Vertices[y];
        Vec3 V2 = Vertices[z];
        Triangles.push_back(Triangle(V0, V1, V2, m));
    }
};

TriangleMesh::TriangleMesh(std::string filename, std::shared_ptr<Material> m, bool is_smooth)
    : mat(m) {
    std::vector<Vec3> Vertices, Indices;
    load_obj(Vertices, Indices, filename);
    std::vector<Vec3> Normals(Vertices.size(), Vec3::zero); // ���_�̖@��
    // �X���[�Y�V�F�[�f�B���O
    if (is_smooth) {
        // �e���_��T��
        for (int i = 0; i < (int)Vertices.size(); i++) {
            // ���_i�̗אڎO�p�|���S����T��
            for (const auto& index : Indices) {
                int x = (int)index.get_x() - 1;
                int y = (int)index.get_y() - 1;
                int z = (int)index.get_z() - 1;
                if (i == x || i == y || i == z) {
                    // ���_���擾
                    Vec3 V0 = Vertices[x];
                    Vec3 V1 = Vertices[y];
                    Vec3 V2 = Vertices[z];
                    // �@�����v�Z
                    Vec3 E1 = V1 - V0;
                    Vec3 E2 = V2 - V0;
                    Vec3 N = cross(E1, E2);
                    Normals[i] += N;
                }
            }
            Normals[i] = unit_vector(Normals[i]); // ���K��
        }
    }
    // �e���_�C���f�b�N�X����O�p�|���S�����\��
    for (const auto& index : Indices) {
        int x = std::max((int)index.get_x() - 1, 0);
        int y = std::max((int)index.get_y() - 1, 0);
        int z = std::max((int)index.get_z() - 1, 0);
        Vec3 V0 = Vertices[x];
        Vec3 V1 = Vertices[y];
        Vec3 V2 = Vertices[z];
        // �X���[�Y�V�F�[�f�B���O�ŏꍇ����
        if (is_smooth) {
            Vec3 N0 = Normals[x];
            Vec3 N1 = Normals[y];
            Vec3 N2 = Normals[z];
            Triangles.push_back(Triangle(V0, V1, V2, N0, N1, N2, m));
        }
        else {
            Triangles.push_back(Triangle(V0, V1, V2, m));
        }
    }
};

bool TriangleMesh::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    p.t = t_max;
    bool first_isect = false;
    intersection temp;  // �����_
    for (const auto& tri : Triangles) {
        if (tri.intersect(r, t_min, p.t, temp)) {
            p = temp;
            first_isect = true;
        }
    }
    return first_isect;
}

float TriangleMesh::area() const {
    float a = 0.0f;
    for (const auto& tri : Triangles) {
        a += tri.area();
    }
    return a;
}

intersection TriangleMesh::sample(const intersection& p) const {
    // �ʐςɖ��֌W�Ɉ�̎O�p�V�F�C�v����T���v�����O
    auto index = Random::uniform_int(0, Triangles.size() - 1);
    auto t = Triangles[index];
    return t.sample(p);
}