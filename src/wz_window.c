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
#include <stdlib.h>
#include <string.h>
#include "wz_internal.h"

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
};

static void wz_window_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWindow *window;

	assert(widget);
	window = (struct wzWindow *)widget;

	if (mouseButton == 1)
	{
		wzRect r = wz_window_get_header_rect(window);

		if (WZ_POINT_IN_RECT(mouseX, mouseY, r))
		{
			window->drag = WZ_DRAG_HEADER;
			wz_desktop_push_lock_input_widget(widget->desktop, widget);
			wz_desktop_set_dock_icons_visible(widget->desktop, true);
			return;
		}

		// N
		r = widget->rect;
		r.x += window->borderSize;
		r.w -= window->borderSize * 2;
		r.h = window->borderSize;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, r))
		{
			window->drag = WZ_DRAG_RESIZE_N;
			wz_desktop_push_lock_input_widget(widget->desktop, widget);
			return;
		}

		// NE
		r = widget->rect;
		r.x += r.w - window->borderSize;
		r.w = window->borderSize;
		r.h = window->borderSize;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, r))
		{
			window->drag = WZ_DRAG_RESIZE_NE;
			wz_desktop_push_lock_input_widget(widget->desktop, widget);
			return;
		}

		// E
		r = widget->rect;
		r.x += r.w - window->borderSize;
		r.y += window->borderSize;
		r.w = window->borderSize;
		r.h -= window->borderSize * 2;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, r))
		{
			window->drag = WZ_DRAG_RESIZE_E;
			wz_desktop_push_lock_input_widget(widget->desktop, widget);
			return;
		}

		// SE
		r = widget->rect;
		r.x += r.w - window->borderSize;
		r.y += r.h - window->borderSize;
		r.w = window->borderSize;
		r.h = window->borderSize;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, r))
		{
			window->drag = WZ_DRAG_RESIZE_SE;
			wz_desktop_push_lock_input_widget(widget->desktop, widget);
			return;
		}

		// S
		r = widget->rect;
		r.x += window->borderSize;
		r.y += r.h - window->borderSize;
		r.w -= window->borderSize * 2;
		r.h = window->borderSize;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, r))
		{
			window->drag = WZ_DRAG_RESIZE_S;
			wz_desktop_push_lock_input_widget(widget->desktop, widget);
			return;
		}

		// SW
		r = widget->rect;
		r.y += r.h - window->borderSize;
		r.w = window->borderSize;
		r.h = window->borderSize;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, r))
		{
			window->drag = WZ_DRAG_RESIZE_SW;
			wz_desktop_push_lock_input_widget(widget->desktop, widget);
			return;
		}

		// W
		r = widget->rect;
		r.y += window->borderSize;
		r.w = window->borderSize;
		r.h -= window->borderSize * 2;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, r))
		{
			window->drag = WZ_DRAG_RESIZE_W;
			wz_desktop_push_lock_input_widget(widget->desktop, widget);
			return;
		}

		// NW
		r = widget->rect;
		r.w = window->borderSize;
		r.h = window->borderSize;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, r))
		{
			window->drag = WZ_DRAG_RESIZE_NW;
			wz_desktop_push_lock_input_widget(widget->desktop, widget);
			return;
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
			wz_desktop_set_dock_icons_visible(widget->desktop, false);
		}

		window->drag = WZ_DRAG_NONE;
		wz_desktop_pop_lock_input_widget(widget->desktop, widget);
	}
}

static void wz_window_call_parent_window_move_recursive(struct wzWidget *widget)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->vtable.parent_window_move)
	{
		widget->vtable.parent_window_move(widget);
	}

	child = widget->firstChild;

	while (child)
	{
		wz_window_call_parent_window_move_recursive(child);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

static void wz_window_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzWindow *window;
	struct wzWidget *child;

	assert(widget);
	window = (struct wzWindow *)widget;

	switch (window->drag)
	{
	case WZ_DRAG_HEADER:
		widget->rect.x += mouseDeltaX;
		widget->rect.y += mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_N:
		widget->rect.y += mouseDeltaY;
		widget->rect.h -= mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_NE:
		widget->rect.y += mouseDeltaY;
		widget->rect.w += mouseDeltaX;
		widget->rect.h -= mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_E:
		widget->rect.w += mouseDeltaX;
		break;
	case WZ_DRAG_RESIZE_SE:
		widget->rect.w += mouseDeltaX;
		widget->rect.h += mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_S:
		widget->rect.h += mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_SW:
		widget->rect.x += mouseDeltaX;
		widget->rect.w -= mouseDeltaX;
		widget->rect.h += mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_W:
		widget->rect.x += mouseDeltaX;
		widget->rect.w -= mouseDeltaX;
		break;
	case WZ_DRAG_RESIZE_NW:
		widget->rect.x += mouseDeltaX;
		widget->rect.y += mouseDeltaY;
		widget->rect.w -= mouseDeltaX;
		widget->rect.h -= mouseDeltaY;
		break;
	default:
		return; // Not dragging, don't call parent_window_move.
	}

	// Dragging: call parent_window_move on child and ancestor widgets.
	child = widget->firstChild;

	while (child)
	{
		wz_window_call_parent_window_move_recursive(child);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

struct wzWindow *wz_window_create(struct wzContext *context)
{
	struct wzWindow *window;

	assert(context);
	window = (struct wzWindow *)malloc(sizeof(struct wzWindow));
	memset(window, 0, sizeof(struct wzWindow));
	window->base.type = WZ_TYPE_WINDOW;
	window->base.drawPriority = WZ_DRAW_PRIORITY_WINDOW_START;
	window->base.context = context;
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
	rect.h -= window->headerHeight;
	return rect;
}
