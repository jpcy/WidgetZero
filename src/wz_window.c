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

struct wzWindow *wz_window_create(struct wzContext *context)
{
	struct wzWindow *window;

	assert(context);
	window = (struct wzWindow *)malloc(sizeof(struct wzWindow));
	window->context = context;
	window->firstChild = NULL;
	return window;
}

void wz_window_destroy(struct wzWindow *window)
{
	struct wzWidget *widget;
	
	assert(window);
	widget = window->firstChild;

	while (widget)
	{
		struct wzWidget *f = widget;
		widget = widget->next == window->firstChild ? NULL : widget->next;
		free(f);
	}

	free(window);
}

void wz_window_mouse_button_down(struct wzWindow *window, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	assert(window);
	widget = window->firstChild;

	while (widget)
	{
		if (widget->hover && widget->vtable.mouse_button_down)
		{
			widget->vtable.mouse_button_down(widget, mouseButton, mouseX, mouseY);
		}

		widget = widget->next == window->firstChild ? NULL : widget->next;
	}
}

void wz_window_mouse_button_up(struct wzWindow *window, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	assert(window);
	widget = window->firstChild;

	while (widget)
	{
		if (widget->vtable.mouse_button_up)
		{
			widget->vtable.mouse_button_up(widget, mouseButton, mouseX, mouseY);
		}

		widget = widget->next == window->firstChild ? NULL : widget->next;
	}
}

void wz_window_mouse_move(struct wzWindow *window, int mouseX, int mouseY)
{
	struct wzWidget *widget;
	wzRect rect;

	assert(window);
	widget = window->firstChild;

	while (widget)
	{
		rect = wz_widget_get_rect(widget);
		widget->hover = (mouseX >= rect.x && mouseX < rect.x + rect.w && mouseY >= rect.y && mouseY < rect.y + rect.h);
		widget = widget->next == window->firstChild ? NULL : widget->next;
	}
}

void wz_window_draw(struct wzWindow *window)
{
	struct wzWidget *widget;
	wzSize size;

	assert(window);
	widget = window->firstChild;

	while (widget)
	{
		if (widget->vtable.autosize)
		{
			size = widget->vtable.autosize(widget);
			widget->rect.w = size.w;
			widget->rect.h = size.h;
		}

		if (widget->vtable.draw)
		{
			widget->vtable.draw(widget);
		}

		widget = widget->next == window->firstChild ? NULL : widget->next;
	}
}

void wz_window_add_widget(struct wzWindow *window, struct wzWidget *widget)
{
	assert(window);
	assert(widget);

	if (window->firstChild == NULL)
	{
		window->firstChild = widget;
		window->firstChild->prev = window->firstChild;
		window->firstChild->next = window->firstChild;
	}
	else
	{
		struct wzWidget *prev, *next;

		prev = window->firstChild->prev;
		next = window->firstChild;
		widget->next = next;
		widget->prev = prev;
		next->prev = widget;
		prev->next = widget;
	}
}
