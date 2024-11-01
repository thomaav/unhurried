#pragma once

#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#pragma clang diagnostic pop

#include "entity.h"
#include "types.h"

enum class sprite_type
{
	/* (TODO, thoave01): Defaults for all enums? */
	CLICK_YELLOW,
	CLICK_RED,
	HITSPLAT_RED,
	HITSPLAT_BLUE,
};

/* (TODO, thoave01): Image data is not strictly needed once we upload to a texture? */
struct sprite
{
	Image m_image;
	Texture2D m_texture;
};

class sprite_animation
{
public:
	sprite_animation() = delete;
	sprite_animation(sprite_type type)
	    : m_type(type)
	{
	}

	/* (TODO, thoave01): Everything related to constructor, and destructor should unload stuff. */
	void add_sprite(const char *path);
	void draw(u32 frame, u32 x, u32 y) const;

	std::vector<sprite> m_sprites = {};
	sprite_type m_type = sprite_type::CLICK_YELLOW;
};

class active_sprite_animation
{
public:
	active_sprite_animation(sprite_animation &sprite_animation, Vector2 position)
	    : m_sprite_animation(sprite_animation)
	    , m_position(position)
	    , m_frame(0)
	{
	}

	void draw() const;
	bool tick();

	const sprite_animation &m_sprite_animation;
	const Vector2 m_position;
	u32 m_frame;
	float m_tick = 0;
	const float m_tick_rate = SPRITE_ANIMATION_TICK_RATE;
};
