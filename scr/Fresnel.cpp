#include "Fresnel.h"
#include <complex>

/**
* @brief s�Ό��̃t���l�����ˌW�����v�Z����֐�
* @param[in] ni   :�}��i�̋��ܗ�
* @param[in] nj   :�}��j�̋��ܗ�
* @param[in] cosi :�}��i�ւ̓��ˊp�]��
* @param[in] cosj :�}��j�ւ̓��ˊp�]��
* @return float   :s�Ό��̃t���l�����ˌW��
*/
inline float fresnel_rs(float ni, float nj, float cosi, float cosj) {
    return (ni * cosi - nj * cosj) / (ni * cosi + nj * cosj);
}

/**
* @brief s�Ό��̃t���l�����ߌW�����v�Z����֐�
* @param[in] ni   :�}��i�̋��ܗ�
* @param[in] nj   :�}��j�̋��ܗ�
* @param[in] cosi :�}��i�ւ̓��ˊp�]��
* @param[in] cosj :�}��j�ւ̓��ˊp�]��
* @return float   :s�Ό��̃t���l�����ߌW��
*/
inline float fresnel_ts(float ni, float nj, float cosi, float cosj) {
    return (ni * cosi + ni * cosi) / (ni * cosi + nj * cosj);
}

/**
* @brief p�Ό��̃t���l�����ˌW�����v�Z����֐�
* @param[in] ni   :�}��i�̋��ܗ�
* @param[in] nj   :�}��j�̋��ܗ�
* @param[in] cosi :�}��i�ւ̓��ˊp�]��
* @param[in] cosj :�}��j�ւ̓��ˊp�]��
* @return float   :s�Ό��̃t���l�����ˌW��
*/
inline float fresnel_rp(float ni, float nj, float cosi, float cosj) {
    return (nj * cosi - ni * cosj) / (ni * cosj + nj * cosi);
}

/**
* @brief p�Ό��̃t���l�����ߌW�����v�Z����֐�
* @param[in] ni   :�}��i�̋��ܗ�
* @param[in] nj   :�}��j�̋��ܗ�
* @param[in] cosi :�}��i�ւ̓��ˊp�]��
* @param[in] cosj :�}��j�ւ̓��ˊp�]��
* @return float   :s�Ό��̃t���l�����ߌW��
*/
inline float fresnel_tp(float ni, float nj, float cosi, float cosj) {
    return (ni * cosi + ni * cosi) / (ni * cosj + nj * cosi);
}


/**
* @brief �����l�������t���l�����ˌW�����v�Z����֐�
* @param[in] r            :�t���l�����ˌW��
* @param[in] R            :�t���l�����ˌW��
* @param[in] phi          :�ʑ���
* @return complex<float>  :�������ˌW��
* @note: �Q�l: Hirayama+. "Visualization of optical phenomena caused by multilayer films based on wave optics". 2001.
*/
static std::complex<float> composit_r(float r, float R, float phi) {
    std::complex<float>i(0.f, 1.f);
    return (r + R * exp(i * 2.0f * phi)) / (1.0f + r * R * exp(i * 2.0f * phi));
}
/**
* @brief �����l�������t���l�����ߌW�����v�Z����֐�
* @param[in] r            :�t���l�����ˌW��
* @param[in] R            :�t���l�����ˌW��
* @param[in] t            :�t���l�����ߌW��
* @param[in] T            :�t���l�����ߌW��
* @param[in] phi          :�ʑ���
* @return complex<float>  :�������ߌW��
* @note: �Q�l: Hirayama+. "Visualization of optical phenomena caused by multilayer films based on wave optics". 2001.
*/
static std::complex<float> composit_t(float r, float R, float t, float T, float phi) {
    std::complex<float>i(0.f, 1.f);
    return (t * T * exp(i * phi)) / (1.0f + r * R * exp(i * 2.0f * phi));
}

