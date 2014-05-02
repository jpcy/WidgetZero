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
#include "wz_internal.h"

struct wzDesktop
{
	struct wzWidget base;
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

static void wz_widget_mouse_button_down_recursive(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *child;

	assert(widget);

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
	assert(desktop);
	wz_widget_mouse_button_down_recursive((struct wzWidget *)desktop, mouseButton, mouseX, mouseY);
}

static void wz_widget_mouse_button_up_recursive(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *child;

	assert(widget);

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
	assert(desktop);
	wz_widget_mouse_button_up_recursive((struct wzWidget *)desktop, mouseButton, mouseX, mouseY);
}

static void wz_widget_mouse_move_recursive(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzWidget *child;
	wzRect rect;

	assert(widget);

	rect = wz_widget_get_rect(widget);
	widget->hover = WZ_POINT_IN_RECT(mouseX, mouseY, rect);

	if (widget->vtable.mouse_move)
	{
		widget->vtable.mouse_move(widget, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
	}

	child = widget->firstChild;

	while (child)
	{
		wz_widget_mouse_move_recursive(child, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_mouse_move(struct wzDesktop *desktop, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	assert(desktop);
	wz_widget_mouse_move_recursive((struct wzWidget *)desktop, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
}

static void wz_widget_draw_recursive(struct wzWidget *widget)
{
	struct wzWidget *child;

	assert(widget);

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
	assert(desktop);
	wz_widget_draw_recursive((struct wzWidget *)desktop);
}
