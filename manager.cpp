#include <algorithm>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlImGui.h"
#pragma clang diagnostic pop

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
	/* Initialize graphics. */
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib");
	SetTargetFPS(144);
	rlImGuiSetup(true);

	/* Initialize entities. */
	m_player.m_color_render = BLACK;
	m_player.load_model("assets/models/player.glb");
	m_boss.m_color_render = GRAY;

	/* Initialize camera. */
	m_camera.target = { m_player.m_position_render.x, m_player.m_position_render.y, 0.0f };
	m_camera.position = { m_player.m_position_render.x - 10.0f, m_player.m_position_render.y - 10.0f, 10.0f };
}

void manager::set_map(map &map)
{
	m_current_map = &map;

	m_root_camera = {};
	m_root_camera.target = { m_player.m_position_render.x, m_player.m_position_render.y, 0.0f };
	m_root_camera.position = { m_player.m_position_render.x - 10.0f, m_player.m_position_render.y - 10.0f, 10.0f };
	m_root_camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	m_root_camera.fovy = 45.0f;
	m_root_camera.projection = CAMERA_PERSPECTIVE;

	m_camera = {};
	m_camera.target = { m_player.m_position_render.x, m_player.m_position_render.y, 0.0f };
	m_camera.position = { m_player.m_position_render.x - 10.0f, m_player.m_position_render.y - 10.0f, 10.0f };
	m_camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	m_camera.fovy = 45.0f;
	m_camera.projection = CAMERA_PERSPECTIVE;
}

void manager::update_camera()
{
	/* Update camera around player. */
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		/* (TODO, thoave01): Clamp z-value. */
		UpdateCamera(&m_root_camera, CAMERA_THIRD_PERSON);
	}

	/* Allow zooming camera. */
	CameraMoveToTarget(&m_root_camera, -GetMouseWheelMove());

	/* Follow player. */
	Vector3 camera_offset = { m_player.m_position_render.x, m_player.m_position_render.y, 0.0f };
	m_camera.target = { m_player.m_position_render.x, m_player.m_position_render.y, 0.0f };
	m_camera.position = m_root_camera.position + camera_offset;
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
			tile clicked_tile = { (i32)intersection.point.x, (i32)intersection.point.y };

			/* Generate path and start player movement. */
			if (clicked_tile == m_player.m_position_logic)
			{
				/* Do nothing. */
			}
			else if (m_player.m_moving)
			{
				/* Empty current paths. */
				std::deque<tile>().swap(m_player.m_path_logic);
				std::deque<tile>().swap(m_player.m_path_render);

				m_map.generate_path(m_player.m_position_logic, clicked_tile, m_player.m_path_logic);

				m_player.m_target_logic = m_player.m_path_logic.front();
				m_player.m_path_logic.pop_front();
				m_player.m_path_render.push_back(m_player.m_target_logic);
			}
			else
			{
				m_map.generate_path(m_player.m_position_logic, clicked_tile, m_player.m_path_logic);

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
	m_boss.draw(m_camera);

	/* Draw debug information. */
	BeginMode3D(m_camera);
	{
		draw_tile_overlay(m_player.m_position_logic.x, m_player.m_position_logic.y, YELLOW);
		draw_tile_overlay(m_player.m_target_logic.x, m_player.m_target_logic.y, RED);
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
