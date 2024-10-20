#pragma once

#include <iostream>

#include "third_party/raylib.h"

template <typename... Ts> //
void draw_printf(const char *string, int x, int y, Ts... ts)
{
	const char *formatted_string = TextFormat(string, ts...);
	DrawText(formatted_string, x, y, 12, BLACK);
}
