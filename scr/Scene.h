#pragma once

#include "Shape.h"

// *** �V�[���N���X ***
class scene : Shape {
public:
	scene() {}
	scene(std::shared_ptr<Shape> _object) { add(_object); }

	// �V�[���ɃI�u�W�F�N�g��ǉ�
	void add(std::shared_ptr<Shape> object) { object_list.push_back(object); }
	void clear() { object_list.clear(); }

	// ���C�ƃV�[���̌�������
	// TODO: VBH�ɂ�鍂����
	bool intersect(const Ray& r, float t_min, float t_max, intersection& p) const {
		intersection isect;
		bool is_isect = false;
		auto t_first = t_max;
		for (const auto& object : object_list) {
			if (object->intersect(r, t_min, t_first, isect)) {
				is_isect = true;
				t_first = isect.t;
				p = isect;
			}
		}
		return is_isect;
	}

private:
	std::vector<std::shared_ptr<Shape>> object_list; // �V�[�����̃I�u�W�F�N�g
};