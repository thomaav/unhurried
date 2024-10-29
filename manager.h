#pragma once

#include <deque>

#include "raylib.h"

#include "entity.h"
#include "map.h"

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
};
