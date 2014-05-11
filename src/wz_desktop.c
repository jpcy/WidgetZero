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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stb_arr.h"
#include "wz_internal.h"

struct wzDesktop
{
	struct wzWidget base;

	struct wzWidget **lockInputWidgetStack;

	// Lock input to this window, i.e. don't call mouse_move, mouse_button_down or mouse_button_up on any widget that isn't this window or it's descendants.
	struct wzWindow *lockInputWindow;
};

struct wzDesktop *wz_desktop_create(struct wzContext *context)
{
	struct wzDesktop *desktop;

	assert(context);
	desktop = (struct wzDesktop *)malloc(sizeof(struct wzDesktop));
	memset(desktop, 0, sizeof(struct wzDesktop));
	desktop->base.type = WZ_TYPE_DESKTOP;
	desktop->base.context = context;
	return desktop;
}

void wz_desktop_set_size(struct wzDesktop *desktop, wzSize size)
{
	assert(desktop);
	desktop->base.rect.w = size.w;
	desktop->base.rect.h = size.h;
}

void wz_desktop_set_size_args(struct wzDesktop *desktop, int width, int height)
{
	assert(desktop);
	desktop->base.rect.w = width;
	desktop->base.rect.h = height;
}

wzSize wz_desktop_get_size(const struct wzDesktop *desktop)
{
	wzSize size;

	assert(desktop);
	size.w = desktop->base.rect.w;
	size.h = desktop->base.rect.h;
	return size;
}

// Returns the window that the mouse cursor is hovering over. NULL if there isn't one.
static struct wzWindow *wz_desktop_get_hover_window(struct wzDesktop *desktop, int mouseX, int mouseY)
{
	struct wzWidget *widget;
	struct wzWindow *result;
	int drawPriority;

	assert(desktop);
	widget = desktop->base.firstChild;
	result = NULL;
	drawPriority = WZ_DRAW_PRIORITY_WINDOW_START;

	while (widget)
	{
		if (widget->type == WZ_TYPE_WINDOW && WZ_POINT_IN_RECT(mouseX, mouseY, widget->rect) && widget->drawPriority >= drawPriority)
		{
			drawPriority = widget->drawPriority;
			result = (struct wzWindow *)widget;
		}

		widget = widget->next == desktop->base.firstChild ? NULL : widget->next;
	}

	return result;
}

static int wz_compare_widget_draw_priorities(const void *a, const void *b)
{
	return ((struct wzWidget *)a)->drawPriority - ((struct wzWidget *)b)->drawPriority;
}

// top can be NULL
static void wz_desktop_update_window_draw_priorities(struct wzDesktop *desktop, struct wzWindow *top)
{
	struct wzWidget *widget;
	struct wzWidget *windows[WZ_DRAW_PRIORITY_WINDOW_END];
	int nWindows;
	int i;

	assert(desktop);

	// Get a list of windows (excluding top).
	widget = desktop->base.firstChild;
	nWindows = 0;

	while (widget)
	{
		if (widget->type == WZ_TYPE_WINDOW && widget != (struct wzWidget *)top)
		{
			windows[nWindows] = widget;
			nWindows++;
		}

		widget = widget->next == desktop->base.firstChild ? NULL : widget->next;
	}

	// Sort them in ascending order by draw priority.
	qsort(windows, nWindows, sizeof(struct wzWindow *), wz_compare_widget_draw_priorities);

	// Assign each window a new draw priority starting at WZ_DRAW_PRIORITY_WINDOW_START and ascending.
	for (i = 0; i < nWindows; i++)
	{
		windows[i]->drawPriority = WZ_DRAW_PRIORITY_WINDOW_START + i;
	}

	// Give the top window the highest priority.
	if (top)
	{
		((struct wzWidget *)top)->drawPriority = WZ_DRAW_PRIORITY_WINDOW_START + i;
	}
}

