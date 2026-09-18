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

#ifndef RME_LOADING_BAR_CANVAS_H_
#define RME_LOADING_BAR_CANVAS_H_

#include <wx/glcanvas.h>

// A thin OpenGL canvas pinned to the bottom of the main frame that renders the
// ImGui footer loading bar. It is created on demand while a load bar is active
// and destroyed as soon as the operation finishes. This gives the loading bar
// an OpenGL surface even during phases where no MapCanvas exists yet, such as
// opening a map.
class LoadingBarCanvas : public wxGLCanvas {
public:
	LoadingBarCanvas(wxWindow* parent);

	void OnPaint(wxPaintEvent &event);
	void OnEraseBackground(wxEraseEvent &event) { }
	void OnMouseMove(wxMouseEvent &event);
	void OnMouseLeftDown(wxMouseEvent &event);
	void OnMouseLeftUp(wxMouseEvent &event);
};

#endif
