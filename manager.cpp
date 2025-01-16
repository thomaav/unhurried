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

	/* Prime the model cache by loading a model. */
	m_player->m_model.load(model_id::PLAYER);

	/* Initialize camera. */
	m_camera.target = { m_player->m_position_render.x, m_player->m_position_render.y, 0.0f };
	m_camera.position = { m_player->m_position_render.x - 10.0f, m_player->m_position_render.y - 10.0f, 10.0f };

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
		m_current_context = context_type::ENTITY_SELECTOR;
		return;
	}

	if (IsKeyPressed('R'))
	{
		m_player->m_running = !m_player->m_running;
		m_player->m_movement_tick_rate = m_player->m_running ? RUN_TICK_RATE : WALK_TICK_RATE;
		if (m_player->m_current_action == action::MOVE)
		{
			m_player->m_model.set_active_animation(m_player->m_running ? animation_id::PLAYER_RUN :
			                                                             animation_id::PLAYER_WALK);
		}
	}

	if (IsKeyPressed('X'))
	{
		m_player->m_path_logic.clear();
	}

	if (IsKeyPressed('C'))
	{
		m_attack_select_active = !m_attack_select_active;
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		bool found_hit = false;
		const Ray ray = GetScreenToWorldRay(GetMousePosition(), m_camera);

		/* First check if we hit the boss. */
		BoundingBox bbox = m_boss->get_active_bounding_box();
		const RayCollision bbox_intersection = GetRayCollisionBox(ray, bbox);
		if (!found_hit && bbox_intersection.hit)
		{
			found_hit = true;

			/* Set action. */
			if (!(m_player->m_current_action == action::ATTACK))
			{
				m_player->set_action({ .action = action::ATTACK, .ATTACK.entity = *m_boss });
			}

			/* Push sprites. */
			/* (TODO, thoave01): Handle duplicates. */
			add_active_sprite_animation(sprite_type::CLICK_RED, GetMousePosition());
		}

		/* If we hit nothing, check if we hit the map. */
		const Vector3 p1 = { 0.0f, 0.0f, MAP_HEIGHT };
		const Vector3 p2 = { 0.0f, (float)m_map.m_height, MAP_HEIGHT };
		const Vector3 p3 = { (float)m_map.m_width, (float)m_map.m_height, MAP_HEIGHT };
		const Vector3 p4 = { (float)m_map.m_width, 0.0f, MAP_HEIGHT };
		const RayCollision tile_intersection = GetRayCollisionQuad(ray, p1, p2, p3, p4);
		if (!found_hit && tile_intersection.hit)
		{
			found_hit = true;

			/* Push event. */
			tile clicked_tile = { (i32)tile_intersection.point.x, (i32)tile_intersection.point.y };
			m_player->m_target = nullptr;
			m_player->set_action({ .action = action::MOVE, .MOVE.end = clicked_tile });

			/* Push sprite. */
			add_active_sprite_animation(sprite_type::CLICK_YELLOW, GetMousePosition());
		}
	}
}

void manager::tick_attacks()
{
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
}

