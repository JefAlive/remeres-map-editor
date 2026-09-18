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

class wxWindow;

// Central dark theme for the editor, based on the "Tokyo Night" palette.
//
// The theme is applied in two steps:
//   1. Theme::Initialize() requests the dark appearance from the platform so
//      native widgets (menus, file pickers, message boxes) match the editor.
//   2. Theme::Apply(window) recursively colours a window and all of its
//      children. Call it after the main frame and its palettes are created,
//      and for every dialog/palette that is shown later.
namespace Theme {

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