/**
* @brief �����l���������˗����v�Z����֐�
* @param[in] cos0 :���ˊp�]��
* @param[in] d    :�����̖���
* @param[in] n0   :���˔}���̋��ܗ�
* @param[in] n1   :�����̋��ܗ�
* @param[in] n2   :�o�˔}���̋��ܗ�
* @return Vec3    :���������l���������˗�
* @note: �Q�l: Hirayama+. "Visualization of optical phenomena caused by multilayer films based on wave optics". 2001.
*/
Vec3 iridterm(float cos0, float d, float n0, float n1, float n2) {
    // ���ߊp�v�Z
    float sin0 = std::sqrt(std::max(0.0f, 1.0f - cos0 * cos0));
    float sin1 = n0 / n1 * sin0;
    if (sin1 >= 1.0) return  Vec3(1.0f, 1.0f, 1.0f); // �S����
    float cos1 = std::sqrt(std::max(0.0f, 1.0f - sin1 * sin1));
    float sin2 = n0 / n2 * sin0;
    if (sin2 >= 1.0) return Vec3(1.0f, 1.0f, 1.0f);  // �S����
    float cos2 = std::sqrt(std::max(0.0f, 1 - sin2 * sin2));
    // �t���l�����ˌW���v�Z
    float r01s = fresnel_rs(n0, n1, cos0, cos1);
    float r12s = fresnel_rs(n1, n2, cos1, cos2);
    float r01p = fresnel_rp(n0, n1, cos0, cos1);
    float r12p = fresnel_rp(n1, n2, cos1, cos2);
    // �������˗��v�Z
    float lambda[3] = { 640.0, 540.0, 450.0 };
    float rgb[3] = { 0.0 };
    std::complex<float>r_s[3];
    std::complex<float>r_p[3];
    for (int i = 0; i < 3; i++) {
        float phi = 2 * pi / lambda[i] * n1 * d * cos1;
        r_s[i] = composit_r(r01s, r12s, phi); // �������ˌW��(s�g)
        r_p[i] = composit_r(r01p, r12p, phi); // �������ˌW��(p�g)
        rgb[i] = (norm(r_s[i]) + norm(r_p[i])) / 2;
    }
    return Vec3(rgb);
}


// *** ��蔽�˗� ***
FresnelConstant::FresnelConstant(Vec3 _F0)
    : F0(_F0) {}

Vec3 FresnelConstant::evaluate(float cos_theta) const {
    return F0;
}


// *** Schlick�t���l�� ***
FresnelSchlick::FresnelSchlick(Vec3 _F0)
    : F0(_F0) {}

Vec3 FresnelSchlick::evaluate(float cos_theta) const {
    return F0 + (Vec3(1.0f,1.0f,1.0f) - F0) * (float)std::pow(1.0f - cos_theta, 5);
}


// *** �U�d�̃t���l�� ***
FresnelDielectric::FresnelDielectric(float _ni, float _no) 
    : ni(_ni), no(_no) {}

Vec3 FresnelDielectric::evaluate(float cos_theta) const {
    bool is_inside = cos_theta < 0; // ���˃��C���}���̓����ɂ��邩����
    cos_theta = std::abs(cos_theta);
    auto n_inside  = is_inside ? no : ni;
    auto n_outside = is_inside ? ni : no;
    float sin_theta = std::sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));
    float sin_out = no / ni * sin_theta;
    if (sin_out >= 1.0f) return Vec3::zero;
    float cos_out = std::sqrt(std::max(0.0f, 1.0f - sin_out * sin_out));
    float Rs = fresnel_rs(ni, no, cos_theta, cos_out);
    float Rp = fresnel_rp(ni, no, cos_theta, cos_out);
    float R =  (Rs * Rs + Rp * Rp) / 2;
    return Vec3(R, R, R);
}


// *** �������t���l�� ***
FresnelThinfilm::FresnelThinfilm(float _d, float _ni, float _nf, float _no)
    : d(_d), ni(_ni), nf(_nf), no(_no) {}

Vec3 FresnelThinfilm::evaluate(float cos_theta) const {
    return iridterm(cos_theta, d, ni, nf, no);
}