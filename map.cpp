#include <map>
#include <queue>

#include "third_party/raylib.h"

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

void map::generate_path(tile from, tile to, std::queue<tile> &path)
{
	using ts = std::pair<float, tile>;

	/* Priority queue and lookup. */
	std::priority_queue<ts, std::vector<ts>, std::greater<ts>> q = {};
	std::map<tile, float> scores = {};

	path.push({ 0, 1 });
	path.push({ 0, 2 });
	path.push({ 0, 3 });
	path.push({ 1, 3 });
	path.push({ 2, 3 });
}
