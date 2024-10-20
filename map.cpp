#include "third_party/raylib.h"

#include "debug.h"
#include "map.h"
#include "types.h"

tile::tile(i64 x, i64 y)
    : m_p{ x, y }
{
}

void tile::draw()
{
	/* Create tile quad. */
	Vector3 vertices[] = {
		{ (float)m_p.x, (float)m_p.y, 0.0f },               //
		{ (float)m_p.x + 1.0f, (float)m_p.y, 0.0f },        //
		{ (float)m_p.x, (float)m_p.y + 1.0f, 0.0f },        //
		{ (float)m_p.x + 1.0f, (float)m_p.y + 1.0f, 0.0f }, //
	};

	/* Draw quad. */
	Color color = m_toggled ? BLUE : DARKGRAY;
	DrawTriangleStrip3D(&vertices[0], 4, color);

	/* Draw outline. */
	DrawLine3D(vertices[0], vertices[1], WHITE);
	DrawLine3D(vertices[1], vertices[3], WHITE);
	DrawLine3D(vertices[3], vertices[2], WHITE);
	DrawLine3D(vertices[2], vertices[0], WHITE);
}

tile_set::tile_set(u32 width, u32 height)
    : m_width(width)
    , m_height(height)
{
	m_tiles.resize(m_height, std::vector<tile>(m_width));
	for (u32 y = 0; y < m_height; ++y)
	{
		for (u32 x = 0; x < m_width; ++x)
		{
			m_tiles[y][x] = { x, y };
		}
	}
}

void tile_set::toggle_tile(u32 x, u32 y)
{
	m_tiles[y][x].m_toggled ^= 1;
}

void tile_set::draw(Camera3D &camera)
{
	BeginMode3D(camera);
	{
		for (u32 row = 0; row < m_height; ++row)
		{
			for (u32 col = 0; col < m_width; ++col)
			{
				m_tiles[row][col].draw();
			}
		}
	}
	EndMode3D();
}

void map::draw(Camera3D &camera)
{
	m_tile_set.draw(camera);
}

void map::set_recommended_camera(Camera3D &camera)
{
	/* Find recommended position. */
	float x = (float)m_tile_set.m_width / 2.0f;
	float y = (float)m_tile_set.m_height / 2.0f;
	float z = (float)m_tile_set.m_width / 5.0f;

	/* Set camera. */
	camera = {};
	camera.position = { .x = x, .y = y, .z = z };
	camera.target = { .x = x, .y = y, .z = 0.0f };
	camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;
}
