#pragma once

#include <wx/window.h>

#include <string>

class MapCanvas;
class Brush;

// Bridge between the wxWidgets MapCanvas and the ImRAD-generated editor layout
// (Rme). The layout is an always-on overlay: every paint draws it on top of the
// map, and wx mouse/keyboard events are forwarded into ImGui. When ImGui wants
// input capture, the map stops processing the corresponding events.
namespace RmeLayout {

	// The ImGui frame is split so the map can be drawn *inside* the child10
	// viewport recorded by the layout before ImGui emits its draw data.
	// Begin() must be called with a current GL context (i.e. from
	// MapCanvas::OnPaint) and must be paired with a single End() call. Callers
	// draw the map between Begin() and End(); the Rme layout widgets are
	// submitted by Begin() and rendered by End().
	bool Begin(wxWindow* canvas);
	void End();
	// Convenience full frame (NewFrame -> Rme::Draw -> Render).
	void Render(wxWindow* canvas);

	// Returns the recorded screen-space rect of the transparent map viewport
	// (child10) in logical client pixels, relative to the canvas top-left.
	// Returns false when the layout is inactive or the rect was not recorded.
	bool getMapViewport(float& x, float& y, float& w, float& h);

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
	// True when the point lands on a keepout rect (ImGui-owned UI such as the
	// floor-button strip). Unlike isMapPoint, this does not depend on ImGui's
	// capture state, which lags one frame behind fast clicks: press handlers
	// use it to never drive the editor from overlay UI.
	bool isMapKeepout(int x, int y);

	// Recording the map viewport / keepout rects, called from the Rme layout
	// while it draws each frame.
	void clearMapRects();
	void setMapViewport(float x, float y, float w, float h);
	void addMapKeepout(float x, float y, float w, float h);

	// Draws the transparent rectangle where the live map shows (the spot the old
	// mapeditor.png placeholder occupied) and records it as the map viewport.
	// Everything outside it in the map area is opaque.
	void drawMapViewport(float width, float height);

	// Native ImGui replacement for the map right-click menu (MapPopupMenu).
	// The canvas arms it on right-button release over the live viewport via
	// openMapContextMenu(); DrawMapContextMenu() opens and draws the popup on
	// the next frame from DrawLiveMap, rendering the shared item model
	// (MapCanvas::CollectContextMenuItems) and dispatching to the same
	// MapCanvas::OnXxx callbacks as the wx menu.
	void openMapContextMenu();
	void DrawMapContextMenu(MapCanvas* canvas);

	// Fire the wx menu command behind a toggle/check item (SHOW_LIGHTS,
	// AUTOMAGIC, ...): toggles the actual menu check and dispatches the same
	// wxCommandEvent the menu would send, so ImGui widgets reuse the existing
	// callbacks (settings, refresh, status text) instead of duplicating them.
	// Mirrors the keyboard-shortcut path in MapCanvas::DispatchMenuShortcut.
	void FireMenuToggle(int actionId);
	// Same for the FLOOR_0..15 radio group: checks the radio for `floor`.
	void FireMenuFloor(int floor);
	// Floor-strip button (labels "+7".."0".."-7", i.e. floor = 7 - k):
	// accent-highlights the current floor and fires on click. Disabled with
	// no map open (same guard as the menu, which needs an editor).
	void FloorButton(const char* label, int floor);

	// Left-panel tool button: selects the editor brush on click (the same
	// g_gui.SelectBrush call the wx brush toolbar makes), accent-highlighted
	// while it is the current brush. Disabled with no map open.
	void BrushButton(const char* label, Brush* brush);
	// Left-panel "Single Select": enters selection mode, highlighted while
	// selection mode is active. Disabled with no map open.
	void SelectionModeButton(const char* label);
	// Brush shape (BRUSHSHAPE_CIRCLE / BRUSHSHAPE_SQUARE as int): same
	// g_gui.SetBrushShape call as the wx sizes toolbar, always marking the
	// current shape. Independent from tool selection. Disabled with no map.
	void BrushShapeButton(const char* label, int shape);

	// Transient notification toaster, rendered where the "notification popup"
	// placeholder sits (top-center over the map). Notify() queues a message
	// (thread-safe: some producers run on worker threads); DrawNotifications()
	// draws the live stack -- newest first, auto-expired after a few seconds.
	// Replaces the legacy wxInfoBar and the field-0 status messages.
	enum class ToastLevel {
		Info, // blue token: confirmations, status changes
		Warning, // orange token: load warnings, failures
	};
	void Notify(const std::string& text, ToastLevel level = ToastLevel::Info);
	void DrawNotifications();
	// Presents the live map (rendered by MapDrawer into its offscreen surface)
	// as an ImGui::Image inside the current window, aspect-fitted within the
	// given available space, and records the drawn rect as the map viewport so
	// camera, scrolling and input keep following the visible map region.
	void DrawLiveMap(wxWindow* canvas, float availWidth, float availHeight);

	// Current widths of the [left | center | right] row panels, updated by
	// drawPanelSizers() and read by the layout when sizing the panels.
	float leftPanelWidth();
	float rightPanelWidth();

	// Draggable vertical dividers at the left/right panel seams. Call once per
	// frame at the start of the row layout, before any panel child is drawn.
	void drawPanelSizers();

} // namespace RmeLayout