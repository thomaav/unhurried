#include <algorithm>

#include "third_party/raylib.h"

#include "debug.h"
#include "manager.h"

void manager::run()
{
	init();
	loop();
}

void manager::init()
{
	/* Initialize window. */
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib");
	SetTargetFPS(60);
}

void manager::set_map(map &map)
{
	m_current_map = &map;
	map.set_recommended_camera(m_camera);
}

void manager::update_camera()
{
	/* Follow player. */
	m_camera.target = { m_player.m_p_f.x, (float)m_player.m_p_f.y, 0.0f };

	/* Actually update camera based on input. */
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		/* (TODO, thoave01): Clamp z-value. */
		UpdateCamera(&m_camera, CAMERA_THIRD_PERSON);
	}
}

void manager::tick()
{
	/* (TODO, thoave01): Event handling probably doesn't fit in here. */
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		const Vector3 p1 = { 0.0f, 0.0f, 0.05f };
		const Vector3 p2 = { 0.0f, (float)m_map.m_tile_set.m_height, 0.05f };
		const Vector3 p3 = { (float)m_map.m_tile_set.m_width, (float)m_map.m_tile_set.m_height, 0.05f };
		const Vector3 p4 = { (float)m_map.m_tile_set.m_width, 0.0f, 0.05f };
		const Ray intersection_ray = GetScreenToWorldRay(GetMousePosition(), m_camera);
		const RayCollision intersection = GetRayCollisionQuad(intersection_ray, p1, p2, p3, p4);
		if (intersection.hit)
		{
			/* Untoggle previous tile. */
			if (m_map.m_is_toggled_tile)
			{
				m_map.m_tile_set.m_tiles[m_map.m_toggled_tile.y][m_map.m_toggled_tile.x].m_toggled = false;
			}

			m_map.m_tile_set.toggle_tile((u32)intersection.point.x, (u32)intersection.point.y);
			m_player.m_target = { (u32)intersection.point.x, (u32)intersection.point.y };

			m_map.m_is_toggled_tile = true;
			m_map.m_toggled_tile = { (u32)intersection.point.x, (u32)intersection.point.y };
		}
	}

	/* Tick all the entities. */
	m_player.tick();
}

void manager::loop()
{
	while (!WindowShouldClose())
	{
		if (m_current_map != &m_map)
		{
			set_map(m_map);
		}

		BeginDrawing();
		{
			/* Initialize frame. */
			update_camera();

			/* Tick the game. */
			tick();

			/* Render frame. */
			ClearBackground(RAYWHITE);
			m_map.draw(m_camera);
			m_player.draw(m_camera);

			/* Finalize frame. */
			DrawFPS(0, 0);
		}
		EndDrawing();
	}
	CloseWindow();
}
