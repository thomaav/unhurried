#include <algorithm>

#include "sprite.h"

void sprite_animation::add_sprite(const char *path)
{
	sprite sd = {};
	sd.m_image = LoadImage(path);
	sd.m_texture = LoadTextureFromImage(sd.m_image);
	m_sprites.push_back(sd);
}

void sprite_animation::draw(u32 frame, u32 x, u32 y) const
{
	const Texture2D texture = m_sprites[frame].m_texture;
	float draw_x = x - (float)texture.width / 2.0f;
	float draw_y = y - (float)texture.height / 2.0f;
	DrawTexture(texture, (int)draw_x, (int)draw_y, WHITE);
}

void active_sprite_animation::draw() const
{
	if (m_is_screen_space)
	{
		m_sprite_animation.draw(m_frame, (u32)m_screen_position.x, (u32)m_screen_position.y);
	}
	else if (m_target != nullptr)
	{
		BoundingBox bb = m_target->m_animation_data.m_bounding_boxes[m_target->m_animation_current_frame];
		float sprite_x = m_target->m_position_render.x;
		float sprite_y = m_target->m_position_render.y;
		float sprite_z = 2.0f * (bb.max.y + bb.min.y) / 3.0f; /* (TODO, thoave01): Jitters with animation. */
		Vector3 sprite_position = { sprite_x, sprite_y, sprite_z };
		Vector2 screen_position = GetWorldToScreen(sprite_position, *m_camera);
		m_sprite_animation.draw(m_frame, (u32)screen_position.x, (u32)screen_position.y);
	}
	else
	{
		Vector2 screen_position =
		    GetWorldToScreen({ m_world_position.x, m_world_position.y, m_world_position.z }, *m_camera);
		m_sprite_animation.draw(m_frame, (u32)screen_position.x, (u32)screen_position.y);
	}
}

/* (TODO, thoave01): Do a fun load test of many sprites? */
/* (TODO, thoave01): Add uniqueness? Solves all problems for adding sprite. */
bool active_sprite_animation::tick()
{
	m_tick += GetFrameTime();
	while (m_tick > m_tick_rate)
	{
		m_tick -= m_tick_rate;
		if (m_frame + 1 == m_sprite_animation.m_sprites.size())
		{
			/* We're done with the animation so don't increment and report done. */
			return true;
		}
		++m_frame;
	}
	return false;
}
