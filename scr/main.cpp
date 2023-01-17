//---------------------------------------------------------------------------------------
// testpt
// yoyolon - December 2022
// 
// Development
// Microsoft Visual Studio 2019
// C++17
// 
// External Library
// stb: https://github.com/nothings/stb
// 
// Reference(this software is based on listed below.)
// Raytracing in one weekend: https://raytracing.github.io/
// pbrt-v3: https://pbrt.org/
// 
// More information on these licenses can be found in NOTICE.txt
//---------------------------------------------------------------------------------------

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define __STDC_LIB_EXT1__

#include "external/stb_image_write.h"
#include "external/stb_image.h"
#include "Camera.h"
#include "Film.h"
#include "Fresnel.h"
#include "Light.h"
#include "MakeScene.h"
#include "Material.h"
#include "Microfacet.h"
#include "ONB.h"
#include "Random.h"
#include "Ray.h"
#include "Scene.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Vec3.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>


enum class Sampling {
    BSDF  = 1, // BSDFによる重点的サンプリング
    LIGHT = 2, // 光源による重点的サンプリング
    MIS   = 3  // 多重重点的サンプリング
};
Sampling sampling_strategy = Sampling::MIS;

// デバッグ用
constexpr bool DEBUG_MODE           = false; // (デバッグモード)法線可視化を有効にする
constexpr bool GLOBAL_ILLUMINATION  = true;  // 大域照明効果(GI)を有効にする
constexpr bool IMAGE_BASED_LIGHTING = false; // IBLを有効にする
constexpr bool IS_GAMMA_CORRECTION  = true;  // ガンマ補正を有効にする
constexpr bool BIASED_DENOISING     = false; // 寄与に上限値を設定することで
constexpr int  RUSSIAN_ROULETTE     = 3;     // ロシアンルーレット適用までのレイのバウンス数
constexpr int  SAMPLES              = 128;   // 1ピクセル当たりのサンプル数


/**
* @brief 無効な値(NaNやinf)を除外
* @param[in]  color :輝度
* @return Vec3      :有効値に変換後の値
*/
Vec3 exclude_invalid(const Vec3& color) {
    float r = color.get_x();
    float g = color.get_y();
    float b = color.get_z();
    // 無効な値をゼロにする
    if (!isfinite(r)) r = 0.0f;
    if (!isfinite(g)) g = 0.0f;
    if (!isfinite(b)) b = 0.0f;
    return Vec3(r, g, b);
}


/**
* @brief BSDFに沿った直接光の入射方向をサンプリング
* @pram[in] r             :追跡レイ
* @pram[in] isect         :交差点情報
* @pram[in] world         :シーン
* @pram[in] shading_coord :シェーディング座標系
* @return Vec3 :直接光の重み付き入射放射輝度
* @note スペキュラレイでは実行されない
*/
Vec3 explict_bsdf(const Ray& r, const intersection& isect, const Scene& world,
                  const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    Vec3 wi_local; // 直接光の入射方向
    float pdf_scattering, pdf_light, weight = 1.0f;

    // BSDFに基づき直接光の入射方向をサンプリング
    auto wo = unit_vector(r.get_dir());          // 出射方向は必ず正規化する
    auto wo_local = -shading_coord.to_local(wo); // ローカルな出射方向
    BxDFType sampled_type;
    auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf_scattering, 
                                    sampled_type);
    if (pdf_scattering == 0 || is_zero(bsdf)) {
        return Ld;
    }

    // 光源と交差しなければ寄与はゼロ
    auto wi = shading_coord.to_world(wi_local); // 直接光の入射方向
    auto r_new = Ray(isect.pos, wi); // 光源へ向かうのレイ
    intersection isect_light;
    if (!world.intersect_light(r_new, eps_isect, inf, isect_light)) {
        return Ld;
    }

    // 光源の放射輝度を計算
    auto L = isect_light.light->emitte();
    pdf_light = isect_light.light->eval_pdf(isect, wi);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // 光源へのレイが遮蔽されると寄与はゼロ
    if (world.intersect_object(r_new, eps_isect, isect_light.t)) {
        return Ld;
    }

    // 寄与の計算
    if (sampling_strategy == Sampling::MIS) {
        weight = Random::power_heuristic(1, pdf_scattering, 1, pdf_light);
    }
    auto cos_term = dot(isect.normal, wi);
    Ld += bsdf * cos_term * weight * L / pdf_scattering;
    return Ld;
}

