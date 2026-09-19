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

#include "imgui_doodad_palette.h"
#include "materials.h"
#include "brush.h"
#include "tileset.h"
#include "graphics.h"
#include "gui.h"
#include "theme.h"
#include "gl_imgui_overlay.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>

namespace ImGuiDoodadPalette {

static Config g_config;
static State g_state;
static Brush* g_selected_brush = nullptr;
static bool g_initialized = false;

void Initialize() {
    if (g_initialized) {
        return;
    }

    // Wait for materials to be loaded
    if (g_materials.tilesets.empty()) {
        fprintf(stderr, "[ImGuiDoodadPalette] Waiting for materials to load (tilesets empty)\n");
        return;
    }

    // Load tileset names that have doodad category
    for (const auto& [name, tileset] : g_materials.tilesets) {
        const TilesetCategory* doodad_cat = tileset->getCategory(TILESET_DOODAD);
        size_t doodad_count = doodad_cat ? doodad_cat->size() : 0;
        fprintf(stderr, "[ImGuiDoodadPalette] Tileset: '%s', doodad count: %zu\n", name.c_str(), doodad_count);
        if (doodad_cat && doodad_cat->size() > 0) {
            g_state.tileset_names.push_back(name);
        }
    }

    // Sort tileset names alphabetically
    std::sort(g_state.tileset_names.begin(), g_state.tileset_names.end());

    // Select first tileset with doodads by default
    if (!g_state.tileset_names.empty()) {
        g_state.selected_tileset = g_state.tileset_names[0];
    }

    g_initialized = true;
}

void Shutdown() {
    g_state.current_brushes.clear();
    g_state.tileset_names.clear();
    g_selected_brush = nullptr;
    g_initialized = false;
}

void RefreshBrushList(State& state) {
    state.current_brushes.clear();

    if (state.selected_tileset.empty()) {
        return;
    }

    auto it = g_materials.tilesets.find(state.selected_tileset);
    if (it == g_materials.tilesets.end()) {
        return;
    }

    const TilesetCategory* doodad_cat = it->second->getCategory(TILESET_DOODAD);
    if (!doodad_cat) {
        return;
    }

    // Copy brush pointers (they're owned by the materials system)
    state.current_brushes.reserve(doodad_cat->size());
    for (Brush* brush : doodad_cat->brushlist) {
        if (brush) {
            state.current_brushes.push_back(brush);
        }
    }

    state.needs_refresh = false;
}

void SetSelectedBrush(Brush* brush) {
    g_selected_brush = brush;
}

Brush* GetSelectedBrush() {
    return g_selected_brush;
}

bool Render(wxWindow* canvas, const Config& config, State& state) {
    if (!canvas) {
        return false;
    }

    if (!ImGuiOverlay::ensureInitialized()) {
        return false;
    }

    const wxSize clientSize = canvas->GetClientSize();
    if (clientSize.x <= 0 || clientSize.y <= 0) {
        return false;
    }

    static std::chrono::steady_clock::time_point last_frame_time = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - last_frame_time).count();
    last_frame_time = now;
    if (deltaTime <= 0.0f || deltaTime > 0.25f) {
        deltaTime = 1.0f / 60.0f;
    }

    float scale = static_cast<float>(canvas->GetContentScaleFactor());
    if (scale <= 0.0f) {
        scale = 1.0f;
    }

    if (!g_initialized) {
        Initialize();
    }

