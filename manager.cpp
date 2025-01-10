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
#include "ui.h"

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
	entity.m_animation_current_frame = 0;
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

	/* Initialize assets. */
	m_asset_manager.load_assets();

	/* Initialize entities. */
	m_player = new entity({ 0, 0 }, m_map, m_asset_manager, *this);
	m_boss = new entity({ 8, 5 }, m_map, m_asset_manager, *this);

	m_boss->m_model_rotation = matrix_rotation_glb();
	m_asset_manager.set_animation(*m_boss, animation::BOSS);

	m_player->m_model_rotation = matrix_rotation_glb();
	m_asset_manager.set_animation(*m_player, animation::IDLE);

	/* Initialize camera. */
	m_camera.target = { m_player->m_position_render.x, m_player->m_position_render.y, 0.0f };
	m_camera.position = { m_player->m_position_render.x - 10.0f, m_player->m_position_render.y - 10.0f, 10.0f };

	/* Menu. */
	m_player_texture = LoadRenderTexture(SCREEN_WIDTH / 3.5f, SCREEN_WIDTH / 3.5f);
	m_boss_texture = LoadRenderTexture(SCREEN_WIDTH / 3.5f, SCREEN_WIDTH / 3.5f);

	m_player_idle.m_model_rotation = matrix_rotation_glb();
	m_asset_manager.set_animation(m_player_idle, animation::IDLE);
	m_player_idle.m_draw_bbox = false;

	m_boss_idle.m_model_rotation = matrix_rotation_glb();
	m_asset_manager.set_animation(m_boss_idle, animation::BOSS);
	m_boss_idle.m_draw_bbox = false;

	float x = m_player_idle.m_position_render.x;
	float y = m_player_idle.m_position_render.y;
	BoundingBox bbox = m_player_idle.m_animation_data.m_bounding_boxes[m_player_idle.m_animation_current_frame];
	float height = bbox.max.y - bbox.min.y;
	m_player_menu_camera.target = { x, y, height / 2.0f };
	m_player_menu_camera.position = { x, y - 5.0f, height / 2.0f + 1.0f };
	m_player_menu_camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	m_player_menu_camera.fovy = 45.0f;
	m_player_menu_camera.projection = CAMERA_PERSPECTIVE;

	x = m_boss_idle.m_position_render.x;
	y = m_boss_idle.m_position_render.y;
	bbox = m_boss_idle.m_animation_data.m_bounding_boxes[m_boss_idle.m_animation_current_frame];
	height = bbox.max.y - bbox.min.y;
	m_boss_menu_camera.target = { x, y, height / 2.0f };
	m_boss_menu_camera.position = { x, y - 10.0f, height / 2.0f + 1.0f };
	m_boss_menu_camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	m_boss_menu_camera.fovy = 45.0f;
	m_boss_menu_camera.projection = CAMERA_PERSPECTIVE;

	/* Playground. */
	if (nullptr != getenv("PLAYGROUND"))
	{
		init_playground();
		m_current_context = context_type::PLAYGROUND;
	}
}

void manager::set_map(map &map)
{
	m_current_map = &map;

	m_root_camera = {};
	m_root_camera.target = { m_player->m_position_render.x, m_player->m_position_render.y, 0.0f };
	m_root_camera.position = { m_player->m_position_render.x - 10.0f, m_player->m_position_render.y - 10.0f, 10.0f };
	m_root_camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	m_root_camera.fovy = 45.0f;
	m_root_camera.projection = CAMERA_PERSPECTIVE;

	m_camera = {};
	m_camera.target = { m_player->m_position_render.x, m_player->m_position_render.y, 0.0f };
	m_camera.position = { m_player->m_position_render.x - 10.0f, m_player->m_position_render.y - 10.0f, 10.0f };
	m_camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	m_camera.fovy = 45.0f;
	m_camera.projection = CAMERA_PERSPECTIVE;
}

