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

#ifndef RME_CANVAS_OVERLAY_H_
#define RME_CANVAS_OVERLAY_H_

#include <algorithm>

namespace CanvasOverlay {
	constexpr float kStatusBarHeight = 24.0f;
	constexpr float kScrollbarThickness = 10.0f;
	constexpr float kScrollbarMargin = 20.0f;
	constexpr float kScrollbarMinThumb = 40.0f;

	struct BarGeometry {
		bool visible = false;       // content is scrollable in this axis
		float track_start = 0.0f;   // along-axis start of track (px)
		float track_length = 0.0f;  // along-axis length of track (px)
		float thumb_start = 0.0f;   // along-axis start of thumb (px)
		float thumb_length = 0.0f;  // along-axis length of thumb (px)
		float cross_start = 0.0f;   // other-axis start of the bar (px)
		float thickness = 0.0f;     // bar thickness (px)
	};

	// client_w/client_h: canvas client size (logical px).
	// scroll/range: map-space pixels.
	// view: visible extent in map-space pixels (canvas_dim * zoom).
	// reserved_bottom: space reserved at the bottom for status footer (px).
	inline BarGeometry VerticalBar(float client_w, float client_h, float scroll, float range, float view, float reserved_bottom) {
		BarGeometry g;
		g.thickness = kScrollbarThickness;
		g.cross_start = client_w - kScrollbarMargin - kScrollbarThickness;
		g.track_start = kScrollbarMargin;
		g.track_length = client_h - reserved_bottom - kScrollbarMargin * 2.0f;
		if (g.track_length <= 0.0f || range <= 0.0f) return g;

		const float fraction = std::min(1.0f, view / range);
		g.thumb_length = std::max(kScrollbarMinThumb, g.track_length * fraction);
		if (g.thumb_length > g.track_length) g.thumb_length = g.track_length;

		const float max_scroll = std::max(0.0f, range - view);
		const float t = max_scroll > 0.0f ? std::min(1.0f, std::max(0.0f, scroll / max_scroll)) : 0.0f;
		g.thumb_start = g.track_start + (g.track_length - g.thumb_length) * t;
		g.visible = fraction < 0.999f;
		return g;
	}

	inline BarGeometry HorizontalBar(float client_w, float client_h, float scroll, float range, float view, float reserved_bottom) {
		BarGeometry g;
		g.thickness = kScrollbarThickness;
		g.cross_start = client_h - reserved_bottom - kScrollbarMargin - kScrollbarThickness;
		g.track_start = kScrollbarMargin;
		g.track_length = client_w - kScrollbarMargin * 2.0f;
		if (g.track_length <= 0.0f || range <= 0.0f) return g;

		const float fraction = std::min(1.0f, view / range);
		g.thumb_length = std::max(kScrollbarMinThumb, g.track_length * fraction);
		if (g.thumb_length > g.track_length) g.thumb_length = g.track_length;

		const float max_scroll = std::max(0.0f, range - view);
		const float t = max_scroll > 0.0f ? std::min(1.0f, std::max(0.0f, scroll / max_scroll)) : 0.0f;
		g.thumb_start = g.track_start + (g.track_length - g.thumb_length) * t;
		g.visible = fraction < 0.999f;
		return g;
	}

	// Helper for hit-testing: returns true if (mx, my) is over the vertical/horizontal scrollbar.
	inline bool HitTestVertical(const BarGeometry& g, float mx, float my) {
		if (!g.visible) return false;
		return mx >= g.cross_start - 2.0f && mx <= g.cross_start + g.thickness + 2.0f &&
		       my >= g.track_start - 2.0f && my <= g.track_start + g.track_length + 2.0f;
	}

	inline bool HitTestHorizontal(const BarGeometry& g, float mx, float my) {
		if (!g.visible) return false;
		return my >= g.cross_start - 2.0f && my <= g.cross_start + g.thickness + 2.0f &&
		       mx >= g.track_start - 2.0f && mx <= g.track_start + g.track_length + 2.0f;
	}

	// Returns true if the point is over the thumb (for dragging).
	inline bool HitTestThumbVertical(const BarGeometry& g, float mx, float my) {
		if (!g.visible) return false;
		return mx >= g.cross_start - 2.0f && mx <= g.cross_start + g.thickness + 2.0f &&
		       my >= g.thumb_start - 2.0f && my <= g.thumb_start + g.thumb_length + 2.0f;
	}

	inline bool HitTestThumbHorizontal(const BarGeometry& g, float mx, float my) {
		if (!g.visible) return false;
		return my >= g.cross_start - 2.0f && my <= g.cross_start + g.thickness + 2.0f &&
		       mx >= g.thumb_start - 2.0f && mx <= g.thumb_start + g.thumb_length + 2.0f;
	}
} // namespace CanvasOverlay

#endif // RME_CANVAS_OVERLAY_H_