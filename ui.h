#pragma once

bool ui_progress_bar(const char *name, float frac, ImVec2 pos, ImVec2 size, ImVec4 color);
bool ui_render_texture(const char *name, ImVec2 pos, ImVec2 size, RenderTexture &texture);
