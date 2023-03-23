#include "Shape.h"

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