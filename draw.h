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
void draw_printf(int x, int y, const char *string, Ts... ts)
{
	const char *formatted_string = TextFormat(string, ts...);
	DrawText(formatted_string, x, y, 12, BLACK);
}
void draw_printf_vector3(int x, int y, const char *name, Vector3 v3);
void draw_printf_vector2(int x, int y, const char *name, Vector3 v2);
void draw_tile(i32 x, i32 y, Color color);
void draw_tile_wall(i32 x, i32 y, float height, Color color, float bias = 0.0f);
void draw_tile_overlay(i32 x, i32 y, Color color);
void draw_model_mesh(Model model, int mesh, Vector3 position, Vector3 axis, float angle, Vector3 scale, Color tint);