static void wz_widget_mouse_button_down_recursive(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->vtable.mouse_button_down)
	{
		widget->vtable.mouse_button_down(widget, mouseButton, mouseX, mouseY);
	}

	child = widget->firstChild;

	while (child)
	{
		if (child->hover)
		{
			wz_widget_mouse_button_down_recursive(child, mouseButton, mouseX, mouseY);
		}

		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_mouse_button_down(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	assert(desktop);
	desktop->lockInputWindow = wz_desktop_get_hover_window(desktop, mouseX, mouseY);

	if (stb_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		wz_desktop_update_window_draw_priorities(desktop, desktop->lockInputWindow);
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_mouse_button_down_recursive(widget, mouseButton, mouseX, mouseY);
}

static void wz_widget_mouse_button_up_recursive(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->vtable.mouse_button_up)
	{
		widget->vtable.mouse_button_up(widget, mouseButton, mouseX, mouseY);
	}

	child = widget->firstChild;

	while (child)
	{
		wz_widget_mouse_button_up_recursive(child, mouseButton, mouseX, mouseY);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_mouse_button_up(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	assert(desktop);

	if (stb_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_mouse_button_up_recursive(widget, mouseButton, mouseX, mouseY);
}

// If window is not NULL, only call mouse_move in widgets that are children of the window and the window itself.
static void wz_widget_mouse_move_recursive(struct wzWindow *window, struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	wzRect rect;
	bool widgetIsChildOfWindow;
	bool oldHover;
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	rect = wz_widget_get_absolute_rect(widget);
	widgetIsChildOfWindow = !window || (window && widget->window == window);
	oldHover = widget->hover;
	widget->hover = widgetIsChildOfWindow && WZ_POINT_IN_RECT(mouseX, mouseY, rect);

	if (!oldHover && widget->hover && widget->vtable.mouse_hover_on)
	{
		widget->vtable.mouse_hover_on(widget);
	}
	else if (oldHover && !widget->hover && widget->vtable.mouse_hover_off)
	{
		widget->vtable.mouse_hover_off(widget);
	}

	if (widgetIsChildOfWindow && widget->vtable.mouse_move)
	{
		widget->vtable.mouse_move(widget, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
	}

	child = widget->firstChild;

	while (child)
	{
		wz_widget_mouse_move_recursive(window, child, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_mouse_move(struct wzDesktop *desktop, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	assert(desktop);

	if (stb_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		wz_widget_mouse_move_recursive(NULL, desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)], mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		return;
	}

	desktop->lockInputWindow = wz_desktop_get_hover_window(desktop, mouseX, mouseY);
	wz_widget_mouse_move_recursive(desktop->lockInputWindow, (struct wzWidget *)desktop, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
}

static void wz_widget_mouse_wheel_move_recursive(struct wzWidget *widget, int x, int y)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->vtable.mouse_wheel_move)
	{
		widget->vtable.mouse_wheel_move(widget, x, y);
	}

	child = widget->firstChild;

	while (child)
	{
		if (child->hover)
		{
			wz_widget_mouse_wheel_move_recursive(child, x, y);
		}

		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_mouse_wheel_move(struct wzDesktop *desktop, int x, int y)
{
	struct wzWidget *widget;

	assert(desktop);

	if (stb_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_mouse_wheel_move_recursive(widget, x, y);
}

static void wz_widget_calculate_unique_draw_priorities_recursive(int *drawPriorities, int *nDrawPriorities, struct wzWidget *widget)
{
	struct wzWidget *child;
	int i;

	assert(widget);

	if (widget->hidden)
		return;

	// Add the draw priority if it doesn't exist.
	for (i = 0; i < *nDrawPriorities; i++)
	{
		if (drawPriorities[i] == widget->drawPriority)
			break;
	}

	if (i == *nDrawPriorities)
	{
		drawPriorities[*nDrawPriorities] = widget->drawPriority;
		(*nDrawPriorities)++;
	}

	child = widget->firstChild;

	while (child)
	{
		wz_widget_calculate_unique_draw_priorities_recursive(drawPriorities, nDrawPriorities, child);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

static int wz_compare_draw_priorities(const void *a, const void *b)
{
	return *(const int *)a - *(const int *)b;
}

static void wz_widget_draw_by_less_than_or_equals_priority_recursive(int priority, struct wzWidget *widget)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->drawPriority <= priority && widget->vtable.draw)
	{
		widget->vtable.draw(widget);
	}

	child = widget->firstChild;

	while (child)
	{
		wz_widget_draw_by_less_than_or_equals_priority_recursive(priority, child);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

static void wz_widget_draw_by_priority_recursive(int priority, struct wzWidget *widget)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->drawPriority == priority && widget->vtable.draw)
	{
		widget->vtable.draw(widget);
	}

	child = widget->firstChild;

	while (child)
	{
		// If the priority is a match, draw children with <= priority.
		if (widget->drawPriority == priority)
		{
			wz_widget_draw_by_less_than_or_equals_priority_recursive(priority, child);
		}
		else
		{
			wz_widget_draw_by_priority_recursive(priority, child);
		}

		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_draw(struct wzDesktop *desktop)
{
	int drawPriorities[WZ_DRAW_PRIORITY_MAX];
	int nDrawPriorities;
	int i;

	assert(desktop);

	// Calculate unique draw priorities.
	nDrawPriorities = 0;
	wz_widget_calculate_unique_draw_priorities_recursive(drawPriorities, &nDrawPriorities, (struct wzWidget *)desktop);

	// Sort draw priorities in ascending order.
	qsort(drawPriorities, nDrawPriorities, sizeof(int), wz_compare_draw_priorities);

	// Do one draw pass per draw priority.
	for (i = 0; i < nDrawPriorities; i++)
	{
		wz_widget_draw_by_priority_recursive(drawPriorities[i], (struct wzWidget *)desktop);
	}
}

void wz_desktop_push_lock_input_widget(struct wzDesktop *desktop, struct wzWidget *widget)
{
	assert(desktop);
	stb_arr_push(desktop->lockInputWidgetStack, widget);
}

void wz_desktop_pop_lock_input_widget(struct wzDesktop *desktop, struct wzWidget *widget)
{
	// Only pop if the widget is on the top of the stack.
	if (stb_arr_len(desktop->lockInputWidgetStack) == 0)
		return;

	if (widget == desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)])
	{	
		stb_arr_pop(desktop->lockInputWidgetStack);
	}
}

