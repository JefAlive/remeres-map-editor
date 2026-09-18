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

#include "loading_bar_canvas.h"
#include "gl_imgui_overlay.h"
#include "gui.h"

namespace {
	// Match the pixel format of MapCanvas so the shared GL context works.
	const int* loadingCanvasAttributes() {
		static int attributes[8] = {
			WX_GL_RGBA,
			WX_GL_DOUBLEBUFFER,
			WX_GL_CORE_PROFILE,
			WX_GL_MAJOR_VERSION, 3,
			WX_GL_MINOR_VERSION, 3,
			0
		};
		return attributes;
	}
}

LoadingBarCanvas::LoadingBarCanvas(wxWindow* parent) :
	wxGLCanvas(parent, wxID_ANY, loadingCanvasAttributes(), wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	Bind(wxEVT_PAINT, &LoadingBarCanvas::OnPaint, this);
	Bind(wxEVT_ERASE_BACKGROUND, &LoadingBarCanvas::OnEraseBackground, this);
	Bind(wxEVT_MOTION, &LoadingBarCanvas::OnMouseMove, this);
	Bind(wxEVT_LEFT_DOWN, &LoadingBarCanvas::OnMouseLeftDown, this);
	Bind(wxEVT_LEFT_UP, &LoadingBarCanvas::OnMouseLeftUp, this);
}

void LoadingBarCanvas::OnPaint(wxPaintEvent &event) {
	if (!IsShownOnScreen()) {
		return;
	}

	SetCurrent(*g_gui.GetGLContext(this));

	// The ImGui footer window is drawn with an opaque background and covers the
	// whole canvas, so no explicit clear is required here.
	ImGuiOverlay::renderLoadingFooter(this, g_gui.GetLoadingMessage(), g_gui.GetLoadingProgress(), g_gui.IsLoadingBarCancelAllowed());

	SwapBuffers();
}

void LoadingBarCanvas::OnMouseMove(wxMouseEvent &event) {
	ImGuiOverlay::forwardMouseMove(event.GetX(), event.GetY());
	event.Skip();
}

void LoadingBarCanvas::OnMouseLeftDown(wxMouseEvent &event) {
	ImGuiOverlay::forwardMouseMove(event.GetX(), event.GetY());
	ImGuiOverlay::forwardMouseButton(true);
	event.Skip();
}

void LoadingBarCanvas::OnMouseLeftUp(wxMouseEvent &event) {
	ImGuiOverlay::forwardMouseMove(event.GetX(), event.GetY());
	ImGuiOverlay::forwardMouseButton(false);
	event.Skip();
}
