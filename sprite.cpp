#include "sprite.h"

void sprite_animation::add_sprite(const char *path)
{
	sprite sd = {};
	sd.m_image = LoadImage(path);
	sd.m_texture = LoadTextureFromImage(sd.m_image);
	m_sprites.push_back(sd);
}

void sprite_animation::draw(u32 frame)
{
	const Texture2D texture = m_sprites[frame].m_texture;
	const Vector2 mp = GetMousePosition();
	float x = mp.x - (float)texture.width / 2.0f;
	float y = mp.y - (float)texture.height / 2.0f;
	DrawTexture(texture, (int)x, (int)y, WHITE);
}
