#pragma once

#include <deque>
#include <unordered_map>

#include "raylib.h"

#include "entity.h"
#include "map.h"
#include "sprite.h"

enum class event
{
	NONE = 0,
	MOVE_TILE = 1,
	CLICK_BOSS = 2,
};

struct event_data
{
	event event;
	union
	{
		struct
		{
			tile clicked_tile;
		} MOVE_TILE;
	};
};

/* (TODO, thoave01): Perhaps also add some sort of pre-loading/caching for models for animations? */
class asset_manager
{
public:
	void load_assets();
	void load_animation(animation animation);
	void set_animation(entity &entity, animation animation);

	sprite_animation m_click_yellow = {};
	sprite_animation m_click_red = {};
	std::unordered_map<animation, animation_data> m_animations = {};

private:
};

class manager
{
public:
	manager() = default;
	~manager() = default;

	manager &operator=(const manager &manager) = delete;
	manager(const manager &manager) = delete;

	void run();

private:
	void init();
	void set_map(map &map);
	void update_camera();

	void parse_events();
	void handle_move_tile_event(event_data &event_data);
	void handle_click_boss_event();

	void tick();
	void draw();
	void loop();

	float m_game_tick = 0.0f;
	std::deque<event_data> m_events = {};

	map *m_current_map = nullptr;

	entity m_player = { { 0, 0 } };
	entity m_boss = { { 8, 8 } };

	Camera3D m_root_camera = {};
	Camera3D m_camera = {};

	/* (TODO, thoave01): Temporary, should have a map manager/loader? */
	map m_map = {};

	/* Clicking. */
	bool m_clicked = false;
	u32 m_click_frame = 0;
	const u32 m_click_frame_count = 4;
	float m_click_tick = 0;
	Color m_click_color = WHITE;

	/* (TODO, thoave01): I don't really know how to do this correctly. */
	/* Assets. */
	asset_manager m_asset_manager = {};
};