void manager::tick_sprites()
{
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

void manager::tick()
{
	/* Handle user input. */
	parse_events();

	/* Update camera. */
	update_camera();

	/* (TODO, thoave01): Combine all logic. */
	/* Update logic. */
	m_player->tick_movement_logic();
	m_boss->tick_movement_logic();

	/* Update combat logic. */
	m_player->tick_combat();
	m_boss->tick_combat();
	tick_attacks();

	/* Update rendering information. */
	m_player->tick_render();
	m_boss->tick_render();

	/* Sprites. */
	tick_sprites();
}

void manager::draw()
{
	/* Draw actual frame. */
	ClearBackground(RAYWHITE);
	m_map.draw(m_camera);
	m_player->draw(m_camera);
	m_boss->draw(m_camera);

	/* Draw attacks. */
	for (attack &attack : m_active_attacks)
	{
		attack.draw(m_camera);
	}

	/* Draw attack range. */
	if (m_attack_select_active)
	{
		BeginMode3D(m_camera);
		{
			const i32 range = (i32)m_player->m_current_attack_range;
			const tile player_tile = m_player->m_position_logic;
			for (i32 y = -range; y < range; ++y)
			{
				for (i32 x = -range; x < range; ++x)
				{
					if (sqrtf(x * x + y * y) < range)
					{
						const tile overlay_tile = { player_tile.x + x, player_tile.y + y };
						draw_tile_overlay(overlay_tile.x, overlay_tile.y, { 41, 41, 55, 127 });
					}
				}
			}
		}
		EndMode3D();
	}

	/* Draw debug information. */
	BeginMode3D(m_camera);
	{
		draw_tile_overlay(m_player->m_position_logic.x, m_player->m_position_logic.y, YELLOW);
		for (const auto &tile : m_player->m_path_logic)
		{
			draw_tile_overlay(tile.x, tile.y, RED);
		}
		draw_tile_overlay(m_player->m_target_logic.x, m_player->m_target_logic.y, BLUE);
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
		ui_progress_bar("cooldown", cooldown, { pos.x, SCREEN_HEIGHT - 30.0f }, size, cooldown_bar_color);

		/* Display debug settings. */
		ImGui::SetNextWindowCollapsed(true, ImGuiCond_Appearing);
		ImGui::SetNextWindowPos({ (float)SCREEN_WIDTH, 0.0f }, ImGuiCond_Appearing, { 1.0f, 0.0f });
		ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
		{
			ImGui::SliderFloat("TURN_TICK_RATE", &TURN_TICK_RATE, 0.05f, 5.0f);
			ImGui::SliderFloat("GAME_TICK_RATE", &GAME_TICK_RATE, 0.05f, 2.4f);
			ImGui::SliderFloat("SPRITE_ANIMATION_TICK_RATE", &SPRITE_ANIMATION_TICK_RATE, 0.05f, 0.5f);
			ImGui::SliderFloat("ATTACK_TICK_RATE", &ATTACK_TICK_RATE, 0.05f, 15.0f);
			ImGui::SliderFloat("m_player.m_movement_tick_rate", &m_player->m_movement_tick_rate, 0.05f, 1.0f);
			ImGui::SliderFloat("m_player.m_attack_cast_time", &m_player->m_attack_cast_time, 0.05f, 2.0f);
			ImGui::SliderFloat("m_player.m_attack_cooldown", &m_player->m_attack_cooldown, 0.05f, 2.0f);
			if (ImGui::Button("Reset"))
			{
				/* (TODO, thoave01): These don't necessarily mirror actual defaults. */
				TURN_TICK_RATE = 2.1f;
				GAME_TICK_RATE = 0.6f;
				SPRITE_ANIMATION_TICK_RATE = 0.12f;
				ATTACK_TICK_RATE = 8.0f;
				m_player->m_movement_tick_rate = m_player->m_running ? RUN_TICK_RATE : WALK_TICK_RATE;
				m_player->m_attack_cast_time = 1.02f;
				m_player->m_attack_cooldown = GAME_TICK_RATE * 1.25f;
			}
		}
		ImGui::End();

		/* Display debug statistics. */
#if 0
		ImGui::SetNextWindowCollapsed(true, ImGuiCond_Appearing);
#endif
		ImGui::SetNextWindowPos({ 0.0f, 0.0f }, ImGuiCond_Appearing, { 0.0f, 0.0f });
		ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
		{
			m_frame_times.push_back(GetFrameTime());
			if (m_frame_times.size() > 100)
			{
				m_frame_times.erase(m_frame_times.begin());
			}
			auto [min, max] = std::minmax_element(m_frame_times.begin(), m_frame_times.end());
			ImGui::PlotLines("##frame_time", m_frame_times.data(), m_frame_times.size(), 0, "Frame time", *min,
			                 0.016f * 2.0f, ImVec2(0, 100));
			ImGui::Text("FPS %d", GetFPS());
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
		m_menu.draw();
		if (m_menu.m_closed)
		{
			m_current_context = context_type::ENTITY_SELECTOR;
			init_entity_selector_context();
		}
	}
	EndDrawing();
}

void manager::init_entity_selector_context()
{
	m_player_texture = LoadRenderTexture(SCREEN_WIDTH / 3.5f, SCREEN_WIDTH / 3.5f);
	m_boss_texture = LoadRenderTexture(SCREEN_WIDTH / 3.5f, SCREEN_WIDTH / 3.5f);

	m_player_idle.m_model_rotation = matrix_rotation_glb();
	m_player_idle.m_model.load(model_id::PLAYER);
	m_player_idle.m_model.set_active_animation(animation_id::PLAYER_IDLE);
	m_player_idle.m_draw_bbox = false;

	m_boss_idle.m_model_rotation = matrix_rotation_glb();
	m_boss_idle.m_model.load(model_id::BOSS);
	m_boss_idle.m_model.set_active_animation(animation_id::BOSS_IDLE);
	m_boss_idle.m_draw_bbox = false;

	float x = m_player_idle.m_position_render.x;
	float y = m_player_idle.m_position_render.y;
	BoundingBox bbox = m_player_idle.get_active_bounding_box();
	float height = bbox.max.z - bbox.min.z;
	m_player_menu_camera.target = { x, y, height / 2.0f };
	m_player_menu_camera.position = { x, y - 5.0f, height / 2.0f + 1.0f };
	m_player_menu_camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	m_player_menu_camera.fovy = 45.0f;
	m_player_menu_camera.projection = CAMERA_PERSPECTIVE;

	x = m_boss_idle.m_position_render.x;
	y = m_boss_idle.m_position_render.y;
	bbox = m_boss_idle.get_active_bounding_box();
	height = bbox.max.z - bbox.min.z;
	m_boss_menu_camera.target = { x, y, height / 2.0f };
	m_boss_menu_camera.position = { x, y - 10.0f, height / 2.0f + 1.0f };
	m_boss_menu_camera.up = { .x = 0.0f, .y = 0.0f, .z = 1.0f };
	m_boss_menu_camera.fovy = 45.0f;
	m_boss_menu_camera.projection = CAMERA_PERSPECTIVE;
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
				m_boss_model_id = model_id::PLAYER;
				init_game_context();

				UnloadRenderTexture(m_player_texture);
				UnloadRenderTexture(m_boss_texture);
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
				m_boss_model_id = model_id::BOSS;
				init_game_context();

				UnloadRenderTexture(m_player_texture);
				UnloadRenderTexture(m_boss_texture);
			}
		}
		rlImGuiEnd();
	}
	EndDrawing();
}

