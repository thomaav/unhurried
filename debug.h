#pragma once

#include <iostream>

#include "raylib.h"
#include "rlgl.h"

#include "types.h"

template <typename... Ts> //
static inline void draw_printf(const char *string, int x, int y, Ts... ts)
{
	const char *formatted_string = TextFormat(string, ts...);
	DrawText(formatted_string, x, y, 12, BLACK);
}

static inline void draw_tile(i32 x, i32 y)
{
	/* Create tile quad. */
	Vector3 vertices[] = {
		{ (float)x, (float)y, 0.0f },               //
		{ (float)x + 1.0f, (float)y, 0.0f },        //
		{ (float)x, (float)y + 1.0f, 0.0f },        //
		{ (float)x + 1.0f, (float)y + 1.0f, 0.0f }, //
	};

	/* Draw quad. */
	DrawTriangleStrip3D(&vertices[0], 4, DARKGRAY);

	/* Draw outline. */
	DrawLine3D(vertices[0], vertices[1], WHITE);
	DrawLine3D(vertices[1], vertices[3], WHITE);
	DrawLine3D(vertices[3], vertices[2], WHITE);
	DrawLine3D(vertices[2], vertices[0], WHITE);
}

static inline void draw_tile_overlay(i32 x, i32 y, Color color)
{
	/* Create tile quad. */
	Vector3 vertices[] = {
		{ (float)x, (float)y, 0.005f },               //
		{ (float)x + 1.0f, (float)y, 0.005f },        //
		{ (float)x, (float)y + 1.0f, 0.005f },        //
		{ (float)x + 1.0f, (float)y + 1.0f, 0.005f }, //
	};

	/* Draw quad. */
	DrawTriangleStrip3D(&vertices[0], 4, color);

	/* Draw outline. */
	DrawLine3D(vertices[0], vertices[1], BLACK);
	DrawLine3D(vertices[1], vertices[3], BLACK);
	DrawLine3D(vertices[3], vertices[2], BLACK);
	DrawLine3D(vertices[2], vertices[0], BLACK);
}
