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

void wz_widget_destroy(struct wzWidget *widget)
{
	int i;

	assert(widget);

	// Destroy children.
	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_destroy(widget->children[i]);
	}

	wz_arr_free(widget->children);

	if (widget->vtable.destroy)
	{
		widget->vtable.destroy(widget);
	}

	free(widget);
}

struct wzDesktop *wz_widget_get_desktop(struct wzWidget *widget)
{
	assert(widget);
	assert(widget->desktop);
	return widget->desktop;
}

wzWidgetType wz_widget_get_type(const struct wzWidget *widget)
{
	assert(widget);
	return widget->type;
}

void wz_widget_set_position_args(struct wzWidget *widget, int x, int y)
{
	wzPosition position;
	position.x = x;
	position.y = y;
	wz_widget_set_position(widget, position);
}

void wz_widget_set_position(struct wzWidget *widget, wzPosition position)
{
	wzRect rect;

	assert(widget);
	rect = widget->rect;
	rect.x = position.x;
	rect.y = position.y;
	wz_widget_set_rect(widget, rect);
}

wzPosition wz_widget_get_position(const struct wzWidget *widget)
{
	wzPosition pos;

	assert(widget);
	pos.x = widget->rect.x;
	pos.y = widget->rect.y;
	return pos;
}

void wz_widget_set_width(struct wzWidget *widget, int w)
{
	wzRect rect;

	assert(widget);
	rect = widget->rect;
	rect.w = w;
	wz_widget_set_rect(widget, rect);
}

void wz_widget_set_height(struct wzWidget *widget, int h)
{
	wzRect rect;

	assert(widget);
	rect = widget->rect;
	rect.h = h;
	wz_widget_set_rect(widget, rect);
}

void wz_widget_set_size_args(struct wzWidget *widget, int w, int h)
{
	wzSize size;
	size.w = w;
	size.h = h;
	wz_widget_set_size(widget, size);
}

void wz_widget_set_size(struct wzWidget *widget, wzSize size)
{
	wzRect rect;

	assert(widget);
	rect = widget->rect;
	rect.w = size.w;
	rect.h = size.h;
	wz_widget_set_rect(widget, rect);
}

wzSize wz_widget_get_size(const struct wzWidget *widget)
{
	wzSize size;

	assert(widget);
	size.w = widget->rect.w;
	size.h = widget->rect.h;
	return size;
}

void wz_widget_set_rect_args(struct wzWidget *widget, int x, int y, int w, int h)
{
	wzRect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	wz_widget_set_rect(widget, rect);
}

void wz_widget_set_rect(struct wzWidget *widget, wzRect rect)
{
	assert(widget);

	if (widget->vtable.set_rect)
	{
		widget->vtable.set_rect(widget, rect);
	}
	else
	{
		widget->rect = rect;
	}
}

wzRect wz_widget_get_rect(const struct wzWidget *widget)
{
	assert(widget);
	return widget->rect;
}

wzRect wz_widget_get_absolute_rect(const struct wzWidget *widget)
{
	wzRect rect, contentRect;

	assert(widget);
	rect = widget->rect;

	// Adjust for desktop or window content rects.
	memset(&contentRect, 0, sizeof(contentRect));

	if (widget->window)
	{
		contentRect = wz_window_get_content_rect(widget->window);
	}
	else if (widget->type != WZ_TYPE_WINDOW)
	{
		contentRect = wz_desktop_get_content_rect(widget->desktop);		
	}

	rect.x += contentRect.x;
	rect.y += contentRect.y;

	// HACK: adjust for tab page rect.
	if (widget->parent && widget->parent->type == WZ_TYPE_TAB_PAGE)
	{
		contentRect = wz_widget_get_absolute_rect(widget->parent);
		rect.x += contentRect.x;
		rect.y += contentRect.y;
	}

	return rect;
}

bool wz_widget_get_hover(const struct wzWidget *widget)
{
	assert(widget);
	return widget->hover;
}

void wz_widget_set_visible(struct wzWidget *widget, bool visible)
{
	assert(widget);

	if (widget->vtable.set_visible)
	{
		widget->vtable.set_visible(widget, visible);
	}
	else
	{
		widget->hidden = !visible;
	}
}

bool wz_widget_get_visible(const struct wzWidget *widget)
{
	assert(widget);
	return !widget->hidden;
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

// Do this recursively, since it's possible to setup a widget heirarchy *before* adding the root widget via wz_widget_add_child_widget.
// Example: scroller does this with it's button children.
static void wz_widget_set_ancestors_recursive(struct wzWidget *widget, struct wzDesktop *desktop, struct wzWindow *window)
{
	int i;

	assert(widget);

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		struct wzWidget *child = widget->children[i];
		child->window = window;
		wz_widget_set_ancestors_recursive(child, desktop, window);
	}
}

void wz_widget_add_child_widget(struct wzWidget *widget, struct wzWidget *child)
{
	assert(widget);
	assert(child);

	// Desktops cannot be added as children.
	if (child->type == WZ_TYPE_DESKTOP)
		return;

	// Windows can only be children of desktop.
	if (child->type == WZ_TYPE_WINDOW && widget->type != WZ_TYPE_DESKTOP)
		return;

	// Find the closest ancestor window.
	child->window = (struct wzWindow *)wz_widget_find_closest_ancestor(widget, WZ_TYPE_WINDOW);

	wz_widget_set_ancestors_recursive(child, child->desktop, child->window);
	child->parent = widget;
	wz_arr_push(widget->children, child);
}

bool wz_widget_is_descendant_of(struct wzWidget *widget, wzWidgetType type)
{
	assert(widget);

	for (;;)
	{
		if (!widget->parent)
			break;

		if (widget->parent->type == type)
			return true;

		widget = widget->parent;
	}

	return false;
}

struct wzWidget *wz_widget_get_parent(struct wzWidget *widget)
{
	assert(widget);
	return widget->parent;
}

struct wzWindow *wz_widget_get_parent_window(struct wzWidget *widget)
{
	assert(widget);
	return widget->window;
}

struct wzWidget *wz_widget_find_closest_ancestor(struct wzWidget *widget, wzWidgetType type)
{
	struct wzWidget *temp = widget;

	for (;;)
	{
		if (temp == NULL)
			break;

		if (temp->type == type)
		{
			return temp;
		}

		temp = temp->parent;
	}

	return NULL;
}

void wz_widget_set_draw_priority_recursive(struct wzWidget *widget, int drawPriority)
{
	int i;

	assert(widget);
	widget->drawPriority = drawPriority;
	
	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_set_draw_priority_recursive(widget->children[i], drawPriority);
	}
}

void wz_widget_set_draw_priority(struct wzWidget *widget, int drawPriority)
{
	wz_widget_set_draw_priority_recursive(widget, drawPriority);
}

int wz_widget_get_draw_priority(const struct wzWidget *widget)
{
	assert(widget);
	return widget->drawPriority;
}

bool wz_widget_overlaps_parent_window(const struct wzWidget *widget)
{
	if (!widget->window)
		return true;

	// Always return true if the widget draw priority is higher than any window.
	if (widget->drawPriority > WZ_DRAW_PRIORITY_WINDOW_END)
		return true;

	return WZ_RECTS_OVERLAP(wz_widget_get_absolute_rect((struct wzWidget *)widget->window), wz_widget_get_absolute_rect(widget));
}

void wz_widget_set_clip_input_to_parent(struct wzWidget *widget, bool value)
{
	assert(widget);
	widget->inputNotClippedToParent = !value;
}