void manager::update_camera()
{
	/* Max viewing angles. */
	const float min_angle = PI / 2.0f + PI / 8.0f;
	const float max_angle = PI - PI / 8.0f;

	/* Movement by mouse. */
	if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
	{
		constexpr float camera_sensitivity = 0.003f;
		const Vector2 mouse_delta = GetMouseDelta();
		const bool moving_up = mouse_delta.y > 0.0f;

		/* Left and right. */
		CameraYaw(&m_root_camera, -mouse_delta.x * camera_sensitivity, true);

		/* Up and down. */
		Vector3 up = GetCameraUp(&m_root_camera);
		Vector3 target = Vector3Subtract(m_root_camera.target, m_root_camera.position);
		Vector3 right = GetCameraRight(&m_root_camera);

		float angle = -mouse_delta.y * camera_sensitivity;
		float current_angle = Vector3Angle(up, target) - 0.001f;
		if ((moving_up && current_angle > max_angle) || (!moving_up && current_angle < min_angle))
		{
			angle = 0;
		}

		/* Update camera. */
		target = Vector3RotateByAxisAngle(target, right, angle);
		m_root_camera.position = Vector3Subtract(m_root_camera.target, target);
	}

	/* Movement by keys. */
	const float camera_rotation_speed = GetFrameTime() * (PI / 4.0f);
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
		/* CameraPitch does not automatically lock where we want, so do it manually. */
		Vector3 up = GetCameraUp(&m_root_camera);
		Vector3 target = Vector3Subtract(m_root_camera.target, m_root_camera.position);
		Vector3 right = GetCameraRight(&m_root_camera);

		/* Clamp view up. */
		float angle = -camera_rotation_speed;
		float current_angle = Vector3Angle(up, target) - 0.001f;
		if (current_angle > max_angle)
		{
			angle = 0;
		}

		/* Update camera. */
		target = Vector3RotateByAxisAngle(target, right, angle);
		m_root_camera.position = Vector3Subtract(m_root_camera.target, target);
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
	Vector3 camera_offset = { m_player->m_position_render.x, m_player->m_position_render.y, 0.0f };
	m_camera.target = { m_player->m_position_render.x, m_player->m_position_render.y, 0.0f };
	m_camera.position = m_root_camera.position + camera_offset;
}

/* (TODO, thoave01): Differentiate events for game and other menus/scenes. */
void manager::parse_events()
{
	if (IsKeyPressed('B'))
	{
		/* (TODO, thoave01): Make it possible to actually reset game. */
		m_current_context = context_type::ENTITY_SELECTOR;
		return;
	}

	/* (TODO, thoave01): This does not work anymore. */
	if (IsKeyPressed('R'))
	{
		m_player->m_running = !m_player->m_running;
		m_player->m_movement_tick_rate = m_player->m_running ? RUN_TICK_RATE : WALK_TICK_RATE;
		if (m_player->m_current_action == action::MOVE)
		{
			m_asset_manager.set_animation(*m_player, m_player->m_running ? animation::RUN : animation::WALK);
		}
	}

	/* (TODO, thoave01): We shouldn't return early. */
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		const Ray ray = GetScreenToWorldRay(GetMousePosition(), m_camera);

		/* First check if we hit the boss. */
		const BoundingBox bbox_ = m_boss->m_animation_data.m_bounding_boxes[m_boss->m_animation_current_frame];
		BoundingBox bbox = bbox_;
		bbox.min.x = bbox_.min.x;
		bbox.min.y = bbox_.min.z;
		bbox.min.z = bbox_.min.y;
		bbox.max.x = bbox_.max.x;
		bbox.max.y = bbox_.max.z;
		bbox.max.z = bbox_.max.y;
		bbox.min = Vector3Add(bbox.min, m_boss->m_position_render);
		bbox.max = Vector3Add(bbox.max, m_boss->m_position_render);

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
	m_player->m_target = nullptr;
	m_boss->m_tint = WHITE;

	/* Handle movement. */
	tile clicked_tile = event_data.MOVE_TILE.clicked_tile;
	m_player->set_action({ .action = action::MOVE, .MOVE.end = clicked_tile });
}

