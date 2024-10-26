#pragma once

#include <deque>

#include "raylib.h"

#include "entity.h"
#include "map.h"

enum class event
{
	NONE = 0,
	LEFT_MOUSE_CLICK = 1,
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
	void tick();
	void draw();
	void loop();

	float m_game_tick = 0.0f;
	std::deque<event> m_events = {};

	map *m_current_map = nullptr;

	entity m_player = { { 0, 0 } };
	entity m_boss = { { 8, 8 } };

	Camera3D m_root_camera = {};
	Camera3D m_camera = {};

	/* (TODO, thoave01): Temporary, should have a map manager/loader? */
	map m_map = {};
};
