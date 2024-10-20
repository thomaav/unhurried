#pragma once

#include "third_party/raylib.h"

#include "entity.h"
#include "map.h"

constexpr int SCREEN_WIDTH = 1080;
constexpr int SCREEN_HEIGHT = 720;

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
	void tick();
	void loop();

	map *m_current_map = nullptr;
	Camera3D m_camera = {};
	entity m_player = {};

	/* (TODO, thoave01): Temporary, should have a map manager/loader? */
	map m_map = {};
};
