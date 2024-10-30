#pragma once

#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#pragma clang diagnostic pop

#include "types.h"

inline bool operator==(const Color &c1, const Color &c2)
{
	return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
}

template <typename... Ts> //
void draw_printf(const char *string, int x, int y, Ts... ts);
void draw_printf_vector3(const char *name, int x, int y, Vector3 v3);
void draw_printf_vector2(const char *name, int x, int y, Vector3 v2);
void draw_tile(i32 x, i32 y);
void draw_tile_overlay(i32 x, i32 y, Color color);
void draw_model_mesh(Model model, int mesh, Vector3 position, Vector3 axis, float angle, Vector3 scale, Color tint);
