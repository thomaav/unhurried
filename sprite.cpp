#include "sprite.h"

void sprite_animation::add_sprite(const char *path)
{
	sprite sd = {};
	sd.m_image = LoadImage(path);
	sd.m_texture = LoadTextureFromImage(sd.m_image);
	m_sprites.push_back(sd);
}

void sprite_animation::draw(u32 frame, u32 x, u32 y)
{
	const Texture2D texture = m_sprites[frame].m_texture;
	float draw_x = x - (float)texture.width / 2.0f;
	float draw_y = y - (float)texture.height / 2.0f;
	DrawTexture(texture, (int)draw_x, (int)draw_y, WHITE);
}
