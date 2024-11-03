#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#pragma clang diagnostic pop

#include "manager.h"
#include "menu.h"

void menu::draw()
{
	const float width = 120.0f;
	const float height = 30.0f;
	const float x = (float)SCREEN_WIDTH / 2.0f - width / 2.0f;
	const float y = (float)SCREEN_HEIGHT / 2.0f - height / 2.0f;
	const char *text = "TBD";
	if (GuiButton((Rectangle){ x, y, width, height }, text))
	{
		m_closed = true;
	}
}
