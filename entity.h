#pragma once

#include <queue>

#include "third_party/raylib.h"

#include "map.h"

class entity
{
public:
	entity() = delete;
	entity(tile p);
	~entity() = default;

	entity &operator=(const entity &entity) = delete;
	entity(const entity &entity) = delete;

	void tick_logic();
	void tick_render(float tick_rate);
	void draw(Camera3D &camera);

	tile m_position_logic;
	Vector2 m_position_render;

	tile m_target = {};
	std::queue<tile> m_path = {};
	bool m_moving = false;
};
