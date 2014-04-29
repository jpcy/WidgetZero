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
#include <string.h>
#include "wz_internal.h"

wzWidgetType wz_widget_get_type(const struct wzWidget *widget)
{
	assert(widget);
	return widget->type;
}

void wz_widget_set_position(struct wzWidget *widget, int x, int y)
{
	assert(widget);
	widget->rect.x = x;
	widget->rect.y = y;
}

wzPosition wz_widget_get_position(const struct wzWidget *widget)
{
	wzPosition pos;

	assert(widget);
	pos.x = widget->rect.x;
	pos.y = widget->rect.y;
	return pos;
}

void wz_widget_set_size(struct wzWidget *widget, wzSize size)
{
	assert(widget);
	widget->rect.w = size.w;
	widget->rect.h = size.h;
}

wzSize wz_widget_get_size(const struct wzWidget *widget)
{
	wzSize size;

	assert(widget);
	size.w = widget->rect.w;
	size.h = widget->rect.h;
	return size;
}

void wz_widget_set_rect(struct wzWidget *widget, wzRect rect)
{
	assert(widget);
	memcpy(&widget->rect, &rect, sizeof(wzRect));
}

wzRect wz_widget_get_rect(const struct wzWidget *widget)
{
	assert(widget);
	return widget->rect;
}

bool wz_widget_get_hover(const struct wzWidget *widget)
{
	assert(widget);
	return widget->hover;
}

void wz_widget_set_metadata(struct wzWidget *widget, void *metadata)
{
	assert(widget);
	widget->metadata = metadata;
}

void *wz_widget_get_metadata(struct wzWidget *widget)
{
	assert(widget);
	return widget->metadata;
}

void wz_widget_set_draw_function(struct wzWidget *widget, void (*draw)(struct wzWidget *))
{
	assert(widget);
	widget->vtable.draw = draw;
}

bool wz_widget_can_have_child_widgets(const struct wzWidget *widget)
{
	assert(widget);
	return widget->canHaveChildWidgets;
}

void wz_widget_add_child_widget(struct wzWidget *widget, struct wzWidget *child)
{
	assert(widget);
	assert(child);

	if (!widget->canHaveChildWidgets)
		return;

	if (!widget->firstChild)
	{
		widget->firstChild = child;
		widget->firstChild->prev = widget->firstChild;
		widget->firstChild->next = widget->firstChild;
	}
	else
	{
		struct wzWidget *prev, *next;

		prev = widget->firstChild->prev;
		next = widget->firstChild;
		child->next = next;
		child->prev = prev;
		next->prev = child;
		prev->next = child;
	}
}
