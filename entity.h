#pragma once

#include <deque>

#include "raylib.h"

#include "map.h"
#include "menu.h"
#include "model.h"

/* (TODO, thoave01): Some settings file. */
extern float WALK_TICK_RATE;
extern float RUN_TICK_RATE;
extern float TURN_TICK_RATE;
extern float GAME_TICK_RATE;
extern float SPRITE_ANIMATION_TICK_RATE;
extern float ATTACK_TICK_RATE;

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

	/* Actions. */
	void set_action(action_data action_data);
	const char *get_action_string();
	void reset();
	void idle();
	void move(tile end);
	void attack(entity &target);

	/* Helpful methods. */
	BoundingBox get_active_bounding_box();
	float get_attack_distance(entity &target) const;
	tile get_closest_tile(entity &target) const;

	/* (TODO, thoave01): Shouldn't really be here... ? Who should be in charge? */
	map &m_map;
	asset_manager &m_asset_manager;
	manager &m_manager;

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
	model m_model = {};

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

	float m_current_attack_cast_time = 0.0f;
	float m_attack_cast_time = 1.02f;

	float m_current_attack_cooldown = 0.0f;
	float m_attack_cooldown = GAME_TICK_RATE * 1.25f;

	float m_current_attack_range = 10.0f;

	/* Debug things. */
	bool m_draw_bbox = true;

	/* Game tick. */
	/* (TODO, thoave01): This should be in the manager, common for all entities. */
	float m_game_tick = 0;

	/* Actions. */
	action m_current_action = action::IDLE;
};

class attack
{
public:
	attack() = delete;
	attack(entity &source, entity &target);
	~attack() = default;

	attack &operator=(const attack &attack) = delete;
	attack(const attack &attack) = delete;

	bool tick_render();
	void draw(Camera3D &camera);

	entity &m_source_entity;
	entity &m_target_entity;

	Vector3 m_position_render = {};

	model m_model = {};
};

class aoe_attack
{
public:
	aoe_attack() = delete;
	aoe_attack(tile &center_tile);
	~aoe_attack() = default;

	aoe_attack &operator=(const aoe_attack &aoe_attack) = delete;
	aoe_attack(const aoe_attack &aoe_attack) = delete;

	bool tick_render();
	void draw(Camera3D &camera);

	tile m_center_tile = {};
	i32 m_range = 3.0f;

	float m_tick = 0.0f;
	float m_length = 2.0f;
	float m_strength = 35.0f;
};