void manager::handle_click_boss_event()
{
	m_player->set_action({ .action = action::ATTACK, .ATTACK.entity = *m_boss });
	/* (TODO, thoave01): Not right. */
	m_boss->m_tint = RED;
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

		if (m_boss->m_health == 0.0f)
		{
			m_boss->m_health = 100.0f;
		}
	}

	/* Update logic. */
	m_player->tick_movement_logic();
	m_boss->tick_movement_logic();

	/* (TODO, thoave01): Some sort of behavior system. */
	if (!(m_boss->m_current_action == action::MOVE))
	{
		tile start = m_boss->m_position_logic;
		tile end = { start.x, (start.y + 3) % m_map.m_width };
		m_boss->set_action({ .action = action::MOVE, .MOVE.end = end });

		/* (TODO, thoave01): Overwrite animation. */
		if (m_boss_entity_type == animation::BOSS)
		{
			m_asset_manager.set_animation(*m_boss, animation::BOSS);
		}
	}

	/* Tick entities. */
	m_player->tick_combat();
	m_boss->tick_combat();

	/* Update rendering information. */
	m_player->tick_render();
	m_boss->tick_render();

	for (auto it = m_active_attacks.begin(); it != m_active_attacks.end();)
	{
		if (it->tick_render())
		{
			/* Trigger attack. */
			entity &source = it->m_source_entity;
			entity &target = it->m_target_entity;
			add_active_sprite_animation(sprite_type::HITSPLAT_RED, target, &m_camera);
			target.m_health = std::max(0.0f, target.m_health - source.m_attack_strength);

			/* Attack is completed; remove it. */
			it = m_active_attacks.erase(it);
		}
		else
		{
			++it;
		}
	}

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
	m_player->draw(m_camera);
	m_boss->draw(m_camera);

	for (const attack &attack : m_active_attacks)
	{
		attack.draw(m_camera);
	}

	/* Draw debug information. */
	BeginMode3D(m_camera);
	{
		draw_tile_overlay(m_player->m_position_logic.x, m_player->m_position_logic.y, YELLOW);
		draw_tile_overlay(m_player->m_target_logic.x, m_player->m_target_logic.y, RED);
		for (const auto &tile : m_player->m_path_logic)
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
		/* Boss health. */
		const ImVec2 pos = { ImGui::GetMainViewport()->GetCenter().x, 10.0f };
		const ImVec2 size = { SCREEN_WIDTH / 3.0f, 20.0f };
		const ImVec4 health_color = { 0.0f, 0.5f, 0.0f, 1.0f };
		ui_progress_bar("boss_health", m_boss->m_health / m_boss->m_max_health, pos, size, health_color);

		/* Cast and cooldown. */
		const float cast_time = m_player->m_current_attack_cast_time / m_player->m_attack_cast_time;
		const ImVec4 cast_bar_color = { 0.0f, 0.0f, 0.5f, 1.0f };
		ui_progress_bar("cast_time", cast_time, { pos.x, SCREEN_HEIGHT - 55.0f }, size, cast_bar_color);

		const float cooldown = m_player->m_current_attack_cooldown / m_player->m_attack_cooldown;
		const ImVec4 cooldown_bar_color = { 0.5f, 0.0f, 0.0f, 1.0f };
		ui_progress_bar("cooldown", 1.0f - cooldown, { pos.x, SCREEN_HEIGHT - 30.0f }, size, cooldown_bar_color);

		/* Display debug settings. */
		ImGui::SetNextWindowCollapsed(true, ImGuiCond_Appearing);
		ImGui::SetNextWindowPos({ (float)SCREEN_WIDTH, 0.0f }, ImGuiCond_Appearing, { 1.0f, 0.0f });
		ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
		{
			ImGui::SliderFloat("TURN_TICK_RATE", &TURN_TICK_RATE, 0.05f, 5.0f);
			ImGui::SliderFloat("GAME_TICK_RATE", &GAME_TICK_RATE, 0.05f, 2.4f);
			ImGui::SliderFloat("ANIMATION_TICK_RATE", &ANIMATION_TICK_RATE, 0.01f, 0.3f);
			ImGui::SliderFloat("SPRITE_ANIMATION_TICK_RATE", &SPRITE_ANIMATION_TICK_RATE, 0.05f, 0.5f);
			ImGui::SliderFloat("ATTACK_TICK_RATE", &ATTACK_TICK_RATE, 0.05f, 15.0f);
			ImGui::SliderFloat("m_player.m_movement_tick_rate", &m_player->m_movement_tick_rate, 0.05f, 1.0f);
			ImGui::SliderFloat("m_player.m_attack_cast_time", &m_player->m_attack_cast_time, 0.05f, 2.0f);
			ImGui::SliderFloat("m_player.m_attack_cooldown", &m_player->m_attack_cooldown, 0.05f, 3.0f);
			if (ImGui::Button("Reset"))
			{
				/* (TODO, thoave01): These don't necessarily mirror actual defaults. */
				TURN_TICK_RATE = 2.1f;
				GAME_TICK_RATE = 0.6f;
				ANIMATION_TICK_RATE = 0.15f;
				SPRITE_ANIMATION_TICK_RATE = 0.12f;
				ATTACK_TICK_RATE = 8.0f;
				m_player->m_movement_tick_rate = m_player->m_running ? RUN_TICK_RATE : WALK_TICK_RATE;
				m_player->m_attack_cast_time = GAME_TICK_RATE / 1.5f;
				m_player->m_attack_cooldown = GAME_TICK_RATE * 3.0f;
			}
		}
		ImGui::End();
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
			m_current_context = context_type::ENTITY_SELECTOR;
		}
	}
	EndDrawing();
}

