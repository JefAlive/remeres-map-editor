// Generated with ImRAD 0.9.1
// visit https://github.com/tpecholt/imrad
// Adapted for RME: uses wxWidgets + ImGui instead of GLFW

#pragma once
#include "main.h"
#include "imrad/imrad.h"

class wxWindow;

class Rme
{
public:
    /// @begin interface
    void Draw(wxWindow* canvas);

    bool value4 = false;
    std::string value5;
    std::string value6;
    bool value7 = false;
    std::string value8;
    ImGuiTextFilter value9;
    std::string value10;
    ImRad::Color3 value11;
    float value12 = 0;
    int value13;
    int value14;
    int value15;
    bool value16 = false;
    ImRad::Color3 value19;
    int value35;
    std::string value36;
    std::string value37;
    std::string value38;
    std::string value39;
    std::string value40;
    std::string value41;
    std::string value42;
    int value43;
    int value44;
    int value45;
    int value46;
    /// @end interface

private:
    /// @begin impl
    void ResetLayout();

    ImRad::Texture value1;
    ImRad::Texture value2;
    float value3 = 100;
    ImRad::Texture value17;
    ImRad::Texture value18;
    ImRad::Texture value20;
    ImRad::Texture value21;
    ImRad::Texture value22;
    ImRad::Texture value23;
    ImRad::Texture value24;
    ImRad::Texture value25;
    ImRad::Texture value26;
    ImRad::Texture value27;
    ImRad::Texture value28;
    ImRad::Texture value29;
    ImRad::Texture value30;
    ImRad::Texture value31;
    ImRad::Texture value32;
    ImRad::Texture value33;
    ImRad::Texture value34;
    ImRad::VBox vb1;
    ImRad::HBox hb1;
    ImRad::VBox vb01;
    ImRad::HBox hb01;
    ImRad::VBox vb001;
    ImRad::HBox hb002;
    ImRad::VBox vb0011;
    ImRad::HBox hb0012;
    ImRad::HBox hb0015;
    ImRad::HBox hb0018;
    ImRad::HBox hb00110;
    ImRad::HBox hb00123;
    ImRad::HBox hb00125;
    ImRad::VBox vb011;
    ImRad::HBox hb012;
    ImRad::HBox hb013;
    ImRad::HBox hb0121;
    ImRad::VBox vb021;
    ImRad::HBox hb021;
    ImRad::HBox hb0201;
    ImRad::HBox hb022;
    ImRad::VBox vb0211;
    ImRad::HBox hb0211;
    ImRad::HBox hb023;
    ImRad::VBox vb0221;
    ImRad::HBox hb0223;
    ImRad::HBox hb024;
    ImRad::VBox vb0231;
    ImRad::HBox hb0233;
    ImRad::VBox vb02321;
    ImRad::HBox hb02322;
    ImRad::HBox hb02324;
    ImRad::HBox hb02326;
    ImRad::HBox hb02328;
    ImRad::HBox hb023210;
    ImRad::HBox hb023212;
    ImRad::HBox hb023214;
    /// @end impl
};

extern Rme g_rme;