/**
* @brief 直接光を一つの光源からサンプリング
* @pram[in] r             :追跡レイ
* @pram[in] isect         :交差点情報
* @pram[in] world         :シーン
* @pram[in] shading_coord :シェーディング座標系
* @return Vec3 :直接光の重み付き入射放射輝度
* @note スペキュラレイでは実行されない
*/
Vec3 explict_one_light(const Ray& r, const intersection& isect, const Scene& world,
                       const ONB& shading_coord) {
    auto Ld = Vec3::zero;
    Vec3 wi;
    float pdf_scattering, pdf_light, weight = 1.0f;

    // 光源をランダムに一つ選択
    const auto& lights = world.get_light();
    auto num_lights = lights.size();
    if (num_lights == 0) {
        return Ld;
    }
    auto light_index = Random::uniform_int(0, num_lights - 1);
    const auto& light = lights[light_index];

    // 光源のジオメトリから入射方向をサンプリング
    auto L = light->sample_light(isect, wi, pdf_light);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // 光源が遮蔽されると寄与はゼロ
    auto r_light = Ray(isect.pos, wi); // 光源への入射方向
    intersection isect_light;
    light->intersect(r_light, eps_isect, inf, isect_light); // 光源の交差点を取得
    if (world.intersect_object(r_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // BSDFを評価
    auto wo       = unit_vector(r.get_dir());
    auto wo_local = -shading_coord.to_local(wo);
    auto wi_local =  shading_coord.to_local(wi);
    auto bsdf = isect.mat->eval_f(wo_local, wi_local, isect);
    pdf_scattering = isect.mat->eval_pdf(wo_local, wi_local, isect);
    if (pdf_scattering == 0 || is_zero(bsdf)) {
        return Ld;
    }

    // 寄与の計算
    if (sampling_strategy == Sampling::MIS) {
        weight = Random::power_heuristic(1, pdf_light, 1, pdf_scattering);
    }
    auto cos_term = std::abs(dot(isect.normal, wi));
    Ld += bsdf * L * cos_term * weight / pdf_light;
    return Ld;
}

/**
* @brief 明示的に直接光をサンプリング
* @pram[in] r             :追跡レイ
* @pram[in] isect         :交差点情報
* @pram[in] world         :シーン
* @pram[in] shading_coord :シェーディング座標系
* @return Vec3 :光源の重み付き放射輝度
*/
Vec3 explicit_direct_light(const Ray& r, const intersection& isect, const Scene& world,
                           const ONB& shading_coord) {
    auto Ld = Vec3::zero;

    // BSDFに基づきサンプリング
    if ((sampling_strategy == Sampling::BSDF) || (sampling_strategy == Sampling::MIS)) {
        auto L = explict_bsdf(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
    }

    // 光源のジオメトリに基づきサンプリング
    if ((sampling_strategy == Sampling::LIGHT) || (sampling_strategy == Sampling::MIS)) {
        auto L = explict_one_light(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
    }
    return Ld;
}

/**
* @brief 光源サンプリングを行うパストレーシング
* @param[in]  r_in      :カメラ方向からのレイ
* @param[in]  max_depth :レイの最大バウンス回数
* @param[in]  world     :レンダリングするシーンのデータ
* @return Vec3          :レイに沿った放射輝度
* @note 参考: pbrt-v3
*/
Vec3 L_pathtracing(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    bool is_specular_ray = false;
    // パストレーシング
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // 交差点情報
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        // カメラレイとスペキュラレイは光源の寄与を加算
        if (bounces == 0 || is_specular_ray) {
            if (is_intersect) {
                if (isect.type == IsectType::Light) {
                    L += contrib * isect.light->emitte();
                    break;
                }
            }
            else {
                L += contrib * world.sample_envmap(r);
            }
        }
        // 光源ジオメトリと交差時には寄与を加算しない(明示的に光源サンプリングを行うため)
        if (isect.type == IsectType::Light) {
            break;
        }
        // 環境マップのサンプリング
        // NOTE: 環境マップを光源として実装したら寄与は計算しない
        if (!is_intersect) {
            L += contrib * world.sample_envmap(r);
            break;
        }

        // 直接光のサンプリング
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal);// 法線を反転
        L += contrib * explicit_direct_light(r, isect, world, shading_coord);
        // BSDFに基づく出射方向のサンプリング
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        if (pdf == 0.0f || is_zero(bsdf)) break;
        auto wi = shading_coord.to_world(wi_local); // サンプリングした入射方向
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;
        is_specular_ray = is_spacular_type(sampled_type);
        r = Ray(isect.pos, wi); // 次のレイを生成
        //ロシアンルーレット
        if (bounces >= RUSSIAN_ROULETTE) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // 打ち切り確率
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }
    }
    return L;
}


/**
* @brief ナイーブなパストレーシングを実行する関数
* @param[in]  r_in      :カメラ方向からのレイ
* @param[in]  max_depth :レイの最大バウンス回数
* @param[in]  world     :レンダリングするシーンのデータ
* @return Vec3          :レイに沿った放射輝度
*/
Vec3 L_naive_pathtracing(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    bool is_specular_ray = false;
    // パストレーシング
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // 交差点情報
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        // カメラレイとスペキュラレイは光源をの寄与を加算
        if (!is_intersect) {
            L += contrib * world.sample_envmap(r);
            break;
        }
        if (isect.type == IsectType::Light) {
            // 面の裏側をサンプルした場合は寄与をゼロにする
            if (dot(isect.normal, -r.get_dir()) < 0) {
                L = 0.0f;
            }
            else {
                L += contrib * isect.light->emitte();
            }
            break;
        }

        // BSDFに基づく出射方向のサンプリング
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal);
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        if (pdf == 0.0f || is_zero(bsdf)) break;
        auto wi = shading_coord.to_world(wi_local); // サンプリングした入射方向
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;
        is_specular_ray = is_spacular_type(sampled_type);
        r = Ray(isect.pos, wi); // 次のレイを生成
        //ロシアンルーレット
        if (bounces >= RUSSIAN_ROULETTE) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // 打ち切り確率
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }
    }
    return L;
}


