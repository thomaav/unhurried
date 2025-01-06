#include <algorithm>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "imgui.h"
#include "raygui.h"
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlImGui.h"
#pragma clang diagnostic pop

#include "animation.h"
#include "draw.h"
#include "manager.h"
#include "math.h"

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

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

	/* (TODO, thoave01): Durations per frame? Something better... */
	m_hitsplat_red.add_sprite("assets/sprites/hitsplat_red.png");
	m_hitsplat_red.add_sprite("assets/sprites/hitsplat_red.png");
	m_hitsplat_red.add_sprite("assets/sprites/hitsplat_red.png");
	m_hitsplat_red.add_sprite("assets/sprites/hitsplat_red.png");
	m_hitsplat_red.add_sprite("assets/sprites/hitsplat_red.png");
	m_hitsplat_red.add_sprite("assets/sprites/hitsplat_red.png");

	m_hitsplat_blue.add_sprite("assets/sprites/hitsplat_blue.png");
	m_hitsplat_blue.add_sprite("assets/sprites/hitsplat_blue.png");
	m_hitsplat_blue.add_sprite("assets/sprites/hitsplat_blue.png");
	m_hitsplat_blue.add_sprite("assets/sprites/hitsplat_blue.png");
	m_hitsplat_blue.add_sprite("assets/sprites/hitsplat_blue.png");
	m_hitsplat_blue.add_sprite("assets/sprites/hitsplat_blue.png");

	/* (TODO, thoave01): for-loop? Are we loading everything? */
	load_animation(animation::IDLE);
	load_animation(animation::WALK);
	load_animation(animation::RUN);
	load_animation(animation::ATTACK);
	load_animation(animation::BOSS);
}

void asset_manager::load_animation(animation animation)
{
	if (m_animations.find(animation) == m_animations.end())
	{
		m_animations[animation] = get_animation(animation);
	}
}

void asset_manager::set_animation(entity &entity, animation animation)
{
	/* Look up in cache. */
	if (m_animations.find(animation) != m_animations.end())
	{
		entity.m_animation_data = m_animations[animation];
		return;
	}

	/* If not found, load the animation. */
	entity.m_animation_data = get_animation(animation);
	m_animations[animation] = entity.m_animation_data;
}

