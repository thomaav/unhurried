#include <algorithm>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlImGui.h"
#pragma clang diagnostic pop

#include "animation.h"
#include "draw.h"
#include "manager.h"
#include "math.h"

constexpr int SCREEN_WIDTH = 1080;
constexpr int SCREEN_HEIGHT = 720;

void asset_manager::load_assets()
{
	m_click_yellow.add_sprite("assets/sprites/yellow_0.png");
	m_click_yellow.add_sprite("assets/sprites/yellow_1.png");
	m_click_yellow.add_sprite("assets/sprites/yellow_2.png");
	m_click_yellow.add_sprite("assets/sprites/yellow_3.png");

	m_click_red.add_sprite("assets/sprites/red_0.png");
	m_click_red.add_sprite("assets/sprites/red_1.png");
	m_click_red.add_sprite("assets/sprites/red_2.png");
	m_click_red.add_sprite("assets/sprites/red_3.png");
}

void manager::run()
{
	init();
	loop();
}

static Matrix matrix_rotation_glb()
{
	/* GLB is +Y up, +Z forward, -X right. We're +Z up. */
	return MatrixRotateX(90.0f * DEG2RAD);
}

void manager::init()
{
	/* Initialize graphics. */
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib");
	SetTargetFPS(144);
	rlImGuiSetup(true);

	/* Initialize assets. */
	m_asset_manager.load_assets();

	/* Initialize entities. */
	m_boss.m_model_rotation = matrix_rotation_glb();
	m_boss.set_animation(animation::BOSS);
	m_boss.m_idle_animation = animation::BOSS;

	m_player.m_model_rotation = matrix_rotation_glb();
	m_player.set_animation(animation::IDLE);
	m_player.m_idle_animation = animation::IDLE;

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
	/* Movement by mouse. */
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
	{
		UpdateCamera(&m_root_camera, CAMERA_THIRD_PERSON);
	}

	/* Movement by keys. */
	const float camera_rotation_speed = GetFrameTime() * (PI / 4.0f);
	const float min_angle = PI / 2.0f + PI / 8.0f;
	if (IsKeyDown(KEY_DOWN) || IsKeyDown('S'))
	{
		/* CameraPitch does not automatically lock where we want, so do it manually. */
		Vector3 up = GetCameraUp(&m_root_camera);
		Vector3 target = Vector3Subtract(m_root_camera.target, m_root_camera.position);
		Vector3 right = GetCameraRight(&m_root_camera);

		/* Clamp view down. */
		float angle = camera_rotation_speed;
		float current_angle = Vector3Angle(up, target) - 0.001f;
		if (current_angle < min_angle)
		{
			angle = 0;
		}

		/* Update camera. */
		target = Vector3RotateByAxisAngle(target, right, angle);
		m_root_camera.position = Vector3Subtract(m_root_camera.target, target);
	}
	if (IsKeyDown(KEY_UP) || IsKeyDown('W'))
	{
		/* CameraPitch automatically locks up. */
		CameraPitch(&m_root_camera, -camera_rotation_speed, true, true, false);
	}
	if (IsKeyDown(KEY_RIGHT) || IsKeyDown('D'))
	{
		CameraYaw(&m_root_camera, camera_rotation_speed, true);
	}
	if (IsKeyDown(KEY_LEFT) || IsKeyDown('A'))
	{
		CameraYaw(&m_root_camera, -camera_rotation_speed, true);
	}

	/* Allow zooming camera. */
	CameraMoveToTarget(&m_root_camera, -GetMouseWheelMove());

	/* Follow player. */
	Vector3 camera_offset = { m_player.m_position_render.x, m_player.m_position_render.y, 0.0f };
	m_camera.target = { m_player.m_position_render.x, m_player.m_position_render.y, 0.0f };
	m_camera.position = m_root_camera.position + camera_offset;
}

