#include "Shape.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Material.h"
#include "ONB.h"
#include "Random.h"

/**
* @brief 文字列を指定した文字で区切る関数
* @param[in]  line      :区切りたい文字列
* @param[in]  delimiter :区切り文字
* @return std::vector<std::string> 文字列配列
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
* @brief obj形式のポリゴンモデルを読み込む関数
* @param[out]  vertex   :ポリゴンの頂点配列
* @param[out]  index    :ポリゴンのインデックス配列
* @param[in]   filename :ファイル名
* @return std::vector<std::string> 文字列配列
* @details データ構造: https://en.wikipedia.org/wiki/Wavefront_.obj_file
* @note objファイルに二つ以上の空白があるとエラー
*/
void load_obj(std::vector<Vec3>& vertex, std::vector<Vec3>& index, const std::string& filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ios::in); // 読み込み専用でファイルを開く
    if (!ifs) {
        std::cerr << "Can't open " << filename << '\n';
        exit(1);
    }
    // ファイルを行ごとに読み込む
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue; // 空行処理
        std::vector<std::string> s = split_string(line);
        // 頂点の場合
        if (s[0] == "v") {
            float x = std::stof(s[1]);
            float y = std::stof(s[2]);
            float z = std::stof(s[3]);
            vertex.push_back(Vec3(x, y, z));
        }
        // インデックスの場合
        else if (s[0] == "f") {
            int x = atoi(s[1].c_str());
            int y = atoi(s[2].c_str());
            int z = atoi(s[3].c_str());
            index.push_back(Vec3((float)x, (float)y, (float)z));
        }
    }
}


// *** シェイプ抽象クラス ***
float Shape::eval_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // refからジオメトリへ向かうレイ
    intersection isect;                    // ジオメトリの交差点
    // refと交差しない場合はPDFはゼロ
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.0f;
    }
    // ジオメトリの裏面をサンプルした場合はPDFはゼロ
    if (dot(isect.normal, -w) < 0) {
        return 0.0f;
    }
    // ジオメトリを一様サンプリングした場合の立体角に関するPDFを返す
    auto d = ref.pos - isect.pos;
    return d.length2() / (std::abs(dot(isect.normal, -w)) * area()); // 測度変換
}


// *** 球 ***
Sphere::Sphere(Vec3 c, float r, std::shared_ptr<Material> m)
    : center(c), radius(r), mat(m) {};

bool Sphere::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // 二次方程式の判別式D/4 = (b/2)^2 - a*cを利用(bは偶数)
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
    // 交差点情報の更新
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
    // 球の可視領域(半球)を考慮してサンプリング
    auto z = unit_vector(ref.pos - center);
    auto sampling_coord = ONB(z);
    auto sampling_local_pos = Random::uniform_hemisphere_sample();
    intersection isect;
    isect.normal = sampling_coord.to_world(sampling_local_pos);
    isect.pos = radius * isect.normal + center;
    return isect;

    // 球から一様サンプリング(没)
    //intersection isect;
    //isect.normal = Random::uniform_sphere_sample();
    //isect.pos = radius * isect.normal + center;
    //return isect;
}

float Sphere::eval_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // refからジオメトリへのレイ
    intersection isect;                    // ジオメトリの交差点
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.0f;
    }
    // 面の裏側をサンプルした場合pdfをゼロにする
    if (dot(isect.normal, -w) < 0) {
        return 0.0f;
    }
    auto d = ref.pos - isect.pos;
    return d.length2() / (std::abs(dot(isect.normal, -w)) * area());
}


// *** 円盤 ***
Disk::Disk(Vec3 c, float r, std::shared_ptr<Material> m, bool is_flip)
    : center(c), radius(r), mat(m), is_flip_normal(is_flip) {};

bool Disk::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // レイとxz平面の交差点が円内部にあるか判定
    if (r.get_dir().get_y() == 0) {
        return false;
    }
    auto t = (center.get_y() - r.get_origin().get_y()) / r.get_dir().get_y();
    if (t < t_min || t > t_max) {
        return false;
    }
    auto pos = r.at(t); // xz平面との交差点
    // 円との交差判定
    auto dx = pos.get_x() - center.get_x();
    auto dz = pos.get_z() - center.get_z();
    auto dist2 = dx * dx + dz * dz;
    if (dist2 > radius * radius) {
        return false;
    }
    // 交差点情報の更新
    p.t = t;
    p.pos = pos;
    p.normal = normal;
    if (is_flip_normal) p.normal *= -1; // 法線の反転
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
    if (is_flip_normal) isect.normal *= -1; // 法線の反転
    isect.pos = Vec3(x, y, z);
    return isect;
}

