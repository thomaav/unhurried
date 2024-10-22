#include <deque>
#include <float.h>
#include <map>

#include "raylib.h"

#include "debug.h"
#include "map.h"
#include "types.h"

void map::draw(Camera3D &camera)
{
	BeginMode3D(camera);
	for (i32 y = 0; y < m_height; ++y)
	{
		for (i32 x = 0; x < m_width; ++x)
		{
			draw_tile(x, y);
		}
	}
	EndMode3D();
}

void map::set_recommended_camera(Camera3D &camera)
{
	/* Find recommended position. */
	float x = (float)m_width / 2.0f;
	float y = (float)m_height / 2.0f;
	float z = (float)m_width / 5.0f;

	/* Set camera. */
	camera = {};
	camera.position = { .x = x, .y = y, .z = z };
	camera.target = { .x = x, .y = y, .z = 0.0f };
	camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;
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
	float straight_line_bias = cross * 0.001f;
	float goal_bias = (dx1 * dx1 + dy1 * dy1) * 0.0001f;

	return octile + straight_line_bias + goal_bias;
}

/* (TODO, thoave01): Use A* instead of Dijkstra's to produce more natural paths. */
void map::generate_path(tile from, tile to, std::deque<tile> &path)
{
	using ts = std::pair<float, tile>;
	using dir = std::pair<i32, i32>;

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

			/* If outside map, ignore. */
			if (neighbor.x < 0 || neighbor.x >= m_width || neighbor.y < 0 || neighbor.y >= m_height)
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
