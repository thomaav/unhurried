#pragma once

#include <vector>

#include "third_party/raylib.h"

#include "types.h"

struct tile
{
	i32 x;
	i32 y;
};

inline bool operator==(const tile &t1, const tile &t2)
{
	return t1.x == t2.x && t1.y == t2.y;
}

inline bool operator!=(const tile &t1, const tile &t2)
{
	return t1.x != t2.x || t1.y != t2.y;
}

class map
{
public:
	map() = default;
	~map() = default;

	map &operator=(const map &map) = delete;
	map(const map &map) = delete;

	void draw(Camera3D &camera);
	void set_recommended_camera(Camera3D &camera);
	void generate_path(tile from, tile to, std::queue<tile> &path);

	tile origin;
	i32 m_width = 16;
	i32 m_height = 16;
};
