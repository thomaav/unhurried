#include <algorithm>
#include <cmath>

#include "raylib.h"

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
	Vector2 target = { (float)m_target_render.x, (float)m_target_render.y };
	if (length(position - target) <= 0.05f)
	{
		/* Move to exact location of target. */
		m_position_render = { (float)m_target_render.x, (float)m_target_render.y };

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

void entity::draw(Camera3D &camera)
{
	BeginMode3D(camera);
	{
		/* Determine position to render entity in. */
		float x = m_position_render.x;
		float y = m_position_render.y;

		/* Render entity. */
		DrawCube({ x + 0.5f, y + 0.5f, 0.5f }, 0.5f, 0.5f, 1.0f, BLACK);
		DrawCubeWires({ x + 0.5f, y + 0.5f, 0.5f }, 0.5f, 0.5f, 1.0f, WHITE);
	}
	EndMode3D();
}