sprite_animation &asset_manager::get_sprite_animation(sprite_type sprite)
{
	switch (sprite)
	{
	case sprite_type::CLICK_YELLOW:
		return m_click_yellow;
	case sprite_type::CLICK_RED:
		return m_click_red;
	case sprite_type::HITSPLAT_RED:
		return m_hitsplat_red;
	case sprite_type::HITSPLAT_BLUE:
		return m_hitsplat_blue;
	}
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
	unsigned int flags = FLAG_MSAA_4X_HINT;
#if !defined(__EMSCRIPTEN__) && !defined(__wasm__)
	flags |= FLAG_FULLSCREEN_MODE;
	SCREEN_WIDTH = 0;
	SCREEN_HEIGHT = 0;
#else
	SCREEN_WIDTH = 1080;
	SCREEN_HEIGHT = 720;
#endif

	SetConfigFlags(flags);
	SetTraceLogLevel(LOG_WARNING);

	/* (TODO, thoave01): Add playground thing. */
	/* (TODO, thoave01): Add blinking for FPS drops. */

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Unhurried");
	SetWindowPosition(0, 0);
	rlImGuiSetup(true);

	/* (TODO, thoave01): I don't really understand why this is necessary. */
	if (IsWindowFullscreen())
	{
		SCREEN_WIDTH = GetScreenWidth();
		SCREEN_HEIGHT = GetScreenHeight();
	}

	/* Initialize ImGui. */
	rlImGuiSetup(true);
	ImGuiStyle &style = ImGui::GetStyle();
	style.Colors[ImGuiCol_PlotHistogram] = { 0.0f, 0.5f, 0.0f, 1.0f };
	style.Colors[ImGuiCol_WindowBg] = { 1.0f, 0.0f, 0.0f, 1.0f };

	/* Initialize assets. */
	m_asset_manager.load_assets();

	/* Initialize entities. */
	m_boss.m_model_rotation = matrix_rotation_glb();
	m_asset_manager.set_animation(m_boss, animation::BOSS);

	m_player.m_model_rotation = matrix_rotation_glb();
	m_asset_manager.set_animation(m_player, animation::IDLE);

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
	/* (TODO, thoave01): This does not work anymore. */
	if (IsKeyPressed('R'))
	{
		m_player.m_running = !m_player.m_running;
		m_player.m_movement_tick_rate = m_player.m_running ? RUN_TICK_RATE : WALK_TICK_RATE;
		if (m_player.m_current_action == action::MOVE)
		{
			m_asset_manager.set_animation(m_player, m_player.m_running ? animation::RUN : animation::WALK);
		}
	}

	/* (TODO, thoave01): We shouldn't return early. */
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		const Ray ray = GetScreenToWorldRay(GetMousePosition(), m_camera);

		/* First check if we hit the boss. */
		const BoundingBox bbox_ = m_boss.m_animation_data.m_bounding_boxes[m_boss.m_animation_current_frame];
		BoundingBox bbox = bbox_;
		bbox.min.x = bbox_.min.x;
		bbox.min.y = bbox_.min.z;
		bbox.min.z = bbox_.min.y;
		bbox.max.x = bbox_.max.x;
		bbox.max.y = bbox_.max.z;
		bbox.max.z = bbox_.max.y;
		bbox.min = Vector3Add(bbox.min, m_boss.m_position_render);
		bbox.max = Vector3Add(bbox.max, m_boss.m_position_render);

		const RayCollision bbox_intersection = GetRayCollisionBox(ray, bbox);
		if (bbox_intersection.hit)
		{
			/* Push event. */
			event_data event_data = { .event = event::CLICK_BOSS };
			m_events.push_back(event_data);

			/* Push sprites. */
			/* (TODO, thoave01): Handle duplicates. */
			add_active_sprite_animation(sprite_type::CLICK_RED, GetMousePosition());

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
			/* Push event. */
			tile clicked_tile = { (i32)tile_intersection.point.x, (i32)tile_intersection.point.y };
			event_data event_data = { .event = event::MOVE_TILE, .MOVE_TILE = { clicked_tile } };
			m_events.push_back(event_data);

			/* Push sprite. */
			add_active_sprite_animation(sprite_type::CLICK_YELLOW, GetMousePosition());

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
	m_player.set_action({ .action = action::MOVE, .MOVE.end = clicked_tile });
}

void manager::handle_click_boss_event()
{
	m_player.set_action({ .action = action::ATTACK, .ATTACK.entity = m_boss });
	/* (TODO, thoave01): Not right. */
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

		/* Handle events. */
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

		/* Tick entities. */
		m_player.tick_game_logic();
		m_boss.tick_game_logic();

		if (m_boss.m_health == 0.0f)
		{
			m_boss.m_health = 100.0f;
		}
	}

	/* Update logic. */
	m_player.tick_movement_logic();
	m_boss.tick_movement_logic();

	/* (TODO, thoave01): Some sort of behavior system. */
	if (!(m_boss.m_current_action == action::MOVE))
	{
		tile start = m_boss.m_position_logic;
		tile end = { start.x, (start.y + 3) % m_map.m_width };
		m_boss.set_action({ .action = action::MOVE, .MOVE.end = end });
	}

	/* Update rendering information. */
	m_player.tick_render();
	m_boss.tick_render();

	/* Tick sprites. */
	for (auto it = m_active_sprite_animations.begin(); it != m_active_sprite_animations.end();)
	{
		bool complete = it->tick();
		if (complete)
		{
			it = m_active_sprite_animations.erase(it);
		}
		else
		{
			++it;
		}
	}
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

	/* Draw active sprites. */
	for (const auto &active_sprite_animation : m_active_sprite_animations)
	{
		active_sprite_animation.draw();
	}

	/* Display combat statistics. */
	rlImGuiBegin();
	{
		/* How window should behave and look. */
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
		const ImGuiWindowFlags flags =          //
		    ImGuiWindowFlags_NoTitleBar |       //
		    ImGuiWindowFlags_AlwaysAutoResize | //
		    ImGuiWindowFlags_NoMove |           //
		    ImGuiWindowFlags_NoScrollbar;

		/* Where to put window. */
		ImVec2 pos = ImGui::GetMainViewport()->GetCenter();
		pos.y = 10.0f;
		ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing, { 0.5f, 0.0f });
		ImGui::SetNextWindowSize({ SCREEN_WIDTH / 3.0f, 20.0f });

		/* Draw. */
		if (ImGui::Begin("Combat", nullptr, flags))
		{
			ImGui::ProgressBar(m_boss.m_health / m_boss.m_max_health, { -0.01f, -0.01f });
		}
		ImGui::End();

		ImGui::PopStyleVar(4);
	}
	rlImGuiEnd();
}

void manager::loop_menu_context()
{
	BeginDrawing();
	{
		ClearBackground(RAYWHITE);
		DrawFPS(0, 0);
		m_menu.draw();
		if (m_menu.m_closed)
		{
			m_current_context = context_type::GAME;
		}
	}
	EndDrawing();
}

void manager::loop_game_context()
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

void manager::loop()
{
	while (!WindowShouldClose())
	{
		switch (m_current_context)
		{
		case context_type::MENU:
			loop_menu_context();
			break;
		case context_type::GAME:
			loop_game_context();
			break;
		}
	}
	CloseWindow();
	rlImGuiShutdown();
}

void manager::add_active_sprite_animation(sprite_type type, Vector2 position)
{
	sprite_animation &sa = m_asset_manager.get_sprite_animation(type);
	m_active_sprite_animations.push_back({ sa, position });
}

void manager::add_active_sprite_animation(sprite_type type, Vector3 position, Camera3D *camera)
{
	sprite_animation &sa = m_asset_manager.get_sprite_animation(type);
	m_active_sprite_animations.push_back({ sa, position, camera });
}

void manager::add_active_sprite_animation(sprite_type type, entity &entity, Camera3D *camera)
{
	sprite_animation &sa = m_asset_manager.get_sprite_animation(type);
	m_active_sprite_animations.push_back({ sa, &entity, camera });
}
