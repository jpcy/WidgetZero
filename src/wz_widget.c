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
	wzRect rect;
	wzPosition pos;

	assert(widget);
	rect = wz_widget_get_rect(widget);
	pos.x = rect.x;
	pos.y = rect.y;
	return pos;
}

wzPosition wz_widget_get_absolute_position(const struct wzWidget *widget)
{
	wzRect rect;
	wzPosition pos;

	assert(widget);
	rect = wz_widget_get_absolute_rect(widget);
	pos.x = rect.x;
	pos.y = rect.y;
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

int wz_widget_get_width(const struct wzWidget *widget)
{
	return wz_widget_get_rect(widget).w;
}

int wz_widget_get_height(const struct wzWidget *widget)
{
	return wz_widget_get_rect(widget).h;
}

wzSize wz_widget_get_size(const struct wzWidget *widget)
{
	wzRect rect;
	wzSize size;

	assert(widget);
	rect = wz_widget_get_rect(widget);
	size.w = rect.w;
	size.h = rect.h;
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
	wzRect rect;

	assert(widget);

	if (widget->vtable.get_rect)
	{
		rect = widget->vtable.get_rect(widget);
	}
	else
	{
		rect = widget->rect;
	}

	// Handle autosizing.
	if (widget->parent && (widget->autosize & WZ_AUTOSIZE) != 0)
	{
		wzSize parentSize = wz_widget_get_size(widget->parent);

		if ((widget->autosize & WZ_AUTOSIZE_WIDTH) != 0)
		{
			rect.x = widget->margin.left;
			rect.w = parentSize.w - (widget->margin.left + widget->margin.right);
		}

		if ((widget->autosize & WZ_AUTOSIZE_HEIGHT) != 0)
		{
			rect.y = widget->margin.top;
			rect.h = parentSize.h - (widget->margin.top + widget->margin.bottom);
		}
	}

	return rect;
}

wzRect wz_widget_get_absolute_rect(const struct wzWidget *widget)
{
	wzRect rect;

	assert(widget);
	rect = wz_widget_get_rect(widget);

	if (widget->parent)
	{
		const wzPosition parentPosition = wz_widget_get_absolute_position(widget->parent);
		rect.x += parentPosition.x;
		rect.y += parentPosition.y;
	}

	return rect;
}

void wz_widget_set_margin(struct wzWidget *widget, wzBorder margin)
{
	assert(widget);
	widget->margin = margin;
}

wzBorder wz_widget_get_margin(const struct wzWidget *widget)
{
	assert(widget);
	return widget->margin;
}

void wz_widget_set_autosize(struct wzWidget *widget, int autosize)
{
	assert(widget);
	widget->autosize = autosize;
}

int wz_widget_get_autosize(const struct wzWidget *widget)
{
	assert(widget);
	return widget->autosize;
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

void wz_widget_set_draw_function(struct wzWidget *widget, void (*draw)(struct wzWidget *, wzRect))
{
	assert(widget);
	widget->vtable.draw = draw;
}

// Do this recursively, since it's possible to setup a widget heirarchy *before* adding the root widget via wz_widget_add_child_widget.
// Example: scroller does this with it's button children.
static void wz_widget_set_window_recursive(struct wzWidget *widget, struct wzWindow *window)
{
	int i;

	assert(widget);

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		struct wzWidget *child = widget->children[i];
		child->window = window;
		wz_widget_set_window_recursive(child, window);
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

	wz_widget_set_window_recursive(child, child->type == WZ_TYPE_WINDOW ? (struct wzWindow *)child : child->window);
	child->parent = widget;
	wz_arr_push(widget->children, child);
}

void wz_widget_destroy_child_widget(struct wzWidget *widget, struct wzWidget *child)
{
	int i, deleteIndex;

	assert(widget);
	assert(child);

	// Ensure the child is actually a child of widget before destroying it.
	deleteIndex = -1;

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		if (widget->children[i] == child)
		{
			deleteIndex = i;
			break;
		}
	}

	if (deleteIndex == -1)
		return;

	wz_arr_delete(widget->children, deleteIndex);
	wz_widget_destroy(child);
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

void wz_widget_set_internal_metadata(struct wzWidget *widget, void *metadata)
{
	assert(widget);
	widget->internalMetadata = metadata;
}

void *wz_widget_get_internal_metadata(struct wzWidget *widget)
{
	assert(widget);
	return widget->internalMetadata;
}

static void wz_widget_autosize_recursive(struct wzWidget *widget)
{
	int i;

	assert(widget);

	if (widget->vtable.autosize && (widget->autosize & (WZ_AUTOSIZE_WIDTH | WZ_AUTOSIZE_HEIGHT)) != 0)
	{
		widget->vtable.autosize(widget);
	}

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_autosize_recursive(widget->children[i]);
	}
}

void wz_widget_autosize_children(struct wzWidget *widget)
{
	int i;

	assert(widget);

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_autosize_recursive(widget->children[i]);
	}
}
