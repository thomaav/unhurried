#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#include "raymath.h"
#pragma clang diagnostic pop

enum animation
{
	/* Player. */
	IDLE,
	WALK,
	ATTACK,

	/* Other entities. */
	BOSS,
};

struct animation_data
{
	const char *m_file;
	animation m_animation;
	Model m_model;
};

animation_data get_animation(animation animation);
