#include "Renderer.h"
#include "external/stb_image_write.h"
#include "external/stb_image.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "Camera.h"
#include "Film.h"
#include "Fresnel.h"
#include "Light.h"
#include "MakeScene.h"
#include "Material.h"
#include "Math.h"
#include "Microfacet.h"
#include "ONB.h"
#include "Random.h"
#include "Ray.h"
#include "Scene.h"
#include "Shape.h"
#include "Math.h"


// デバッグ用
constexpr bool DEBUG_MODE = false; // (デバッグモード)法線可視化を有効にする
constexpr bool IS_GAMMA_CORRECTION = true;  // ガンマ補正を有効にする

/**
* @brief  Low-Discrepancy数列の生成に利用する関数
* @param[in] i  :
* @return float :[0,1]の実数
* @note // Ref. T. Kollig and A. Keller. "Efficient Multidimensional Sampling" 2002.
*/
float radical_inverse(unsigned int i) {
    i = (i << 16) | (i >> 16);
    i = ((i & 0x00FF00FF) << 8) | ((i & 0xFF00FF00) >> 8);
    i = ((i & 0x0F0F0F0F) << 4) | ((i & 0xF0F0F0F0) >> 4);
    i = ((i & 0x33333333) << 2) | ((i & 0xCCCCCCCC) >> 2);
    i = ((i & 0x55555555) << 1) | ((i & 0xAAAAAAAA) >> 1);
    return (float)i / (float)0x100000000;
}


Renderer::Renderer(int _spp, Sampling _strategy)
    : spp(_spp), strategy(_strategy)
{}

