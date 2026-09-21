#include "main.h"

#include "imgui_layout/rme_widget.h"

#include "gl_imgui_overlay.h"
#include "imgui_layout/rme.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>

#include "editor.h"
#include "gui.h"
#include "map.h"
#include "map_display.h"
#include "map_drawer.h"
#include "tile.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>

namespace {

std::chrono::steady_clock::time_point s_last_frame_time = std::chrono::steady_clock::now();

float s_mouse_x = -1.0e9f;
float s_mouse_y = -1.0e9f;

// The Rme layout replaces the legacy status footer and overlay scrollbars.
bool s_overlay_active = true;

// Live minimap texture, rebuilt lazily from the current editor's map. The
	// rebuild is rate-limited so continuous editing costs nothing while unchanged.
	GLuint s_minimap_tex = 0;
	int s_minimap_w = 0;
	int s_minimap_h = 0;
	Map* s_minimap_map = nullptr;
	bool s_minimap_valid = false;
	std::chrono::steady_clock::time_point s_minimap_built_at = std::chrono::steady_clock::now();

	// Screen-space rect of the transparent center (live map viewport) plus any
	// interactive keepouts (floor buttons, ...) recorded while the layout draws.
	// Used to let the map receive mouse input under ImGui even when a window
	// captures the mouse.
	struct Rect {
		float x;
		float y;
		float w;
		float h;
	};
	Rect s_map_viewport{ 0, 0, 0, 0 };
	std::vector<Rect> s_map_keepouts;

	// Widths of the [left | center | right] row panels, resized by the draggable
	// dividers drawn in drawPanelSizers(). Runtime state for now.
	constexpr float kPanelThickness = 6.0f;
	constexpr float kPanelMinLeftWidth = 220.0f;
	constexpr float kPanelMinRightWidth = 280.0f;
	constexpr float kPanelMinCenterWidth = 160.0f;
	float s_left_panel_width = 300.0f;
	float s_right_panel_width = 350.0f;

