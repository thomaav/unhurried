#include <algorithm>

#include "third_party/raylib.h"

#include "debug.h"
#include "entity.h"

entity::entity(position p)
    : m_p(p)
    , m_p_f({ (float)p.x, (float)p.y })
    , m_target(p)
{
}

void entity::tick()
{
	draw_printf("target: %u %u\n", 5, 20, m_target.x, m_target.y);
	float frame_time = GetFrameTime();

	i64 dx = std::abs(m_target.x - m_p.x);
	i64 dy = std::abs(m_target.y - m_p.y);
}

void entity::draw(Camera3D &camera)
{
	BeginMode3D(camera);
	{
		DrawCube({ (float)m_p.x + 0.5f, (float)m_p.y + 0.5f, 0.5f }, 1.0f, 1.0f, 1.0f, BLACK);
		DrawCubeWires({ (float)m_p.x + 0.5f, (float)m_p.y + 0.5f, 0.5f }, 1.0f, 1.0f, 1.0f, WHITE);
	}
	EndMode3D();
}
