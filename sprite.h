#pragma once

#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#pragma clang diagnostic pop

#include "types.h"

/* (TODO, thoave01): Image data is not strictly needed once we upload to a texture? */
struct sprite
{
	Image m_image;
	Texture2D m_texture;
};

class sprite_animation
{
public:
	/* (TODO, thoave01): Everything related to constructor, and destructor should unload stuff. */
	void add_sprite(const char *path);
	void draw(u32 frame);

private:
	std::vector<sprite> m_sprites = {};
};