	bool pointInRect(const Rect& r, float px, float py) {
		return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
	}

void buildMinimapTexture(Map& map) {
	int min_x = 0x10000, min_y = 0x10000;
	int max_x = 0x00000, max_y = 0x00000;

	for (MapIterator mit = map.begin(); mit != map.end(); ++mit) {
		const Tile* tile = (*mit)->get();
		if (!tile) {
			continue;
		}
		const Position& pos = (*mit)->getPosition();
		if (pos.z != rme::MapGroundLayer) {
			continue;
		}
		min_x = std::min(min_x, pos.x);
		min_y = std::min(min_y, pos.y);
		max_x = std::max(max_x, pos.x);
		max_y = std::max(max_y, pos.y);
	}

	s_minimap_valid = false;
	if (max_x < min_x || max_y < min_y) {
		return; // empty map
	}

	const int width = max_x - min_x + 1;
	const int height = max_y - min_y + 1;
	// Guard against absurd worlds: 4M pixels (16 MB RGBA) is the budget.
	if (width <= 0 || height <= 0 || static_cast<int64_t>(width) * height > 4 * 1024 * 1024) {
		return;
	}

	std::vector<uint8_t> rgba(static_cast<size_t>(width) * height * 4, 0);
	for (MapIterator mit = map.begin(); mit != map.end(); ++mit) {
		const Tile* tile = (*mit)->get();
		if (!tile) {
			continue;
		}
		const Position& pos = (*mit)->getPosition();
		if (pos.z != rme::MapGroundLayer) {
			continue;
		}
		const wxColor color = colorFromEightBit(tile->getMiniMapColor());
		const size_t pixel = (static_cast<size_t>(pos.x - min_x) + static_cast<size_t>(pos.y - min_y) * width) * 4;
		rgba[pixel + 0] = static_cast<uint8_t>(color.Red());
		rgba[pixel + 1] = static_cast<uint8_t>(color.Green());
		rgba[pixel + 2] = static_cast<uint8_t>(color.Blue());
		rgba[pixel + 3] = 0xFF;
	}

	if (!s_minimap_tex) {
		glGenTextures(1, &s_minimap_tex);
	}
	glBindTexture(GL_TEXTURE_2D, s_minimap_tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

	s_minimap_w = width;
	s_minimap_h = height;
	s_minimap_map = &map;
	s_minimap_valid = true;
}

void ensureMinimapTexture() {
	Editor* editor = g_gui.GetCurrentEditor();
	if (!editor) {
		s_minimap_valid = false;
		return;
	}
	Map& map = editor->getMap();

	const bool source_changed = (s_minimap_map != &map);
	const auto now = std::chrono::steady_clock::now();
	const bool throttled = (now - s_minimap_built_at) < std::chrono::milliseconds(800);
	if (source_changed || (map.hasChanged() && !throttled)) {
		buildMinimapTexture(map);
		s_minimap_built_at = now;
	}
}

ImGuiKey mapKeyCode(int keyCode) {
	keyCode = std::toupper(keyCode);
	if (keyCode >= 'A' && keyCode <= 'Z') {
		return static_cast<ImGuiKey>(ImGuiKey_A + (keyCode - 'A'));
	}
	if (keyCode >= '0' && keyCode <= '9') {
		return static_cast<ImGuiKey>(ImGuiKey_0 + (keyCode - '0'));
	}
	switch (keyCode) {
		case WXK_BACK:
			return ImGuiKey_Backspace;
		case WXK_TAB:
			return ImGuiKey_Tab;
		case WXK_RETURN:
		case WXK_NUMPAD_ENTER:
			return ImGuiKey_Enter;
		case WXK_ESCAPE:
			return ImGuiKey_Escape;
		case WXK_SPACE:
			return ImGuiKey_Space;
		case WXK_DELETE:
			return ImGuiKey_Delete;
		case WXK_HOME:
			return ImGuiKey_Home;
		case WXK_END:
			return ImGuiKey_End;
		case WXK_PAGEUP:
			return ImGuiKey_PageUp;
		case WXK_PAGEDOWN:
			return ImGuiKey_PageDown;
		case WXK_INSERT:
			return ImGuiKey_Insert;
		case WXK_LEFT:
			return ImGuiKey_LeftArrow;
		case WXK_RIGHT:
			return ImGuiKey_RightArrow;
		case WXK_UP:
			return ImGuiKey_UpArrow;
		case WXK_DOWN:
			return ImGuiKey_DownArrow;
	}
	if (keyCode >= WXK_F1 && keyCode <= WXK_F12) {
		return static_cast<ImGuiKey>(ImGuiKey_F1 + (keyCode - WXK_F1));
	}
	return ImGuiKey_None;
}

} // namespace

namespace RmeLayout {

bool Begin(wxWindow* canvas) {
	if (!canvas || !ImGuiOverlay::ensureInitialized()) {
		return false;
	}

	const wxSize clientSize = canvas->GetClientSize();
	if (clientSize.x <= 0 || clientSize.y <= 0) {
		return false;
	}

	const auto now = std::chrono::steady_clock::now();
	float deltaTime = std::chrono::duration<float>(now - s_last_frame_time).count();
	s_last_frame_time = now;
	if (deltaTime <= 0.0f || deltaTime > 0.25f) {
		deltaTime = 1.0f / 60.0f;
	}

	float scale = static_cast<float>(canvas->GetContentScaleFactor());
	if (scale <= 0.0f) {
		scale = 1.0f;
	}

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(clientSize.x), static_cast<float>(clientSize.y));
	io.DisplayFramebufferScale = ImVec2(scale, scale);
	io.DeltaTime = deltaTime;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	g_rme.Draw(canvas);
	return true;
}

void End() {
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Render(wxWindow* canvas) {
	if (Begin(canvas)) {
		End();
	}
}

bool getMapViewport(float& x, float& y, float& w, float& h) {
	if (s_map_viewport.w <= 0.0f || s_map_viewport.h <= 0.0f) {
		return false;
	}
	x = s_map_viewport.x;
	y = s_map_viewport.y;
	w = s_map_viewport.w;
	h = s_map_viewport.h;
	return true;
}

void forwardMouseMove(int x, int y) {
	s_mouse_x = static_cast<float>(x);
	s_mouse_y = static_cast<float>(y);
	ImGui::GetIO().AddMousePosEvent(s_mouse_x, s_mouse_y);
}

void forwardMouseButton(int button, bool down) {
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseButtonEvent(button, down);
	if (down && !io.WantCaptureMouse) {
		// The click landed on the map, so hand keyboard control back to it.
		ImGui::SetNextFrameWantCaptureKeyboard(false);
	}
}

void forwardMouseWheel(int rotation) {
	ImGui::GetIO().AddMouseWheelEvent(0.0f, rotation / 120.0f);
}

void forwardKey(int keyCode, int unicodeChar, bool down, bool ctrl, bool shift, bool alt) {
	ImGuiIO& io = ImGui::GetIO();

	const ImGuiKey key = mapKeyCode(keyCode);
	if (key != ImGuiKey_None) {
		io.AddKeyEvent(key, down);
	}
	io.AddKeyEvent(ImGuiKey_LeftCtrl, ctrl);
	io.AddKeyEvent(ImGuiKey_LeftShift, shift);
	io.AddKeyEvent(ImGuiKey_LeftAlt, alt);

	if (down && unicodeChar != 0 && unicodeChar != WXK_NONE) {
		io.AddInputCharacter(static_cast<unsigned int>(unicodeChar));
	}
}

bool wantsCaptureMouse() {
	return ImGui::GetIO().WantCaptureMouse;
}

bool wantsCaptureKeyboard() {
	return ImGui::GetIO().WantCaptureKeyboard;
}

void clearMapRects() {
	s_map_viewport = { 0, 0, 0, 0 };
	s_map_keepouts.clear();
}

void setMapViewport(float x, float y, float w, float h) {
	s_map_viewport = { x, y, w, h };
}

void drawMapViewport(float width, float height) {
	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const float w = std::min(width, avail.x);
	const float h = std::min(height, avail.y);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	if (ImGui::BeginChild("child10map", { w, h }, ImGuiChildFlags_None, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar)) {
		// The map painted by MapDrawer on the canvas shows through this child only.
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
	setMapViewport(pos.x, pos.y, w, h);
}

void addMapKeepout(float x, float y, float w, float h) {
	s_map_keepouts.push_back({ x, y, w, h });
}

void DrawLiveMap(wxWindow* canvas, float availWidth, float availHeight) {
	const ImVec2 pos = ImGui::GetCursorScreenPos();

	GLuint tex = 0;
	int texW = 0;
	int texH = 0;
	if (canvas) {
		MapCanvas* map_canvas = dynamic_cast<MapCanvas*>(canvas);
		MapDrawer* drawer = map_canvas ? map_canvas->GetDrawer() : nullptr;
		if (drawer) {
			tex = drawer->getMapSurfaceTexture();
			texW = drawer->getMapSurfaceWidth();
			texH = drawer->getMapSurfaceHeight();
		}
	}

	if (!tex || texW <= 0 || texH <= 0) {
		// The surface is not rendered yet (first frame / no scene): reserve the
		// space and record the rect so the camera and input stay consistent.
		ImGui::Dummy({ availWidth, availHeight });
		setMapViewport(pos.x, pos.y, availWidth, availHeight);
		return;
	}

	const float scale = ImGui::GetIO().DisplayFramebufferScale.x > 0.0f ? ImGui::GetIO().DisplayFramebufferScale.x : 1.0f;
	const float natW = static_cast<float>(texW) / scale;
	const float natH = static_cast<float>(texH) / scale;

	// The viewport is the whole available region so the surface regenerates to
	// the region's aspect on the next frame; the transient frame in between is
	// filled with a uniform aspect-fit so the map never distorts.
	float k = std::min(availWidth / natW, availHeight / natH);
	// The surface already matches the region in steady state (k == 1); never
	// upscale beyond that so the pixel art stays crisp.
	k = std::min(k, 1.0f);
	if (k <= 0.0f) {
		setMapViewport(pos.x, pos.y, availWidth, availHeight);
		return;
	}
	const float drawW = natW * k;
	const float drawH = natH * k;
	const float padX = (availWidth - drawW) * 0.5f;
	const float padY = (availHeight - drawH) * 0.5f;
	if (padX > 0.0f) {
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padX);
	}
	if (padY > 0.0f) {
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padY);
	}
	// The surface stores the scene like an OpenGL framebuffer: memory row 0 is
	// texture V=0, which corresponds to the bottom of the screen (map south), and
	// V=1 holds the map north. ImGui places U/V (0, 1) .. (1, 0) top-left to
	// bottom-right, so V must be inverted to keep north pointing up.
	ImGui::Image((ImTextureID)(intptr_t)tex, { drawW, drawH }, { 0, 1 }, { 1, 0 });
	setMapViewport(pos.x, pos.y, availWidth, availHeight);
}

bool isMapPoint(int x, int y) {
	if (s_map_viewport.w <= 0.0f || s_map_viewport.h <= 0.0f || !pointInRect(s_map_viewport, static_cast<float>(x), static_cast<float>(y))) {
		return false;
	}
	for (const Rect& keepout : s_map_keepouts) {
		if (pointInRect(keepout, static_cast<float>(x), static_cast<float>(y))) {
			return false;
		}
	}
	return true;
}

void DrawMinimap(float availWidth, float availHeight) {
	ensureMinimapTexture();

	if (!s_minimap_valid || !s_minimap_tex) {
		if (availWidth > 48.0f && availHeight > 24.0f) {
			ImGui::TextUnformatted("No map");
		}
		return;
	}

	const float scale =
		std::min(availWidth / static_cast<float>(s_minimap_w), availHeight / static_cast<float>(s_minimap_h));
	if (scale <= 0.0f) {
		return;
	}
	const float draw_w = static_cast<float>(s_minimap_w) * scale;
	const float draw_h = static_cast<float>(s_minimap_h) * scale;
	const float pad_x = (availWidth - draw_w) * 0.5f;
	const float pad_y = (availHeight - draw_h) * 0.5f;
	if (pad_x > 0.0f) {
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + pad_x);
	}
	if (pad_y > 0.0f) {
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + pad_y);
	}
	ImGui::Image((ImTextureID)(intptr_t)s_minimap_tex, { draw_w, draw_h }, { 0, 0 }, { 1, 1 });
}