const Vec3 Disk::normal = Vec3(0.0f, -1.0f, 0.0f);


// *** 円柱 ***
Cylinder::Cylinder(Vec3 c, float r, float h, std::shared_ptr<Material> m)
    : center(c), radius(r), height(h), mat(m) {};

bool Cylinder::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // 二次方程式の判別式D/4 = (b/2)^2 - a*cを利用(bは偶数)
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
    // 交差点のy座標が円柱の範囲内か判定
    auto y = r.at(t).get_y();
    auto y_min = center.get_y();
    auto y_max = y_min + height;
    if (y <= y_min || y >= y_max) {
        return false;
        t = (-b + d) / (2 * a); // 円柱の内側
        auto y = r.at(t).get_y();
        if (y <= y_min || y >= y_max) {
            return false;
        }
    }
    // 交差点情報の更新
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
    // NOTE: y軸正の方向が上向き
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


// *** 三角形クラス ***
Triangle::Triangle() {};

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<Material> m)
    : V0(v0), V1(v1), V2(v2), mat(m)
{
    // 面法線を計算
    Vec3 E1 = V1 - V0;
    Vec3 E2 = V2 - V0;
    N0 = unit_vector(cross(E1, E2));
    N1 = N0;
    N2 = N0;
};

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 n0, Vec3 n1, Vec3 n2, std::shared_ptr<Material> m)
    : V0(v0), V1(v1), V2(v2), N0(n0), N1(n1), N2(n2), mat(m) {};

bool Triangle::intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
    // 参考: http://www.graphics.cornell.edu/pubs/1997/MT97.html
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
    // 交差点情報の更新
    p.t = t;
    Vec3 N_lerp = (1.0f - u - v) * N0 + u * N1 + v * N2; // 法線補間
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


// *** 三角形メッシュクラス ***
TriangleMesh::TriangleMesh() {}

TriangleMesh::TriangleMesh(std::vector<Vec3> Vertices, std::vector<Vec3> Indices, std::shared_ptr<Material> m)
    : mat(m) {
    // 各頂点インデックスから三角ポリゴンを構成
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
    std::vector<Vec3> Normals(Vertices.size(), Vec3::zero); // 頂点の法線
    // スムーズシェーディング
    if (is_smooth) {
        // 各頂点を探索
        for (int i = 0; i < (int)Vertices.size(); i++) {
            // 頂点iの隣接三角ポリゴンを探索
            for (const auto& index : Indices) {
                int x = (int)index.get_x() - 1;
                int y = (int)index.get_y() - 1;
                int z = (int)index.get_z() - 1;
                if (i == x || i == y || i == z) {
                    // 頂点を取得
                    Vec3 V0 = Vertices[x];
                    Vec3 V1 = Vertices[y];
                    Vec3 V2 = Vertices[z];
                    // 法線を計算
                    Vec3 E1 = V1 - V0;
                    Vec3 E2 = V2 - V0;
                    Vec3 N = cross(E1, E2);
                    Normals[i] += N;
                }
            }
            Normals[i] = unit_vector(Normals[i]); // 正規化
        }
    }
    // 各頂点インデックスから三角ポリゴンを構成
    for (const auto& index : Indices) {
        int x = std::max((int)index.get_x() - 1, 0);
        int y = std::max((int)index.get_y() - 1, 0);
        int z = std::max((int)index.get_z() - 1, 0);
        Vec3 V0 = Vertices[x];
        Vec3 V1 = Vertices[y];
        Vec3 V2 = Vertices[z];
        // スムーズシェーディングで場合分け
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
    intersection temp;  // 交差点
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
    // 面積に無関係に一つの三角シェイプからサンプリング
    auto index = Random::uniform_int(0, Triangles.size() - 1);
    auto t = Triangles[index];
    return t.sample(p);
}