#pragma once

#include "map.h"

struct Camera3D;

class entity
{
public:
	entity() = default;
	entity(position p);
	~entity() = default;

	entity &operator=(const entity &entity) = delete;
	entity(const entity &entity) = delete;

	void tick();
	void draw(Camera3D &camera);

	position m_p = {};
	position_f m_p_f = {};

	position m_target = {};
};
