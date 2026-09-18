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

		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.086f, 0.094f, 0.110f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.30f, 0.66f, 1.00f, 1.00f);
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
