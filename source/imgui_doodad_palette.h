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

#ifndef RME_IMGUI_DOODAD_PALETTE_H_
#define RME_IMGUI_DOODAD_PALETTE_H_

#include <wx/window.h>
#include <vector>
#include <string>
#include <memory>

class Brush;
class Tileset;

namespace ImGuiDoodadPalette {

// Configuration for the palette window
struct Config {
    bool show_window = true;
    float window_width = 320.0f;
    float min_window_width = 200.0f;
    float max_window_width = 600.0f;
    int grid_columns = 4;
    float item_size = 64.0f; // pixels per item (including padding)
};

// State for the palette
struct State {
    std::string selected_tileset;
    std::vector<std::string> tileset_names;
    std::vector<Brush*> current_brushes;
    int hover_index = -1;
    int selected_index = -1;
    bool needs_refresh = true;
};

// Initialize the palette (call once at startup)
void Initialize();

// Shutdown the palette (call at application exit)
void Shutdown();

// Main render function - call from within an ImGui frame
// Returns true if the window is open
bool Render(wxWindow* canvas, const Config& config, State& state);

// Call when the doodad selection changes externally
void SetSelectedBrush(Brush* brush);

// Get the currently selected brush
Brush* GetSelectedBrush();

// Refresh the brush list for the current tileset
void RefreshBrushList(State& state);

} // namespace ImGuiDoodadPalette

#endif // RME_IMGUI_DOODAD_PALETTE_H_