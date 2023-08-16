#include "Shape.h"
#include <string>
#include <iostream>
#include <fstream>
#include "Material.h"
#include "ONB.h"
#include "Random.h"
#include "Ray.h"
#include "utility.h"

/**
* @brief .obj�`���̃|���S�����f����ǂݍ��ފ֐�
* @param[out]  vertex   :�|���S���̒��_�z��
* @param[out]  index    :�|���S���̃C���f�b�N�X�z��
* @param[in]   filename :�t�@�C����
* @return std::vector<std::string> ������z��
* @details .obj�`���̃f�[�^�\��: https://en.wikipedia.org/wiki/Wavefront_.obj_file
* @note .obj�t�@�C���ɓ�ȏ�̋󔒂�����ƃG���[
*/
void load_obj(std::vector<Vec3>& vertex, std::vector<Vec3>& index, const std::string& filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ios::in); // �ǂݍ��ݐ�p�Ńt�@�C�����J��
    if (!ifs) {
        std::cerr << "Failed to load " << filename << '\n';
        exit(1);
    }
    // �t�@�C�����s���Ƃɓǂݍ���
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue; // ��s����
        std::vector<std::string> s = split_string(line);
        // TODO: �󔒃X�y�[�X������
        // ���_�̏���
        if (s[0] == "v") {
            float x = std::stof(s[1]);
            float y = std::stof(s[2]);
            float z = std::stof(s[3]);
            vertex.push_back(Vec3(x, y, z));
        }
        // �C���f�b�N�X�̏���
        else if (s[0] == "f") {
            int x = std::stoi(s[1].c_str());
            int y = std::stoi(s[2].c_str());
            int z = std::stoi(s[3].c_str());
            // NOTE: Vec3��float�v�f��float�^�Ȃ̂ŃL���X�g����(���ƂŏC��������)
            index.push_back(Vec3((float)x, (float)y, (float)z));
        }
    }
}


/**
* @brief ���̕\�ʂ̕\���𔻒肷��֐�
* @param[in] r :���̕\�ʂւ̓��˃��C
* @param[in] n :���̕\�ʂ̖@��
* @return bool :�\�Ȃ�true��Ԃ�
*/
inline bool is_front(const Ray& r, const Vec3  n) {
    // ���̕\�ʂ��痣�������𐳂ɂ��邽�߂�-1����Z
    return dot(n, -r.get_dir()) > 0;
}


// *** �V�F�C�v���ۃN���X ***

Shape::Shape(std::shared_ptr<Material> m)
    : mat(m)
{
    if (mat == nullptr) {
        mat = std::make_shared<Diffuse>(Vec3::one); // �f�t�H���g
    }
}

float Shape::eval_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // ref����V�F�C�v�֌��������C
    intersection isect; // �V�F�C�v�̌����_
    // �V�F�C�v�ƃ��C���������Ȃ��ꍇ��PDF�̓[��
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.f;
    }
    // �V�F�C�v�̗��ʂ��T���v�������ꍇ��PDF�̓[��
    if (!isect.is_front) {
        return 0.f;
    }
    // �W�I���g������l�T���v�����O�����ꍇ�̗��̊p�Ɋւ���PDF��Ԃ�
    auto distance = ref.pos - isect.pos;
    return distance.length2() / (std::abs(dot(isect.normal, -w)) * area());
}


// *** �� ***

Sphere::Sphere(Vec3 c, float r, std::shared_ptr<Material> m)
    : Shape(m), center(c), radius(r) {};

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

    // ��l�T���v�����O(�v)
    //intersection isect;
    //isect.normal = Random::uniform_sphere_sample();
    //isect.pos = radius * isect.normal + center;
    //return isect;
}


// *** �O�p�`�N���X ***
Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<Material> m)
    : Shape(m), V0(v0), V1(v1), V2(v2)
{
    // �ʖ@�����v�Z
    Vec3 E1 = V1 - V0;
    Vec3 E2 = V2 - V0;
    N0 = unit_vector(cross(E1, E2));
    N1 = N0;
    N2 = N0;
};

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 n0, Vec3 n1, Vec3 n2, std::shared_ptr<Material> m)
    : Shape(m), V0(v0), V1(v1), V2(v2), N0(n0), N1(n1), N2(n2) {};

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
    if (u < 0.f || v < 0.f || u + v > 1.0f) {
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
TriangleMesh::TriangleMesh(std::vector<Vec3> Vertices, std::vector<Vec3> Indices, std::shared_ptr<Material> m)
    : Shape(m) {
    // ���_�C���f�b�N�X����O�p�`���\��
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
    : Shape(m) {
    std::vector<Vec3> Vertices, Indices;
    load_obj(Vertices, Indices, filename);
    std::vector<Vec3> Normals(Vertices.size(), Vec3::zero); // ���_�̖@���z��
    // �X���[�Y�V�F�[�f�B���O
    if (is_smooth) {
        for (int i = 0; i < (int)Vertices.size(); i++) {
            // ���_i�̖@����אڂ���O�p�`�̖@���̏d�ݕt���a�Ōv�Z
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
    // ���_�C���f�b�N�X����O�p�`���\��
    for (const auto& index : Indices) {
        int x = std::max((int)index.get_x(), 0);
        int y = std::max((int)index.get_y(), 0);
        int z = std::max((int)index.get_z(), 0);
        Vec3 V0 = Vertices[x];
        Vec3 V1 = Vertices[y];
        Vec3 V2 = Vertices[z];
        // �X���[�Y�V�F�[�f�B���O
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
    bool is_isect = false;
    intersection first_isect_temp;
    // �O�p�n�̒��ň�ԍŏ��Ɍ�����������_��T��
    for (const auto& tri : Triangles) {
        if (tri.intersect(r, t_min, p.t, first_isect_temp)) {
            p = first_isect_temp;
            is_isect = true;
        }
    }
    return is_isect;
}

float TriangleMesh::area() const {
    float a = 0.f;
    for (const auto& tri : Triangles) {
        a += tri.area();
    }
    return a;
}

intersection TriangleMesh::sample(const intersection& p) const {
    // �ʐςɖ��֌W�Ɉ�̎O�p�V�F�C�v����T���v�����O
    auto index = Random::uniform_int(0, Triangles.size() - 1);
    return Triangles[index].sample(p);
}