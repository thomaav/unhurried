#include <algorithm>
#include <cassert>
#include <cmath>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#include "raymath.h"
#pragma clang diagnostic pop

#include "draw.h"
#include "entity.h"
#include "manager.h"
#include "math.h"

entity::entity(tile p, map &map, asset_manager &asset_manager, manager &manager)
    : m_map(map)
    , m_asset_manager(asset_manager)
    , m_manager(manager)
    , m_position_logic(p)
    , m_position_render({ (float)p.x + 0.5f, (float)p.y + 0.5f, 0.0f })
    , m_target_logic(p)
    , m_target_render({ (float)p.x + 0.5f, (float)p.y + 0.5f, 0.0f })
{
}

void entity::tick_game_logic()
{
	m_attack_cooldown = std::max(0.0f, m_attack_cooldown - GAME_TICK_RATE);
	if (m_current_action == action::ATTACK && m_attack_cooldown == 0.0f)
	{
		m_attack_cast_time += GAME_TICK_RATE + 0.001f;
		if (m_attack_cast_time >= m_current_attack_cast_time)
		{
			m_manager.add_active_sprite_animation(sprite_type::HITSPLAT_RED, *m_target, &m_manager.m_camera);
			m_attack_cast_time = GAME_TICK_RATE + 0.001f;
			m_attack_cooldown = m_current_attack_cooldown;
		}
	}
}

void entity::tick_movement_logic()
{
	m_movement_tick += GetFrameTime();
	while (m_movement_tick > m_movement_tick_rate)
	{
		m_movement_tick -= m_movement_tick_rate;
		m_position_logic = m_target_logic;
		if (!m_path_logic.empty())
		{
			m_target_logic = m_path_logic.front();
			m_path_logic.pop_front();
			m_path_render.push_back(m_target_logic);
		}
	}
}

void entity::tick_render()
{
	/* (TODO, thoave01): Debug to desync movement. */
	if (IsKeyDown('P'))
	{
		return;
	}

	/* Update animation. */
	m_animation_tick += GetFrameTime();
	while (m_animation_tick > ANIMATION_TICK_RATE)
	{
		m_animation_tick -= ANIMATION_TICK_RATE;
		m_animation_current_frame = (m_animation_current_frame + 1) % m_animation_data.m_model.meshCount;
	}

	if (m_current_action == action::IDLE)
	{
		return;
	}

	/* Don't move if we're close, to avoid stuttering. */
	Vector3 position = m_position_render;
	Vector3 target = m_target_render;
	if (Vector3Length(position - target) <= 0.05f)
	{
		/* Move to exact location of target. */
		m_position_render = target;

		/* Set next target tile. */
		if (!m_path_render.empty())
		{
			tile next = m_path_render.front();
			m_target_render = { (float)next.x + 0.5f, (float)next.y + 0.5f, 0.0f };
			m_path_render.pop_front();
		}
		else if (m_current_action == action::MOVE && m_path_logic.empty())
		{
			set_action({ action::IDLE, {} });
			return;
		}
	}

	/* Update render position by some increment. */
	Vector2 direction = { target.x - m_position_render.x, target.y - m_position_render.y };
	Vector2 direction_normalized = normalize(direction);

	/* (TODO, thoave01): Improve desync catch-up mechanic to sync all the to the logic position. */
	float tick_scale = m_path_render.size() > 1 ? 2.0f : 1.0f;
	float normalized_tick_rate = m_movement_tick_rate / tick_scale;
	float increment = GetFrameTime() / normalized_tick_rate;

	m_position_render.x += direction.x > 0 ? std::min(direction_normalized.x * increment, direction.x) :
	                                         std::max(direction_normalized.x * increment, direction.x);
	m_position_render.y += direction.y > 0 ? std::min(direction_normalized.y * increment, direction.y) :
	                                         std::max(direction_normalized.y * increment, direction.y);
}

static void matrix_to_rotation(Matrix m, Vector3 &axis, float &angle)
{
	Quaternion q = QuaternionFromMatrix(m);
	QuaternionToAxisAngle(q, &axis, &angle);
	angle *= RAD2DEG;
}

