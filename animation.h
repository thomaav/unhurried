#pragma once

#include "raylib.h"

enum animation
{
	IDLE = 1,
	WALK = 2,
};

struct animation_data
{
	const char *m_file;
	animation m_animation;
	Model m_model;
};

void switch_animation(const char *path, animation a, animation_data &ad);
