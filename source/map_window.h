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

#ifndef RME_MAP_WINDOW_H_
#define RME_MAP_WINDOW_H_

#include "position.h"
#include "replace_items_window.h"

class MapCanvas;

// Map window, a window displaying a map. This is the window that's inside each
// tab in the editor. It owns the map camera state and the containing canvas.
// The scrollbars are not native widgets anymore: the canvas draws thin overlay
// scrollbars on top of the map and drives the camera through this window.
// Does NOT control any map rendering or editing at all. (mapdisplay.h)
class MapWindow : public wxPanel {
public:
	MapWindow(wxWindow* parent, Editor &editor);
	virtual ~MapWindow();

	// Event handlers
	void OnSize(wxSizeEvent &event);

	// Custom interface for MapWindow

	// GetViewSize returns the size of the containing canvas, in pixels
	void GetViewSize(int* x, int* y);
	// Returns the start of the camera on the map, in pixels
	void GetViewStart(int* x, int* y);

	// Returns the total scrollable content size, in map pixels
	void GetScrollRange(int* x, int* y) const;

	// Set size of this window (in pixels)
	// if center is true, the camera will be moved to the center of the map.
	void SetSize(int x, int y, bool center = false);

	// Scroll to the specified, absolute position (in pixels)
	void Scroll(int x, int y, bool center = false);

	// Scroll this many pixels in X/Y, relative to current position
	void ScrollRelative(int x, int y);

	// Resize scrollbars to fit to the map dimensions
	// This needs to be called after updating map height/width
	void FitToMap();

	// Screen position.
	Position GetScreenCenterPosition();
	void SetScreenCenterPosition(const Position &position, bool showIndicator = false);
	void GoToPreviousCenterPosition();

	// Return the containing canvas
	MapCanvas* GetCanvas() const noexcept {
		return canvas;
	}

	void ShowReplaceItemsDialog(bool selectionOnly);
	void CloseReplaceItemsDialog();
	void OnReplaceItemsDialogClose(wxCloseEvent &event);

protected:
	// Keeps the camera inside the scrollable area after a resize.
	void ClampScroll();
	void UpdateDialogs(bool show);

protected:
	Editor &editor;
	MapCanvas* canvas;

	// Camera offset and content size, in map pixels.
	int scroll_x;
	int scroll_y;
	int range_x;
	int range_y;

private:
	ReplaceItemsDialog* replaceItemsDialog;
	Position previous_position;

	friend class MainFrame;
	friend class MapCanvas;

	DECLARE_EVENT_TABLE()
};

#endif
