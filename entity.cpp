#include <algorithm>

#include "third_party/raylib.h"

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
		while (m_movement_tick > TICK_RATE)
		{
			m_movement_tick -= TICK_RATE;
			m_position_logic = m_target;
			if (!m_path.empty())
			{
				m_target = m_path.front();
				m_path.pop();
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
	/* Don't move if we're close, to avoid stuttering. */
	Vector2 position = m_position_render;
	Vector2 target = { (float)m_target.x, (float)m_target.y };
	if (length(position - target) <= 0.05f)
	{
		return;
	}

	/* Only update render when we're not moving if we've lagged behind. */
	if (!m_moving && length(position - target) <= 0.05f)
	{
		return;
	}

	/* Update render position by some increment. */
	Vector2 direction = { target.x - m_position_render.x, target.y - m_position_render.y };
	Vector2 direction_normalized = normalize(direction);
	float increment = GetFrameTime() / TICK_RATE;
	m_position_render.x += std::min(direction_normalized.x * increment, direction.x);
	m_position_render.y += std::min(direction_normalized.y * increment, direction.y);
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
