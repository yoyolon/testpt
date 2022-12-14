#include "Triangle.h"

/**
* @brief ��������w�肵�������ŋ�؂�
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
* @brief obj�`���̃|���S�����f����ǂݍ���
* @param[out]  vertex   :�|���S���̒��_�z��
* @param[out]  index    :�|���S���̃C���f�b�N�X�z��
* @param[in]   filename :�t�@�C����
* @return std::vector<std::string> ������z��
* @details �f�[�^�\��: https://en.wikipedia.org/wiki/Wavefront_.obj_file
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


void Triangle::move(Vec3 pos) {
    V0 -= pos;
    V1 -= pos;
    V2 -= pos;
    N0 -= pos;
    N1 -= pos;
    N2 -= pos;
}

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
    p.pos = r.at(t);
    p.mat = mat;
    return true;
}

float Triangle::area() const {
    return 0.5f * cross(V1 - V0, V2 - V0).length();
}

float Triangle::sample_pdf(const intersection& ref, const Vec3& w) const {
    // TODO: ����
    return 0.0f;
}

intersection Triangle::sample(const intersection& ref) const {
    // TODO: ����
    intersection isect;
    return isect;
}


// *** �O�p�`���b�V���N���X ***
TriangleMesh::TriangleMesh() {}

TriangleMesh::TriangleMesh(std::vector<Vec3> Vertices, std::vector<Vec3> Indices, std::shared_ptr<Material> m, Vec3 p)
    : mat(m), pos(p) {
    // �e���_�C���f�b�N�X����O�p�|���S�����\��
    for (const auto& index : Indices) {
        int x = std::max((int)index.get_x(), 0);
        int y = std::max((int)index.get_y(), 0);
        int z = std::max((int)index.get_z(), 0);
        Vec3 V0 = Vertices[x] - pos;
        Vec3 V1 = Vertices[y] - pos;
        Vec3 V2 = Vertices[z] - pos;
        Triangles.push_back(Triangle(V0, V1, V2, m));
    }
};

TriangleMesh::TriangleMesh(std::string filename, std::shared_ptr<Material> m, Vec3 p, bool is_smooth)
    : mat(m), pos(p) {
    std::vector<Vec3> Vertices, Indices;
    load_obj(Vertices, Indices, filename);
    std::vector<Vec3> Normals(Vertices.size(), Vec3(0.0f, 0.0f, 0.0f)); // ���_�̖@��
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
        Vec3 V0 = Vertices[x] - pos;
        Vec3 V1 = Vertices[y] - pos;
        Vec3 V2 = Vertices[z] - pos;
        if (is_smooth) {
            Vec3 N0 = Normals[x];
            Vec3 N1 = Normals[y];
            Vec3 N2 = Normals[z];
            Triangles.push_back(Triangle(V0, V1, V2, N0, N1, N2, m)); // �X���[�Y�V�F�[�f�B���O����
        }
        else {
            Triangles.push_back(Triangle(V0, V1, V2, m)); // �X���[�Y�V�F�[�f�B���O�Ȃ�
        }
    }
};

void TriangleMesh::move(Vec3 pos) {
    for (auto& Triangle : Triangles) {
        Triangle.move(pos);
    }
}

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

float TriangleMesh::sample_pdf(const intersection& p, const Vec3& w) const {
    return 1.0f;
}

intersection TriangleMesh::sample(const intersection& p) const {
    intersection isect;
    return isect;
}