/*
The MIT License (MIT)

Copyright (c) 2014 Jonathan Young

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <string>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "wz_main_window.h"
#include "wz_widget.h"
#include "wz_renderer.h"
#include "wz_skin.h"

#define WZ_WINDOW_UNDOCK_DISTANCE 16

typedef enum
{
	WZ_DRAG_NONE,
	WZ_DRAG_HEADER,
	WZ_DRAG_RESIZE_N,
	WZ_DRAG_RESIZE_NE,
	WZ_DRAG_RESIZE_E,
	WZ_DRAG_RESIZE_SE,
	WZ_DRAG_RESIZE_S,
	WZ_DRAG_RESIZE_SW,
	WZ_DRAG_RESIZE_W,
	WZ_DRAG_RESIZE_NW,
}
wzWindowDrag;

struct wzWindow
{
	struct wzWidget base;
	int drawPriority;
	int headerHeight;
	int borderSize;
	std::string title;
	
	struct wzWidget *content;

	wzWindowDrag drag;

	// Dragging a docked window header doesn't undock the window until the mouse has moved WZ_WINDOW_UNDOCK_DISTANCE.
	wzPosition undockStartPosition;

	wzPosition resizeStartPosition;
	wzRect resizeStartRect;

	// Remember the window size when it is docked, so when the window is undocked the size can be restored.
	wzSize sizeBeforeDocking;
};

static void wz_window_draw(struct wzWidget *widget, wzRect clip)
{
	const struct wzWindow *window = (struct wzWindow *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzRect rect = wz_widget_get_absolute_rect(widget);
	const wzRect contentRect = wz_widget_get_absolute_rect(window->content);
	const wzRect headerRect = wz_window_get_header_rect(window);

	nvgSave(vg);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h, WZ_SKIN_WINDOW_CORNER_RADIUS);

	// Border/header background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x + rect.w, (float)rect.y, WZ_SKIN_WINDOW_BORDER_BG_COLOR1, WZ_SKIN_WINDOW_BORDER_BG_COLOR2));
	nvgFill(vg);

	// Outer border.
	nvgStrokeColor(vg, WZ_SKIN_WINDOW_BORDER_COLOR);
	nvgStroke(vg);

	// Inner border.
	nvgBeginPath(vg);
	nvgRect(vg, contentRect.x - 1.0f, contentRect.y - 1.0f, contentRect.w + 2.0f, contentRect.h + 2.0f);
	nvgStrokeColor(vg, WZ_SKIN_WINDOW_INNER_BORDER_COLOR);
	nvgStroke(vg);

	// Background/content.
	nvgBeginPath(vg);
	nvgRect(vg, (float)contentRect.x, (float)contentRect.y, (float)contentRect.w, (float)contentRect.h);
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)contentRect.x, (float)contentRect.y, (float)contentRect.x, (float)contentRect.y + contentRect.h, WZ_SKIN_WINDOW_BG_COLOR1, WZ_SKIN_WINDOW_BG_COLOR2));
	nvgFill(vg);

	// Header.
	if (headerRect.w > 0 && headerRect.h > 0)
	{
		wz_renderer_clip_to_rect(vg, headerRect);
		wz_renderer_print(widget->renderer, headerRect.x + 10, headerRect.y + headerRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_WINDOW_TEXT_COLOR, window->title.c_str(), 0);
	}

	nvgRestore(vg);
}

static void wz_window_refresh_header_height(struct wzWidget *widget)
{
	struct wzWindow *window = (struct wzWindow *)widget;
	window->headerHeight = wz_widget_get_line_height(widget) + 6; // Padding.
}

static void wz_window_renderer_changed(struct wzWidget *widget)
{
	struct wzWindow *window = (struct wzWindow *)widget;
	WZ_ASSERT(window);
	window->borderSize = 4;
	wz_window_refresh_header_height(widget);
	wz_widget_refresh_rect(widget);
}

static void wz_window_font_changed(struct wzWidget *widget, const char *fontFace, float fontSize)
{
	wz_window_refresh_header_height(widget);
	wz_widget_refresh_rect(widget);
}

// rects parameter size should be WZ_NUM_COMPASS_POINTS
static void wz_window_calculate_border_rects(struct wzWindow *window, wzRect *rects)
{
	WZ_ASSERT(window);
	WZ_ASSERT(rects);

	rects[WZ_COMPASS_N] = window->base.rect;
	rects[WZ_COMPASS_N].x += window->borderSize;
	rects[WZ_COMPASS_N].w -= window->borderSize * 2;
	rects[WZ_COMPASS_N].h = window->borderSize;

	rects[WZ_COMPASS_NE] = window->base.rect;
	rects[WZ_COMPASS_NE].x += rects[WZ_COMPASS_NE].w - window->borderSize;
	rects[WZ_COMPASS_NE].w = window->borderSize;
	rects[WZ_COMPASS_NE].h = window->borderSize;

	rects[WZ_COMPASS_E] = window->base.rect;
	rects[WZ_COMPASS_E].x += rects[WZ_COMPASS_E].w - window->borderSize;
	rects[WZ_COMPASS_E].y += window->borderSize;
	rects[WZ_COMPASS_E].w = window->borderSize;
	rects[WZ_COMPASS_E].h -= window->borderSize * 2;

	rects[WZ_COMPASS_SE] = window->base.rect;
	rects[WZ_COMPASS_SE].x += rects[WZ_COMPASS_SE].w - window->borderSize;
	rects[WZ_COMPASS_SE].y += rects[WZ_COMPASS_SE].h - window->borderSize;
	rects[WZ_COMPASS_SE].w = window->borderSize;
	rects[WZ_COMPASS_SE].h = window->borderSize;

	rects[WZ_COMPASS_S] = window->base.rect;
	rects[WZ_COMPASS_S].x += window->borderSize;
	rects[WZ_COMPASS_S].y += rects[WZ_COMPASS_S].h - window->borderSize;
	rects[WZ_COMPASS_S].w -= window->borderSize * 2;
	rects[WZ_COMPASS_S].h = window->borderSize;

	rects[WZ_COMPASS_SW] = window->base.rect;
	rects[WZ_COMPASS_SW].y += rects[WZ_COMPASS_SW].h - window->borderSize;
	rects[WZ_COMPASS_SW].w = window->borderSize;
	rects[WZ_COMPASS_SW].h = window->borderSize;

	rects[WZ_COMPASS_W] = window->base.rect;
	rects[WZ_COMPASS_W].y += window->borderSize;
	rects[WZ_COMPASS_W].w = window->borderSize;
	rects[WZ_COMPASS_W].h -= window->borderSize * 2;

	rects[WZ_COMPASS_NW] = window->base.rect;
	rects[WZ_COMPASS_NW].w = window->borderSize;
	rects[WZ_COMPASS_NW].h = window->borderSize;
}

// borderRects and mouseOverBorderRects parameter sizes should be WZ_NUM_COMPASS_POINTS
static void wz_window_calculate_mouse_over_border_rects(struct wzWindow *window, int mouseX, int mouseY, wzRect *borderRects, bool *mouseOverBorderRects)
{
	wzDockPosition dockPosition;

	WZ_ASSERT(window);
	dockPosition = wz_main_window_get_window_dock_position(window->base.mainWindow, window);

	// Take into account dockPositioning, e.g. north dockPositioned window can only be resized south.
	mouseOverBorderRects[WZ_COMPASS_N] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_N]) && (dockPosition == WZ_DOCK_POSITION_NONE || dockPosition == WZ_DOCK_POSITION_SOUTH));
	mouseOverBorderRects[WZ_COMPASS_NE] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_NE]) && dockPosition == WZ_DOCK_POSITION_NONE);
	mouseOverBorderRects[WZ_COMPASS_E] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_E]) && (dockPosition == WZ_DOCK_POSITION_NONE || dockPosition == WZ_DOCK_POSITION_WEST));
	mouseOverBorderRects[WZ_COMPASS_SE] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_SE]) && dockPosition == WZ_DOCK_POSITION_NONE);
	mouseOverBorderRects[WZ_COMPASS_S] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_S]) && (dockPosition == WZ_DOCK_POSITION_NONE || dockPosition == WZ_DOCK_POSITION_NORTH));
	mouseOverBorderRects[WZ_COMPASS_SW] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_SW]) && dockPosition == WZ_DOCK_POSITION_NONE);
	mouseOverBorderRects[WZ_COMPASS_W] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_W]) && (dockPosition == WZ_DOCK_POSITION_NONE || dockPosition == WZ_DOCK_POSITION_EAST));
	mouseOverBorderRects[WZ_COMPASS_NW] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_NW]) && dockPosition == WZ_DOCK_POSITION_NONE);
}

static void wz_window_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWindow *window;
	
	WZ_ASSERT(widget);
	window = (struct wzWindow *)widget;

	if (mouseButton == 1)
	{
		wzRect borderRects[WZ_NUM_COMPASS_POINTS];
		bool mouseOverBorderRects[WZ_NUM_COMPASS_POINTS];
		int i;

		// Drag the header.
		if (WZ_POINT_IN_RECT(mouseX, mouseY, wz_window_get_header_rect(window)))
		{
			window->drag = WZ_DRAG_HEADER;
			wz_main_window_push_lock_input_widget(widget->mainWindow, widget);

			// Don't actually move the window yet if it's docked.
			if (wz_main_window_get_window_dock_position(window->base.mainWindow, window) == WZ_DOCK_POSITION_NONE)
			{
				wz_main_window_set_moving_window(widget->mainWindow, window);
			}
			else
			{
				window->undockStartPosition.x = mouseX;
				window->undockStartPosition.y = mouseY;
			}

			return;
		}

		// Resize by dragging the border.
		wz_window_calculate_border_rects(window, borderRects);
		wz_window_calculate_mouse_over_border_rects(window, mouseX, mouseY, borderRects, mouseOverBorderRects);

		for (i = 0; i < WZ_NUM_COMPASS_POINTS; i++)
		{
			if (mouseOverBorderRects[i])
			{
				window->drag = (wzWindowDrag)(WZ_DRAG_RESIZE_N + i);
				window->resizeStartPosition.x = mouseX;
				window->resizeStartPosition.y = mouseY;
				window->resizeStartRect = window->base.rect;
				wz_main_window_push_lock_input_widget(widget->mainWindow, widget);
				return;
			}
		}
	}
}

static void wz_window_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWindow *window;

	WZ_ASSERT(widget);
	window = (struct wzWindow *)widget;

	if (mouseButton == 1)
	{
		if (window->drag == WZ_DRAG_HEADER)
		{
			wz_main_window_set_moving_window(widget->mainWindow, NULL);
		}

		window->drag = WZ_DRAG_NONE;
		wz_main_window_pop_lock_input_widget(widget->mainWindow, widget);
	}
}

static void wz_window_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzWindow *window;
	wzRect borderRects[WZ_NUM_COMPASS_POINTS];
	bool mouseOverBorderRects[WZ_NUM_COMPASS_POINTS];
	wzDockPosition dockPosition;
	wzSize minimumWindowSize;
	wzPosition resizeDelta;
	wzRect rect;

	WZ_ASSERT(widget);
	window = (struct wzWindow *)widget;

	// Set the mouse cursor.
	wz_window_calculate_border_rects(window, borderRects);
	wz_window_calculate_mouse_over_border_rects(window, mouseX, mouseY, borderRects, mouseOverBorderRects);

	if (mouseOverBorderRects[WZ_COMPASS_N] || mouseOverBorderRects[WZ_COMPASS_S] || window->drag == WZ_DRAG_RESIZE_N || window->drag == WZ_DRAG_RESIZE_S)
	{
		wz_main_window_set_cursor(widget->mainWindow, WZ_CURSOR_RESIZE_N_S);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_E] || mouseOverBorderRects[WZ_COMPASS_W] || window->drag == WZ_DRAG_RESIZE_E || window->drag == WZ_DRAG_RESIZE_W)
	{
		wz_main_window_set_cursor(widget->mainWindow, WZ_CURSOR_RESIZE_E_W);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NE] || mouseOverBorderRects[WZ_COMPASS_SW] || window->drag == WZ_DRAG_RESIZE_NE || window->drag == WZ_DRAG_RESIZE_SW)
	{
		wz_main_window_set_cursor(widget->mainWindow, WZ_CURSOR_RESIZE_NE_SW);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NW] || mouseOverBorderRects[WZ_COMPASS_SE] || window->drag == WZ_DRAG_RESIZE_NW || window->drag == WZ_DRAG_RESIZE_SE)
	{
		wz_main_window_set_cursor(widget->mainWindow, WZ_CURSOR_RESIZE_NW_SE);
	}

	// Don't actually move the window yet if it's docked.
	dockPosition = wz_main_window_get_window_dock_position(widget->mainWindow, window);

	if (window->drag == WZ_DRAG_HEADER && dockPosition != WZ_DOCK_POSITION_NONE)
	{
		wzPosition delta;

		delta.x = mouseX - window->undockStartPosition.x;
		delta.y = mouseY - window->undockStartPosition.y;

		// Undock and start moving if the mouse has moved far enough.
		if ((int)sqrtf((float)(delta.x * delta.x + delta.y * delta.y)) < WZ_WINDOW_UNDOCK_DISTANCE)
			return;

		wz_main_window_undock_window(widget->mainWindow, window);
		wz_main_window_set_moving_window(widget->mainWindow, window);

		// Re-position and resize the window.
		rect = wz_widget_get_rect(widget);
		rect.x += delta.x;
		rect.y += delta.y;
		rect.w = WZ_MAX(200, window->sizeBeforeDocking.w);
		rect.h = WZ_MAX(200, window->sizeBeforeDocking.h);

		// If the mouse cursor would be outside the window, center the window on the mouse cursor.
		if (mouseX < rect.x + window->borderSize || mouseX > rect.x + rect.w - window->borderSize)
		{
			rect.x = mouseX - rect.w / 2;
		}

		wz_widget_set_rect_internal(widget, rect);
		wz_main_window_update_content_rect(widget->mainWindow);
	}

	// Calculate the minimum allowed window size.
	minimumWindowSize.w = window->borderSize * 2;
	minimumWindowSize.h = window->headerHeight + window->borderSize * 2;

	// Calculate mouse deltas for dragging. Deltas are relative to the dragging start position (mouseDeltaX and mouseDeltaY are relative to the last mouse position).
	if (window->drag >= WZ_DRAG_RESIZE_N)
	{
		resizeDelta.x = mouseX - window->resizeStartPosition.x;
		resizeDelta.y = mouseY - window->resizeStartPosition.y;
	}

	rect = wz_widget_get_rect(widget);

	switch (window->drag)
	{
	case WZ_DRAG_HEADER:
		rect.x += mouseDeltaX;
		rect.y += mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_N:
		{
			int delta = WZ_MIN(resizeDelta.y, window->resizeStartRect.h - minimumWindowSize.h);
			rect.y = window->resizeStartRect.y + delta;
			rect.h = window->resizeStartRect.h - delta;
		}
		break;
	case WZ_DRAG_RESIZE_NE:
		{
			int delta = WZ_MIN(resizeDelta.y, window->resizeStartRect.h - minimumWindowSize.h);
			rect.y = window->resizeStartRect.y + delta;
			rect.w = WZ_MAX(minimumWindowSize.w, window->resizeStartRect.w + resizeDelta.x);
			rect.h = window->resizeStartRect.h - delta;
		}
		break;
	case WZ_DRAG_RESIZE_E:
		rect.w = WZ_MAX(minimumWindowSize.w, window->resizeStartRect.w + resizeDelta.x);
		break;
	case WZ_DRAG_RESIZE_SE:
		rect.w = WZ_MAX(minimumWindowSize.w, window->resizeStartRect.w + resizeDelta.x);
		rect.h = WZ_MAX(minimumWindowSize.h, window->resizeStartRect.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_S:
		rect.h = WZ_MAX(minimumWindowSize.h, window->resizeStartRect.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_SW:
		{
			int delta = WZ_MIN(resizeDelta.x, window->resizeStartRect.w - minimumWindowSize.w);
			rect.x = window->resizeStartRect.x + delta;
			rect.w = window->resizeStartRect.w - delta;
			rect.h = WZ_MAX(minimumWindowSize.h, window->resizeStartRect.h + resizeDelta.y);
		}
		break;
	case WZ_DRAG_RESIZE_W:
		{
			int delta = WZ_MIN(resizeDelta.x, window->resizeStartRect.w - minimumWindowSize.w);
			rect.x = window->resizeStartRect.x + delta;
			rect.w = window->resizeStartRect.w - delta;
		}
		break;
	case WZ_DRAG_RESIZE_NW:
		{
			int deltaX, deltaY;
			deltaX = WZ_MIN(resizeDelta.x, window->resizeStartRect.w - minimumWindowSize.w);
			deltaY = WZ_MIN(resizeDelta.y, window->resizeStartRect.h - minimumWindowSize.h);
			rect.x = window->resizeStartRect.x + deltaX;
			rect.y = window->resizeStartRect.y + deltaY;
			rect.w = window->resizeStartRect.w - deltaX;
			rect.h = window->resizeStartRect.h - deltaY;
		}
		break;
	default:
		return; // Not dragging, don't call wz_main_window_update_content_rect.
	}

	wz_widget_set_rect_internal(widget, rect);

	// Resizing a docked window: 
	if (wz_main_window_get_window_dock_position(widget->mainWindow, window) != WZ_DOCK_POSITION_NONE)
	{
		// Tell the mainWindow so it can resize other windows docked at the same position too.
		wz_main_window_update_docked_window_rect(widget->mainWindow, window);

		// Update the mainWindow content rect.
		wz_main_window_update_content_rect(widget->mainWindow);
	}
}

static wzRect wz_window_get_children_clip_rect(struct wzWidget *widget)
{
	struct wzWindow *window;

	WZ_ASSERT(widget);
	window = (struct wzWindow *)widget;
	return wz_widget_get_absolute_rect(window->content);
}

static void wz_window_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzWindow *window;
	wzRect contentRect;

	WZ_ASSERT(widget);
	window = (struct wzWindow *)widget;
	window->base.rect = rect;

	contentRect.x = window->borderSize;
	contentRect.y = window->borderSize + window->headerHeight;
	contentRect.w = rect.w - window->borderSize * 2;
	contentRect.h = rect.h - (window->headerHeight + window->borderSize * 2);

	wz_widget_set_rect_internal(window->content, contentRect);
}

struct wzWindow *wz_window_create(const char *title)
{
	struct wzWindow *window = (struct wzWindow *)malloc(sizeof(struct wzWindow));

	memset(window, 0, sizeof(struct wzWindow));
	window->base.type = WZ_TYPE_WINDOW;
	window->base.vtable.draw = wz_window_draw;
	window->base.vtable.renderer_changed = wz_window_renderer_changed;
	window->base.vtable.font_changed = wz_window_font_changed;
	window->base.vtable.mouse_button_down = wz_window_mouse_button_down;
	window->base.vtable.mouse_button_up = wz_window_mouse_button_up;
	window->base.vtable.mouse_move = wz_window_mouse_move;
	window->base.vtable.get_children_clip_rect = wz_window_get_children_clip_rect;
	window->base.vtable.set_rect = wz_window_set_rect;
	window->title = title ? std::string(title) : std::string();

	window->content = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(window->content, 0, sizeof(struct wzWidget));
	wz_widget_add_child_widget((struct wzWidget *)window, window->content);

	return window;
}

int wz_window_get_header_height(const struct wzWindow *window)
{
	WZ_ASSERT(window);
	return window->headerHeight;
}

int wz_window_get_border_size(const struct wzWindow *window)
{
	WZ_ASSERT(window);
	return window->borderSize;
}

wzRect wz_window_get_header_rect(const struct wzWindow *window)
{
	wzRect rect;

	WZ_ASSERT(window);
	rect.x = window->base.rect.x + window->borderSize;
	rect.y = window->base.rect.y + window->borderSize;
	rect.w = window->base.rect.w - window->borderSize * 2;
	rect.h = window->headerHeight;
	return rect;
}

void wz_window_set_title(struct wzWindow *window, const char *title)
{
	WZ_ASSERT(window);
	window->title = std::string(title);
}

const char *wz_window_get_title(const struct wzWindow *window)
{
	WZ_ASSERT(window);
	return window->title.c_str();
}

void wz_window_add(struct wzWindow *window, struct wzWidget *widget)
{
	WZ_ASSERT(window);
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	wz_widget_add_child_widget(window->content, widget);
}

void wz_window_remove(struct wzWindow *window, struct wzWidget *widget)
{
	WZ_ASSERT(window);
	WZ_ASSERT(widget);
	wz_widget_remove_child_widget(window->content, widget);
}

struct wzWidget *wz_window_get_content_widget(struct wzWindow *window)
{
	WZ_ASSERT(window);
	return window->content;
}

// Save the window size before docking so it can be restored if the window is undocked later.
void wz_window_dock(struct wzWindow *window)
{
	WZ_ASSERT(window);
	window->sizeBeforeDocking.w = window->base.rect.w;
	window->sizeBeforeDocking.h = window->base.rect.h;
}

int wz_window_get_draw_priority(const struct wzWindow *window)
{
	WZ_ASSERT(window);
	return window->drawPriority;
}

void wz_window_set_draw_priority(struct wzWindow *window, int drawPriority)
{
	WZ_ASSERT(window);
	window->drawPriority = drawPriority;
}
