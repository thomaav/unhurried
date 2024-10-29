#pragma once

#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#pragma clang diagnostic pop

#include "types.h"

template <typename... Ts> //
void draw_printf(const char *string, int x, int y, Ts... ts);
void draw_printf_vector3(const char *name, int x, int y, Vector3 v3);
void draw_printf_vector2(const char *name, int x, int y, Vector3 v2);
void draw_tile(i32 x, i32 y);
void draw_tile_overlay(i32 x, i32 y, Color color);
void draw_model_mesh(Model model, int mesh, Vector3 position, Vector3 axis, float angle, Vector3 scale, Color tint);
void draw_click(u32 frame, Color color);