void entity::draw(Camera3D &camera)
{
	/* Positioning. */
	Vector3 position = { m_position_render.x, m_position_render.y, 0.0f };
	Vector3 move_target = m_target_render;

	/* Directions. */
	Vector3 eye_direction = Vector3Transform({ 0.0f, 0.0f, 1.0f }, m_model_rotation);
	Vector3 target_look_direction = Vector3Normalize(move_target - position);
	if (m_target != nullptr)
	{
		Vector3 target_position = { m_target->m_position_render.x, m_target->m_position_render.y, 0.0f };
		target_look_direction = Vector3Normalize(target_position - position);
	}
	float dot = Vector3DotProduct(eye_direction, target_look_direction);

	/* Work out rotations. */
	if (m_target == nullptr && Vector3Length(move_target - position) <= 0.05f)
	{
		/* Do nothing when we're close. */
	}
	else if (fabsf(dot - 1.0f) < 0.000001f)
	{
		/* Parallel. Do nothing. */
	}
	else if (fabs(dot + 1.0f) < 0.000001f)
	{
		/* Antiparallel. Reverse. */
		Vector3 up = { 0.0f, 0.0f, 1.0f };
		float max_angle = GetFrameTime() * TURN_TICK_RATE * PI;
		m_model_rotation = MatrixMultiply(m_model_rotation, MatrixRotate(up, max_angle));
	}
	else
	{
		/* Angle. */
		Vector3 rotation_axis = Vector3CrossProduct(eye_direction, target_look_direction);
		float rotation_angle = acosf(Clamp(dot, -1.0f, 1.0f));
		float max_angle = GetFrameTime() * TURN_TICK_RATE * PI;
		rotation_angle = Clamp(rotation_angle, -max_angle, max_angle);
		m_model_rotation = MatrixMultiply(m_model_rotation, MatrixRotate(rotation_axis, rotation_angle));
	}

	/* Convert to axis/angle so we don't yet have to write a custom draw path for models. */
	Vector3 rotation_axis;
	float rotation_angle;
	matrix_to_rotation(m_model_rotation, rotation_axis, rotation_angle);

	/* Draw. */
	Vector3 draw_position = { m_position_render.x, m_position_render.y, 0.0f };
	Vector3 draw_scale = { 1.0f, 1.0f, 1.0f };
	BeginMode3D(camera);
	{
		draw_model_mesh(m_animation_data.m_model, m_animation_current_frame, draw_position, rotation_axis,
		                rotation_angle, draw_scale, m_tint);
	}
	EndMode3D();
}

void entity::set_animation(animation animation)
{
	m_animation_data = get_animation(animation);
}

void entity::set_action(action_data action_data)
{
	switch (action_data.action)
	{
	case action::IDLE:
		idle();
		break;
	case action::MOVE:
		move(action_data.MOVE.end);
		break;
	case action::ATTACK:
		attack(action_data.ATTACK.entity);
		break;
	}

	/* (TODO, thoave01): We have to set action after, so we know what the previous action was. */
	m_current_action = action_data.action;
}

void entity::reset()
{
	/* Movement. */
	std::deque<tile>().swap(m_path_logic);
	std::deque<tile>().swap(m_path_render);

	/* Combat. */
	m_target = nullptr;
}

void entity::idle()
{
	reset();
	m_asset_manager.set_animation(*this, animation::IDLE);
}

void entity::move(tile end)
{
	if (end == m_position_logic)
	{
		/* Do nothing. */
	}
	else if (m_current_action == action::MOVE)
	{
		/* Empty current paths. */
		std::deque<tile>().swap(m_path_logic);
		std::deque<tile>().swap(m_path_render);

		m_map.generate_path(m_position_logic, end, m_path_logic);

		m_target_logic = m_path_logic.front();
		m_path_logic.pop_front();
		m_path_render.push_back(m_target_logic);
	}
	else
	{
		reset();
		m_map.generate_path(m_position_logic, end, m_path_logic);

		m_target_logic = m_path_logic.front();
		m_path_logic.pop_front();
		m_path_render.push_back(m_target_logic);

		animation animation = m_running ? animation::RUN : animation::WALK;
		m_asset_manager.set_animation(*this, animation);
	}
}

void entity::attack(entity &entity)
{
	m_path_logic.clear();
	m_target = &entity;
	m_asset_manager.set_animation(*this, animation::ATTACK);

	/* Combat. */
	animation_data &ad = m_asset_manager.m_animations[animation::ATTACK];
	u32 frame_count = ad.m_model.meshCount;
	float animation_length = frame_count * ANIMATION_TICK_RATE;

	/* Weapon configuration. */
	m_current_attack_cast_time = GAME_TICK_RATE * 2.0f;
	m_current_attack_cooldown = animation_length - fmod(animation_length, GAME_TICK_RATE);

	/* Current attack state. Let cooldown persist. */
	m_attack_cast_time = 0.0f;
}
