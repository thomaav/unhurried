#pragma once

#include <deque>

#include "raylib.h"

#include "map.h"

/* (TODO, thoave01): Some settings file. */
constexpr float MOVEMENT_TICK_RATE = 0.3f;

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
	void load_model(const char *path);
	void draw(Camera3D &camera);

	tile m_position_logic;
	Vector2 m_position_render; /* (TODO, thoave01): Make m_position_render into a Vector3. */

	float m_movement_tick = 0.0f;
	bool m_moving = false;

	tile m_target_logic = {};
	std::deque<tile> m_path_logic = {};

	tile m_target_render = {};
	std::deque<tile> m_path_render = {};

	Color m_color_render = BLACK;
	Model m_model = {};
	bool m_has_model = false;
};
