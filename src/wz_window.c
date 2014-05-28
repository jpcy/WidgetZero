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
#include "wz_internal.h"

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
	int headerHeight;
	int borderSize;
	wzWindowDrag drag;
	wzDock dock;

	// Dragging a docked window header doesn't undock the window until the mouse has moved WZ_WINDOW_UNDOCK_DISTANCE.
	wzPosition undockStartPosition;

	wzPosition resizeStartPosition;
	wzRect resizeStartRect;

	// Remember the window size when it is docked, so when the window is undocked the size can be restored.
	wzSize sizeWhenDocked;
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
	// Take into account docking, e.g. north docked window can only be resized south.
	mouseOverBorderRects[WZ_COMPASS_N] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_N]) && (window->dock == WZ_DOCK_NONE || window->dock == WZ_DOCK_SOUTH));
	mouseOverBorderRects[WZ_COMPASS_NE] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_NE]) && window->dock == WZ_DOCK_NONE);
	mouseOverBorderRects[WZ_COMPASS_E] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_E]) && (window->dock == WZ_DOCK_NONE || window->dock == WZ_DOCK_WEST));
	mouseOverBorderRects[WZ_COMPASS_SE] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_SE]) && window->dock == WZ_DOCK_NONE);
	mouseOverBorderRects[WZ_COMPASS_S] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_S]) && (window->dock == WZ_DOCK_NONE || window->dock == WZ_DOCK_NORTH));
	mouseOverBorderRects[WZ_COMPASS_SW] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_SW]) && window->dock == WZ_DOCK_NONE);
	mouseOverBorderRects[WZ_COMPASS_W] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_W]) && (window->dock == WZ_DOCK_NONE || window->dock == WZ_DOCK_EAST));
	mouseOverBorderRects[WZ_COMPASS_NW] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_NW]) && window->dock == WZ_DOCK_NONE);
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
			if (window->dock == WZ_DOCK_NONE)
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

static void wz_window_call_parent_window_move_recursive(struct wzWidget *widget)
{
	int i;

	assert(widget);

	if (widget->vtable.parent_window_move)
	{
		widget->vtable.parent_window_move(widget);
	}

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_window_call_parent_window_move_recursive(widget->children[i]);
	}
}

