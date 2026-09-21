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

#include "map_window.h"
#include "gui.h"
#include "sprites.h"
#include "editor.h"

MapWindow::MapWindow(wxWindow* parent, Editor &editor) :
	wxPanel(parent, PANE_MAIN),
	editor(editor),
	canvas(nullptr),
	scroll_x(0),
	scroll_y(0),
	range_x(1),
	range_y(1),
	replaceItemsDialog(nullptr) {
	int GL_settings[8];
	GL_settings[0] = WX_GL_RGBA;
	GL_settings[1] = WX_GL_DOUBLEBUFFER;
	GL_settings[2] = WX_GL_CORE_PROFILE;
	GL_settings[3] = WX_GL_MAJOR_VERSION;
	GL_settings[4] = 3;
	GL_settings[5] = WX_GL_MINOR_VERSION;
	GL_settings[6] = 3;
	GL_settings[7] = 0;
	canvas = newd MapCanvas(this, editor, GL_settings);

	// The canvas fills the whole panel; scrolling is drawn by the canvas as an
	// overlay and is not laid out as a separate widget.
	wxBoxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	topsizer->Add(canvas, wxSizerFlags(1).Expand());
	SetSizer(topsizer);
}

MapWindow::~MapWindow() {
	////
}

void MapWindow::ShowReplaceItemsDialog(bool selectionOnly) {
	if (replaceItemsDialog) {
		return;
	}

	replaceItemsDialog = new ReplaceItemsDialog(this, selectionOnly);
	replaceItemsDialog->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MapWindow::OnReplaceItemsDialogClose), nullptr, this);
	replaceItemsDialog->Show();
}

void MapWindow::CloseReplaceItemsDialog() {
	if (replaceItemsDialog) {
		replaceItemsDialog->Close();
	}
}

void MapWindow::OnReplaceItemsDialogClose(wxCloseEvent &event) {
	if (replaceItemsDialog) {
		replaceItemsDialog->Disconnect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MapWindow::OnReplaceItemsDialogClose), nullptr, this);
		replaceItemsDialog->Destroy();
		replaceItemsDialog = nullptr;
	}
}

void MapWindow::SetSize(int x, int y, bool center) {
	if (x <= 0 || y <= 0) {
		return;
	}

	range_x = x;
	range_y = y;

	if (center) {
		// Center the view: camera offset = content_center - view_center
		double zoom = g_gui.GetCurrentZoom();
		double view_cx, view_cy;
		int vpx, vpy, vpw, vph;
		if (canvas->getMapViewport(&vpx, &vpy, &vpw, &vph)) {
			view_cx = vpx + vpw / 2.0;
			view_cy = vpy + vph / 2.0;
		} else {
			int windowSizeX, windowSizeY;
			canvas->GetSize(&windowSizeX, &windowSizeY);
			view_cx = windowSizeX / 2.0;
			view_cy = windowSizeY / 2.0;
		}
		scroll_x = x / 2 - int(view_cx * zoom);
		scroll_y = y / 2 - int(view_cy * zoom);
	}
	ClampScroll();
}

void MapWindow::ClampScroll() {
	double zoom = g_gui.GetCurrentZoom();
	int view_w, view_h;
	int vpx, vpy, vpw, vph;
	if (canvas->getMapViewport(&vpx, &vpy, &vpw, &vph)) {
		view_w = int(vpw * zoom);
		view_h = int(vph * zoom);
	} else {
		int windowSizeX, windowSizeY;
		canvas->GetSize(&windowSizeX, &windowSizeY);
		view_w = int(windowSizeX * zoom);
		view_h = int(windowSizeY * zoom);
	}
	int max_x = std::max(0, range_x - view_w);
	int max_y = std::max(0, range_y - view_h);
	scroll_x = std::min(std::max(0, scroll_x), max_x);
	scroll_y = std::min(std::max(0, scroll_y), max_y);
}

void MapWindow::UpdateDialogs(bool show) {
	if (replaceItemsDialog) {
		replaceItemsDialog->Show(show);
	}
}

void MapWindow::GetViewStart(int* x, int* y) {
	*x = scroll_x;
	*y = scroll_y;
}

void MapWindow::GetScrollRange(int* x, int* y) const {
	*x = range_x;
	*y = range_y;
}

void MapWindow::GetViewSize(int* x, int* y) {
	canvas->GetSize(x, y);
	*x *= canvas->GetContentScaleFactor();
	*y *= canvas->GetContentScaleFactor();
}

void MapWindow::FitToMap() {
	const Map &map = editor.getMap();
	SetSize(map.getWidth() * rme::TileSize, map.getHeight() * rme::TileSize, true);
}

Position MapWindow::GetScreenCenterPosition() {
	int x, y;
	canvas->GetScreenCenter(&x, &y);
	return Position(x, y, canvas->GetFloor());
}

void MapWindow::SetScreenCenterPosition(const Position &position, bool showIndicator) {
	if (!position.isValid()) {
		return;
	}

	int x = position.x * rme::TileSize;
	int y = position.y * rme::TileSize;
	int z = position.z;
	if (position.z < 8) {
		// Compensate for floor offset above ground
		x -= (rme::MapGroundLayer - z) * rme::TileSize;
		y -= (rme::MapGroundLayer - z) * rme::TileSize;
	}

	const Position &center = GetScreenCenterPosition();
	if (previous_position != center) {
		previous_position.x = center.x;
		previous_position.y = center.y;
		previous_position.z = center.z;
	}

	Scroll(x, y, true);
	canvas->ChangeFloor(z);

	if (showIndicator) {
		canvas->ShowPositionIndicator(position);
		Refresh();
	}
}

void MapWindow::GoToPreviousCenterPosition() {
	SetScreenCenterPosition(previous_position, true);
}

void MapWindow::Scroll(int x, int y, bool center) {
	if (center) {
		double zoom = g_gui.GetCurrentZoom();
		double view_cx, view_cy;
		int vpx, vpy, vpw, vph;
		if (canvas->getMapViewport(&vpx, &vpy, &vpw, &vph)) {
			view_cx = vpx + vpw / 2.0;
			view_cy = vpy + vph / 2.0;
		} else {
			int windowSizeX, windowSizeY;
			canvas->GetSize(&windowSizeX, &windowSizeY);
			view_cx = windowSizeX / 2.0;
			view_cy = windowSizeY / 2.0;
		}
		x -= int(view_cx * zoom);
		y -= int(view_cy * zoom);
	}

	scroll_x = x;
	scroll_y = y;
	ClampScroll();
	g_gui.UpdateMinimap();
}

void MapWindow::ScrollRelative(int x, int y) {
	scroll_x += x;
	scroll_y += y;
	ClampScroll();
	g_gui.UpdateMinimap();
}

void MapWindow::OnSize(wxSizeEvent &event) {
	ClampScroll();
	event.Skip();
}
