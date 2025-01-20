#include <deque>
#include <float.h>
#include <map>
#include <queue>

#include "raylib.h"

#include "draw.h"
#include "map.h"
#include "types.h"

float tile_distance(tile from, tile to)
{
	const float dx = from.x - to.x;
	const float dy = from.y - to.y;
	return sqrtf(dx * dx + dy * dy);
}

float tile_euclidean_distance(tile from, tile to)
{
	return tile_distance(from, to);
}

float tile_manhattan_distance(tile from, tile to)
{
	return std::abs(from.x - to.x) + std::abs(from.y - to.y);
}

/* Amanatides and Woo, “A Fast Voxel Traversal Algorithm For Ray Tracing”. */
std::vector<tile> grid_traversal(tile from, tile to)
{
	std::vector<tile> tiles = {};

	const float x1 = from.x + 0.5f;
	const float y1 = from.y + 0.5f;
	const float x2 = to.x + 0.5f;
	const float y2 = to.y + 0.5f;

	const float dx = x2 - x1;
	const float dy = y2 - y1;

	const i32 step_x = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
	const i32 step_y = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

	/* Line is not straight, so use Amanatides and Woo. */
	float t_max_x = (dx > 0) ? (std::ceil(x1) - x1) / dx : (x1 - std::floor(x1)) / -dx;
	float t_max_y = (dy > 0) ? (std::ceil(y1) - y1) / dy : (y1 - std::floor(y1)) / -dy;

	float t_delta_x = std::abs(1.0f / dx);
	float t_delta_y = std::abs(1.0f / dy);

	/* Handle straight lines and points. */
	if (dx == 0.0f && dy == 0.0f)
	{
		return tiles;
	}
	if (dx == 0.0f)
	{
		t_max_x = std::numeric_limits<float>::infinity();
		t_delta_x = 0.0f;
	}
	if (dy == 0.0f)
	{
		t_max_y = std::numeric_limits<float>::infinity();
		t_delta_y = 0.0f;
	}

	/* Traverse. */
	i32 x = (i32)x1;
	i32 y = (i32)y1;
	tiles.push_back({ x, y });
	while (x != (i32)x2 || y != (i32)y2)
	{
		if (t_max_x < t_max_y)
		{
			t_max_x += t_delta_x;
			x += step_x;
		}
		else
		{
			t_max_y += t_delta_y;
			y += step_y;
		}
		tiles.push_back({ x, y });
	}

	return tiles;
}

void map::draw(Camera3D &camera)
{
	BeginMode3D(camera);
	for (i32 x = 0; x < m_height; ++x)
	{
		for (i32 y = 0; y < m_width; ++y)
		{
			switch (m_tile_types[x][y])
			{
			case tile_type::OPEN:
				draw_tile(x, y, DARKGRAY);
				break;
			case tile_type::OCCUPIED:
			{
				draw_tile_wall(x, y, DARKGRAY);
				break;
			}
			}
		}
	}
	EndMode3D();
}

float octile_heuristic(const tile &start, const tile &t1, const tile &t2)
{
	float dx1 = std::abs(t1.x - t2.x);
	float dy1 = std::abs(t1.y - t2.y);
	float D1 = 1.0f;
	float D2 = 1.414f;
	float octile = D1 * (dx1 + dy1) + (D2 - 2.0f * D1) * std::min(dx1, dy1);

	/* Favor straight lines. */
	float dx2 = t1.x - t2.x;
	float dy2 = t1.y - t2.y;
	float dx3 = start.x - t2.x;
	float dy3 = start.y - t2.y;
	float cross = std::abs(dx2 * dy3 - dx3 * dy2);

	/* Larger bias will prefer straight lines more strongly. */
	const float slb = 0.000f;
	float straight_line_bias = cross * slb;
	const float gb = 0.0000f;
	float goal_bias = (dx1 * dx1 + dy1 * dy1) * gb;

	return octile + straight_line_bias + goal_bias;
}

bool map::is_open_tile(tile tile)
{
	/* If outside map, ignore. */
	if (tile.x < 0 || tile.x >= m_width || tile.y < 0 || tile.y >= m_height)
	{
		return false;
	}

	/* If occupied, ignore. */
	if (m_tile_types[tile.x][tile.y] == tile_type::OCCUPIED)
	{
		return false;
	}

	return true;
}

