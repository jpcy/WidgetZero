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
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "wz_desktop.h"
#include "wz_widget.h"

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
	
	struct wzWidget *content;

	wzWindowDrag drag;

	// Dragging a docked window header doesn't undock the window until the mouse has moved WZ_WINDOW_UNDOCK_DISTANCE.
	wzPosition undockStartPosition;

	wzPosition resizeStartPosition;
	wzRect resizeStartRect;

	// Remember the window size when it is docked, so when the window is undocked the size can be restored.
	wzSize sizeBeforeDocking;
};

// rects parameter size should be WZ_NUM_COMPASS_POINTS
static void wz_window_calculate_border_rects(struct wzWindow *window, wzRect *rects)
{
	assert(window);
	assert(rects);

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

	assert(window);
	dockPosition = wz_desktop_get_window_dock_position(window->base.desktop, window);

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
	
	assert(widget);
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
			wz_desktop_push_lock_input_widget(widget->desktop, widget);

			// Don't actually move the window yet if it's docked.
			if (wz_desktop_get_window_dock_position(window->base.desktop, window) == WZ_DOCK_POSITION_NONE)
			{
				wz_desktop_set_moving_window(widget->desktop, window);
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
				window->drag = WZ_DRAG_RESIZE_N + i;
				window->resizeStartPosition.x = mouseX;
				window->resizeStartPosition.y = mouseY;
				window->resizeStartRect = window->base.rect;
				wz_desktop_push_lock_input_widget(widget->desktop, widget);
				return;
			}
		}
	}
}

static void wz_window_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWindow *window;

	assert(widget);
	window = (struct wzWindow *)widget;

	if (mouseButton == 1)
	{
		if (window->drag == WZ_DRAG_HEADER)
		{
			wz_desktop_set_moving_window(widget->desktop, NULL);
		}

		window->drag = WZ_DRAG_NONE;
		wz_desktop_pop_lock_input_widget(widget->desktop, widget);
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

	assert(widget);
	window = (struct wzWindow *)widget;

	// Set the mouse cursor.
	wz_window_calculate_border_rects(window, borderRects);
	wz_window_calculate_mouse_over_border_rects(window, mouseX, mouseY, borderRects, mouseOverBorderRects);

	if (mouseOverBorderRects[WZ_COMPASS_N] || mouseOverBorderRects[WZ_COMPASS_S] || window->drag == WZ_DRAG_RESIZE_N || window->drag == WZ_DRAG_RESIZE_S)
	{
		wz_desktop_set_cursor(widget->desktop, WZ_CURSOR_RESIZE_N_S);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_E] || mouseOverBorderRects[WZ_COMPASS_W] || window->drag == WZ_DRAG_RESIZE_E || window->drag == WZ_DRAG_RESIZE_W)
	{
		wz_desktop_set_cursor(widget->desktop, WZ_CURSOR_RESIZE_E_W);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NE] || mouseOverBorderRects[WZ_COMPASS_SW] || window->drag == WZ_DRAG_RESIZE_NE || window->drag == WZ_DRAG_RESIZE_SW)
	{
		wz_desktop_set_cursor(widget->desktop, WZ_CURSOR_RESIZE_NE_SW);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NW] || mouseOverBorderRects[WZ_COMPASS_SE] || window->drag == WZ_DRAG_RESIZE_NW || window->drag == WZ_DRAG_RESIZE_SE)
	{
		wz_desktop_set_cursor(widget->desktop, WZ_CURSOR_RESIZE_NW_SE);
	}

	// Don't actually move the window yet if it's docked.
	dockPosition = wz_desktop_get_window_dock_position(widget->desktop, window);

	if (window->drag == WZ_DRAG_HEADER && dockPosition != WZ_DOCK_POSITION_NONE)
	{
		wzPosition delta;

		delta.x = mouseX - window->undockStartPosition.x;
		delta.y = mouseY - window->undockStartPosition.y;

		// Undock and start moving if the mouse has moved far enough.
		if (sqrt(delta.x * delta.x + delta.y * delta.y) < WZ_WINDOW_UNDOCK_DISTANCE)
			return;

		wz_desktop_undock_window(widget->desktop, window);
		wz_desktop_set_moving_window(widget->desktop, window);

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

		wz_widget_set_rect(widget, rect);
		wz_desktop_update_content_rect(widget->desktop);
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
		return; // Not dragging, don't call wz_desktop_update_content_rect.
	}

	wz_widget_set_rect(widget, rect);

	// Resizing a docked window: 
	if (wz_desktop_get_window_dock_position(widget->desktop, window) != WZ_DOCK_POSITION_NONE)
	{
		// Tell the desktop so it can resize other windows docked at the same position too.
		wz_desktop_update_docked_window_rect(widget->desktop, window);

		// Update the desktop content rect.
		wz_desktop_update_content_rect(widget->desktop);
	}
}

