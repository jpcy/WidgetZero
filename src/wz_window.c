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
	memset(window->widgets, 0, sizeof(struct wzWidget *) * WZ_MAX_WINDOW_WIDGETS);
	window->nWidgets = 0;
	return window;
}

void wz_window_destroy(struct wzWindow *window)
{
	size_t i;

	for (i = 0; i < window->nWidgets; i++)
	{
		if (window->widgets[i] != NULL)
			free(window->widgets[i]);
	}

	free(window);
}

void wz_window_mouse_button_down(struct wzWindow *window, int mouseButton)
{
	assert(window);
}

void wz_window_mouse_button_up(struct wzWindow *window, int mouseButton)
{
	assert(window);
}

void wz_window_mouse_move(struct wzWindow *window, int mouseX, int mouseY)
{
	size_t i;
	struct wzWidget *widget;
	wzRect rect;

	assert(window);

	for (i = 0; i < window->nWidgets; i++)
	{
		widget = window->widgets[i];

		if (widget == NULL)
			continue;
	
		rect = wz_widget_get_rect(widget);
		widget->hover = (mouseX >= rect.x && mouseX < rect.x + rect.w && mouseY >= rect.y && mouseY < rect.y + rect.h);
	}
}

void wz_window_draw(struct wzWindow *window)
{
	size_t i;
	struct wzWidget *widget;
	const wzWidgetBehavior *behavior;
	wzSize size;

	assert(window);

	for (i = 0; i < window->nWidgets; i++)
	{
		widget = window->widgets[i];

		if (widget == NULL)
			continue;
	
		behavior = window->context->behaviors[widget->type];
		size = behavior->autosize(widget);
		widget->rect.w = size.w;
		widget->rect.h = size.h;
		behavior->draw(widget);
	}
}

void wz_window_add_widget(struct wzWindow *window, struct wzWidget *widget)
{
	size_t i;

	assert(window);
	assert(widget);

	for (i = 0; i < WZ_MAX_WINDOW_WIDGETS; i++)
	{
		if (window->widgets[i] == NULL)
		{
			window->widgets[i] = widget;
			
			if (i + 1 > window->nWidgets)
				window->nWidgets = i + 1;

			return;
		}
	}
}
