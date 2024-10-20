#pragma once

#include <queue>

#include "third_party/raylib.h"

#include "map.h"

/* (TODO, thoave01): Some settings file. */
constexpr float TICK_RATE = 0.6f;

class entity
{
public:
	entity() = delete;
	entity(tile p);
	~entity() = default;

	entity &operator=(const entity &entity) = delete;
	entity(const entity &entity) = delete;

	void tick_logic();
	void tick_render();
	void draw(Camera3D &camera);

	tile m_position_logic;
	Vector2 m_position_render;

	tile m_target = {};
	std::queue<tile> m_path = {};
	bool m_moving = false;
	float m_movement_tick = 0.0f;
};
