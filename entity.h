#pragma once

#include <deque>

#include "raylib.h"

#include "animation.h"
#include "map.h"
#include "menu.h"

/* (TODO, thoave01): Some settings file. */
extern float WALK_TICK_RATE;
extern float RUN_TICK_RATE;
extern float TURN_TICK_RATE;
extern float GAME_TICK_RATE;
extern float ANIMATION_TICK_RATE;
extern float SPRITE_ANIMATION_TICK_RATE;

class entity;
class asset_manager;
class manager;

enum class action
{
	IDLE,
	MOVE,
	ATTACK
};

struct action_data
{
	action action;
	union
	{
		struct
		{
			tile end;
		} MOVE;
		struct
		{
			entity &entity;
		} ATTACK;
	};
};

class entity
{
public:
	entity() = delete;
	entity(tile p, map &map, asset_manager &asset_manager, manager &manager);
	~entity() = default;

	entity &operator=(const entity &entity) = delete;
	entity(const entity &entity) = delete;

	void tick_combat();
	void tick_movement_logic();
	void tick_render();
	void draw(Camera3D &camera);
	void set_animation(animation animation);

	/* Actions. */
	void set_action(action_data action_data);
	void reset();
	void idle();
	void move(tile end);
	void attack(entity &entity);

	/* (TODO, thoave01): Shouldn't really be here... ? Who should be in charge? */
	map &m_map;
	asset_manager &m_asset_manager;
	manager &m_manager;

	menu main_menu = {};

	tile m_position_logic;
	Vector3 m_position_render;

	float m_movement_tick = 0.0f;
	bool m_running = false;
	float m_movement_tick_rate = WALK_TICK_RATE;

	tile m_target_logic = {};
	std::deque<tile> m_path_logic = {};

	/* (TODO, thoave01): Make target_render into a Vector3. */
	Vector3 m_target_render = {};
	std::deque<tile> m_path_render = {};

	/* (TODO, thoave01): Unload models etc. for animation. */
	animation_data m_animation_data = {};
	u32 m_animation_current_frame = 0;
	float m_animation_tick = 0.0f;

	/* Preferably this would be baked in the model, but it doesn't work with our animation approach. We can't mix and
	 * match; either we have to put everything in the model, or nothing, because of the order of operations for the
	 * matrix multiplications. */
	Matrix m_model_rotation = {};

	/* Combat. */
	/* (TODO, thoave01): Read this from entity type file or something. */
	float m_health = 100.0f;
	float m_max_health = 100.0f;
	float m_attack_strength = 20.0f;
	entity *m_target = nullptr;
	Color m_tint = WHITE;

	float m_current_attack_cast_time = 0.0f;
	float m_attack_cast_time = GAME_TICK_RATE / 1.5f;

	float m_current_attack_cooldown = 0.0f;
	float m_attack_cooldown = GAME_TICK_RATE * 3.0f;

	bool m_draw_bbox = true;

	/* Game tick. */
	/* (TODO, thoave01): This should be in the manager, common for all entities. */
	float m_game_tick = 0;

	/* Actions. */
	action m_current_action = action::IDLE;
};