bool isOverlayActive() {
	return s_overlay_active;
}

float leftPanelWidth() {
	return s_left_panel_width;
}

float rightPanelWidth() {
	return s_right_panel_width;
}

void drawPanelSizers() {
	const ImVec2 avail = ImGui::GetContentRegionAvail();
	if (avail.x <= 0.0f || avail.y <= 0.0f) {
		return;
	}
	const ImVec2 row_pos = ImGui::GetCursorScreenPos();

	// Keep the stored panel widths inside the current row (the window may have
	// shrunk under them).
	s_left_panel_width =
		std::clamp(s_left_panel_width, kPanelMinLeftWidth, avail.x - kPanelMinCenterWidth - kPanelMinRightWidth);
	s_right_panel_width =
		std::clamp(s_right_panel_width, kPanelMinRightWidth, avail.x - s_left_panel_width - kPanelMinCenterWidth);

	// The splitter is painted with ImGuiCol_Separator at rest; hide the bar
	// until hovered so only the interaction gives it away. ImGuiCol_Separator
	// is used by plain separators elsewhere, so the override is scoped here.
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	// Divider between the left panel and the rest of the row.
	const ImRect left_bb{
		ImVec2(row_pos.x + s_left_panel_width, row_pos.y),
		ImVec2(row_pos.x + s_left_panel_width + kPanelThickness, row_pos.y + avail.y)
	};
	float left_size1 = s_left_panel_width;
	float left_size2 = avail.x - left_size1 - kPanelThickness;
	ImGui::SplitterBehavior(left_bb, ImGui::GetID("##RmeSplitLeft"), ImGuiAxis_X, &left_size1, &left_size2,
		kPanelMinLeftWidth, kPanelMinCenterWidth + kPanelMinRightWidth, 8.0f, 0.0f, 0);
	s_left_panel_width = left_size1;

	// Divider between the center area and the right panel.
	const float left_of_right = avail.x - s_right_panel_width - kPanelThickness;
	const ImRect right_bb{
		ImVec2(row_pos.x + left_of_right, row_pos.y),
		ImVec2(row_pos.x + left_of_right + kPanelThickness, row_pos.y + avail.y)
	};
	float right_size1 = left_of_right;
	float right_size2 = s_right_panel_width;
	ImGui::SplitterBehavior(right_bb, ImGui::GetID("##RmeSplitRight"), ImGuiAxis_X, &right_size1, &right_size2,
		kPanelMinCenterWidth + kPanelMinLeftWidth, kPanelMinRightWidth, 8.0f, 0.0f, 0);
	s_right_panel_width = right_size2;

	ImGui::PopStyleColor();
}

} // namespace RmeLayout