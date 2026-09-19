#pragma once

#include <wx/window.h>

// Bridge between the wxWidgets MapCanvas and the ImRAD-generated editor layout
// (Rme). The layout is an always-on overlay: every paint draws it on top of the
// map, and wx mouse/keyboard events are forwarded into ImGui. When ImGui wants
// input capture, the map stops processing the corresponding events.
namespace RmeLayout {

	// Full ImGui frame (NewFrame -> Rme::Draw -> Render) for the given canvas.
	// Must be called with a current GL context, i.e. from MapCanvas::OnPaint.
	void Render(wxWindow* canvas);

	// Input forwarding. Called from the MapCanvas event handlers.
	void forwardMouseMove(int x, int y);
	void forwardMouseButton(int button, bool down); // 0 left, 1 right, 2 middle
	void forwardMouseWheel(int rotation);
	void forwardKey(int keyCode, int unicodeChar, bool down, bool ctrl, bool shift,
					bool alt);

	// Whether ImGui currently wants to capture a given input channel. When true,
	// the map should not process that event.
	bool wantsCaptureMouse();
	bool wantsCaptureKeyboard();

	// Renders the live minimap of the current editor inside the current ImGui
	// window, aspect-fitted within the given available space. Call it in the Rme
	// layout wherever the minimap should appear.
	void DrawMinimap(float availWidth, float availHeight);

	// The always-on Rme layout is active and replaces the legacy status footer
	// and overlay scrollbars; callers should skip that drawing when true.
	bool isOverlayActive();

	// Hit-testing for the live map viewport. The Rme layout records the screen
	// rect of the transparent center (where the live map shows) and any keepout
	// rects (interactive overlays like the floor buttons). MapCanvas asks this
	// before deciding whether an ImGui capture should swallow a mouse event.
	bool isMapPoint(int x, int y);

	// Recording the map viewport / keepout rects, called from the Rme layout
	// while it draws each frame.
	void clearMapRects();
	void setMapViewport(float x, float y, float w, float h);
	void addMapKeepout(float x, float y, float w, float h);

	// Draws the transparent rectangle where the live map shows (the spot the old
	// mapeditor.png placeholder occupied) and records it as the map viewport.
	// Everything outside it in the map area is opaque.
	void drawMapViewport(float width, float height);

	// Current widths of the [left | center | right] row panels, updated by
	// drawPanelSizers() and read by the layout when sizing the panels.
	float leftPanelWidth();
	float rightPanelWidth();

	// Draggable vertical dividers at the left/right panel seams. Call once per
	// frame at the start of the row layout, before any panel child is drawn.
	void drawPanelSizers();

} // namespace RmeLayout