bool map::find_closest_open_tile(tile root, tile &closest)
{
	std::deque<tile> tiles = {};
	tiles.push_back(root);

	using dir = std::pair<i32, i32>;
	const struct
	{
		dir direction;
		float cost;
	} directions[] = {
		{ { 0, -1 }, 1.0f },    /* S */
		{ { 1, 0 }, 1.0f },     /* E */
		{ { 0, 1 }, 1.0f },     /* N */
		{ { -1, 0 }, 1.0f },    /* W */
		{ { 1, -1 }, 1.414f },  /* SE */
		{ { 1, 1 }, 1.414f },   /* NE */
		{ { -1, 1 }, 1.414f },  /* NW */
		{ { -1, -1 }, 1.414f }, /* SW */
	};

	/* BFS to find a tile. */
	while (!tiles.empty())
	{
		tile current_tile = tiles.front();
		tiles.pop_front();

		/* If found, return it. */
		if (is_open_tile(current_tile))
		{
			closest = current_tile;
			return true;
		}

		/* If not, try all neighbors. */
		for (const auto &move : directions)
		{
			tiles.push_back({ current_tile.x + move.direction.first, current_tile.y + move.direction.second });
		}
	}

	return false;
}

bool map::is_legal_move(tile from, tile to)
{
	const float distance = tile_distance(from, to);

	/* Single tile moves only. */
	if (distance > (1.414f + 0.05f))
	{
		return false;
	}

	/* If non-diagonal, it's legal. */
	if (tile_manhattan_distance(from, to) <= (1.0f + 0.05f))
	{
		return true;
	}

	/* If some tile along the diagonal path is occupied, it's illegal. */
	const i32 dx = to.x - from.x;
	const i32 dy = to.y - from.y;

	const bool t0 = is_open_tile(from);
	const bool t1 = is_open_tile({ from.x + dx, from.y });
	const bool t2 = is_open_tile({ from.x, from.y + dy });
	const bool t3 = is_open_tile(to);

	return t0 && t1 && t2 && t3;
}

void map::generate_path(tile from, tile to, std::deque<tile> &path)
{
	using ts = std::pair<float, tile>;
	using dir = std::pair<i32, i32>;

	if (m_tile_types[to.x][to.y] == tile_type::OCCUPIED)
	{
		if (!find_closest_open_tile(to, to))
		{
			/* No path to an open tile available, so give up. */
			return;
		}
	}

	/* Priority queue and lookup. */
	std::priority_queue<ts, std::vector<ts>, std::greater<ts>> q = {};
	std::map<tile, float> costs = {};
	std::map<tile, tile> previous = {};

	/* Initialize lookup to infinite distances. */
	for (i32 y = 0; y < m_height; ++y)
	{
		for (i32 x = 0; x < m_width; ++x)
		{
			costs[{ x, y }] = FLT_MAX;
		}
	}

	/* Set initial position. */
	costs[from] = 0;
	q.push({ octile_heuristic(from, from, to), from });

	/* Neighborhood. */
	const struct
	{
		dir direction;
		float cost;
	} directions[] = {
		{ { 0, -1 }, 1.0f },    /* S */
		{ { 1, 0 }, 1.0f },     /* E */
		{ { 0, 1 }, 1.0f },     /* N */
		{ { -1, 0 }, 1.0f },    /* W */
		{ { 1, -1 }, 1.414f },  /* SE */
		{ { 1, 1 }, 1.414f },   /* NE */
		{ { -1, 1 }, 1.414f },  /* NW */
		{ { -1, -1 }, 1.414f }, /* SW */
	};

	while (!q.empty())
	{
		/* Get next tile to evaluate. */
		tile current_tile = q.top().second;
		q.pop();

		/* Quit if we've arrived. */
		if (current_tile == to)
		{
			break;
		}

		/* Evaluate all directions. */
		for (const auto &move : directions)
		{
			tile neighbor = { current_tile.x + move.direction.first, current_tile.y + move.direction.second };

			if (!is_open_tile(neighbor))
			{
				continue;
			}

			if (!is_legal_move(current_tile, neighbor))
			{
				continue;
			}

			/* If we found a better path, update it. */
			float move_cost = costs[current_tile] + move.cost;
			if (move_cost < costs[neighbor])
			{
				costs[neighbor] = move_cost;
				previous[neighbor] = current_tile;

				/* f(x) = g(x) + h(x) */
				float f = move_cost + octile_heuristic(from, neighbor, to);
				q.push({ f, neighbor });
			}
		}
	}

	/* If no path found. */
	if (costs[to] == FLT_MAX)
	{
		return;
	}

	/* Backtrack. */
	std::vector<tile> reverse_path = {};
	tile current_tile = to;

	while (!(current_tile == from))
	{
		reverse_path.push_back(current_tile);
		current_tile = previous[current_tile];
	}

	for (auto it = reverse_path.rbegin(); it != reverse_path.rend(); ++it)
	{
		path.push_back(*it);
	}
}
