#pragma once

#include <cmath>

#include "raylib.h"

#include "map.h"

/* (TODO, thoave01): Remove this file. Everything is in raymath.h. */

static inline Vector2 normalize(Vector2 v)
{
	float length = std::sqrt(v.x * v.x + v.y * v.y);
	if (length > 0)
	{
		v.x /= length;
		v.y /= length;
	}
	return v;
}
