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

#include "theme.h"

#include "gui.h"

#include <wx/dataview.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>

namespace Theme {

wxColour Bg() {
	return wxColour(0x1a, 0x1b, 0x26);
}

wxColour BgDark() {
	return wxColour(0x16, 0x16, 0x1e);
}

wxColour BgHighlight() {
	return wxColour(0x29, 0x2e, 0x42);
}

wxColour Border() {
	return wxColour(0x3b, 0x42, 0x61);
}

wxColour InputBg() {
	return wxColour(0x1f, 0x23, 0x35);
}

wxColour Fg() {
	return wxColour(0xc0, 0xca, 0xf5);
}

wxColour FgDark() {
	return wxColour(0xa9, 0xb1, 0xd6);
}

wxColour Comment() {
	return wxColour(0x56, 0x5f, 0x89);
}

wxColour Blue() {
	return wxColour(0x7a, 0xa2, 0xf7);
}

wxColour Cyan() {
	return wxColour(0x7d, 0xcf, 0xff);
}

wxColour Green() {
	return wxColour(0x9e, 0xce, 0x6a);
}

wxColour Orange() {
	return wxColour(0xff, 0x9e, 0x64);
}

wxColour Purple() {
	return wxColour(0xbb, 0x9a, 0xf7);
}

wxColour Red() {
	return wxColour(0xf7, 0x76, 0x8e);
}

wxColour Yellow() {
	return wxColour(0xe0, 0xaf, 0x68);
}

wxColour SelectionBg() {
	return Blue();
}

wxColour SelectionFg() {
	return BgDark();
}

namespace {

	void ApplyAuiManagerArt() {
		static bool applied = false;
		if (applied) {
			return;
		}

		wxAuiManager* manager = g_gui.GetAuiManager();
		if (manager == nullptr) {
			return;
		}
		applied = true;

		auto* art = newd wxAuiDefaultDockArt();
		art->SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR, Bg());
		art->SetColour(wxAUI_DOCKART_SASH_COLOUR, BgDark());
		art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR, BgHighlight());
		art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR, BgHighlight());
		art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, BgDark());
		art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, BgDark());
		art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR, Fg());
		art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR, FgDark());
		art->SetColour(wxAUI_DOCKART_BORDER_COLOUR, Border());
		art->SetColour(wxAUI_DOCKART_GRIPPER_COLOUR, Comment());
		manager->SetArtProvider(art);
	}

	void StyleWindow(wxWindow* window) {
		// GL canvases (map view, loading bar) render their own content.
		if (window->IsKindOf(CLASSINFO(wxGLCanvas))) {
			return;
		}

		// Menus and menu bars are native; the dark appearance handles them.
		if (window->IsKindOf(CLASSINFO(wxMenuBar)) || window->IsKindOf(CLASSINFO(wxMenu))) {
			return;
		}

		if (wxAuiNotebook* notebook = wxDynamicCast(window, wxAuiNotebook)) {
			notebook->SetBackgroundColour(BgDark());
			notebook->SetForegroundColour(Fg());

			auto* art = newd wxAuiDefaultTabArt();
			art->SetColour(BgDark());
			art->SetActiveColour(BgHighlight());
			notebook->SetArtProvider(art);
			return;
		}

		if (window->IsKindOf(CLASSINFO(wxAuiTabCtrl))) {
			window->SetBackgroundColour(BgDark());
			window->SetForegroundColour(Fg());
			return;
		}

		if (window->IsKindOf(CLASSINFO(wxAuiToolBar))) {
			window->SetBackgroundColour(BgDark());
			window->SetForegroundColour(Fg());
			return;
		}

		if (wxVListBox* vlist = wxDynamicCast(window, wxVListBox)) {
			vlist->SetBackgroundColour(InputBg());
			vlist->SetForegroundColour(Fg());
			vlist->SetSelectionBackground(SelectionBg());
			return;
		}

		// Text controls (wxSearchCtrl and wxComboBox derive from wxTextCtrl).
		if (window->IsKindOf(CLASSINFO(wxTextCtrl))) {
			window->SetBackgroundColour(InputBg());
			window->SetForegroundColour(Fg());
			return;
		}

		if (window->IsKindOf(CLASSINFO(wxListBox)) ||
			window->IsKindOf(CLASSINFO(wxListCtrl)) ||
			window->IsKindOf(CLASSINFO(wxTreeCtrl)) ||
			window->IsKindOf(CLASSINFO(wxDataViewCtrl)) ||
			window->IsKindOf(CLASSINFO(wxGrid))) {
			window->SetBackgroundColour(InputBg());
			window->SetForegroundColour(Fg());
			return;
		}

		if (window->IsKindOf(CLASSINFO(wxButton)) ||
			window->IsKindOf(CLASSINFO(wxToggleButton)) ||
			window->IsKindOf(CLASSINFO(wxBitmapButton))) {
			window->SetBackgroundColour(BgHighlight());
			window->SetForegroundColour(Fg());
			return;
		}

		if (window->IsKindOf(CLASSINFO(wxChoice)) ||
			window->IsKindOf(CLASSINFO(wxComboBox))) {
			window->SetBackgroundColour(InputBg());
			window->SetForegroundColour(Fg());
			return;
		}

		if (window->IsKindOf(CLASSINFO(wxStaticText)) ||
			window->IsKindOf(CLASSINFO(wxStaticBitmap))) {
			// Preserve explicitly-set colours (e.g. red validation messages).
			if (window->GetForegroundColour() != *wxRED) {
				window->SetForegroundColour(Fg());
			}
			wxWindow* parent = window->GetParent();
			window->SetBackgroundColour(parent != nullptr ? parent->GetBackgroundColour() : Bg());
			return;
		}

		// Plain containers (frames, dialogs, panels, scrolled windows...).
		window->SetBackgroundColour(Bg());
		window->SetForegroundColour(Fg());
	}

} // namespace

void Initialize() {
	if (wxTheApp != nullptr) {
		wxTheApp->SetAppearance(wxApp::Appearance::Dark);
	}
}

void Apply(wxWindow* window) {
	if (window == nullptr) {
		return;
	}

	ApplyAuiManagerArt();
	StyleWindow(window);

	for (wxWindow* child : window->GetChildren()) {
		Apply(child);
	}
}

} // namespace Theme