void manager::init_game_context()
{
	/* Initialize self. */
	m_active_attacks.clear();

	/* Initialize entities. */
	delete m_player;
	m_player = new entity({ 0, 0 }, m_map, m_asset_manager, *this);

	delete m_boss;
	m_boss = new entity({ 8, 5 }, m_map, m_asset_manager, *this);

	m_boss->m_model_rotation = matrix_rotation_glb();
	m_boss->m_model.load(m_boss_model_id);
	animation_id boss_animation_id =
	    m_boss_model_id == model_id::BOSS ? animation_id::BOSS_IDLE : animation_id::PLAYER_WALK;
	m_boss->m_model.set_active_animation(boss_animation_id);
	m_boss->m_is_boss = true;

	m_player->m_model_rotation = matrix_rotation_glb();
	m_player->m_model.load(model_id::PLAYER);
	m_player->m_model.set_active_animation(animation_id::PLAYER_IDLE);
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
	}
	EndDrawing();
}

void manager::init_playground()
{
	m_pg_model.load(model_id::PLAYER);
	m_pg_model.set_active_animation(animation_id::PLAYER_ATTACK);
}

void manager::loop_playground()
{
	if (m_current_map != &m_map)
	{
		set_map(m_map);
	}

	/* Update. */
	m_pg_model.tick_render();

	/* Draw. */
	BeginDrawing();
	{
		ClearBackground(RAYWHITE);
		BeginMode3D(m_camera);
		{
			/* Put playground code here. */
			animation &animation = *m_pg_model.get_active_animation();

			int mesh_idx = m_pg_model.m_animation_current_frame;
			Mesh &mesh = animation.m_model.meshes[mesh_idx];
			Material &material = animation.m_model.materials[animation.m_model.meshMaterial[mesh_idx]];
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
