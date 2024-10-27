#pragma once

#include <deque>

#include "raylib.h"

#include "map.h"

/* (TODO, thoave01): Some settings file. */
constexpr float MOVEMENT_TICK_RATE = 0.3f;
constexpr float TURN_TICK_RATE = 1.2f;
constexpr float GAME_TICK_RATE = 0.6f;
constexpr float ANIMATION_TICK_RATE = 0.15;

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
	/* (TODO, thoave01): There is a Model.transform in raylib. */
	/* (TODO, thoave01): Unload models etc. */
	Matrix m_model_transform = {};

	/* Animation stuff. */
	ModelAnimation *m_model_animations = nullptr;
	i32 m_animation_count = 0;
	u32 m_animation_index = 0;

	/* Shape key animation stuff. */
	u32 m_animation_current_frame = 0;
	float m_animation_tick = 0.0f;
};
