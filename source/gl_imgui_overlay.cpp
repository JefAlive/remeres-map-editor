//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "gl_imgui_overlay.h"
#include "canvas_overlay.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <atomic>
#include <cfloat>
#include <chrono>
#include <cstdio>

namespace {
	bool overlay_initialized = false;
	bool frame_open = false;
	std::atomic<bool> cancel_requested {false};
	std::chrono::steady_clock::time_point last_frame_time;
	float mouse_x = -FLT_MAX;
	float mouse_y = -FLT_MAX;
	bool mouse_down = false;

	void applyStyle() {
		ImGui::StyleColorsDark();

		ImGuiStyle &style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.FrameRounding = 3.0f;
		style.WindowPadding = ImVec2(10.0f, 6.0f);
		style.FramePadding = ImVec2(8.0f, 3.0f);
		style.ItemSpacing = ImVec2(8.0f, 4.0f);

		const auto col = [](unsigned int hex, float a = 1.0f) {
			return ImVec4(
				((hex >> 16) & 0xFF) / 255.0f,
				((hex >> 8) & 0xFF) / 255.0f,
				(hex & 0xFF) / 255.0f,
				a);
		};

		// Aura theme (opencode packages/ui/src/theme/themes/aura.json dark
// palette plus the TUI accent tokens). Surfaces stay dark and neutral --
// the purple primary is reserved for interactive labels: active tab,
// checkbox, button hover/active, selection, caret, drag target.
		const ImVec4 bg        = col(0x15141b); // neutral (window)
		const ImVec4 deep      = col(0x101016); // wells / deep panels
		const ImVec4 panel     = col(0x1c1c23); // lifted surfaces
		const ImVec4 popup     = col(0x23232b); // floating surfaces
		const ImVec4 neutral   = col(0x2d2d2d); // border / scrollbar
		const ImVec4 muted     = col(0x6d6a7e); // comment / tree lines
		const ImVec4 soft      = col(0x858298); // disabled text (less dim)

		const ImVec4 fg        = col(0xedecee); // ink

		const ImVec4 purple    = col(0xa277ff); // primary
		const ImVec4 pink      = col(0xf694ff); // secondary
		const ImVec4 blue      = col(0x82e2ff); // info
		const ImVec4 cyanGreen = col(0x61ffca); // success
		const ImVec4 amber     = col(0xffca85); // warning
		const ImVec4 red       = col(0xff6767); // error

		style.Colors[ImGuiCol_Text] = fg;
		style.Colors[ImGuiCol_TextDisabled] = soft;
		style.Colors[ImGuiCol_TextSelectedBg] = col(0xa277ff, 0.40f);
		style.Colors[ImGuiCol_TextLink] = blue;

		style.Colors[ImGuiCol_WindowBg] = bg;
		style.Colors[ImGuiCol_ChildBg] = panel;
		style.Colors[ImGuiCol_PopupBg] = popup;
		style.Colors[ImGuiCol_TitleBg] = deep;
		style.Colors[ImGuiCol_TitleBgActive] = panel;
		style.Colors[ImGuiCol_TitleBgCollapsed] = bg;
		style.Colors[ImGuiCol_MenuBarBg] = deep;

		style.Colors[ImGuiCol_Border] = col(0x2d2d2d, 0.80f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

		style.Colors[ImGuiCol_FrameBg] = panel;
		style.Colors[ImGuiCol_FrameBgHovered] = col(0xa277ff, 0.30f);
		style.Colors[ImGuiCol_FrameBgActive] = col(0xa277ff, 0.45f);

		style.Colors[ImGuiCol_CheckMark] = purple;
		style.Colors[ImGuiCol_CheckboxSelectedBg] = col(0xa277ff, 0.40f);
		style.Colors[ImGuiCol_SliderGrab] = purple;
		style.Colors[ImGuiCol_SliderGrabActive] = pink;
		style.Colors[ImGuiCol_InputTextCursor] = purple;

		style.Colors[ImGuiCol_Button] = panel;
		style.Colors[ImGuiCol_ButtonHovered] = col(0xa277ff, 0.85f);
		style.Colors[ImGuiCol_ButtonActive] = purple;

		style.Colors[ImGuiCol_Header] = col(0xa277ff, 0.45f);
		style.Colors[ImGuiCol_HeaderHovered] = col(0xa277ff, 0.75f);
		style.Colors[ImGuiCol_HeaderActive] = col(0xa277ff, 1.0f);
		style.Colors[ImGuiCol_Separator] = neutral;
		style.Colors[ImGuiCol_SeparatorHovered] = amber;
		style.Colors[ImGuiCol_SeparatorActive] = pink;
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_ResizeGripHovered] = purple;
		style.Colors[ImGuiCol_ResizeGripActive] = pink;

		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_ScrollbarGrab] = neutral;
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = muted;
		style.Colors[ImGuiCol_ScrollbarGrabActive] = purple;

