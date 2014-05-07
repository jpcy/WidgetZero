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

// Returns the window that the mouse cursor is hovering over. NULL if there isn't one.
static struct wzWindow *wz_desktop_get_hover_window(struct wzDesktop *desktop, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	assert(desktop);
	widget = desktop->base.firstChild;

	while (widget)
	{
		if (widget->type == WZ_TYPE_WINDOW && WZ_POINT_IN_RECT(mouseX, mouseY, widget->rect))
			return (struct wzWindow *)widget;

		widget = widget->next == desktop->base.firstChild ? NULL : widget->next;
	}

	return NULL;
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

static void wz_widget_draw_recursive(struct wzWidget *widget)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->vtable.draw)
	{
		widget->vtable.draw(widget);
	}

	child = widget->firstChild;

	while (child)
	{
		wz_widget_draw_recursive(child);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_draw(struct wzDesktop *desktop)
{
	struct wzWidget *child;

	assert(desktop);

	// Skip window children.
	child = desktop->base.firstChild;

	while (child)
	{
		if (child->type != WZ_TYPE_WINDOW)
			wz_widget_draw_recursive(child);

		child = child->next == desktop->base.firstChild ? NULL : child->next;
	}

	// Now draw window children.
	child = desktop->base.firstChild;

	while (child)
	{
		if (child->type == WZ_TYPE_WINDOW)
			wz_widget_draw_recursive(child);

		child = child->next == desktop->base.firstChild ? NULL : child->next;
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
