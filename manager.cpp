#include <algorithm>

#include "third_party/raylib/raylib.h"

#include "debug.h"
#include "manager.h"
#include "math.h"

constexpr int SCREEN_WIDTH = 1080;
constexpr int SCREEN_HEIGHT = 720;

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
	// m_camera.target = { m_player.m_position_render.x, m_player.m_position_render.y, 0.0f };
	m_camera.target = { 0.0f, 0.0f, 0.0f };

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
		const Vector3 p2 = { 0.0f, (float)m_map.m_height, 0.05f };
		const Vector3 p3 = { (float)m_map.m_width, (float)m_map.m_height, 0.05f };
		const Vector3 p4 = { (float)m_map.m_width, 0.0f, 0.05f };
		const Ray intersection_ray = GetScreenToWorldRay(GetMousePosition(), m_camera);
		const RayCollision intersection = GetRayCollisionQuad(intersection_ray, p1, p2, p3, p4);
		if (intersection.hit)
		{
			/* (TODO, thoave01): Not needed anymore? */
			/* Color target tile. */
			tile toggled_tile = { (i32)intersection.point.x, (i32)intersection.point.y };

			/* Generate path and start player movement. */
			if (m_player.m_moving)
			{
				/* Empty current paths. */
				std::deque<tile>().swap(m_player.m_path_logic);
				std::deque<tile>().swap(m_player.m_path_render);

				m_map.generate_path(m_player.m_position_logic, toggled_tile, m_player.m_path_logic);

				m_player.m_target_logic = m_player.m_path_logic.front();
				m_player.m_path_logic.pop_front();
				m_player.m_path_render.push_back(m_player.m_target_logic);
			}
			else
			{
				m_map.generate_path(m_player.m_position_logic, toggled_tile, m_player.m_path_logic);

				m_player.m_moving = true;
				m_player.m_movement_tick = MOVEMENT_TICK_RATE / 2.0f;

				m_player.m_target_logic = m_player.m_path_logic.front();
				m_player.m_path_logic.pop_front();
				m_player.m_path_render.push_back(m_player.m_target_logic);
			}
		}
	}

	/* (TODO, thoave01): Event handling probably doesn't fit in here. */
	if (IsKeyPressed('Z'))
	{
		if (m_player.m_moving)
		{
		}
		else
		{
			m_map.generate_path({ 0, 0 }, { 2, 4 }, m_player.m_path_logic);

			m_player.m_moving = true;
			m_player.m_movement_tick = MOVEMENT_TICK_RATE / 2.0f;

			m_player.m_target_logic = m_player.m_path_logic.front();
			m_player.m_path_logic.pop_front();
			m_player.m_path_render.push_back(m_player.m_target_logic);
		}
	}

	/* (TODO, thoave01): Event handling probably doesn't fit in here. */
	if (IsKeyPressed('R'))
	{
		m_player.m_position_logic = { 0, 0 };
		m_player.m_position_render = { 0.0f, 0.0f };

		m_player.m_target_logic = {};
		m_player.m_path_logic = {};
		m_player.m_moving = false;

		m_player.m_target_render = {};
		m_player.m_path_render = {};
	}

	/* Update logic. */
	m_player.tick_logic();

	/* Update rendering information. */
	m_player.tick_render();
}

void manager::draw()
{
	/* Draw actual frame. */
	ClearBackground(RAYWHITE);
	m_map.draw(m_camera);
	m_player.draw(m_camera);

	/* Draw debug information. */
	BeginMode3D(m_camera);
	{
		draw_tile_overlay(m_player.m_position_logic.x, m_player.m_position_logic.y, YELLOW);
		draw_tile_overlay(m_player.m_target_logic.x, m_player.m_target_logic.y, GREEN);
		for (const auto &tile : m_player.m_path_logic)
		{
			draw_tile_overlay(tile.x, tile.y, RED);
		}
	}
	EndMode3D();
}

void manager::loop()
{
	while (!WindowShouldClose())
	{
		if (m_current_map != &m_map)
		{
			set_map(m_map);
		}

		/* Update world. */
		update_camera();
		tick();

		/* (TODO, thoave01): What has to be inside/outside BeginDrawing? */
		BeginDrawing();
		{
			/* Render frame. */
			draw();

			/* Finalize frame. */
			DrawFPS(0, 0);
		}
		EndDrawing();
	}
	CloseWindow();
}