		style.Colors[ImGuiCol_Tab] = deep;
		style.Colors[ImGuiCol_TabHovered] = col(0xa277ff, 0.55f);
		style.Colors[ImGuiCol_TabSelected] = purple;
		style.Colors[ImGuiCol_TabSelectedOverline] = pink;
		style.Colors[ImGuiCol_TabDimmed] = deep;
		style.Colors[ImGuiCol_TabDimmedSelected] = purple;
		style.Colors[ImGuiCol_TabDimmedSelectedOverline] = muted;
		style.Colors[ImGuiCol_UnsavedMarker] = red;

		style.Colors[ImGuiCol_PlotLines] = purple;
		style.Colors[ImGuiCol_PlotLinesHovered] = pink;
		style.Colors[ImGuiCol_PlotHistogram] = cyanGreen;
		style.Colors[ImGuiCol_PlotHistogramHovered] = blue;

		style.Colors[ImGuiCol_TableHeaderBg] = panel;
		style.Colors[ImGuiCol_TableBorderStrong] = neutral;
		style.Colors[ImGuiCol_TableBorderLight] = col(0x2d2d2d, 0.5f);
		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_TableRowBgAlt] = col(0x1c1c23, 0.5f);

		style.Colors[ImGuiCol_DragDropTarget] = purple;
		style.Colors[ImGuiCol_DragDropTargetBg] = col(0xa277ff, 0.20f);

		style.Colors[ImGuiCol_NavCursor] = purple;
		style.Colors[ImGuiCol_NavWindowingHighlight] = col(0xf694ff, 0.70f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.30f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);

		style.Colors[ImGuiCol_TreeLines] = muted;
	}
}

bool ImGuiOverlay::isInitialized() {
	return overlay_initialized;
}

