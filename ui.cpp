#include <stdio.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "imgui.h"
#include "rlImGui.h"
#pragma clang diagnostic pop

#include "manager.h"
#include "ui.h"

bool ui_progress_bar(const char *name, float frac, ImVec2 pos, ImVec2 size, ImVec4 color)
{
	ImGuiStyle &style = ImGui::GetStyle();
	style.Colors[ImGuiCol_PlotHistogram] = color;
	style.Colors[ImGuiCol_WindowBg] = { 0.0f, 0.0f, 0.0f, 1.0f };

	/* How window should behave and look. */
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
	const ImGuiWindowFlags flags =          //
	    ImGuiWindowFlags_NoTitleBar |       //
	    ImGuiWindowFlags_AlwaysAutoResize | //
	    ImGuiWindowFlags_NoMove |           //
	    ImGuiWindowFlags_NoScrollbar;

	/* Where to put window. */
	ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing, { 0.5f, 0.0f });
	ImGui::SetNextWindowSize(size);

	/* Draw. */
	bool clicked = false;
	if (ImGui::Begin(name, nullptr, flags))
	{
		ImGui::ProgressBar(frac, { -0.01f, -0.01f });
		clicked = ImGui::IsItemClicked();
	}
	ImGui::End();

	/* Revert state and return. */
	/* (TODO, thoave01): Maybe also revert colors? */
	ImGui::PopStyleVar(4);
	return clicked;
}

bool ui_render_texture(const char *name, ImVec2 pos, ImVec2 size, RenderTexture &texture)
{
	/* How window should behave and look. */
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
	const ImGuiWindowFlags flags =          //
	    ImGuiWindowFlags_NoTitleBar |       //
	    ImGuiWindowFlags_AlwaysAutoResize | //
	    ImGuiWindowFlags_NoMove |           //
	    ImGuiWindowFlags_NoScrollbar;

	/* Where to put window. */
	ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing, { 0.5f, 0.5f });
	ImGui::SetNextWindowSize(size);

	/* Draw. */
	bool clicked = false;
	ImGui::Begin(name, nullptr, flags);
	{
		ClearBackground(RAYWHITE);
		rlImGuiImageRenderTextureFit(&texture, true);
		clicked = ImGui::IsItemClicked();

		if (ImGui::IsItemHovered())
		{
			const ImVec2 min = ImGui::GetItemRectMin();
			const ImVec2 max = ImGui::GetItemRectMax();
			const ImU32 color = IM_COL32(127, 127, 127, 255);
			const float size = 5.0f;
			ImDrawList *draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRect(min, max, color, 0.0f, ImDrawFlags_None, size);
		}
	}
	ImGui::End();

	/* Revert state and return. */
	ImGui::PopStyleVar(4);
	return clicked;
}