static void wz_window_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzWindow *window;
	wzRect borderRects[WZ_NUM_COMPASS_POINTS];
	bool mouseOverBorderRects[WZ_NUM_COMPASS_POINTS];
	wzSize minimumWindowSize;
	wzPosition resizeDelta;
	int i;

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
	if (window->drag == WZ_DRAG_HEADER && window->dock != WZ_DOCK_NONE)
	{
		wzPosition delta;
		wzRect rect;

		delta.x = mouseX - window->undockStartPosition.x;
		delta.y = mouseY - window->undockStartPosition.y;

		// Undock and start moving if the mouse has moved far enough.
		if (sqrt(delta.x * delta.x + delta.y * delta.y) < WZ_WINDOW_UNDOCK_DISTANCE)
			return;

		window->dock = WZ_DOCK_NONE;
		wz_desktop_set_moving_window(widget->desktop, window);

		// Re-position and resize the window.
		rect = wz_widget_get_rect(widget);
		rect.x += delta.x;
		rect.y += delta.y;
		rect.w = WZ_MAX(200, window->sizeWhenDocked.w);
		rect.h = WZ_MAX(200, window->sizeWhenDocked.h);

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

	switch (window->drag)
	{
	case WZ_DRAG_HEADER:
		widget->rect.x += mouseDeltaX;
		widget->rect.y += mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_N:
		{
			int delta = WZ_MIN(resizeDelta.y, window->resizeStartRect.h - minimumWindowSize.h);
			widget->rect.y = window->resizeStartRect.y + delta;
			widget->rect.h = window->resizeStartRect.h - delta;
		}
		break;
	case WZ_DRAG_RESIZE_NE:
		{
			int delta = WZ_MIN(resizeDelta.y, window->resizeStartRect.h - minimumWindowSize.h);
			widget->rect.y = window->resizeStartRect.y + delta;
			widget->rect.w = WZ_MAX(minimumWindowSize.w, window->resizeStartRect.w + resizeDelta.x);
			widget->rect.h = window->resizeStartRect.h - delta;
		}
		break;
	case WZ_DRAG_RESIZE_E:
		widget->rect.w = WZ_MAX(minimumWindowSize.w, window->resizeStartRect.w + resizeDelta.x);
		break;
	case WZ_DRAG_RESIZE_SE:
		widget->rect.w = WZ_MAX(minimumWindowSize.w, window->resizeStartRect.w + resizeDelta.x);
		widget->rect.h = WZ_MAX(minimumWindowSize.h, window->resizeStartRect.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_S:
		widget->rect.h = WZ_MAX(minimumWindowSize.h, window->resizeStartRect.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_SW:
		{
			int delta = WZ_MIN(resizeDelta.x, window->resizeStartRect.w - minimumWindowSize.w);
			widget->rect.x = window->resizeStartRect.x + delta;
			widget->rect.w = window->resizeStartRect.w - delta;
			widget->rect.h = WZ_MAX(minimumWindowSize.h, window->resizeStartRect.h + resizeDelta.y);
		}
		break;
	case WZ_DRAG_RESIZE_W:
		{
			int delta = WZ_MIN(resizeDelta.x, window->resizeStartRect.w - minimumWindowSize.w);
			widget->rect.x = window->resizeStartRect.x + delta;
			widget->rect.w = window->resizeStartRect.w - delta;
		}
		break;
	case WZ_DRAG_RESIZE_NW:
		{
			int deltaX, deltaY;
			deltaX = WZ_MIN(resizeDelta.x, window->resizeStartRect.w - minimumWindowSize.w);
			deltaY = WZ_MIN(resizeDelta.y, window->resizeStartRect.h - minimumWindowSize.h);
			widget->rect.x = window->resizeStartRect.x + deltaX;
			widget->rect.y = window->resizeStartRect.y + deltaY;
			widget->rect.w = window->resizeStartRect.w - deltaX;
			widget->rect.h = window->resizeStartRect.h - deltaY;
		}
		break;
	default:
		return; // Not dragging, don't call wz_desktop_update_content_rect or parent_window_move.
	}

	// Resizing a docked window: update the desktop content rect.
	if (window->dock != WZ_DOCK_NONE)
	{
		wz_desktop_update_content_rect(widget->desktop);
	}

	// Dragging: call parent_window_move on child and ancestor widgets.
	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_window_call_parent_window_move_recursive(widget->children[i]);
	}
}

struct wzWindow *wz_window_create(struct wzDesktop *desktop)
{
	struct wzWindow *window;

	assert(desktop);
	window = (struct wzWindow *)malloc(sizeof(struct wzWindow));
	memset(window, 0, sizeof(struct wzWindow));
	window->base.type = WZ_TYPE_WINDOW;
	window->base.drawPriority = WZ_DRAW_PRIORITY_WINDOW_START;
	window->base.desktop = desktop;
	window->base.vtable.mouse_button_down = wz_window_mouse_button_down;
	window->base.vtable.mouse_button_up = wz_window_mouse_button_up;
	window->base.vtable.mouse_move = wz_window_mouse_move;
	return window;
}

void wz_window_set_header_height(struct wzWindow *window, int height)
{
	assert(window);
	window->headerHeight = height;
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

wzRect wz_window_get_content_rect(struct wzWindow *window)
{
	wzRect rect;

	assert(window);
	rect = window->base.rect;
	rect.x += window->borderSize;
	rect.y += window->borderSize + window->headerHeight;
	rect.w -= window->borderSize * 2;
	rect.h -= (window->headerHeight + window->borderSize * 2);
	return rect;
}

void wz_window_set_dock(struct wzWindow *window, wzDock dock)
{
	assert(window);
	window->dock = dock;

	if (dock != WZ_DOCK_NONE)
	{
		window->sizeWhenDocked.w = window->base.rect.w;
		window->sizeWhenDocked.h = window->base.rect.h;
	}

	wz_desktop_update_content_rect(window->base.desktop);
}

wzDock wz_window_get_dock(struct wzWindow *window)
{
	assert(window);
	return window->dock;
}
