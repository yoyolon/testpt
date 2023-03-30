/**
* @file  Math.h
* @brief ���w(���`�㐔)�N���X
*/

#pragma once

#include <cmath>
#include <iostream>
#include "utility.h"

// *** �֐� ***
inline float to_degree(float radian) { return 180.0f * radian / pi; }
inline float to_radian(float degree) { return pi * degree / 180.0f; }
inline float lerp(float a, float b, float t) { return (1 - t) * a + t * b; }


/** �O�����x�N�g���N���X */
class Vec3 {
public:
    /**
    * @brief �e�v�f�Ńx�N�g����������
    * @param[in] x :x����
    * @param[in] y :y����
    * @param[in] z :z����
    */
    Vec3(float x=0.0f, float y=0.0f, float z=0.0f) : e{x, y, z} {};

    /**
    * @brief �z��Ńx�N�g����������
    * @param[in] e :�e�v�f���i�[�����z��s
    */
    Vec3(float* _e) : e{_e[0], _e[1], _e[2]} {};

    // �z��A�N�Z�X���Z�q
    float operator[](int i) const { return e[i]; }
    float& operator[](int i) { return e[i]; }

    // �P�����Z�q
    Vec3 operator+() const { return *this; }
    Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }

    // ����������Z�q
    Vec3& operator+=(const Vec3 & a) {
        e[0] += a.e[0];
        e[1] += a.e[1];
        e[2] += a.e[2];
        return *this;
    }
    Vec3& operator-=(const Vec3& a) {
        e[0] -= a.e[0];
        e[1] -= a.e[1];
        e[2] -= a.e[2];
        return *this;
    }
    Vec3& operator*=(float t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }
    Vec3& operator/=(float t) {
        if (t == 0) return *this; // �[�����Z�̏���(�b��)
        float invt = 1.0f / t;
        return *this *= invt;
    }

    // �񍀉��Z�q
    friend inline Vec3 operator+(const Vec3& a, const Vec3& b) {
        return Vec3(a.e[0] + b.e[0], a.e[1] + b.e[1], a.e[2] + b.e[2]);
    }
    friend inline Vec3 operator-(const Vec3& a, const Vec3& b) {
        return Vec3(a.e[0] - b.e[0], a.e[1] - b.e[1], a.e[2] - b.e[2]);
    }
    friend inline Vec3 operator*(const Vec3& a, const Vec3& b) {
        return Vec3(a.e[0] * b.e[0], a.e[1] * b.e[1], a.e[2] * b.e[2]);
    }
    friend inline Vec3 operator*(const Vec3& a, float t) {
        return Vec3(a.e[0] * t, a.e[1] * t, a.e[2] * t);
    }
    friend inline Vec3 operator*(float t, const Vec3& a) {
        return Vec3(a.e[0] * t, a.e[1] * t, a.e[2] * t);
    }
    friend inline Vec3 operator/(const Vec3& a, float t) {
        if (t == 0) return a; // �[�����Z�̏���(�b��)
        float invt = 1.0f / t;
        return Vec3(a.e[0] * invt, a.e[1] * invt, a.e[2] * invt);
    }

    /**
    * @brief x�������擾����֐�
    * @return float :�x�N�g����x����
    */
    float get_x() const { return e[0]; }

    /**
    * @brief y�������擾����֐�
    * @return float :�x�N�g����y����
    */
    float get_y() const { return e[1]; }

    /**
    * @brief z�������擾����֐�
    * @return float :�x�N�g����z����
    */
    float get_z() const { return e[2]; }

    /**
    * @brief �x�N�g����L2�m�������v�Z����֐�
    * @return float :�x�N�g����L2�m����
    */
    float length2() const { return e[0] * e[0] + e[1] * e[1] + e[2] * e[2]; }

    /**
    * @brief �x�N�g���̃m�������v�Z����֐�
    * @return float :�x�N�g���̃m����
    */
    float length() const { return std::sqrt(length2()); }

    /**
    * @brief �x�N�g���̕��ϒl���v�Z����֐�
    * @return float :�x�N�g���̊e�����̕��ϒl
    */
    float average() const { return (e[0] + e[1] + e[2]) / 3; }

    /**
     * @brief 2�̃x�N�g���̓��ς��v�Z����֐�
     * @param[in]  a :�x�N�g��1
     * @param[in]  b :�x�N�g��2
     * @return float :�x�N�g���̓���
     */
    friend inline float dot(Vec3 a, Vec3 b) { 
        return a.e[0] * b.e[0] + a.e[1] * b.e[1] + a.e[2] * b.e[2]; 
    }

     /**
     * @brief 2�̃x�N�g���̊O�ς��v�Z����֐�
     * @param[in]  a :�x�N�g��1
     * @param[in]  b :�x�N�g��2
     * @return float :�x�N�g���̊O��
     */
    friend inline Vec3 cross(Vec3 a, Vec3 b) {
        return Vec3(a.e[1] * b.e[2] - a.e[2] * b.e[1],
                    a.e[2] * b.e[0] - a.e[0] * b.e[2],
                    a.e[0] * b.e[1] - a.e[1] * b.e[0]);
    }

