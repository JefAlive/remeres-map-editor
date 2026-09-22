// Generated with ImRAD 0.9.1
// visit https://github.com/tpecholt/imrad

#include "rme.h"
#include "rme_widget.h"
#include "gui.h"
#include "main_menubar.h"

Rme g_rme;


void Rme::Draw(wxWindow* canvas)
{
    /// @dpi-info 93.2912,1
    /// @style Dark
    /// @unit px
    /// @begin TopWindow
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    // Transparent root window: the live map painted on the full canvas shows
    // through everywhere an opaque panel is not drawn, and mouse input falls
    // through to the editor wherever no layout widget is hovered.
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ (float)canvas->GetClientSize().GetWidth(), (float)canvas->GetClientSize().GetHeight() });
    bool tmpOpen;
    if (ImGui::Begin("###Rme", &tmpOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMouseInputs))
    {
        RmeLayout::clearMapRects();
        /// @separator

        // TODO: Add Draw calls of dependent popup windows here

        /// @begin Child
        vb1.BeginLayout();
        hb1.BeginLayout();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
        if (ImGui::BeginChild("child1", { hb1.GetSize(), vb1.GetSize() }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
        {
            /// @separator

            /// @begin Child
                vb01.BeginLayout();
                hb01.BeginLayout();
                RmeLayout::drawPanelSizers();
                ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                if (ImGui::BeginChild("child1", { RmeLayout::leftPanelWidth(), vb01.GetSize() }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar))
                {
                /// @separator

                /// @begin TabBar
                vb001.BeginLayout();
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 6 });
                if (ImGui::BeginTabBar("tabBar1", 0))
                {
                    /// @separator

                    /// @begin TabItem
                    if (ImGui::BeginTabItem("Objects", nullptr, ImGuiTabItemFlags_None))
                    {
                        /// @separator

                        /// @separator
                        ImGui::EndTabItem();
                    }
                    /// @end TabItem

                    /// @begin TabItem
                    if (ImGui::BeginTabItem("Towns", nullptr, ImGuiTabItemFlags_None))
                    {
                        /// @separator

                        /// @separator
                        ImGui::EndTabItem();
                    }
                    /// @end TabItem

                    /// @begin TabItem
                    if (ImGui::BeginTabItem("Houses", nullptr, ImGuiTabItemFlags_None))
                    {
                        /// @separator

                        /// @separator
                        ImGui::EndTabItem();
                    }
                    /// @end TabItem

                    /// @begin TabItem
                    if (ImGui::BeginTabItem("Zones", nullptr, ImGuiTabItemFlags_None))
                    {
                        /// @separator

                        /// @separator
                        ImGui::EndTabItem();
                    }
                    /// @end TabItem

                    /// @begin TabItem
                    if (ImGui::BeginTabItem("Project", nullptr, ImGuiTabItemFlags_None))
                    {
                        /// @separator

                        /// @separator
                        ImGui::EndTabItem();
                    }
                    /// @end TabItem

                    /// @separator
                    ImGui::EndTabBar();
                }
                vb001.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                ImGui::PopStyleVar();
                /// @end TabBar

                /// @begin Child
                hb002.BeginLayout();
                ImRad::Spacing(-1);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5, 5 });
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
                if (ImGui::BeginChild("child2", { hb002.GetSize(), vb001.GetSize() }, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
                {
                    /// @separator

                    /// @begin Text
                    vb0011.BeginLayout();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Filter: City / Biome");
                    vb0011.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Combo
                    hb0012.BeginLayout();
                    ImRad::Spacing(-1);
                    ImGui::SetNextItemWidth(hb0012.GetSize());
                    ImRad::Combo("##value6", &value6, "[8.0] Svargrond - Ice & Viking Isle\000", 0);
                    vb0011.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    hb0012.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    /// @end Combo

                    /// @begin Separator
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Horizontal);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Separator

                    /// @begin Text
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Filter: Object Type");
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Combo
                    hb0015.BeginLayout();
                    ImGui::SetNextItemWidth(hb0015.GetSize());
                    ImRad::Combo("##value10", &value10, "Nature Grounds\000", 0);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    hb0015.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    /// @end Combo

                    /// @begin Separator
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Horizontal);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Separator

                    /// @begin Text
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Filter: Item Name");
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Input
                    hb0018.BeginLayout();
                    ImGui::SetNextItemWidth(hb0018.GetSize());
                    if (ImGui::InputTextWithHint("##value9", "Type anything to filter...", value9.InputBuf, IM_ARRAYSIZE(value9.InputBuf), 0))
                    {
                        value9.Build();
                    }
                    if (ImGui::IsItemActive())
                        ImRad::GetUserData().imeType = ImRad::ImeText;
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    hb0018.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    /// @end Input

                    /// @begin Separator
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Horizontal);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Separator

                    /// @begin Child
                    hb00110.BeginLayout();
                    if (ImGui::BeginChild("child2", { hb00110.GetSize(), vb0011.GetSize() }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoSavedSettings))
                    {
                        /// @separator

                        /// @separator
                    }
                    ImGui::EndChild();
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
                    hb00110.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    /// @end Child

                    /// @begin Separator
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Horizontal);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Separator

                    /// @begin Text
                    ImRad::Spacing(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Selection");
                    vb0011.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Button
                    RmeLayout::SelectionModeButton("Single Select");
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushButton("Eraser", g_gui.eraser);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushButton("Border", g_gui.optional_brush);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Text
                    ImRad::Spacing(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Zones");
                    vb0011.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Button
                    RmeLayout::BrushButton("PZ", g_gui.pz_brush);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushButton("NoPvp", g_gui.rook_brush);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushButton("Pvp", g_gui.pvp_brush);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushButton("BlockLogout", g_gui.nolog_brush);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Text
                    ImRad::Spacing(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Doors");
                    vb0011.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Button
                    RmeLayout::BrushButton("Normal##Door", g_gui.normal_door_brush);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushButton("Quest", g_gui.quest_door_brush);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushButton("Locked", g_gui.locked_door_brush);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushButton("Magic", g_gui.magic_door_brush);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Text
                    ImRad::Spacing(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Windows");
                    vb0011.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Button
                    RmeLayout::BrushButton("Normal##Window", g_gui.window_door_brush);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushButton("Hatched", g_gui.hatch_door_brush);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Text
                    ImRad::Spacing(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Auto-bordering");
                    vb0011.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin CheckBox
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                    ImGui::Checkbox("Active", &value16);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleVar();
                    /// @end CheckBox

                    /// @begin Text
                    ImRad::Spacing(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Brush Thickness");
                    vb0011.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Slider
                    hb00123.BeginLayout();
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                    ImGui::SetNextItemWidth(hb00123.GetSize());
                    ImGui::SliderInt("##value14", &value14, 0, 10, nullptr, 0);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    hb00123.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    ImGui::PopStyleVar();
                    /// @end Slider

                    /// @begin Text
                    ImRad::Spacing(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Brush Size");
                    vb0011.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Slider
                    hb00125.BeginLayout();
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                    ImGui::SetNextItemWidth(hb00125.GetSize());
                    ImGui::SliderInt("##value15", &value15, 0, 10, nullptr, 0);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    hb00125.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    ImGui::PopStyleVar();
                    /// @end Slider

                    /// @begin Text
                    ImRad::Spacing(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    ImGui::TextUnformatted("Brush Type");
                    vb0011.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    ImGui::PopStyleColor();
                    /// @end Text

                    /// @begin Button
                    RmeLayout::BrushShapeButton("Circle", BRUSHSHAPE_CIRCLE);
                    vb0011.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::BrushShapeButton("Square", BRUSHSHAPE_SQUARE);
                    vb0011.UpdateSize(0, ImRad::VBox::ItemSize);
                    /// @end Button

                    /// @separator
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
                vb001.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
                hb002.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                /// @end Child

                /// @separator
            }
            ImGui::EndChild();
            vb01.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
            hb01.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, RmeLayout::leftPanelWidth());
            /// @end Child

            /// @begin Child
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
            if (ImGui::BeginChild("child5", { hb01.GetSize(), vb01.GetSize() }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
            {
                /// @separator

                /// @begin TabBar
                vb011.BeginLayout();
                ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 6 });
                if (ImGui::BeginTabBar("tabBar5", ImGuiTabBarFlags_Reorderable))
                {
                    /// @separator

                    /// @begin TabItem
                    bool _open6 = true;
                    if (ImGui::BeginTabItem("Global.otbm", &_open6, ImGuiTabItemFlags_None))
                    {
                        /// @separator

                        /// @separator
                        ImGui::EndTabItem();
                    }
                    /// @end TabItem

                    /// @begin TabItem
                    bool _open7 = true;
                    if (ImGui::BeginTabItem("YurOTs.otbm", &_open7, ImGuiTabItemFlags_None))
                    {
                        /// @separator

                        /// @separator
                        ImGui::EndTabItem();
                    }
                    /// @end TabItem

                    /// @begin TabItem
                    bool _open8 = true;
                    if (ImGui::BeginTabItem("CastleWar.otbm", &_open8, ImGuiTabItemFlags_None))
                    {
                        /// @separator

                        /// @separator
                        ImGui::EndTabItem();
                    }
                    /// @end TabItem

                    /// @begin TabItem
                    bool _open9 = true;
                    if (ImGui::BeginTabItem("Bomberman.otbm", &_open9, ImGuiTabItemFlags_None))
                    {
                        /// @separator

                        /// @separator
                        ImGui::EndTabItem();
                    }
                    /// @end TabItem

                    /// @separator
                    ImGui::EndTabBar();
                }
                vb011.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                ImGui::PopStyleVar();
                /// @end TabBar

                /// @begin Child
                hb012.BeginLayout();
                ImRad::Spacing(-1);
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                if (ImGui::BeginChild("child10", { hb012.GetSize(), vb011.GetSize() }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                {
                    // The live map: MapDrawer renders the scene into an offscreen
                    // surface and this presents it as a texture inside the layout,
                    // recording the drawn rect as the map viewport.
                    RmeLayout::DrawLiveMap(canvas, hb012.GetSize(), vb011.GetSize());
                    /// @separator
                    auto cpos10 = ImRad::GetCursorData();
                    ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                    /// @separator
                    /// @begin Child
                    ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().GetCenter().x-224, ImRad::GetParentInnerRect().Max.y-68 }); //overlayPos=AlignHCenter|AlignBottom,-224,-68
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
                    if (ImGui::BeginChild("child10", { 432, 48 }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                    {
                        /// @separator

                        /// @begin Child
                        if (ImGui::BeginChild("child10", { 48, 48 }, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                        {
                            /// @separator

                            // Empty hotbar slot: favorites will be shown here.

                            /// @separator
                            auto cpos10 = ImRad::GetCursorData();
                            ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                            /// @separator
                            /// @begin Text
                            ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Min.x+0, ImRad::GetParentInnerRect().Min.y+0 }); //overlayPos=AlignLeft|AlignTop,0,0
                            ImGui::TextUnformatted("1");
                            /// @end Text

                            /// @separator
                            ImGui::PopClipRect();
                            ImRad::SetCursorData(cpos10);
                        }
                        ImGui::EndChild();
                        /// @end Child

                        /// @begin Child
                        ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                        if (ImGui::BeginChild("child11", { 48, 48 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                        {
                            /// @separator

                            /// @separator
                            auto cpos11 = ImRad::GetCursorData();
                            ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                            /// @separator
                            /// @begin Text
                            ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Min.x+0, ImRad::GetParentInnerRect().Min.y+0 }); //overlayPos=AlignLeft|AlignTop,0,0
                            ImGui::TextUnformatted("2");
                            /// @end Text

                            /// @separator
                            ImGui::PopClipRect();
                            ImRad::SetCursorData(cpos11);
                        }
                        ImGui::EndChild();
                        /// @end Child

                        /// @begin Child
                        ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                        if (ImGui::BeginChild("child12", { 48, 48 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                        {
                            /// @separator

                            /// @separator
                            auto cpos12 = ImRad::GetCursorData();
                            ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                            /// @separator
                            /// @begin Text
                            ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Min.x+0, ImRad::GetParentInnerRect().Min.y+0 }); //overlayPos=AlignLeft|AlignTop,0,0
                            ImGui::TextUnformatted("3");
                            /// @end Text

                            /// @separator
                            ImGui::PopClipRect();
                            ImRad::SetCursorData(cpos12);
                        }
                        ImGui::EndChild();
                        /// @end Child

                        /// @begin Child
                        ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                        if (ImGui::BeginChild("child13", { 48, 48 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                        {
                            /// @separator

                            /// @separator
                            auto cpos13 = ImRad::GetCursorData();
                            ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                            /// @separator
                            /// @begin Text
                            ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Min.x+0, ImRad::GetParentInnerRect().Min.y+0 }); //overlayPos=AlignLeft|AlignTop,0,0
                            ImGui::TextUnformatted("4");
                            /// @end Text

                            /// @separator
                            ImGui::PopClipRect();
                            ImRad::SetCursorData(cpos13);
                        }
                        ImGui::EndChild();
                        /// @end Child

                        /// @begin Child
                        ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                        if (ImGui::BeginChild("child14", { 48, 48 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                        {
                            /// @separator

                            /// @separator
                            auto cpos14 = ImRad::GetCursorData();
                            ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                            /// @separator
                            /// @begin Text
                            ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Min.x+0, ImRad::GetParentInnerRect().Min.y+0 }); //overlayPos=AlignLeft|AlignTop,0,0
                            ImGui::TextUnformatted("5");
                            /// @end Text

                            /// @separator
                            ImGui::PopClipRect();
                            ImRad::SetCursorData(cpos14);
                        }
                        ImGui::EndChild();
                        /// @end Child

                        /// @begin Child
                        ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                        if (ImGui::BeginChild("child15", { 48, 48 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                        {
                            /// @separator

                            /// @separator
                            auto cpos15 = ImRad::GetCursorData();
                            ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                            /// @separator
                            /// @begin Text
                            ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Min.x+0, ImRad::GetParentInnerRect().Min.y+0 }); //overlayPos=AlignLeft|AlignTop,0,0
                            ImGui::TextUnformatted("6");
                            /// @end Text

                            /// @separator
                            ImGui::PopClipRect();
                            ImRad::SetCursorData(cpos15);
                        }
                        ImGui::EndChild();
                        /// @end Child

                        /// @begin Child
                        ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                        if (ImGui::BeginChild("child16", { 48, 48 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                        {
                            /// @separator

                            /// @separator
                            auto cpos16 = ImRad::GetCursorData();
                            ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                            /// @separator
                            /// @begin Text
                            ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Min.x+0, ImRad::GetParentInnerRect().Min.y+0 }); //overlayPos=AlignLeft|AlignTop,0,0
                            ImGui::TextUnformatted("7");
                            /// @end Text

                            /// @separator
                            ImGui::PopClipRect();
                            ImRad::SetCursorData(cpos16);
                        }
                        ImGui::EndChild();
                        /// @end Child

                        /// @begin Child
                        ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                        if (ImGui::BeginChild("child17", { 48, 48 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                        {
                            /// @separator

                            /// @separator
                            auto cpos17 = ImRad::GetCursorData();
                            ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                            /// @separator
                            /// @begin Text
                            ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Min.x+0, ImRad::GetParentInnerRect().Min.y+0 }); //overlayPos=AlignLeft|AlignTop,0,0
                            ImGui::TextUnformatted("8");
                            /// @end Text

                            /// @separator
                            ImGui::PopClipRect();
                            ImRad::SetCursorData(cpos17);
                        }
                        ImGui::EndChild();
                        /// @end Child

                        /// @begin Child
                        ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                        if (ImGui::BeginChild("child18", { 48, 48 }, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs))
                        {
                            /// @separator

                            /// @separator
                            auto cpos18 = ImRad::GetCursorData();
                            ImGui::PushClipRect(ImRad::GetParentInnerRect().Min, ImRad::GetParentInnerRect().Max, false);
                            /// @separator
                            /// @begin Text
                            ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Min.x+0, ImRad::GetParentInnerRect().Min.y+0 }); //overlayPos=AlignLeft|AlignTop,0,0
                            ImGui::TextUnformatted("9");
                            /// @end Text

                            /// @separator
                            ImGui::PopClipRect();
                            ImRad::SetCursorData(cpos18);
                        }
                        ImGui::EndChild();
                        /// @end Child

                        /// @separator
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                    /// @end Child

                    /// @begin Child
                    ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().Max.x-48, ImRad::GetParentInnerRect().GetCenter().y-140 }); //overlayPos=AlignRight|AlignVCenter,-48,-140
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
                    if (ImGui::BeginChild("child20", { 24, 360 }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar))
                    {
                        RmeLayout::addMapKeepout(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
                        /// @separator

                        /// @begin Button
                        RmeLayout::FloorButton("+7", 0);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("+6", 1);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("+5", 2);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("+4", 3);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("+3", 4);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("+2", 5);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("+1", 6);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("0", 7);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("-1", 8);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("-2", 9);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("-3", 10);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("-4", 11);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("-5", 12);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("-6", 13);
                        /// @end Button

                        /// @begin Button
                        ImRad::Spacing(-1);
                        RmeLayout::FloorButton("-7", 14);
                        /// @end Button

                        /// @separator
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                    /// @end Child

                    /// @begin Child
                    ImGui::SetCursorScreenPos({ ImRad::GetParentInnerRect().GetCenter().x-240, ImRad::GetParentInnerRect().Min.y+20 }); //overlayPos=AlignHCenter|AlignTop,-240,20
                    RmeLayout::DrawNotifications();
                    /// @end Child

                    /// @separator
                    ImGui::PopClipRect();
                    ImRad::SetCursorData(cpos10);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                vb011.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
                hb012.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                if (ImGui::IsWindowAppearing())
                    ImGui::SetKeyboardFocusHere(-1);
                /// @end Child

                /// @begin Child
                hb013.BeginLayout();
                ImRad::Spacing(-1);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5, 5 });
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
                if (ImGui::BeginChild("child23", { hb013.GetSize(), 0 }, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar))
                {
                    RmeLayout::addMapKeepout(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
                    /// @separator

                    /// @begin Button
                    hb0121.BeginLayout();
                    RmeLayout::MenuButton("New", MenuBar::NEW);
                    hb0121.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::MenuButton("Open", MenuBar::OPEN);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::MenuButton("Save", MenuBar::SAVE);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::MenuButton("Save As", MenuBar::SAVE_AS);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::MenuButton("<", MenuBar::UNDO);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Button

                    /// @begin Button
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    RmeLayout::MenuButton(">", MenuBar::REDO);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Button

                    /// @begin Spacer
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImRad::Dummy({ hb0121.GetSize(), 20 });
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    /// @end Spacer

                    /// @begin Text
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    {
                        const auto& h = RmeLayout::getHoverInfo();
                        ImGui::Text("Position: [%d, %d, %d]", h.x, h.y, h.z);
                    }
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Text

                    /// @begin Separator
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Vertical);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Separator

                    /// @begin Text
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    {
                        const auto& h = RmeLayout::getHoverInfo();
                        if (h.itemId > 0) {
                            ImGui::Text("ItemId: %d", h.itemId);
                        } else {
                            ImGui::TextUnformatted("ItemId: -");
                        }
                    }
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Text

                    /// @begin Separator
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Vertical);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Separator

                    /// @begin Text
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    {
                        const auto& h = RmeLayout::getHoverInfo();
                        ImGui::Text("Name: %s", h.itemName.empty() ? "Nothing" : h.itemName.c_str());
                    }
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Text

                    /// @begin Separator
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Vertical);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Separator

                    /// @begin Text
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImGui::TextUnformatted("Navigation: WASD");
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Text

                    /// @begin Separator
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Vertical);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Separator

                    /// @begin Text
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImGui::TextUnformatted("Change Floors: Q & E");
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Text

                    /// @begin Separator
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Vertical);
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Separator

                    /// @begin Text
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    {
                        const auto& h = RmeLayout::getZoomInfo();
                        ImGui::Text("Zoom: %d%s", h.percentage, "%");
                    }
                    hb0121.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Text

                    /// @separator
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
                vb011.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                hb013.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                /// @end Child

                /// @separator
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            vb01.UpdateSize(0, ImRad::VBox::Stretch(1.0f));
            hb01.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
            /// @end Child

            /// @begin Child
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            if (ImGui::BeginChild("child25", { RmeLayout::rightPanelWidth(), vb01.GetSize() }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
            {
                /// @separator

                /// @begin Child
                vb021.BeginLayout();
                hb021.BeginLayout();
                if (ImGui::BeginChild("child25", { hb021.GetSize(), 22 }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar))
                {
                    /// @separator

                    /// @begin Slider
                    hb0201.BeginLayout();
                    ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                    ImGui::SetNextItemWidth(hb0201.GetSize());
                    ImGui::SliderInt("##value13", &value13, 0, 100, nullptr, 0);
                    hb0201.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    ImGui::PopStyleVar();
                    /// @end Slider

                    /// @begin Text
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImGui::TextUnformatted("World Light");
                    hb0201.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Text

                    /// @begin Separator
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Vertical);
                    hb0201.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Separator

                    /// @begin Text
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImGui::TextUnformatted("Active");
                    hb0201.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Text

                    /// @begin CheckBox
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                    ImGui::Checkbox("", &value4);
                    hb0201.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    ImGui::PopStyleVar();
                    /// @end CheckBox

                    /// @separator
                }
                ImGui::EndChild();
                vb021.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, 22);
                hb021.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                /// @end Child

                /// @begin Child
                hb022.BeginLayout();
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
                if (ImGui::BeginChild("child26", { hb022.GetSize(), 200 }, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
                {
                    /// @separator

                    /// @begin Content
                    vb0211.BeginLayout();
                    hb0211.BeginLayout();
                    RmeLayout::DrawMinimap(hb0211.GetSize(), vb0211.GetSize());
                    vb0211.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
                    hb0211.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    /// @end Content

                    /// @separator
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                vb021.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, 200);
                hb022.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                /// @end Child

                /// @begin Child
                hb023.BeginLayout();
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5, 5 });
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
                if (ImGui::BeginChild("child27", { hb023.GetSize(), 400 }, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
                {
                    /// @separator

                    /// @begin Text
                    vb0221.BeginLayout();
                    ImGui::TextUnformatted("Tile properties");
                    vb0221.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Text

                    /// @begin Separator
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Horizontal);
                    vb0221.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Separator

                    /// @begin Child
                    hb0223.BeginLayout();
                    if (ImGui::BeginChild("child27", { hb0223.GetSize(), vb0221.GetSize() }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
                    {
                        /// @separator

                        /// @begin Text
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Walking");
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("Ground Speed: 220");
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("Block Path: false");
                        /// @end Text

                        /// @begin Text
                        ImRad::Spacing(3);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Zones");
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("No Logout Zone: false");
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("Protection Zone: false");
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("PVP Zone: false");
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("No PVP Zone: false");
                        /// @end Text

                        /// @begin Text
                        ImRad::Spacing(3);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Misc");
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("Block Projectile: false");
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("Has Teleport: false");
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("Has Unique Id: false");
                        /// @end Text

                        /// @begin Text
                        ImGui::TextUnformatted("Has Action Id: false");
                        /// @end Text

                        /// @separator
                    }
                    ImGui::EndChild();
                    vb0221.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
                    hb0223.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    /// @end Child

                    /// @begin Separator
                    ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Vertical);
                    vb0221.UpdateSize(0, ImRad::VBox::ItemSize);
                    hb0223.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::ItemSize);
                    /// @end Separator

                    /// @begin Child
                    ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                    if (ImGui::BeginChild("child28", { hb0223.GetSize(), vb0221.GetSize() }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoSavedSettings))
                    {
                        /// @separator

                        /// @begin Image
                        if (!value32)
                            value32 = ImRad::LoadTextureFromFile("C:/Users/T-Gamer/Downloads/Spike_Sword.gif");
                        ImGui::Image(value32.id, { 36, 36 }, { 0, 0 }, { 1, 1 }); //StretchPolicy::Scale
                        /// @end Image

                        /// @begin Text
                        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                        ImGui::TextUnformatted("Sword of Fury");
                        /// @end Text

                        /// @begin Image
                        if (!value33)
                            value33 = ImRad::LoadTextureFromFile("C:/Users/T-Gamer/Downloads/Fire_Field.gif");
                        ImGui::Image(value33.id, { 36, 36 }, { 0, 0 }, { 1, 1 }); //StretchPolicy::Scale
                        /// @end Image

                        /// @begin Text
                        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                        ImGui::TextUnformatted("Fire");
                        /// @end Text

                        /// @begin Image
                        if (!value34)
                            value34 = ImRad::LoadTextureFromFile("C:/Users/T-Gamer/Downloads/Grass.gif");
                        ImGui::Image(value34.id, { 36, 36 }, { 0, 0 }, { 1, 1 }); //StretchPolicy::Scale
                        /// @end Image

                        /// @begin Text
                        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                        ImGui::TextUnformatted("Grass");
                        /// @end Text

                        /// @separator
                    }
                    ImGui::EndChild();
                    vb0221.UpdateSize(0, ImRad::VBox::Stretch(1.0f));
                    hb0223.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    /// @end Child

                    /// @separator
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
                vb021.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, 400);
                hb023.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                /// @end Child

                /// @begin Child
                hb024.BeginLayout();
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5, 5 });
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
                if (ImGui::BeginChild("child30", { hb024.GetSize(), vb021.GetSize() }, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
                {
                    /// @separator

                    /// @begin Text
                    vb0231.BeginLayout();
                    ImGui::TextUnformatted("Item properties");
                    vb0231.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Text

                    /// @begin Separator
                    ImRad::SeparatorEx(ImRad::SeparatorFlags_Horizontal);
                    vb0231.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                    /// @end Separator

                    /// @begin Child
                    hb0233.BeginLayout();
                    if (ImGui::BeginChild("child30", { hb0233.GetSize(), vb0231.GetSize() }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
                    {
                        /// @separator

                        /// @begin Text
                        vb02321.BeginLayout();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Stack");
                        vb02321.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Button
                        hb02322.BeginLayout();
                        ImGui::Button("Move Up", { 80, 0 });
                        vb02321.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        hb02322.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, 80);
                        /// @end Button

                        /// @begin Button
                        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                        ImGui::Button("Move Down", { 80, 0 });
                        vb02321.UpdateSize(0, ImRad::VBox::ItemSize);
                        hb02322.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, 80);
                        /// @end Button

                        /// @begin Spacer
                        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                        ImRad::Dummy({ hb02322.GetSize(), 20 });
                        vb02321.UpdateSize(0, 20);
                        hb02322.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                        /// @end Spacer

                        /// @begin Button
                        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
                        ImGui::PushStyleColor(ImGuiCol_Button, 0xff6767ff);
                        ImGui::Button("Delete", { 80, 0 });
                        vb02321.UpdateSize(0, ImRad::VBox::ItemSize);
                        hb02322.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, 80);
                        ImGui::PopStyleColor();
                        /// @end Button

                        /// @begin Text
                        ImRad::Spacing(1);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Item ID");
                        vb02321.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Input
                        hb02324.BeginLayout();
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                        ImGui::SetNextItemWidth(hb02324.GetSize());
                        ImGui::InputInt("##value43", &value43, 1);
                        if (ImGui::IsItemActive())
                            ImRad::GetUserData().imeType = ImRad::ImeNumber;
                        vb02321.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        hb02324.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                        ImGui::PopStyleVar();
                        /// @end Input

                        /// @begin Text
                        ImRad::Spacing(1);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Unique ID");
                        vb02321.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Input
                        hb02326.BeginLayout();
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                        ImGui::SetNextItemWidth(hb02326.GetSize());
                        ImGui::InputInt("##value44", &value44, 1);
                        if (ImGui::IsItemActive())
                            ImRad::GetUserData().imeType = ImRad::ImeText;
                        vb02321.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        hb02326.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                        ImGui::PopStyleVar();
                        /// @end Input

                        /// @begin Text
                        ImRad::Spacing(1);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Action ID");
                        vb02321.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Input
                        hb02328.BeginLayout();
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                        ImGui::SetNextItemWidth(hb02328.GetSize());
                        ImGui::InputInt("##value45", &value45, 1);
                        if (ImGui::IsItemActive())
                            ImRad::GetUserData().imeType = ImRad::ImeText;
                        vb02321.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        hb02328.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                        ImGui::PopStyleVar();
                        /// @end Input

                        /// @begin Text
                        ImRad::Spacing(1);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Count");
                        vb02321.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Input
                        hb023210.BeginLayout();
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                        ImGui::SetNextItemWidth(hb023210.GetSize());
                        ImGui::InputInt("##value46", &value46, 1);
                        if (ImGui::IsItemActive())
                            ImRad::GetUserData().imeType = ImRad::ImeText;
                        vb02321.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        hb023210.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                        ImGui::PopStyleVar();
                        /// @end Input

                        /// @begin Text
                        ImRad::Spacing(1);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Text");
                        vb02321.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Input
                        hb023212.BeginLayout();
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                        ImGui::InputTextMultiline("##value38", &value38, { hb023212.GetSize(), vb02321.GetSize() }, 0);
                        if (ImGui::IsItemActive())
                            ImRad::GetUserData().imeType = ImRad::ImeText;
                        vb02321.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
                        hb023212.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                        ImGui::PopStyleVar();
                        /// @end Input

                        /// @begin Text
                        ImRad::Spacing(1);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                        ImGui::TextUnformatted("Container Slot");
                        vb02321.AddSize(2 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::ItemSize);
                        ImGui::PopStyleColor();
                        /// @end Text

                        /// @begin Child
                        hb023214.BeginLayout();
                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
                        if (ImGui::BeginChild("child30", { hb023214.GetSize(), vb02321.GetSize() }, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings))
                        {
                            /// @separator

                            /// @separator
                        }
                        ImGui::EndChild();
                        ImGui::PopStyleColor();
                        vb02321.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
                        hb023214.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                        /// @end Child

                        /// @separator
                    }
                    ImGui::EndChild();
                    vb0231.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
                    hb0233.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                    /// @end Child

                    /// @separator
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
                vb021.AddSize(1 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
                hb024.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
                /// @end Child

                /// @separator
            }
            ImGui::EndChild();
            vb01.UpdateSize(0, ImRad::VBox::Stretch(1.0f));
            hb01.AddSize(1 * ImGui::GetStyle().ItemSpacing.x, RmeLayout::rightPanelWidth());
            /// @end Child

            /// @separator
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        vb1.AddSize(0 * ImGui::GetStyle().ItemSpacing.y, ImRad::VBox::Stretch(1.0f));
        hb1.AddSize(0 * ImGui::GetStyle().ItemSpacing.x, ImRad::HBox::Stretch(1.0f));
        /// @end Child

        /// @separator
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    /// @end TopWindow
}

void Rme::ResetLayout()
{
    // ImGui::GetCurrentWindow()->HiddenFramesCannotSkipItems = 2;
    vb1.Reset();
    hb1.Reset();
    vb01.Reset();
    hb01.Reset();
    vb001.Reset();
    hb002.Reset();
    vb0011.Reset();
    hb0012.Reset();
    hb0015.Reset();
    hb0018.Reset();
    hb00110.Reset();
    hb00123.Reset();
    hb00125.Reset();
    vb011.Reset();
    hb012.Reset();
    hb013.Reset();
    hb0121.Reset();
    vb021.Reset();
    hb021.Reset();
    hb0201.Reset();
    hb022.Reset();
    vb0211.Reset();
    hb0211.Reset();
    hb023.Reset();
    vb0221.Reset();
    hb0223.Reset();
    hb024.Reset();
    vb0231.Reset();
    hb0233.Reset();
    vb02321.Reset();
    hb02322.Reset();
    hb02324.Reset();
    hb02326.Reset();
    hb02328.Reset();
    hb023210.Reset();
    hb023212.Reset();
    hb023214.Reset();
}