static wzRect wz_window_get_children_clip_rect(struct wzWidget *widget)
{
	struct wzWindow *window;

	assert(widget);
	window = (struct wzWindow *)widget;
	return wz_widget_get_absolute_rect(window->content);
}

static void wz_window_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzWindow *window;
	wzRect contentRect;

	assert(widget);
	window = (struct wzWindow *)widget;
	window->base.rect = rect;

	contentRect.x = window->borderSize;
	contentRect.y = window->borderSize + window->headerHeight;
	contentRect.w = rect.w - window->borderSize * 2;
	contentRect.h = rect.h - (window->headerHeight + window->borderSize * 2);

	wz_widget_set_rect(window->content, contentRect);
}

struct wzWindow *wz_window_create()
{
	struct wzWindow *window;

	window = (struct wzWindow *)malloc(sizeof(struct wzWindow));
	memset(window, 0, sizeof(struct wzWindow));
	window->base.type = WZ_TYPE_WINDOW;
	window->base.drawPriority = WZ_DRAW_PRIORITY_WINDOW;
	window->base.vtable.mouse_button_down = wz_window_mouse_button_down;
	window->base.vtable.mouse_button_up = wz_window_mouse_button_up;
	window->base.vtable.mouse_move = wz_window_mouse_move;
	window->base.vtable.get_children_clip_rect = wz_window_get_children_clip_rect;
	window->base.vtable.set_rect = wz_window_set_rect;

	window->content = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(window->content, 0, sizeof(struct wzWidget));
	wz_widget_add_child_widget((struct wzWidget *)window, window->content);

	return window;
}

void wz_window_set_header_height(struct wzWindow *window, int height)
{
	assert(window);
	window->headerHeight = height;
	wz_widget_refresh_rect((struct wzWidget *)window);
}

int wz_window_get_header_height(struct wzWindow *window)
{
	assert(window);
	return window->headerHeight;
}

void wz_window_set_border_size(struct wzWindow *window, int size)
{
	assert(window);
	window->borderSize = size;
}

int wz_window_get_border_size(struct wzWindow *window)
{
	assert(window);
	return window->borderSize;
}

wzRect wz_window_get_header_rect(struct wzWindow *window)
{
	wzRect rect;

	assert(window);
	rect.x = window->base.rect.x + window->borderSize;
	rect.y = window->base.rect.y + window->borderSize;
	rect.w = window->base.rect.w - window->borderSize * 2;
	rect.h = window->headerHeight;
	return rect;
}

struct wzWidget *wz_window_get_content_widget(struct wzWindow *window)
{
	assert(window);
	return window->content;
}

// Save the window size before docking so it can be restored if the window is undocked later.
void wz_window_dock(struct wzWindow *window)
{
	assert(window);
	window->sizeBeforeDocking.w = window->base.rect.w;
	window->sizeBeforeDocking.h = window->base.rect.h;
}

int wz_window_get_draw_priority(const struct wzWindow *window)
{
	assert(window);
	return window->drawPriority;
}

void wz_window_set_draw_priority(struct wzWindow *window, int drawPriority)
{
	assert(window);
	window->drawPriority = drawPriority;
}
