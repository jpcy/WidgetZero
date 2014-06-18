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
/*
wz_intersect_rects is SDL_IntersectRect

  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "wz_widget.h"

/*
================================================================================

RECTANGLES

================================================================================
*/

bool wz_is_rect_empty(wzRect rect)
{
	return rect.x == 0 && rect.y == 0 && rect.w == 0 && rect.h == 0;
}

bool wz_intersect_rects(wzRect A, wzRect B, wzRect *result)
{
    int Amin, Amax, Bmin, Bmax;

	assert(result);

    /* Special cases for empty rects */
    if (wz_is_rect_empty(A) || wz_is_rect_empty(B))
	{
		result->x = result->y = result->w = result->h = 0;
        return false;
    }

    /* Horizontal intersection */
    Amin = A.x;
    Amax = Amin + A.w;
    Bmin = B.x;
    Bmax = Bmin + B.w;
    if (Bmin > Amin)
        Amin = Bmin;
    result->x = Amin;
    if (Bmax < Amax)
        Amax = Bmax;
    result->w = Amax - Amin;

    /* Vertical intersection */
    Amin = A.y;
    Amax = Amin + A.h;
    Bmin = B.y;
    Bmax = Bmin + B.h;
    if (Bmin > Amin)
        Amin = Bmin;
    result->y = Amin;
    if (Bmax < Amax)
        Amax = Bmax;
    result->h = Amax - Amin;

    return !wz_is_rect_empty(*result);
}

/*
================================================================================

AUTOSIZING

================================================================================
*/

// Applies autosizing to the provided rect.
static wzRect wz_widget_calculate_autosized_rect(const struct wzWidget *widget, wzRect rect)
{
	assert(widget);

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

static void wz_widget_set_autosize_rect_recursive(struct wzWidget *widget)
{
	int i;
	bool recurse;

	assert(widget);

	// Don't do anything to widgets that aren't set to autosize, but still recurse on children.
	recurse = true;

	if ((widget->autosize & WZ_AUTOSIZE) != 0)
	{
		wzRect oldRect, newRect;

		oldRect = wz_widget_get_rect(widget);
		newRect = wz_widget_calculate_autosized_rect(widget, oldRect);

		if (widget->vtable.set_rect)
		{
			widget->vtable.set_rect(widget, newRect);
			oldRect = wz_widget_get_rect(widget);
		}
		else
		{
			widget->rect = newRect;
		}

		// Don't recurse if the rect hasn't changed.
		recurse = (oldRect.w != newRect.w || oldRect.h != newRect.h);
	}

	if (recurse)
	{
		for (i = 0; i < wz_arr_len(widget->children); i++)
		{
			wz_widget_set_autosize_rect_recursive(widget->children[i]);
		}
	}
}

/*
================================================================================

MISC.

================================================================================
*/

static void wz_widget_refresh_rect(struct wzWidget *widget)
{
	assert(widget);
	wz_widget_set_rect(widget, wz_widget_get_rect(widget));
}

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

bool wz_widget_is_layout(const struct wzWidget *widget)
{
	assert(widget);
	return widget->type == WZ_TYPE_VERTICAL_STACK_LAYOUT;
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
	wzRect oldRect;
	int i;

	assert(widget);
	oldRect = widget->rect;

	// Apply autosizing.
	rect = wz_widget_calculate_autosized_rect(widget, rect);

	if (widget->vtable.set_rect)
	{
		widget->vtable.set_rect(widget, rect);
	}
	else
	{
		widget->rect = rect;
	}

	// Autosize children too.
	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_set_autosize_rect_recursive(widget->children[i]);
	}

	// If the parent is a layout widget, it may need refreshing.
	if (widget->parent && wz_widget_is_layout(widget->parent))
	{
		// Refresh if the width or height has changed and stretching is off.
		if (((widget->stretch & WZ_STRETCH_HORIZONTAL) == 0 && widget->rect.w != oldRect.w) || ((widget->stretch & WZ_STRETCH_VERTICAL) == 0 && widget->rect.h != oldRect.h))
		{
			wz_widget_refresh_rect(widget->parent);
		}
	}
}

wzRect wz_widget_get_rect(const struct wzWidget *widget)
{
	assert(widget);
	return widget->rect;
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

	// If the parent is a layout widget, refresh it.
	if (widget->parent && wz_widget_is_layout(widget->parent))
	{
		wz_widget_refresh_rect(widget->parent);
	}
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

	if ((widget->autosize & WZ_AUTOSIZE) != 0)
	{
		wz_widget_set_autosize_rect_recursive(widget);
	}
}

int wz_widget_get_autosize(const struct wzWidget *widget)
{
	assert(widget);
	return widget->autosize;
}

void wz_widget_set_stretch(struct wzWidget *widget, int stretch)
{
	assert(widget);
	widget->stretch = stretch;

	// If the parent is a layout widget, refresh it.
	if (widget->parent && wz_widget_is_layout(widget->parent))
	{
		wz_widget_refresh_rect(widget->parent);
	}
}

int wz_widget_get_stretch(const struct wzWidget *widget)
{
	assert(widget);
	return widget->stretch;
}

void wz_widget_set_align(struct wzWidget *widget, int align)
{
	assert(widget);
	widget->align = align;

	// If the parent is a layout widget, refresh it.
	if (widget->parent && wz_widget_is_layout(widget->parent))
	{
		wz_widget_refresh_rect(widget->parent);
	}
}

int wz_widget_get_align(const struct wzWidget *widget)
{
	assert(widget);
	return widget->align;
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

	// If the parent is a layout widget, refresh it.
	if (wz_widget_is_layout(widget))
	{
		wz_widget_refresh_rect(widget);
	}
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

void wz_widget_set_draw_priority(struct wzWidget *widget, int drawPriority)
{
	assert(widget);
	widget->drawPriority = drawPriority;
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

	// Always return true if the inherited widget draw priority is higher than window.
	if (wz_widget_calculate_inherited_draw_priority(widget) > WZ_DRAW_PRIORITY_WINDOW)
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

int wz_widget_calculate_inherited_draw_priority(const struct wzWidget *widget)
{
	int drawPriority = WZ_DRAW_PRIORITY_DEFAULT;

	for (;;)
	{
		if (widget == NULL)
			break;

		drawPriority = WZ_MAX(drawPriority, widget->drawPriority);
		widget = widget->parent;
	}

	return drawPriority;
}