Vec3 Renderer::explict_uniform(const Ray& r, const intersection& isect, 
    const Scene& world, const ONB& shading_coord) const {
    auto Ld = Vec3::zero;

    // 入射方向をランダムにサンプリング
    Vec3 wi_local = Random::uniform_hemisphere_sample();
    Vec3 wi = shading_coord.to_world(wi_local);

    // 光源へのレイが光源と交差しなければ寄与はゼロ
    auto r_to_light = Ray(isect.pos, wi); // 光源へ向かうのレイ
    intersection isect_light;
    if (!world.intersect_light(r_to_light, eps_isect, inf, isect_light)) {
        return Ld;
    }

    // 交差した光源の放射輝度を計算
    auto L = isect_light.light->evel_light(wi);
    if (is_zero(L)) {
        return Ld;
    }

    // 光源へのレイがシェイプに遮蔽されると寄与はゼロ
    if (world.intersect_object(r_to_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // サンプリング方向でのBSDFとpdfを評価
    auto wo = unit_vector(r.get_dir());
    auto wo_local = -shading_coord.to_local(wo); // 物体表面から離れる方向が正
    auto bsdf = isect.mat->eval_f(wo_local, wi_local, isect);
    if (is_zero(bsdf)) {
        return Ld;
    }
    float pdf_scattering = 0.5f * invpi; // 一様サンプリングのため
    // 寄与の計算
    auto cos_term = dot(isect.normal, wi);
    Ld += bsdf * cos_term * L / pdf_scattering;
    return Ld;
}

Vec3 Renderer::explict_bsdf(const Ray& r, const intersection& isect, 
    const Scene& world, const ONB& shading_coord) const{
    auto Ld = Vec3::zero;
    Vec3 wi_local;
    float pdf_scattering, pdf_light, weight = 1.0f;

    // BSDFに基づき直接光の入射方向をサンプリング
    auto wo = unit_vector(r.get_dir());
    auto wo_local = -shading_coord.to_local(wo);
    BxDFType sampled_type;
    auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf_scattering, sampled_type);
    if (pdf_scattering == 0 || is_zero(bsdf)) {
        return Ld;
    }
    auto wi = shading_coord.to_world(wi_local); // 直接光の入射方向(交差点から離れる方向が正)

    // 光源へのレイが光源と交差しなければ寄与はゼロ
    auto r_to_light = Ray(isect.pos, wi); // 光源へ向かうのレイ
    intersection isect_light;
    if (!world.intersect_light(r_to_light, eps_isect, inf, isect_light)) {
        return Ld;
    }

    // 交差した光源の放射輝度を計算
    auto L = isect_light.light->evel_light(wi);
    pdf_light = isect_light.light->eval_pdf(isect, wi);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }

    // 光源へのレイがシェイプに遮蔽されると寄与はゼロ
    if (world.intersect_object(r_to_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // 寄与の計算
    bool is_delta_bxdf = is_spacular_type(sampled_type); // デルタ分布ならMIS重みは1
    if (strategy == Sampling::MIS && !is_delta_bxdf) {
        weight = Random::power_heuristic(1, pdf_scattering, 1, pdf_light);
    }
    auto cos_term = dot(isect.normal, wi);
    Ld += bsdf * cos_term * weight * L / pdf_scattering;
    return Ld;
}

Vec3 Renderer::explict_one_light(const Ray& r, const intersection& isect, 
    const Scene& world, const ONB& shading_coord) const {
    auto Ld = Vec3::zero;
    Vec3 wi; // 光源の入射方向(交差点から離れる方向が正)
    float pdf_scattering, pdf_light, weight = 1.0f;

    // 光源をランダムに一つ選択
    // TODO: 光源エネルギー分布をベースに光源を選びたい
    const auto& lights = world.get_light();
    int num_lights = (int)lights.size();
    if (num_lights == 0) {
        return Ld;
    }
    auto light_index = Random::uniform_int(0, num_lights - 1);
    const auto& light = lights[light_index];

    // 選んだ光源から入射方向をサンプリング
    auto L = light->sample_light(isect, wi, pdf_light);
    if (pdf_light == 0 || is_zero(L)) {
        return Ld;
    }
    pdf_light = pdf_light / num_lights; // 光源のランダムな選択を考慮

    // 光源へのレイが遮蔽されると寄与はゼロ
    auto r_to_light = Ray(isect.pos, wi); // 光源へ向かうレイ
    intersection isect_light;
    light->intersect(r_to_light, eps_isect, inf, isect_light); // 光源の交差点を取得
    if (world.intersect_object(r_to_light, eps_isect, isect_light.t)) {
        return Ld;
    }

    // サンプリングした入射方向でのBSDFを評価
    auto wo = unit_vector(r.get_dir());
    auto wo_local = -shading_coord.to_local(wo);
    auto wi_local = shading_coord.to_local(wi);
    auto bsdf = isect.mat->eval_f(wo_local, wi_local, isect);
    pdf_scattering = isect.mat->eval_pdf(wo_local, wi_local, isect);
    if (pdf_scattering == 0 || is_zero(bsdf)) {
        return Ld;
    }

    // 寄与の計算
    bool is_delta_light = light->is_delta_light(); // デルタ分布ならMIS重みは1
    if (strategy == Sampling::MIS && !is_delta_light) {
        weight = Random::power_heuristic(1, pdf_light, 1, pdf_scattering);
    }
    auto cos_term = std::abs(dot(isect.normal, wi));
    Ld += bsdf * L * cos_term * weight / pdf_light;
    return Ld;
}

Vec3 Renderer::explicit_direct_light_sampling(const Ray& r, const intersection& isect,
    const Scene& world, const ONB& shading_coord) const {
    auto Ld = Vec3::zero;
    // 一様サンプリング
    if (strategy == Sampling::UNIFORM) {
        auto L = explict_uniform(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
        return Ld;
    }
    // BSDFに基づきサンプリング
    if ((strategy == Sampling::BSDF) || (strategy == Sampling::MIS)) {
        auto L = explict_bsdf(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
    }
    // 光源に基づきサンプリング
    if ((strategy == Sampling::LIGHT) || (strategy == Sampling::MIS)) {
        auto L = explict_one_light(r, isect, world, shading_coord);
        Ld += exclude_invalid(L);
    }
    return Ld;
}


Vec3 Renderer::L_raytracing(const Ray& r_in, int max_depth, const Scene& world) const {
    const int RUSSIAN_ROULETTE = 1;
    const int SPLIT_SAMPLES = 1; // シャドウレイのサンプル数
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    // スペキュラレイのみを追跡する
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // 交差点情報
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        if (is_intersect == false) {
            L = world.get_bg_color();
            return L;
        }
        // 光源と交差したら寄与を追加
        if (isect.type == IsectType::Light) {
            L += contrib * isect.light->evel_light(r.get_dir());
            return L;
        }

        // シェーディング座標の生成
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal);

        // 交差した物体がスペキュラでないなら直接光のサンプリング
        if (!isect.mat->is_perfect_specular()) {
            for (int i = 0; i < SPLIT_SAMPLES; i++) {
                L += contrib * explicit_direct_light_sampling(r, isect, world, shading_coord);
            }
            L /= SPLIT_SAMPLES;
            break;
        }

        // BSDFに基づく経路(方向)のサンプリング
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir()));
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        auto wi = shading_coord.to_world(wi_local);

        // 寄与の更新
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;

        //ロシアンルーレット
        if (bounces >= RUSSIAN_ROULETTE) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // 打ち切り確率
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }

        // 次のレイを生成
        r = Ray(isect.pos, wi);
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
Vec3 Renderer::L_naive_pathtracing(const Ray& r_in, int max_depth, const Scene& world) const {
    const int RUSSIAN_ROULETTE = 1;
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    bool is_specular_ray = false;
    // パストレーシング
    for (int bounces = 0; bounces < max_depth; bounces++) {
        intersection isect; // 交差点情報
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        if (is_intersect == false) {
            break;
        }
        // 光源と交差したら寄与を加算
        if (isect.type == IsectType::Light) {
            // 面光源かつ法線が逆向きの場合は光源をサンプルしない
            auto light_type = isect.light->get_type();
            if (light_type == LightType::Area && dot(isect.normal, -r.get_dir()) < 0) {
                return Vec3::zero;
            }
            L += contrib * isect.light->evel_light(r.get_dir());
            break;
        }
        // BSDFに基づく入射方向のサンプリング
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


Vec3 Renderer::L_pathtracing(const Ray& r_in, int max_depth, const Scene& world) const {
    const int RUSSIAN_ROULETTE = 1;
    auto L = Vec3::zero, contrib = Vec3::one;
    Ray r = Ray(r_in);
    bool is_specular_ray = false;
    // パストレーシング
    for (int bounces = 0; bounces < max_depth; bounces++) {
        // 交差判定
        intersection isect; // 交差点情報
        bool is_intersect = world.intersect(r, eps_isect, inf, isect);
        if (is_intersect == false) {
            break;
        }
        // カメラレイとスペキュラレイは光源の寄与を加算
        if (bounces == 0 || is_specular_ray) {
            if (isect.type == IsectType::Light) {
                // 面光源かつ法線が逆向きの場合は光源をサンプルしない
                auto light_type = isect.light->get_type();
                if (light_type == LightType::Area && !isect.is_front) {
                    return Vec3::zero;
                }
                L += contrib * isect.light->evel_light(r.get_dir());
                break;
            }
        }
        // カメラレイとスペキュラレイ以外は光源との交差時に寄与を加算しない(明示的に光源をサンプリングするため)
        if (isect.type == IsectType::Light) {
            break;
        }

        // シェーディング座標の生成
        ONB shading_coord(isect.is_front ? isect.normal : -isect.normal); // 透過側なら法線を反転

        // 交差した物体がスペキュラでないなら直接光のサンプリング
        if (!isect.mat->is_perfect_specular()) {
            L += contrib * explicit_direct_light_sampling(r, isect, world, shading_coord);
        }

        // BSDFに基づく経路(方向)のサンプリング
        Vec3 wo_local = -shading_coord.to_local(unit_vector(r.get_dir())); // 物体表面から離れる方向が正
        Vec3 wi_local;
        float pdf;
        BxDFType sampled_type;
        auto bsdf = isect.mat->sample_f(wo_local, isect, wi_local, pdf, sampled_type);
        if (pdf == 0.0f || is_zero(bsdf)) break;
        auto wi = shading_coord.to_world(wi_local);

        // 寄与の更新
        auto cos_term = std::abs(dot(isect.normal, wi));
        contrib = contrib * bsdf * cos_term / pdf;

        //ロシアンルーレット
        if (bounces >= RUSSIAN_ROULETTE) {
            float p_rr = std::max(0.05f, 1.0f - contrib.average()); // 打ち切り確率
            if (p_rr > Random::uniform_float()) break;
            contrib /= std::max(epsilon, 1.0f - p_rr);
        }

        // 次のレイを生成
        is_specular_ray = is_spacular_type(sampled_type);
        r = Ray(isect.pos, wi); // 次のレイを生成
    }
    return L;
}

Vec3 Renderer::L_normal(const Ray& r, const Scene& world) const {
    intersection isect;
    if (world.intersect(r, eps_isect, inf, isect)) {
        if (isect.type == IsectType::Light && isect.light->get_type() == LightType::IBL) {
            return Vec3(0.5f, 0.5f, 0.5f); // 環境マップ
        }
        Vec3 normal_color = 0.5f * (isect.normal + Vec3(1.0f, 1.0f, 1.0f));
        return normal_color;
        //return Vec3(normal_color[0], 0.f, 0.f); // x軸成分のみ表示
        //return Vec3(0.f, normal_color[1], 0.f); // y軸成分のみ表示
        //return Vec3(0.f, 0.f, normal_color[2]); // z軸成分のみ表示
    }
    return Vec3(0.f, 0.f, 0.f); // 交差しない
}


void Renderer::render(const Scene& world, const Camera& cam) const {
    const int max_depth = 100;
    Random::init(); // 乱数の初期化

    // 出力画像の設定
    const auto w = cam.get_w(); // 高さ
    const auto h = cam.get_h(); // 幅
    const auto c = cam.get_c(); // チャンネル数
    std::vector<uint8_t> img(w * h * c);  // 画像データ

    // レイトレーシング
    auto start_time = std::chrono::system_clock::now(); // 計測開始時間
    int index = 0;
    for (int y = 0; y < h; y++) {
        std::cout << '\r' << y + 1 << '/' << h << std::flush;
        for (int x = 0; x < w; x++) {
            Vec3 I(0.f, 0.f, 0.f);
            // index番目のピクセルのサンプルを生成
            for (int k = 0; k < spp; k++) {
                //Vec2 uv(float(k)/spp, radical_inverse(k)); // Low-Discrepancy数列を利用
                Vec2 uv(Random::uniform_float(), Random::uniform_float()); // 一様サンプリング
                Ray r = cam.generate_ray((x + uv[0]) / (w - 1), (y + uv[1]) / (h - 1));
                Vec3 L;
                if (DEBUG_MODE) {
                    L = L_normal(r, world);
                }
                else {
                    //L = L_raytracing(r, max_depth, world);
                    //L = L_naive_pathtracing(r, max_depth, world);
                    L = L_pathtracing(r, max_depth, world);
                }
                I += exclude_invalid(L);
            }
            I *= 1.0f / spp;
            I = clamp(I); // [0, 1]でクランプ(TODO: トーンマッピングの実装)
            if (IS_GAMMA_CORRECTION) I = gamma_correction(I);
            img[index++] = static_cast<uint8_t>(I.get_x() * 255);
            img[index++] = static_cast<uint8_t>(I.get_y() * 255);
            img[index++] = static_cast<uint8_t>(I.get_z() * 255);
        }
    }

    // 画像出力
    auto end_time = std::chrono::system_clock::now(); // 計測終了時間
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << '\n' << time_ms / 1000 << "sec\n";
    stbi_write_png(cam.get_filename(), w, h, 3, img.data(), w * c * sizeof(uint8_t));
}