/**
* @brief 確率的レイトレーシング
* @param[in]  r_in      :カメラ方向からのレイ
* @param[in]  max_depth :レイの最大バウンス回数
* @param[in]  world     :レンダリングするシーンのデータ
* @return Vec3          :レイに沿った放射輝度
* @note ロシアンルーレットによる打ち切りを実装していないのでmax_depthは小さめにしておく
*/
Vec3 L_raytracing(const Ray& r_in, int max_depth, const Scene& world) {
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    // スペキュラレイのみを追跡する
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // 交差点情報
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);

        // 交差しないなら環境マップをサンプリング
        if (!is_intersect) {
            L += contrib * world.sample_envmap(r);
            break;
        }
        // 光源と交差したら寄与を追加
        if (isect.type == IsectType::Light) {
            L += contrib * isect.light->emitte();
            break;
        }
        // 出射方向のサンプリング
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal);
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        // 出射方向がスペキュラでないなら光源を明示的にサンプリング
        if (!is_spacular_type(sampled_type)) {
            L += contrib * explicit_direct_light(r, isect, world, shading_coord);
            break;
        }
        auto wi = shading_coord.to_world(wi_local);
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;
        r = Ray(isect.pos, wi); // 次のレイを生成
    }
    return L;
}

/**
* @brief シーン中のオブジェクトの法線を表示する関数
* @param[in]  r         :追跡するレイ
* @param[in]  world     :レンダリングするシーンのデータ
* @return Vec3          :法線の可視化
* @note z軸正の方向をカメラへ向かう方向とする
*/
Vec3 L_normal(const Ray& r, const Scene& world) {
    intersection isect;
    if (world.intersect(r, eps_isect, inf, isect)) {
        return 0.5f * (isect.normal + Vec3(1.0f, 1.0f, 1.0f));
    }
    return Vec3(1.0f, 1.0f, 1.0f);
}