private:
    float e[3]; /**< �x�N�g���̐��� */

public:
    // �萔
    static const Vec3 zero;  /**< �[���x�N�g��      */
    static const Vec3 one;   /**< �v�f��1�̃x�N�g�� */
    static const Vec3 red;   /**< �ԐF */
    static const Vec3 green; /**< �ΐF */
    static const Vec3 blue;  /**< �F */
};

inline std::ostream& operator<<(std::ostream& s, const Vec3& a) {
    return s << '(' << a.get_x() << ' ' << a.get_y() << ' ' << a.get_z() << ')';
}

/**
* @brief ��x�N�g���̔���
* @param[in]  a :�x�N�g��
* @return bool  :���茋��
*/
inline bool is_zero(const Vec3& a) { return a.length2() > 0 ? false : true; }

/**
* @brief �P�ʃx�N�g�����v�Z����֐�
* @param[in]  a :�x�N�g��
* @return Vec3  :�P�ʃx�N�g��
*/
inline Vec3 unit_vector(const Vec3& a) { return a / a.length(); }

/**
* @brief �e�������w�肵�����l�ŃN�����v����֐�
* @param[in]  a   :�N�����v�����x�N�g��
* @param[in]  min :�ŏ��l
* @param[in]  max :�ő�l
* @return Vec3  :�N�����v���ꂽ�x�N�g��
*/
inline Vec3 clamp(const Vec3& a, float min=0.0f, float max=1.0f) {
    auto x = std::clamp(a.get_x(), min, max);
    auto y = std::clamp(a.get_y(), min, max);
    auto z = std::clamp(a.get_z(), min, max);
    return Vec3(x, y, z); 
}

/**
* @brief ���˃x�N�g���̐����˕����x�N�g�����v�Z����֐�
* @param[in]  w :���˃x�N�g��
* @param[in]  n :�@���x�N�g��
* @return Vec3  :�����˕����x�N�g��
* @note ���̕\�ʂ��痣�������𐳂Ƃ���
*/
inline Vec3 reflect(const Vec3& w, const Vec3& n) {
    return -w + 2 * dot(w, n) * n;
}

/**
* @brief ���˃x�N�g���̋��ܕ����x�N�g�����v�Z����֐�(�S���˂Ȃ�[���x�N�g����Ԃ�)
* @param[in]  w   :���˃x�N�g��
* @param[in]  n   :�@���x�N�g��
* @param[in]  eta :���΋��ܗ�(n_theta / n_refract)
* @return Vec3    :�X�l���̖@���ɏ]�������ܕ����x�N�g��
* @note ���̕\�ʂ��痣�������𐳂Ƃ���
*/
inline Vec3 refract(const Vec3& w, const Vec3& n, float eta) {
    auto cos_theta    = dot(w, n);
    auto sin2_theta   = std::max(0.0f, 1.0f - cos_theta * cos_theta);
    auto sin2_refract = eta * eta * sin2_theta;
    // �S���˔���
    if (sin2_refract >= 1.0f) {
        return Vec3::zero;
    }
    auto cos_refract = std::sqrt(1.0f - sin2_refract);
    return eta * -w + (eta * cos_theta - cos_refract) * n;
}


/** �񎟌��x�N�g���N���X */
class Vec2 {
public:
    /**
    * @brief �e�v�f�Ńx�N�g����������
    * @param[in] x :x����
    * @param[in] y :y����
    */
    Vec2(float x = 0.0f, float y = 0.0f) : e{ x, y} {};

    /**
    * @brief �z��Ńx�N�g����������
    * @param[in] e :�e�v�f���i�[�����z��s
    */
    Vec2(float* _e) : e{ _e[0], _e[1] } {};

    // �z��A�N�Z�X���Z�q
    float operator[](int i) const { return e[i]; }
    float& operator[](int i) { return e[i]; }

