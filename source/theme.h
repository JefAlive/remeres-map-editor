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

#ifndef RME_THEME_H_
#define RME_THEME_H_

#include <wx/colour.h>

#include <cstdint>

class wxWindow;

// Editor theme system, switched at runtime from View > Theme.
//
// Every surface/ink colour is stored as a per-palette token table (this file
// only mirrors the accessor set). The Aura palette is the default dark theme
// and the source of the CSS-like tokens used on both sides of the UI:
//   - Theme::*() (wxColour) colours native windows, dialogs and palettes.
//   - Theme::Rgb(Token) feeds the ImGui style so the overlay tracks the same
//     palette without duplicating values.
//
// The theme is applied in steps:
//   1. Theme::Initialize() requests the dark appearance from the platform so
//      native widgets (menus, file pickers, message boxes) match the editor.
//   2. Theme::SetPalette(palette) switches the active palette and immediately
//      re-colours the whole frame tree.
//   3. Theme::Apply(window) recursively colours a window and all of its
//      children. Call it after the main frame and its palettes are created,
//      and for every dialog/palette that is shown later.
namespace Theme {

enum class Palette {
	Aura,
	SolarizedLight,
	Everforest,
	Carbonfox,
	Synthwave84,
	Tokyonight,
};

// Token names shared by the wx palette accessors and the ImGui style.
enum Token {
	TKN_Bg,
	TKN_Deep,      // darkest wells (title bars / deeply nested panels)
	TKN_Panel,     // lifted surfaces (panels, frames, headers)
	TKN_Popup,     // floating surfaces (popups, inputs)
	TKN_Border,
	TKN_Muted,     // comments, tree lines, disabled text
	TKN_Fg,        // ink
	TKN_FgMuted,   // secondary ink
	TKN_Blue,
	TKN_Cyan,
	TKN_Green,
	TKN_Orange,
	TKN_Purple,
	TKN_Pink,
	TKN_Red,
	TKN_Yellow,
	TKN_Count,
};

Palette CurrentPalette();

// Switch the active palette and re-colour the window tree in place.
// Pass apply=false to only switch the internal palette value without touching
// the window tree -- safe while the frame/AUI manager is still being built
// (e.g. restoring a persisted palette from the menu bar's LoadValues, before
// the frame is fully realised). The real re-colour then happens when the app
// calls Theme::Apply(root) at startup.
void SetPalette(Palette palette, bool apply = true);

// Raw #RRGGBB value (0xrrggbb) of a token in the active palette.
uint32_t Rgb(Token token);

// Backgrounds
wxColour Bg();
wxColour BgDark();
wxColour BgHighlight();
wxColour Border();
wxColour InputBg();

// Text
wxColour Fg();
wxColour FgDark();
wxColour Comment();

// Accents
wxColour Blue();
wxColour Cyan();
wxColour Green();
wxColour Orange();
wxColour Purple();
wxColour Red();
wxColour Yellow();

// Selection / highlight
wxColour SelectionBg();
wxColour SelectionFg();

// Request the dark appearance from the platform. Safe to call once during
// Application::OnInit.
void Initialize();

// Recursively apply the palette to a window and all of its children.
// Safe to call more than once.
void Apply(wxWindow* window);

} // namespace Theme

#endif
