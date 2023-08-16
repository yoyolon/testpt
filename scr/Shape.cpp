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
* @brief .obj形式のポリゴンモデルを読み込む関数
* @param[out]  vertex   :ポリゴンの頂点配列
* @param[out]  index    :ポリゴンのインデックス配列
* @param[in]   filename :ファイル名
* @return std::vector<std::string> 文字列配列
* @details .obj形式のデータ構造: https://en.wikipedia.org/wiki/Wavefront_.obj_file
* @note .objファイルに二つ以上の空白があるとエラー
*/
void load_obj(std::vector<Vec3>& vertex, std::vector<Vec3>& index, const std::string& filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ios::in); // 読み込み専用でファイルを開く
    if (!ifs) {
        std::cerr << "Failed to load " << filename << '\n';
        exit(1);
    }
    // ファイルを行ごとに読み込む
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue; // 空行処理
        std::vector<std::string> s = split_string(line);
        // TODO: 空白スペースを除去
        // 頂点の処理
        if (s[0] == "v") {
            float x = std::stof(s[1]);
            float y = std::stof(s[2]);
            float z = std::stof(s[3]);
            vertex.push_back(Vec3(x, y, z));
        }
        // インデックスの処理
        else if (s[0] == "f") {
            int x = std::stoi(s[1].c_str());
            int y = std::stoi(s[2].c_str());
            int z = std::stoi(s[3].c_str());
            // NOTE: Vec3はfloat要素がfloat型なのでキャストする(あとで修正したい)
            index.push_back(Vec3((float)x, (float)y, (float)z));
        }
    }
}


/**
* @brief 物体表面の表裏を判定する関数
* @param[in] r :物体表面への入射レイ
* @param[in] n :物体表面の法線
* @return bool :表ならtrueを返す
*/
inline bool is_front(const Ray& r, const Vec3  n) {
    // 物体表面から離れる方向を正にするために-1を乗算
    return dot(n, -r.get_dir()) > 0;
}


// *** シェイプ抽象クラス ***

Shape::Shape(std::shared_ptr<Material> m)
    : mat(m)
{
    if (mat == nullptr) {
        mat = std::make_shared<Diffuse>(Vec3::one); // デフォルト
    }
}

float Shape::eval_pdf(const intersection& ref, const Vec3& w) const {
    auto r = Ray(ref.pos, unit_vector(w)); // refからシェイプへ向かうレイ
    intersection isect; // シェイプの交差点
    // シェイプとレイが交差しない場合はPDFはゼロ
    if (!intersect(r, eps_isect, inf, isect)) {
        return 0.f;
    }
    // シェイプの裏面をサンプルした場合はPDFはゼロ
    if (!isect.is_front) {
        return 0.f;
    }
    // ジオメトリを一様サンプリングした場合の立体角に関するPDFを返す
    auto distance = ref.pos - isect.pos;
    return distance.length2() / (std::abs(dot(isect.normal, -w)) * area());
}


// *** 球 ***

Sphere::Sphere(Vec3 c, float r, std::shared_ptr<Material> m)
    : Shape(m), center(c), radius(r) {};

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

    // 一様サンプリング(没)
    //intersection isect;
    //isect.normal = Random::uniform_sphere_sample();
    //isect.pos = radius * isect.normal + center;
    //return isect;
}


// *** 三角形クラス ***
Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<Material> m)
    : Shape(m), V0(v0), V1(v1), V2(v2)
{
    // 面法線を計算
    Vec3 E1 = V1 - V0;
    Vec3 E2 = V2 - V0;
    N0 = unit_vector(cross(E1, E2));
    N1 = N0;
    N2 = N0;
};

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 n0, Vec3 n1, Vec3 n2, std::shared_ptr<Material> m)
    : Shape(m), V0(v0), V1(v1), V2(v2), N0(n0), N1(n1), N2(n2) {};

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
    if (u < 0.f || v < 0.f || u + v > 1.0f) {
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
TriangleMesh::TriangleMesh(std::vector<Vec3> Vertices, std::vector<Vec3> Indices, std::shared_ptr<Material> m)
    : Shape(m) {
    // 頂点インデックスから三角形を構成
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
    std::vector<Vec3> Normals(Vertices.size(), Vec3::zero); // 頂点の法線配列
    // スムーズシェーディング
    if (is_smooth) {
        for (int i = 0; i < (int)Vertices.size(); i++) {
            // 頂点iの法線を隣接する三角形の法線の重み付き和で計算
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
    // 頂点インデックスから三角形を構成
    for (const auto& index : Indices) {
        int x = std::max((int)index.get_x(), 0);
        int y = std::max((int)index.get_y(), 0);
        int z = std::max((int)index.get_z(), 0);
        Vec3 V0 = Vertices[x];
        Vec3 V1 = Vertices[y];
        Vec3 V2 = Vertices[z];
        // スムーズシェーディング
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
    // 三角系の中で一番最初に交差する交差点を探索
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
    // 面積に無関係に一つの三角シェイプからサンプリング
    auto index = Random::uniform_int(0, Triangles.size() - 1);
    return Triangles[index].sample(p);
}