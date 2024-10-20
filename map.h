#pragma once

#include <vector>

#include "types.h"

struct Camera3D;

struct position
{
	i64 x;
	i64 y;
};

struct position_f
{
	float x;
	float y;
};

class tile
{
public:
	tile() = default;
	tile(i64 x, i64 y);
	~tile() = default;

	tile &operator=(const tile &tile) = default;
	tile(const tile &tile) = default;

	void draw();

	position m_p = {};
	bool m_toggled = false;
};

class tile_set
{
public:
	tile_set() = delete;
	tile_set(u32 x, u32 y);
	~tile_set() = default;

	tile_set &operator=(const tile_set &tile_set) = delete;
	tile_set(const tile_set &tile_set) = delete;

	void toggle_tile(u32 x, u32 y);
	void draw(Camera3D &camera);

	u64 m_width = 0;
	u64 m_height = 0;
	std::vector<std::vector<tile>> m_tiles = {};
};

class map
{
public:
	map() = default;
	~map() = default;

	map &operator=(const map &map) = delete;
	map(const map &map) = delete;

	void draw(Camera3D &camera);
	void set_recommended_camera(Camera3D &camera);

	tile_set m_tile_set = { 16, 16 };
};
