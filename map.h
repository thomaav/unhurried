#pragma once

#include <deque>
#include <vector>

#include "raylib.h"

#include "types.h"

static constexpr float MAP_HEIGHT = -0.16f;

struct tile
{
	i32 x;
	i32 y;
};

enum class tile_type
{
	OPEN,
	OCCUPIED,
};

inline bool operator==(const tile &t1, const tile &t2)
{
	return t1.x == t2.x && t1.y == t2.y;
}

inline bool operator!=(const tile &t1, const tile &t2)
{
	return t1.x != t2.x || t1.y != t2.y;
}

inline bool operator<(const tile &t1, const tile &t2)
{
	return t1.x < t2.x || (t1.x == t2.x && t1.y < t2.y);
}

float tile_distance(tile from, tile to);
float tile_euclidean_distance(tile from, tile to);
float tile_manhattan_distance(tile from, tile to);

class map
{
public:
	map() = default;
	~map() = default;

	map &operator=(const map &map) = delete;
	map(const map &map) = delete;

	void draw(Camera3D &camera);
	bool is_open_tile(tile tile);
	bool find_closest_open_tile(tile root, tile &closest);
	bool is_legal_move(tile from, tile to);
	void generate_path(tile from, tile to, std::deque<tile> &path);

	std::vector<std::vector<tile_type>> m_tile_types;

	i32 m_width = 32;
	i32 m_height = 32;
};
