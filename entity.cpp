#include <algorithm>
#include <cassert>
#include <cmath>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#include "raymath.h"
#pragma clang diagnostic pop

#include "debug.h"
#include "entity.h"
#include "math.h"

entity::entity(tile p)
    : m_position_logic(p)
    , m_position_render({ (float)p.x, (float)p.y })
{
}

void entity::tick_logic()
{
	if (m_moving)
	{
		m_movement_tick += GetFrameTime();
		while (m_movement_tick > MOVEMENT_TICK_RATE)
		{
			m_movement_tick -= MOVEMENT_TICK_RATE;
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
		return;
	}

	/* Update render position by some increment. */
	Vector2 direction = { target.x - m_position_render.x, target.y - m_position_render.y };
	Vector2 direction_normalized = normalize(direction);

	/* (TODO, thoave01): Improve desync catch-up mechanic to sync all the to the logic position. */
	float tick_scale = m_path_render.size() > 1 ? 2.0f : 1.0f;
	float normalized_tick_rate = MOVEMENT_TICK_RATE / tick_scale;
	float increment = GetFrameTime() / normalized_tick_rate;

	m_position_render.x += direction.x > 0 ? std::min(direction_normalized.x * increment, direction.x) :
	                                         std::max(direction_normalized.x * increment, direction.x);
	m_position_render.y += direction.y > 0 ? std::min(direction_normalized.y * increment, direction.y) :
	                                         std::max(direction_normalized.y * increment, direction.y);
}

void entity::load_model(const char *path)
{
	m_model = LoadModel(path);
	if (m_model.meshCount == 0)
	{
		assert(false);
	}
	m_has_model = true;
}

static Matrix matrix_transform_glb()
{
	/* GLB is +Y up, +Z forward, -X right. We're +Z up. */
	return MatrixRotateX(90.0f * DEG2RAD);
}

static void matrix_to_rotation(Matrix m, Vector3 &axis, float &angle)
{
	Quaternion q = QuaternionFromMatrix(m);
	QuaternionToAxisAngle(q, &axis, &angle);
	angle *= RAD2DEG;
}

void entity::draw(Camera3D &camera)
{
	if (!m_has_model)
	{
		/* Determine position to render entity in. */
		float x = m_position_render.x;
		float y = m_position_render.y;

		/* Render entity. */
		BeginMode3D(camera);
		{
			DrawCube({ x, y, 0.5f }, 0.5f, 0.5f, 1.0f, m_color_render);
			DrawCubeWires({ x, y, 0.5f }, 0.5f, 0.5f, 1.0f, WHITE);
		}
		EndMode3D();
	}
	else
	{
		/* Transform from GLB, and then look towards target of path. */
		Matrix rotation = matrix_transform_glb();
		if (m_moving)
		{
			/* (TODO, thoave01): Make the transform part of the entity. Using raylib Transform. */
			Vector3 position = { m_position_render.x, m_position_render.y, 0.0f };
			tile target_tile = !m_path_logic.empty() ? m_path_logic.back() : m_path_render.back();
			Vector3 target = { (float)target_tile.x + 0.5f, (float)target_tile.y + 0.5f, 0.0f };

			/* Directions. */
			Vector3 eye_direction = { 0.0f, -1.0f, 0.0f };
			Vector3 target_direction = Vector3Normalize(target - position);
			float dot = Vector3DotProduct(eye_direction, target_direction);

			if (fabsf(dot - 1.0f) < 0.000001f)
			{
				/* Parallel. Do nothing. */
			}
			else if (fabs(dot + 1.0f) < 0.000001f)
			{
				/* Antiparallel. Reverse. */
				Vector3 perpendicular = Vector3Perpendicular(eye_direction);
				rotation = MatrixMultiply(rotation, MatrixRotate(perpendicular, PI));
			}
			else
			{

				/* Work out rotations. */
				Vector3 rotation_axis = Vector3CrossProduct(eye_direction, target_direction);
				float rotation_angle = acosf(Clamp(dot, -1.0f, 1.0f));
				rotation = MatrixMultiply(rotation, MatrixRotate(rotation_axis, rotation_angle));
			}
		}

		/* Convert to axis/angle so we don't yet have to write a custom draw path for models. */
		Vector3 rotation_axis;
		float rotation_angle;
		matrix_to_rotation(rotation, rotation_axis, rotation_angle);

		/* Draw. */
		Vector3 position = { m_position_render.x, m_position_render.y, 0.0f };
		Vector3 scale = { 0.012f, 0.012f, 0.012f }; /* (TODO, thoave01): Scale the model itself. */
		BeginMode3D(camera);
		{
			DrawModelEx(m_model, position, rotation_axis, rotation_angle, scale, WHITE);
			DrawModelWiresEx(m_model, position, rotation_axis, rotation_angle, scale, GRAY);
		}
		EndMode3D();
	}
}
