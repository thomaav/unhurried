#include <algorithm>

#include "third_party/raylib.h"

#include "debug.h"
#include "manager.h"

constexpr int SCREEN_WIDTH = 1080;
constexpr int SCREEN_HEIGHT = 720;

constexpr float tick_rate = 0.6f;

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
			m_toggled_tile = { (i32)intersection.point.x, (i32)intersection.point.y };
			m_is_toggled_tile = true;

			/* (TODO, thoave01): We have to add a full path. */
			// m_player.m_target = m_toggled_tile;
		}
	}

	/* (TODO, thoave01): Event handling probably doesn't fit in here. */
	if (IsKeyPressed('Z') && !m_player.m_moving)
	{
		/* (TODO, thoave01): Add a path here. */
		m_player.m_path.push({ 0, 1 });
		m_player.m_path.push({ 0, 2 });
		m_player.m_path.push({ 0, 3 });
		m_player.m_moving = true;

		/* Allow an instant move. */
		m_player.m_position_logic = m_player.m_path.front();
		m_player.m_path.pop();
		m_player.m_target = m_player.m_path.front();
	}

	/* (TODO, thoave01): Event handling probably doesn't fit in here. */
	if (IsKeyPressed('R'))
	{
		m_player.m_position_logic = { 0, 0 };
		m_player.m_position_render = { 0.0f, 0.0f };

		m_player.m_target = {};
		m_player.m_path = {};
		m_player.m_moving = false;
	}

	static float current_tick = 0.0f;
	current_tick += GetFrameTime();

	/* Update logic. */
	while (current_tick > tick_rate)
	{
		/* (TODO, thoave01): Now we're immediately going to tick after you click. How can we solve this? */
		m_player.tick_logic();
		current_tick -= tick_rate;
	}

	/* Update rendering information. */
	m_player.tick_render(tick_rate);
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
		if (m_is_toggled_tile)
		{
			draw_tile_overlay(m_toggled_tile.x, m_toggled_tile.y, BLUE);
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
