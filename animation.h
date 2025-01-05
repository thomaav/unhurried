#pragma once

#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#include "raymath.h"
#pragma clang diagnostic pop

/* (TODO, thoave01): Differentiate model and sprite animation better. */
enum class animation
{
	/* Player. */
	IDLE,
	WALK,
	RUN,
	ATTACK,

	/* Other entities. */
	BOSS,
};

/* (TODO, thoave01): Re-do lots of naming...? */
struct animation_data
{
	const char *m_file;
	animation m_animation;
	Model m_model;
	std::vector<BoundingBox> m_bounding_boxes = {};
};

animation_data get_animation(animation animation);