bool ImGuiOverlay::ensureInitialized() {
	if (overlay_initialized) {
		return true;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO &io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	applyStyle();

	if (!ImGui_ImplOpenGL3_Init("#version 330")) {
		ImGui::DestroyContext();
		return false;
	}

	last_frame_time = std::chrono::steady_clock::now();
	overlay_initialized = true;
	frame_open = false;
	return true;
}

void ImGuiOverlay::shutdown() {
	if (!overlay_initialized) {
		return;
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	overlay_initialized = false;
	frame_open = false;
}

void ImGuiOverlay::resetCancelRequest() {
	cancel_requested.store(false);
	mouse_down = false;
}

bool ImGuiOverlay::isCancelRequested() {
	return cancel_requested.load();
}

void ImGuiOverlay::forwardMouseMove(int x, int y) {
	mouse_x = static_cast<float>(x);
	mouse_y = static_cast<float>(y);
}

void ImGuiOverlay::forwardMouseButton(bool down) {
	mouse_down = down;
}

bool ImGuiOverlay::renderLoadingFooter(wxWindow* canvas, const wxString& message, int progressPercent, bool canCancel) {
	if (!canvas || frame_open || !ensureInitialized()) {
		return false;
	}

	const wxSize clientSize = canvas->GetClientSize();
	if (clientSize.x <= 0 || clientSize.y <= 0) {
		return false;
	}

	const auto now = std::chrono::steady_clock::now();
	float deltaTime = std::chrono::duration<float>(now - last_frame_time).count();
	last_frame_time = now;
	if (deltaTime <= 0.0f || deltaTime > 0.25f) {
		deltaTime = 1.0f / 60.0f;
	}

	float scale = static_cast<float>(canvas->GetContentScaleFactor());
	if (scale <= 0.0f) {
		scale = 1.0f;
	}

	const float width = static_cast<float>(clientSize.x);
	const float height = static_cast<float>(clientSize.y);

	progressPercent = std::max(0, std::min(100, progressPercent));

	ImGuiIO &io = ImGui::GetIO();
	io.DisplaySize = ImVec2(width, height);
	io.DisplayFramebufferScale = ImVec2(scale, scale);
	io.DeltaTime = deltaTime;

	if (canCancel) {
		io.AddMousePosEvent(mouse_x, mouse_y);
		io.AddMouseButtonEvent(0, mouse_down);
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	frame_open = true;

	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(width, height));
	ImGui::Begin("##rme_loading_footer", nullptr, flags);

	const wxScopedCharBuffer messageBuffer = message.utf8_str();
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(messageBuffer.data());
	ImGui::SameLine();

	const float cancelWidth = canCancel ? 82.0f : 0.0f;
	float barWidth = ImGui::GetContentRegionAvail().x;
	if (canCancel) {
		barWidth -= cancelWidth + ImGui::GetStyle().ItemSpacing.x;
	}
	if (barWidth < 40.0f) {
		barWidth = 40.0f;
	}

	char overlayText[16];
	std::snprintf(overlayText, sizeof(overlayText), "%d%%", progressPercent);
	ImGui::ProgressBar(static_cast<float>(progressPercent) / 100.0f, ImVec2(barWidth, 0.0f), overlayText);

	bool cancelled = false;
	if (canCancel) {
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(cancelWidth, 0.0f))) {
			cancelled = true;
			cancel_requested.store(true);
		}
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	frame_open = false;

	return cancelled || cancel_requested.load();
}

void ImGuiOverlay::renderStatusFooter(wxWindow* canvas, const wxString& message, const wxString& tileText,
                                      const wxString& positionText, const wxString& zoomText,
                                      const ScrollbarInfo* vbar, const ScrollbarInfo* hbar) {
	if (!canvas || frame_open || !ensureInitialized()) {
		return;
	}

	const wxSize clientSize = canvas->GetClientSize();
	if (clientSize.x <= 0 || clientSize.y <= 0) {
		return;
	}

	const auto now = std::chrono::steady_clock::now();
	float deltaTime = std::chrono::duration<float>(now - last_frame_time).count();
	last_frame_time = now;
	if (deltaTime <= 0.0f || deltaTime > 0.25f) {
		deltaTime = 1.0f / 60.0f;
	}

	float scale = static_cast<float>(canvas->GetContentScaleFactor());
	if (scale <= 0.0f) {
		scale = 1.0f;
	}

	const float width = static_cast<float>(clientSize.x);
	const float height = static_cast<float>(clientSize.y);

	constexpr float barHeight = CanvasOverlay::kStatusBarHeight;
	const float barTop = std::max(0.0f, height - barHeight);

	ImGuiIO &io = ImGui::GetIO();
	io.DisplaySize = ImVec2(width, height);
	io.DisplayFramebufferScale = ImVec2(scale, scale);
	io.DeltaTime = deltaTime;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	frame_open = true;

	ImGuiStyle &style = ImGui::GetStyle();
	const ImVec2 oldWindowPadding = style.WindowPadding;
	const ImVec2 oldCellPadding = style.CellPadding;
	const ImVec4 oldWindowBg = style.Colors[ImGuiCol_WindowBg];
	const ImVec4 oldBorder = style.Colors[ImGuiCol_Border];

	style.WindowPadding = ImVec2(8.0f, 4.0f);
	style.CellPadding = ImVec2(6.0f, 0.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.62f);
	style.Colors[ImGuiCol_Border] = ImVec4(1.0f, 1.0f, 1.0f, 0.10f);

	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::SetNextWindowPos(ImVec2(0.0f, barTop));
	ImGui::SetNextWindowSize(ImVec2(width, barHeight));
	ImGui::Begin("##rme_status_bar", nullptr, flags);

	const ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoBordersInBody;
	if (ImGui::BeginTable("##rme_status_fields", 4, tableFlags, ImVec2(0.0f, 0.0f))) {
		ImGui::TableSetupColumn("msg", ImGuiTableColumnFlags_WidthStretch, 0.30f);
		ImGui::TableSetupColumn("tile", ImGuiTableColumnFlags_WidthStretch, 0.42f);
		ImGui::TableSetupColumn("pos", ImGuiTableColumnFlags_WidthStretch, 0.14f);
		ImGui::TableSetupColumn("zoom", ImGuiTableColumnFlags_WidthStretch, 0.14f);
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		const wxScopedCharBuffer messageBuffer = message.utf8_str();
		ImGui::TextUnformatted(messageBuffer.data());

		ImGui::TableSetColumnIndex(1);
		const wxScopedCharBuffer tileBuffer = tileText.utf8_str();
		ImGui::TextUnformatted(tileBuffer.data());

		ImGui::TableSetColumnIndex(2);
		const wxScopedCharBuffer positionBuffer = positionText.utf8_str();
		ImGui::TextUnformatted(positionBuffer.data());

		ImGui::TableSetColumnIndex(3);
		const wxScopedCharBuffer zoomBuffer = zoomText.utf8_str();
		ImGui::TextUnformatted(zoomBuffer.data());

		ImGui::EndTable();
	}

	ImGui::End();

	// Draw overlay scrollbars on top of everything using the foreground draw list.
	// More translucent than status bar.
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();

	if (vbar && vbar->visible && vbar->alpha > 0.0f) {
		ImU32 color = IM_COL32(0, 0, 0, static_cast<int>(255 * vbar->alpha * 0.35f));
		draw_list->AddRectFilled(
			ImVec2(vbar->cross_start, vbar->thumb_start),
			ImVec2(vbar->cross_start + vbar->thickness, vbar->thumb_start + vbar->thumb_length),
			color,
			vbar->thickness * 0.5f
		);
	}
	if (hbar && hbar->visible && hbar->alpha > 0.0f) {
		ImU32 color = IM_COL32(0, 0, 0, static_cast<int>(255 * hbar->alpha * 0.35f));
		draw_list->AddRectFilled(
			ImVec2(hbar->thumb_start, hbar->cross_start),
			ImVec2(hbar->thumb_start + hbar->thumb_length, hbar->cross_start + hbar->thickness),
			color,
			hbar->thickness * 0.5f
		);
	}

	style.WindowPadding = oldWindowPadding;
	style.CellPadding = oldCellPadding;
	style.Colors[ImGuiCol_WindowBg] = oldWindowBg;
	style.Colors[ImGuiCol_Border] = oldBorder;

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	frame_open = false;
}