void manager::loop_entity_selector_context()
{
	m_player_idle.tick_render();
	m_boss_idle.tick_render();

	/* Player idle texture. */
	BeginTextureMode(m_player_texture);
	BeginMode3D(m_player_menu_camera);
	{
		ClearBackground(RAYWHITE);
		m_player_idle.draw(m_player_menu_camera);
	}
	EndMode3D();
	EndTextureMode();

	/* Boss texture. */
	BeginTextureMode(m_boss_texture);
	BeginMode3D(m_boss_menu_camera);
	{
		ClearBackground(RAYWHITE);
		m_boss_idle.draw(m_boss_menu_camera);
	}
	EndMode3D();
	EndTextureMode();

	BeginDrawing();
	{
		ClearBackground(RAYWHITE);
		DrawFPS(0, 0);

		rlImGuiBegin();
		{
			/* Draw player idle texture. */
			float width = (float)m_player_texture.texture.width;
			float height = (float)m_player_texture.texture.height;
			float x = 1.0f * (SCREEN_WIDTH / 3.0f);
			float y = SCREEN_HEIGHT / 2.0f;
			bool clicked = ui_render_texture("player", { x, y }, { width, height }, m_player_texture);
			if (clicked)
			{
				m_current_context = context_type::GAME;
				m_boss_entity_type = animation::IDLE;
				init_game_context();
			}

			/* Draw boss idle texture. */
			width = (float)m_boss_texture.texture.width;
			height = (float)m_boss_texture.texture.height;
			x = 2.0f * (SCREEN_WIDTH / 3.0f);
			y = SCREEN_HEIGHT / 2.0f;
			clicked = ui_render_texture("boss", { x, y }, { width, height }, m_boss_texture);
			if (clicked)
			{
				m_current_context = context_type::GAME;
				m_boss_entity_type = animation::BOSS;
				init_game_context();
			}
		}
		rlImGuiEnd();
	}
	EndDrawing();
}

void manager::init_game_context()
{
	/* Initialize self. */
	m_game_tick = 0.0f;
	m_events.clear();
	m_active_attacks.clear();

	/* Initialize entities. */
	delete m_player;
	m_player = new entity({ 0, 0 }, m_map, m_asset_manager, *this);

	delete m_boss;
	m_boss = new entity({ 8, 5 }, m_map, m_asset_manager, *this);

	m_boss->m_model_rotation = matrix_rotation_glb();
	m_asset_manager.set_animation(*m_boss, m_boss_entity_type);

	m_player->m_model_rotation = matrix_rotation_glb();
	m_asset_manager.set_animation(*m_player, animation::IDLE);
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

void manager::init_playground()
{
	m_pg.m_model = LoadModel("assets/models/jad.gltf");
}

void manager::loop_playground()
{
	if (m_current_map != &m_map)
	{
		set_map(m_map);
	}

	/* Update. */
	tick();

	/* Draw. */
	BeginDrawing();
	{
		ClearBackground(RAYWHITE);
		DrawFPS(0, 0);

		BeginMode3D(m_camera);
		{
			/* Default is a sphere. Put playground code here. */
			int mesh_idx = 0;
			Mesh &mesh = m_pg.m_model.meshes[mesh_idx];
			Material &material = m_pg.m_model.materials[m_pg.m_model.meshMaterial[mesh_idx]];
			DrawMesh(mesh, material, MatrixScale(1.0f, 1.0f, 1.0f));
		}
		EndMode3D();
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
		case context_type::ENTITY_SELECTOR:
			loop_entity_selector_context();
			break;
		case context_type::GAME:
			loop_game_context();
			break;
		case context_type::PLAYGROUND:
			loop_playground();
			break;
		}
	}
	CloseWindow();

	/* (TODO, thoave01): All teardown stuff. */
	rlImGuiShutdown();
	UnloadRenderTexture(m_player_texture);
	UnloadRenderTexture(m_boss_texture);
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
