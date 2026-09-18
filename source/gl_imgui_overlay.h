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

#ifndef RME_GL_IMGUI_OVERLAY_H_
#define RME_GL_IMGUI_OVERLAY_H_

#include <wx/window.h>

// Dear ImGui overlay rendered directly into an OpenGL canvas.
//
// The project does not use an ImGui platform backend (no OS window or input
// backend); the overlay is a passive renderer driven by wxWidgets paint events.
// It draws the in-canvas footer loading bar that replaces the old modal
// wxGenericProgressDialog.
namespace ImGuiOverlay {
	// True once the ImGui context and the OpenGL3 renderer are ready.
	bool isInitialized();

	// Lazily creates the ImGui context and the OpenGL3 backend. Must be called
	// with a current GL context, i.e. from inside a wxGLCanvas paint handler.
	bool ensureInitialized();

	// Releases the ImGui resources. Must be called with a current GL context.
	void shutdown();

	// Draws the footer loading bar docked to the bottom of `canvas`, using the
	// canvas client size as the ImGui display size. `progressPercent` is clamped
	// to 0..100. Returns true when the user requested cancellation (only
	// possible when canCancel is set).
	bool renderLoadingFooter(wxWindow* canvas, const wxString& message, int progressPercent, bool canCancel);

	// Draws the translucent bottom status bar overlay inside `canvas`. The
	// fields mirror the old wxStatusBar: transient message, tile description,
	// cursor position and zoom.
	void renderStatusFooter(wxWindow* canvas, const wxString& message, const wxString& tileText, const wxString& positionText, const wxString& zoomText);

	// Discards a pending cancel request, e.g. when a new bar is created.
	void resetCancelRequest();
	bool isCancelRequested();

	// Mouse input forwarding, used by the optional cancel button.
	void forwardMouseMove(int x, int y);
	void forwardMouseButton(bool down);
}

#endif
