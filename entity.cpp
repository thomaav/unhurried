#include "third_party/raylib.h"

#include "entity.h"

entity::entity(position p)
    : m_p(p)
{
}

void entity::tick()
{
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