    // �P�����Z�q
    Vec2 operator+() const { return *this; }
    Vec2 operator-() const { return Vec2(-e[0], -e[1]); }

    // ����������Z�q
    Vec2& operator+=(const Vec2& a) {
        e[0] += a.e[0];
        e[1] += a.e[1];
        return *this;
    }
    Vec2& operator-=(const Vec2& a) {
        e[0] -= a.e[0];
        e[1] -= a.e[1];
        return *this;
    }
    Vec2& operator*=(float t) {
        e[0] *= t;
        e[1] *= t;
        return *this;
    }
    Vec2& operator/=(float t) {
        if (t == 0) return *this; // �[�����Z�̏���(�b��)
        float invt = 1.0f / t;
        return *this *= invt;
    }

    // �񍀉��Z�q
    friend inline Vec2 operator+(const Vec2& a, const Vec2& b) {
        return Vec2(a.e[0] + b.e[0], a.e[1] + b.e[1]);
    }
    friend inline Vec2 operator-(const Vec2& a, const Vec2& b) {
        return Vec2(a.e[0] - b.e[0], a.e[1] - b.e[1]);
    }
    friend inline Vec2 operator*(const Vec2& a, const Vec2& b) {
        return Vec2(a.e[0] * b.e[0], a.e[1] * b.e[1]);
    }
    friend inline Vec2 operator*(const Vec2& a, float t) {
        return Vec2(a.e[0] * t, a.e[1] * t);
    }
    friend inline Vec2 operator*(float t, const Vec2& a) {
        return Vec2(a.e[0] * t, a.e[1] * t);
    }
    friend inline Vec2 operator/(const Vec2& a, float t) {
        if (t == 0) return a; // �[�����Z�̏���(�b��)
        float invt = 1.0f / t;
        return Vec2(a.e[0] * invt, a.e[1] * invt);
    }

    /**
    * @brief x�������擾����֐�
    * @return float :�x�N�g����x����
    */
    float get_x() const { return e[0]; }

    /**
    * @brief y�������擾����֐�
    * @return float :�x�N�g����y����
    */
    float get_y() const { return e[1]; }

    /**
    * @brief �x�N�g����L2�m�������v�Z����֐�
    * @return float :�x�N�g����L2�m����
    */
    float length2() const { return e[0] * e[0] + e[1] * e[1]; }

    /**
    * @brief �x�N�g���̃m�������v�Z����֐�
    * @return float :�x�N�g���̃m����
    */
    float length() const { return std::sqrt(length2()); }

    /**
    * @brief �x�N�g���̕��ϒl���v�Z����֐�
    * @return float :�x�N�g���̊e�����̕��ϒl
    */
    float average() const { return (e[0] + e[1]) / 2; }

    /**
     * @brief 2�̃x�N�g���̓��ς��v�Z����֐�
     * @param[in]  a :�x�N�g��1
     * @param[in]  b :�x�N�g��2
     * @return float :�x�N�g���̓���
     */
    friend inline float dot(Vec2 a, Vec2 b) {
        return a.e[0] * b.e[0] + a.e[1] * b.e[1];
    }

private:
    float e[2]; /**< �x�N�g���̐��� */

public:
    // �萔
    static const Vec2 zero;  /**< �[���x�N�g��      */
    static const Vec2 one;   /**< �v�f��1�̃x�N�g�� */
};


inline std::ostream& operator<<(std::ostream& s, const Vec2& a) {
    return s << '(' << a.get_x() << ' ' << a.get_y() << ')';
}

/**
* @brief ��x�N�g���̔���
* @param[in]  a :�x�N�g��
* @return bool  :���茋��
*/
inline bool is_zero(const Vec2& a) { return a.length2() > 0 ? false : true; }

/**
* @brief �P�ʃx�N�g�����v�Z����֐�
* @param[in]  a :�x�N�g��
* @return Vec2  :�P�ʃx�N�g��
*/
inline Vec2 unit_vector(const Vec2& a) { return a / a.length(); }

/**
* @brief �e�������w�肵�����l�ŃN�����v����֐�
* @param[in]  a   :�N�����v�����x�N�g��
* @param[in]  min :�ŏ��l
* @param[in]  max :�ő�l
* @return Vec2  :�N�����v���ꂽ�x�N�g��
*/
inline Vec2 clamp(const Vec2& a, float min = 0.0f, float max = 1.0f) {
    auto x = std::clamp(a.get_x(), min, max);
    auto y = std::clamp(a.get_y(), min, max);
    return Vec2(x, y);
}