#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "imgui.h"
#pragma clang diagnostic pop

#include "manager.h"
#include "ui.h"

void ui_progress_bar(const char *name, float frac, ImVec2 pos, ImVec2 size, ImVec4 color)
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
	if (ImGui::Begin(name, nullptr, flags))
	{
		ImGui::ProgressBar(frac, { -0.01f, -0.01f });
	}
	ImGui::End();

	ImGui::PopStyleVar(4);
}
