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
#include "math.h"

entity::entity(tile p)
    : m_position_logic(p)
    , m_position_render({ (float)p.x, (float)p.y })
    , m_target_logic(p)
    , m_target_render(p)
{
}

void entity::tick_logic()
{
	if (m_moving)
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
			else
			{
				m_moving = false;
			}
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

	/* Don't move if we're close, to avoid stuttering. */
	Vector2 position = m_position_render;
	Vector2 target = { (float)m_target_render.x + 0.5f, (float)m_target_render.y + 0.5f };
	if (length(position - target) <= 0.05f)
	{
		/* Move to exact location of target. */
		m_position_render = { (float)target.x, (float)target.y };

		/* Set next target tile. */
		if (!m_path_render.empty())
		{
			m_target_render = m_path_render.front();
			m_path_render.pop_front();
		}
		else
		{
			/* (TODO, thoave01): Not really good to have the model and animation coupled... needs a proper system. */
			/* (TODO, thoave01): We should be using the asset manager somehow here. */
			if (m_animation_data.m_animation == animation::WALK || m_animation_data.m_animation == animation::RUN)
			{
				set_animation(m_idle_animation);
			}
		}

		return;
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
	tile target_tile = m_target_render;
	Vector3 move_target = { (float)target_tile.x + 0.5f, (float)target_tile.y + 0.5f, 0.0f };

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

void entity::stop_moving()
{
	m_path_logic.clear();
}