void manager::parse_events()
{
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		const Ray ray = GetScreenToWorldRay(GetMousePosition(), m_camera);
		const float x = m_boss.m_position_render.x;
		const float y = m_boss.m_position_render.y;
		const float z = 0.0f;

		/* First check if we hit the boss. */
		Mesh &mesh = m_boss.m_animation_data.m_model.meshes[m_boss.m_animation_current_frame];
		Matrix mesh_transform = MatrixMultiply(m_boss.m_model_rotation, MatrixTranslate(x, y, z));
		const RayCollision mesh_intersection = GetRayCollisionMesh(ray, mesh, mesh_transform);
		if (mesh_intersection.hit)
		{
			/* (TODO, thoave01): Click API?. */
			m_clicked = true;
			m_click_frame = 0;
			m_click_color = RED;

			event_data event_data = { .event = event::CLICK_BOSS };
			m_events.push_back(event_data);
			return;
		}

		/* If we hit nothing, check if we hit the map. */
		const Vector3 p1 = { 0.0f, 0.0f, 0.05f };
		const Vector3 p2 = { 0.0f, (float)m_map.m_height, 0.05f };
		const Vector3 p3 = { (float)m_map.m_width, (float)m_map.m_height, 0.05f };
		const Vector3 p4 = { (float)m_map.m_width, 0.0f, 0.05f };
		const RayCollision tile_intersection = GetRayCollisionQuad(ray, p1, p2, p3, p4);
		if (tile_intersection.hit)
		{
			m_clicked = true;
			m_click_frame = 0;
			m_click_color = YELLOW;

			tile clicked_tile = { (i32)tile_intersection.point.x, (i32)tile_intersection.point.y };
			event_data event_data = { .event = event::MOVE_TILE, .MOVE_TILE = { clicked_tile } };
			m_events.push_back(event_data);
			return;
		}
	}
}

void manager::handle_move_tile_event(event_data &event_data)
{
	/* (TODO, thoave01): Undo other actions. Improve this. */
	m_player.m_target = nullptr;
	m_boss.m_tint = WHITE;

	/* Handle movement. */
	tile clicked_tile = event_data.MOVE_TILE.clicked_tile;
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

		/* (TODO, thoave01): Figure out how and when to load animations. */
		m_player.set_animation(animation::WALK);
	}
}

void manager::handle_click_boss_event()
{
	m_player.m_target = &m_boss;
	m_player.stop_moving();
	m_boss.m_tint = RED;
}

void manager::tick()
{
	/* Handle input etc. */
	parse_events();

	/* Update camera. */
	update_camera();

	/* Tick the actual game. */
	m_game_tick += GetFrameTime();
	while (m_game_tick > GAME_TICK_RATE)
	{
		m_game_tick -= GAME_TICK_RATE;
		while (!m_events.empty())
		{
			event_data event_data = m_events.front();
			m_events.pop_front();
			switch (event_data.event)
			{
			case event::MOVE_TILE:
			{
				handle_move_tile_event(event_data);
				break;
			}
			case event::CLICK_BOSS:
				handle_click_boss_event();
				break;
			case event::NONE:
			{
				assert(false);
				break;
			}
			}
		}
	}

	/* Update logic. */
	m_player.tick_logic();
	m_boss.tick_logic();

	/* Update rendering information. */
	m_player.tick_render();
	m_boss.tick_render();
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

	/* Draw click. */
	m_click_tick += GetFrameTime();
	while (m_click_tick > CLICK_TICK_RATE)
	{
		m_click_tick -= CLICK_TICK_RATE;
		++m_click_frame;
		/* (TODO, thoave01): Base this properly on sprites in the click sprite animations. */
		if (m_click_frame == m_click_frame_count)
		{
			m_clicked = false;
		}
	}
	if (m_clicked)
	{
		if (m_click_color == YELLOW)
		{
			m_asset_manager.m_click_yellow.draw(m_click_frame);
		}
		else if (m_click_color == RED)
		{
			m_asset_manager.m_click_red.draw(m_click_frame);
		}
		else
		{
			assert(false);
		}
	}
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
		tick();

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