/**
* @brief 色をガンマ補正
* @param[in]  color :ガンマ補正前の色
* @param[in]  gamma :ガンマ値
* @return Vec3      :ガンマ補正後の色
*/
Vec3 gamma_correction(const Vec3& color, float gamma) {
    // ガンマ補正
    float r = std::pow(color.get_x(), gamma);
    float g = std::pow(color.get_y(), gamma);
    float b = std::pow(color.get_z(), gamma);
    return Vec3(r, g, b);
}


/**
* @brief main関数
*/
int main(int argc, char* argv[]) {
    Random::init(); // 乱数の初期化
    // パラメータ
    const int nsample = (argc == 2) ? atoi(argv[1]) : SAMPLES; // レイのサンプル数
    constexpr auto max_depth = 100;  // レイの最大追跡数
    constexpr auto gamma = 1 / 2.2f; // ガンマ補正用
    // シーン
    Scene world;
    if (IMAGE_BASED_LIGHTING) {
        int w_envmap, h_envmap, c_envmap;
        float* envmap = stbi_loadf("asset/envmap.hdr", &w_envmap, &h_envmap, &c_envmap, 0);
        world = Scene(envmap, w_envmap, h_envmap, c_envmap);
    }
    Camera cam;
    //make_scene_simple(world, cam);
    //make_scene_cylinder(world, cam);
    //make_scene_MIS(world, cam);
    make_scene_cornell_box(world, cam);
    //make_scene_box_with_sphere(world, cam);
    //make_scene_vase(world, cam);
    //make_scene_thinfilm(world, cam);
    // 出力画像
    const auto w = cam.get_w(); // 高さ
    const auto h = cam.get_h(); // 幅
    const auto c = cam.get_c(); // チャンネル数
    std::vector<uint8_t> img(w * h * c);  // 画像データ

    // レイトレーシング
    auto start_time = std::chrono::system_clock::now(); // 計測開始時間
    int index = 0;
    for (int i = 0; i < h; i++) {
        std::cout << '\r' << i+1 << '/' << h << std::flush;
        for (int j = 0; j < w; j++) {
            Vec3 I(0.0f, 0.0f, 0.0f);
            // index番目のピクセルのサンプルを生成
            for (int k = 0; k < nsample; k++) {
                auto v = (i + Random::uniform_float()) / (h - 1);
                auto u = (j + Random::uniform_float()) / (w - 1);
                Ray r = cam.generate_ray(u, v);
                Vec3 L;
                if (DEBUG_MODE) {
                    L = L_normal(r, world);
                }
                else {
                    if (GLOBAL_ILLUMINATION) {
                        L = L_pathtracing(r, max_depth, world);
                        //L = L_naive_pathtracing(r, max_depth, world);
                    }
                    else {
                        L = L_raytracing(r, max_depth, world);
                    }
                }
                if (BIASED_DENOISING) {
                    I += clamp(exclude_invalid(L)); // ノイズが減るが物理ベースでない
                }
                else {
                    I += exclude_invalid(L);
                }
            }
            I *= 1.0f / nsample;
            if (IS_GAMMA_CORRECTION) I = gamma_correction(I, gamma);
            I = clamp(I); // [0, 1]でクランプ
            img[index++] = static_cast<uint8_t>(I.get_x() * 255);
            img[index++] = static_cast<uint8_t>(I.get_y() * 255);
            img[index++] = static_cast<uint8_t>(I.get_z() * 255);
        }
    }

    // 出力
    auto end_time = std::chrono::system_clock::now(); // 計測終了時間
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << '\n' <<  time_ms / 1000 << "sec\n";
    stbi_write_png(cam.get_filename(), w, h, 3, img.data(), w * c * sizeof(uint8_t));
}