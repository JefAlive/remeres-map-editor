#include "main.h"

#include "imgui_layout/rme_widget.h"

#include "gl_imgui_overlay.h"
#include "imgui_layout/rme.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <cctype>

namespace {

std::chrono::steady_clock::time_point s_last_frame_time = std::chrono::steady_clock::now();

float s_mouse_x = -1.0e9f;
float s_mouse_y = -1.0e9f;

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

void Render(wxWindow* canvas) {
	if (!canvas || !ImGuiOverlay::ensureInitialized()) {
		return;
	}

	const wxSize clientSize = canvas->GetClientSize();
	if (clientSize.x <= 0 || clientSize.y <= 0) {
		return;
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
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

} // namespace RmeLayout