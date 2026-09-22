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

namespace {

	// Palette token tables, indexed by Theme::Palette then Theme::Token. The
	// Aura row matches the original hardcoded values; the rest follow the
	// official community palettes (Solarized Light, Everforest, nightfox
	// Carbonfox, Synthwave84, Tokyo Night) mapped onto the same token roles.
	struct PaletteRow {
		uint32_t c[TKN_Count];
	};

	constexpr PaletteRow kPalettes[] = {
		// Aura
		{
			0x15141b, 0x101016, 0x1c1c23, 0x23232b, 0x2d2d2d, 0x6d6a7e,
			0xedecee, 0x858298, 0x82e2ff, 0x61ffca, 0x9dff65, 0xffca85,
			0xa277ff, 0xf694ff, 0xff6767, 0xffb454,
		},
		// Solarized Light
		{
			0xfdf6e3, 0xeee8d5, 0xeee8d5, 0xfdf6e3, 0x93a1a1, 0x93a1a1,
			0x657b83, 0x839496, 0x268bd2, 0x2aa198, 0x859900, 0xcb4b16,
			0x6c71c4, 0xd33682, 0xdc322f, 0xb58900,
		},
		// Everforest
		{
			0x2d353b, 0x272e33, 0x343f44, 0x3d484d, 0x4a555b, 0x859289,
			0xd3c6aa, 0x9da9a0, 0x7fbbb3, 0x83c092, 0xa7c080, 0xe69875,
			0xd699b6, 0xe67e80, 0xe67e80, 0xdbbc7f,
		},
		// Carbonfox
		{
			0x161616, 0x0b0b0b, 0x262626, 0x2f2f2f, 0x444444, 0x8f8f8f,
			0xe2e2e3, 0x9d9d9d, 0x83b7d9, 0x8fd0bf, 0xa2b97c, 0xd08b62,
			0xb7a4d8, 0xd38aa0, 0xd97087, 0xd7b87a,
		},
		// Synthwave84
		{
			0x241b2f, 0x1a1223, 0x2a2139, 0x33284a, 0x3f3552, 0x8f87a8,
			0xefe9ff, 0xa89ec4, 0x36f9f6, 0x41e0b8, 0x72f1b8, 0xff9f6c,
			0xa48cf2, 0xff7edb, 0xfe4450, 0xffe66d,
		},
		// Tokyonight
		{
			0x1a1b26, 0x16161e, 0x24283b, 0x292e42, 0x414868, 0x565f89,
			0xc0caf5, 0xa9b1d6, 0x7aa2f7, 0x7dcfff, 0x9ece6a, 0xff9e64,
			0xbb9af7, 0xf7768e, 0xf7768e, 0xe0af68,
		},
	};

	static_assert(sizeof(kPalettes) / sizeof(kPalettes[0]) == 6, "palette table must cover every Theme::Palette");

	Palette s_palette = Palette::Aura;

	// The AUI dock art is re-applied only when the palette actually changed.
	// Replacing the art provider on every Apply (which also runs from
	// Application::FilterEvent on each wxEVT_SHOW during startup) swaps GTK
	// style providers on half-built panes and segfaults (GTK_IS_WIDGET /
	// GdkRGBA double-init). The original code applied it once; this keeps
	// that once-per-palette semantic while still tracking theme switches.
	Palette s_art_palette = Palette::Aura;
	bool s_art_applied = false;

	wxColour FromRgb(uint32_t rgb) {
		return wxColour((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
	}

	const uint32_t& TokenRgb(Token token) {
		return kPalettes[static_cast<int>(s_palette)].c[token];
	}

	wxColour TokenColour(Token token) {
		return FromRgb(TokenRgb(token));
	}

	void ApplyAuiManagerArt() {
		if (s_art_applied && s_art_palette == s_palette) {
			return;
		}
		wxAuiManager* manager = g_gui.GetAuiManager();
		if (manager == nullptr) {
			return;
		}
		// Fresh provider per application (matches the original code): the
		// manager takes ownership on SetArtProvider, so reusing one static
		// across palette switches would risk a double-free.
		wxAuiDockArt* art = newd wxAuiDefaultDockArt();

		art->SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR, TokenColour(TKN_Bg));
		art->SetColour(wxAUI_DOCKART_SASH_COLOUR, TokenColour(TKN_Deep));
		art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR, TokenColour(TKN_Panel));
		art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR, TokenColour(TKN_Panel));
		art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, TokenColour(TKN_Deep));
		art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, TokenColour(TKN_Deep));
		art->SetColour(wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR, TokenColour(TKN_Fg));
		art->SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR, TokenColour(TKN_FgMuted));
		art->SetColour(wxAUI_DOCKART_BORDER_COLOUR, TokenColour(TKN_Border));
		art->SetColour(wxAUI_DOCKART_GRIPPER_COLOUR, TokenColour(TKN_Muted));
		manager->SetArtProvider(art);
		s_art_palette = s_palette;
		s_art_applied = true;
	}

} // namespace

Palette CurrentPalette() {
	return s_palette;
}

uint32_t Rgb(Token token) {
	return TokenRgb(token);
}

void SetPalette(Palette palette, bool apply) {
	s_palette = palette;
	if (apply && g_gui.root != nullptr) {
		Theme::Apply(g_gui.root);
	}
}

wxColour Bg() {
	return TokenColour(TKN_Bg);
}

wxColour BgDark() {
	return TokenColour(TKN_Deep);
}

wxColour BgHighlight() {
	return TokenColour(TKN_Panel);
}

wxColour Border() {
	return TokenColour(TKN_Border);
}

wxColour InputBg() {
	return TokenColour(TKN_Popup);
}

wxColour Fg() {
	return TokenColour(TKN_Fg);
}

wxColour FgDark() {
	return TokenColour(TKN_FgMuted);
}

wxColour Comment() {
	return TokenColour(TKN_Muted);
}

wxColour Blue() {
	return TokenColour(TKN_Blue);
}

wxColour Cyan() {
	return TokenColour(TKN_Cyan);
}

wxColour Green() {
	return TokenColour(TKN_Green);
}

wxColour Orange() {
	return TokenColour(TKN_Orange);
}

wxColour Purple() {
	return TokenColour(TKN_Purple);
}

wxColour Red() {
	return TokenColour(TKN_Red);
}

wxColour Yellow() {
	return TokenColour(TKN_Yellow);
}

wxColour SelectionBg() {
	return Purple();
}

wxColour SelectionFg() {
	return Fg();
}

namespace {

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