    // Refresh brush list if needed
    if (state.needs_refresh || state.current_brushes.empty()) {
        RefreshBrushList(state);
    }

    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(clientSize.x), static_cast<float>(clientSize.y));
    io.DisplayFramebufferScale = ImVec2(scale, scale);
    io.DeltaTime = deltaTime;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (!config.show_window) {
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return false;
    }

    // Window flags
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(config.window_width, viewport->WorkSize.y));
    ImGui::SetNextWindowBgAlpha(0.95f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoBringToFrontOnFocus;

    bool show_window = config.show_window;
    if (!ImGui::Begin("Doodad Palette##ImGuiDoodadPalette", &show_window, flags)) {
        ImGui::End();
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return show_window;
    }

    // --- Tileset Selector ---
    ImGui::Text("Tileset");
    ImGui::SameLine();

    // Re-check for tilesets with doodads if materials might have loaded
    static size_t last_tileset_count = 0;
    if (g_materials.tilesets.size() != last_tileset_count) {
        last_tileset_count = g_materials.tilesets.size();
        // Re-initialize to pick up new tilesets
        g_state.tileset_names.clear();
        for (const auto& [name, tileset] : g_materials.tilesets) {
            const TilesetCategory* doodad_cat = tileset->getCategory(TILESET_DOODAD);
            if (doodad_cat && doodad_cat->size() > 0) {
                g_state.tileset_names.push_back(name);
            }
        }
        std::sort(g_state.tileset_names.begin(), g_state.tileset_names.end());
        if (!g_state.tileset_names.empty() && state.selected_tileset.empty()) {
            state.selected_tileset = g_state.tileset_names[0];
            state.needs_refresh = true;
        }
    }

    std::string preview = state.selected_tileset.empty() ? "Select tileset..." : state.selected_tileset;
    bool has_tilesets = !state.tileset_names.empty();

    if (!has_tilesets) {
        ImGui::BeginDisabled();
        preview = "No tilesets with doodads available";
    }

    if (ImGui::BeginCombo("##tileset_combo", preview.c_str(), ImGuiComboFlags_HeightLarge)) {
        if (has_tilesets) {
            for (const std::string& name : state.tileset_names) {
                bool is_selected = (state.selected_tileset == name);
                if (ImGui::Selectable(name.c_str(), is_selected)) {
                    state.selected_tileset = name;
                    state.needs_refresh = true;
                    state.selected_index = -1;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
        } else {
            ImGui::TextDisabled("Load a client with doodad tilesets");
        }
        ImGui::EndCombo();
    }

    if (!has_tilesets) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh##tileset_refresh", ImVec2(70, 0))) {
        // Force refresh tileset list
        g_state.tileset_names.clear();
        for (const auto& [name, tileset] : g_materials.tilesets) {
            const TilesetCategory* doodad_cat = tileset->getCategory(TILESET_DOODAD);
            if (doodad_cat && doodad_cat->size() > 0) {
                g_state.tileset_names.push_back(name);
            }
        }
        std::sort(g_state.tileset_names.begin(), g_state.tileset_names.end());
        if (!g_state.tileset_names.empty() && state.selected_tileset.empty()) {
            state.selected_tileset = g_state.tileset_names[0];
            state.needs_refresh = true;
        }
    }

    ImGui::Separator();

    // --- Grid View ---
    if (state.current_brushes.empty()) {
        ImGui::TextDisabled("No doodads in selected tileset");
    } else {
        // Calculate grid layout
        float avail_width = ImGui::GetContentRegionAvail().x;
        float item_width = config.item_size;
        int columns = std::max(1, static_cast<int>(avail_width / item_width));
        columns = std::min(columns, static_cast<int>(state.current_brushes.size()));

        if (ImGui::BeginTable("doodad_grid", columns, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_NoPadOuterX)) {
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(state.current_brushes.size()));
            clipper.IncludeItemsByIndex(0, static_cast<int>(state.current_brushes.size()));

            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    Brush* brush = state.current_brushes[row];
                    if (!brush) continue;

                    bool is_selected = (row == state.selected_index);
                    bool is_hovered = (row == state.hover_index);

                    ImGui::PushID(row);

                    // Item background
                    ImVec4 bg_color = is_selected ? ImGui::GetStyleColorVec4(ImGuiCol_Header)
                                                  : (is_hovered ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered)
                                                                : ImVec4(0, 0, 0, 0));
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::ColorConvertFloat4ToU32(bg_color));

                    // Draw brush placeholder
                    ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Header);
                    ImGui::GetWindowDrawList()->AddRectFilled(
                        ImGui::GetItemRectMin(),
                        ImGui::GetItemRectMax(),
                        ImGui::ColorConvertFloat4ToU32(color)
                    );
                    // Draw brush name centered
                    ImVec2 text_size = ImGui::CalcTextSize(brush->getName().c_str());
                    ImVec2 item_center = ImVec2(
                        (ImGui::GetItemRectMin().x + ImGui::GetItemRectMax().x) * 0.5f,
                        (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f
                    );
                    ImGui::GetWindowDrawList()->AddText(
                        ImVec2(item_center.x - text_size.x * 0.5f, item_center.y - text_size.y * 0.5f),
                        IM_COL32(255, 255, 255, 255),
                        brush->getName().c_str()
                    );

                    // Tooltip on hover
                    if (ImGui::IsItemHovered()) {
                        state.hover_index = row;
                        ImGui::BeginTooltip();
                        ImGui::Text("%s", brush->getName().c_str());
                        ImGui::Text("ID: %d", brush->getID());
                        ImGui::EndTooltip();
                    } else if (state.hover_index == row) {
                        state.hover_index = -1;
                    }

                    // Selection on click
                    if (ImGui::IsItemClicked()) {
                        state.selected_index = row;
                        g_selected_brush = state.current_brushes[row];
                        g_gui.SelectBrush(g_selected_brush);
                    }

                    ImGui::PopID();

                    // Move to next column
                    ImGui::TableNextColumn();
                }
            }
            clipper.End();

            ImGui::EndTable();
        }
    }

    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return config.show_window;
}

} // namespace ImGuiDoodadPalette