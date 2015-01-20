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
#include <stdlib.h>
#include <string.h>
#include "wz_layout.h"
#include "wz_widget.h"

namespace wz {

const wzBorder wzBorder_zero;

wzWidget::wzWidget()
{
	type = WZ_TYPE_WIDGET;
	stretch = 0;
	stretchWidthScale = 0;
	stretchHeightScale = 0;
	align = 0;
	internalMetadata = NULL;
	metadata = NULL;
	flags = 0;
	hover = false;
	hidden = false;
	ignore = false;
	overlap = false;
	drawManually = false;
	inputNotClippedToParent = false;
	fontSize = 0;
	fontFace[0] = NULL;
	renderer = NULL;
	mainWindow = NULL;
	window = NULL;
	parent = NULL;
	memset(&vtable, 0, sizeof(vtable));
}

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

	WZ_ASSERT(result);

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

PUBLIC WIDGET FUNCTIONS

================================================================================
*/

void wz_widget_destroy(struct wzWidget *widget)
{
	WZ_ASSERT(widget);

	// Destroy children.
	for (size_t i = 0; i < widget->children.size(); i++)
	{
		wz_widget_destroy(widget->children[i]);
	}

	widget->children.clear();

	if (widget->vtable.destroy)
	{
		widget->vtable.destroy(widget);
	}

	delete widget;
}

struct wzMainWindow *wz_widget_get_main_window(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->mainWindow;
}

wzWidgetType wz_widget_get_type(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->type;
}

bool wz_widget_is_layout(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->type == WZ_TYPE_STACK_LAYOUT;
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

	WZ_ASSERT(widget);
	rect = widget->userRect;
	rect.x = position.x;
	rect.y = position.y;
	wz_widget_set_rect(widget, rect);
}

wzPosition wz_widget_get_position(const struct wzWidget *widget)
{
	wzRect rect;
	wzPosition pos;

	WZ_ASSERT(widget);
	rect = wz_widget_get_rect(widget);
	pos.x = rect.x;
	pos.y = rect.y;
	return pos;
}

wzPosition wz_widget_get_absolute_position(const struct wzWidget *widget)
{
	wzRect rect;
	wzPosition pos;

	WZ_ASSERT(widget);
	rect = wz_widget_get_absolute_rect(widget);
	pos.x = rect.x;
	pos.y = rect.y;
	return pos;
}

void wz_widget_set_width(struct wzWidget *widget, int w)
{
	wzRect rect;

	WZ_ASSERT(widget);
	rect = widget->userRect;
	rect.w = w;
	wz_widget_set_rect(widget, rect);
}

void wz_widget_set_height(struct wzWidget *widget, int h)
{
	wzRect rect;

	WZ_ASSERT(widget);
	rect = widget->userRect;
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

	WZ_ASSERT(widget);
	rect = widget->userRect;
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

	WZ_ASSERT(widget);
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
	WZ_ASSERT(widget);
	widget->userRect = rect;
	wz_widget_set_rect_internal(widget, rect);
}

wzRect wz_widget_get_rect(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->rect;
}

wzRect wz_widget_get_absolute_rect(const struct wzWidget *widget)
{
	wzRect rect;

	WZ_ASSERT(widget);
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
	WZ_ASSERT(widget);
	widget->margin = margin;
	wz_widget_refresh_rect(widget);
}

void wz_widget_set_margin_args(struct wzWidget *widget, int top, int right, int bottom, int left)
{
	wzBorder margin;
	margin.top = top;
	margin.right = right;
	margin.bottom = bottom;
	margin.left = left;
	wz_widget_set_margin(widget, margin);
}

void wz_widget_set_margin_uniform(struct wzWidget *widget, int value)
{
	wzBorder margin;
	margin.top = margin.right = margin.bottom = margin.left = value;
	wz_widget_set_margin(widget, margin);
}

wzBorder wz_widget_get_margin(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->margin;
}

void wz_widget_set_stretch(struct wzWidget *widget, int stretch)
{
	WZ_ASSERT(widget);
	widget->stretch = stretch;

	// If the parent is a layout widget, refresh it.
	if (widget->parent && wz_widget_is_layout(widget->parent))
	{
		wz_widget_refresh_rect(widget->parent);
	}
}

int wz_widget_get_stretch(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->stretch;
}

void wz_widget_set_stretch_scale(struct wzWidget *widget, float width, float height)
{
	WZ_ASSERT(widget);
	widget->stretchWidthScale = width;
	widget->stretchHeightScale = height;
}

float wz_widget_get_stretch_width_scale(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->stretchWidthScale;
}

float wz_widget_get_stretch_height_scale(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->stretchHeightScale;
}

void wz_widget_set_align(struct wzWidget *widget, int align)
{
	WZ_ASSERT(widget);
	widget->align = align;

	// If the parent is a layout widget, refresh it.
	if (widget->parent && wz_widget_is_layout(widget->parent))
	{
		wz_widget_refresh_rect(widget->parent);
	}
}

int wz_widget_get_align(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->align;
}

void wz_widget_set_font_face(struct wzWidget *widget, const char *fontFace)
{
	WZ_ASSERT(widget);
	strcpy(widget->fontFace, fontFace);
	wz_widget_resize_to_measured(widget);

	if (widget->vtable.font_changed)
	{
		widget->vtable.font_changed(widget, widget->fontFace, widget->fontSize);
	}
}

const char *wz_widget_get_font_face(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->fontFace;
}

void wz_widget_set_font_size(struct wzWidget *widget, float fontSize)
{
	WZ_ASSERT(widget);
	widget->fontSize = fontSize;
	wz_widget_resize_to_measured(widget);

	if (widget->vtable.font_changed)
	{
		widget->vtable.font_changed(widget, widget->fontFace, widget->fontSize);
	}
}

float wz_widget_get_font_size(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->fontSize;
}

void wz_widget_set_font(struct wzWidget *widget, const char *fontFace, float fontSize)
{
	WZ_ASSERT(widget);
	strcpy(widget->fontFace, fontFace);
	widget->fontSize = fontSize;
	wz_widget_resize_to_measured(widget);

	if (widget->vtable.font_changed)
	{
		widget->vtable.font_changed(widget, widget->fontFace, widget->fontSize);
	}
}

bool wz_widget_get_hover(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->hover;
}

void wz_widget_set_visible(struct wzWidget *widget, bool visible)
{
	WZ_ASSERT(widget);

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
	WZ_ASSERT(widget);
	return !widget->hidden;
}

bool wz_widget_has_keyboard_focus(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return wz_main_window_get_keyboard_focus_widget(widget->mainWindow) == widget;
}

void wz_widget_set_metadata(struct wzWidget *widget, void *metadata)
{
	WZ_ASSERT(widget);
	widget->metadata = metadata;
}

void *wz_widget_get_metadata(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->metadata;
}

void wz_widget_set_draw_callback(struct wzWidget *widget, wzWidgetDrawCallback draw)
{
	WZ_ASSERT(widget);
	widget->vtable.draw = draw;
}

void wz_widget_set_measure_callback(struct wzWidget *widget, wzWidgetMeasureCallback measure)
{
	WZ_ASSERT(widget);
	widget->vtable.measure = measure;
}

static struct wzMainWindow *wz_widget_find_main_window(struct wzWidget *widget)
{
	for (;;)
	{
		if (!widget)
			break;

		if (widget->type == WZ_TYPE_MAIN_WINDOW)
			return (struct wzMainWindow *)widget;

		widget = widget->parent;
	}

	return NULL;
}

static void wz_widget_set_renderer(struct wzWidget *widget, struct wzRenderer *renderer)
{
	struct wzRenderer *oldRenderer;

	WZ_ASSERT(widget);
	oldRenderer = widget->renderer;
	widget->renderer = renderer;

	if (oldRenderer != widget->renderer && widget->vtable.renderer_changed)
	{
		widget->vtable.renderer_changed(widget);
	}
}

// Do this recursively, since it's possible to setup a widget heirarchy *before* adding the root widget via wz_widget_add_child_widget.
// Example: scroller does this with it's button children.
static void wz_widget_set_main_window_and_window_recursive(struct wzWidget *widget, struct wzMainWindow *mainWindow, struct wzWindow *window)
{
	WZ_ASSERT(widget);

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		struct wzWidget *child = widget->children[i];
		child->mainWindow = mainWindow;
		child->window = window;

		// Set the renderer too.
		if (mainWindow)
		{
			wz_widget_set_renderer(child, ((struct wzWidget *)mainWindow)->renderer);
		}

		wz_widget_set_main_window_and_window_recursive(child, mainWindow, window);
	}
}

bool wz_widget_is_descendant_of(struct wzWidget *widget, wzWidgetType type)
{
	WZ_ASSERT(widget);

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
	WZ_ASSERT(widget);
	return widget->parent;
}

struct wzWindow *wz_widget_get_parent_window(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->window;
}

/*
================================================================================

WIDGET MEASURING

================================================================================
*/

void wz_widget_resize_to_measured(struct wzWidget *widget)
{
	WZ_ASSERT(widget);

	if (widget->renderer && widget->vtable.measure)
	{
		wzSize size = widget->vtable.measure(widget);

		// The explicitly set size overrides the measured size.
		if (widget->userRect.w != 0)
		{
			size.w = widget->userRect.w;
		}

		if (widget->userRect.h != 0)
		{
			size.h = widget->userRect.h;
		}

		// Keep the current size if 0.
		if (size.w == 0)
		{
			size.w = wz_widget_get_width(widget);
		}

		if (size.h == 0)
		{
			size.h = wz_widget_get_height(widget);
		}
		
		// Set the size.
		wz_widget_set_size_internal(widget, size);
	}
}

static void wz_widget_measure_and_resize_recursive(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	wz_widget_resize_to_measured(widget);

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		wz_widget_measure_and_resize_recursive(widget->children[i]);
	}
}

/*
================================================================================

INTERNAL WIDGET FUNCTIONS

================================================================================
*/

void wz_widget_add_child_widget(struct wzWidget *widget, struct wzWidget *child)
{
	WZ_ASSERT(widget);
	WZ_ASSERT(child);

	// Set mainWindow.
	child->mainWindow = wz_widget_find_main_window(widget);

	// Find the closest ancestor window.
	child->window = (struct wzWindow *)wz_widget_find_closest_ancestor(widget, WZ_TYPE_WINDOW);

	// Set the renderer.
	if (child->mainWindow)
	{
		wz_widget_set_renderer(child, ((struct wzWidget *)child->mainWindow)->renderer);
	}

	// Set children mainWindow, window and renderer.
	wz_widget_set_main_window_and_window_recursive(child, child->mainWindow, child->type == WZ_TYPE_WINDOW ? (struct wzWindow *)child : child->window);

	child->parent = widget;
	widget->children.push_back(child);

	// Resize the widget and children to their measured sizes.
	wz_widget_measure_and_resize_recursive(child);
	wz_widget_refresh_rect(child);

	if (child->vtable.added)
	{
		child->vtable.added(widget, child);
	}
}

void wz_widget_remove_child_widget(struct wzWidget *widget, struct wzWidget *child)
{
	int deleteIndex;

	WZ_ASSERT(widget);
	WZ_ASSERT(child);

	// Ensure the child is actually a child of widget before destroying it.
	deleteIndex = -1;

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		if (widget->children[i] == child)
		{
			deleteIndex = (int)i;
			break;
		}
	}

	if (deleteIndex == -1)
		return;

	widget->children.erase(widget->children.begin() + deleteIndex);

	// The child is no longer connected to the widget hierarchy, so reset some state.
	child->mainWindow = NULL;
	child->parent = NULL;
	child->window = NULL;
}

void wz_widget_destroy_child_widget(struct wzWidget *widget, struct wzWidget *child)
{
	WZ_ASSERT(widget);
	const size_t n = widget->children.size();
	wz_widget_remove_child_widget(widget, child);

	// Don't destroy if the child wasn't removed. Happens if it is not really a child, see wz_widget_remove_child_widget.
	if (n == widget->children.size())
		return;

	wz_widget_destroy(child);
}

void wz_widget_set_position_args_internal(struct wzWidget *widget, int x, int y)
{
	wzPosition position;
	position.x = x;
	position.y = y;
	wz_widget_set_position_internal(widget, position);
}

void wz_widget_set_position_internal(struct wzWidget *widget, wzPosition position)
{
	wzRect rect;

	WZ_ASSERT(widget);
	rect = widget->rect;
	rect.x = position.x;
	rect.y = position.y;
	wz_widget_set_rect_internal(widget, rect);
}

void wz_widget_set_width_internal(struct wzWidget *widget, int w)
{
	wzRect rect;

	WZ_ASSERT(widget);
	rect = widget->rect;
	rect.w = w;
	wz_widget_set_rect_internal(widget, rect);
}

void wz_widget_set_height_internal(struct wzWidget *widget, int h)
{
	wzRect rect;

	WZ_ASSERT(widget);
	rect = widget->rect;
	rect.h = h;
	wz_widget_set_rect_internal(widget, rect);
}

void wz_widget_set_size_args_internal(struct wzWidget *widget, int w, int h)
{
	wzSize size;
	size.w = w;
	size.h = h;
	wz_widget_set_size_internal(widget, size);
}

void wz_widget_set_size_internal(struct wzWidget *widget, wzSize size)
{
	wzRect rect;

	WZ_ASSERT(widget);
	rect = widget->rect;
	rect.w = size.w;
	rect.h = size.h;
	wz_widget_set_rect_internal(widget, rect);
}

void wz_widget_set_rect_args_internal(struct wzWidget *widget, int x, int y, int w, int h)
{
	wzRect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	wz_widget_set_rect_internal(widget, rect);
}

static void wz_widget_set_rect_internal_recursive(struct wzWidget *widget, wzRect rect)
{
	wzRect oldRect;

	WZ_ASSERT(widget);
	oldRect = widget->rect;

	// Apply alignment and stretching.
	rect = wz_widget_calculate_aligned_stretched_rect(widget, rect);

	if (widget->vtable.set_rect)
	{
		widget->vtable.set_rect(widget, rect);
	}
	else
	{
		widget->rect = rect;
	}

	// Don't recurse if the rect hasn't changed.
	if (oldRect.x != rect.x || oldRect.y != rect.y || oldRect.w != rect.w || oldRect.h != rect.h)
	{
		for (size_t i = 0; i < widget->children.size(); i++)
		{
			wz_widget_set_rect_internal_recursive(widget->children[i], widget->children[i]->rect);
		}
	}
}

void wz_widget_set_rect_internal(struct wzWidget *widget, wzRect rect)
{
	wzRect oldRect;

	WZ_ASSERT(widget);
	oldRect = widget->rect;
	wz_widget_set_rect_internal_recursive(widget, rect);	

	// If the parent is a layout widget, it may need refreshing.
	if (widget->parent && wz_widget_is_layout(widget->parent))
	{
		// Refresh if the width or height has changed.
		if (widget->rect.w != oldRect.w || widget->rect.h != oldRect.h)
		{
			wz_widget_refresh_rect(widget->parent);
		}
	}
}

void wz_widget_refresh_rect(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	wz_widget_set_rect_internal(widget, wz_widget_get_rect(widget));
}

struct wzWidget *wz_widget_find_closest_ancestor(const struct wzWidget *widget, wzWidgetType type)
{
	struct wzWidget *temp = (struct wzWidget *)widget;

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

void wz_widget_set_draw_manually(struct wzWidget *widget, bool value)
{
	widget->drawManually = value;
}

void wz_widget_set_draw_last(struct wzWidget *widget, bool value)
{
	if (value)
	{
		widget->flags |= WZ_WIDGET_FLAG_DRAW_LAST;
	}
	else
	{
		widget->flags &= ~WZ_WIDGET_FLAG_DRAW_LAST;
	}
}

void wz_widget_set_overlap(struct wzWidget *widget, bool value)
{
	widget->overlap = value;
}

bool wz_widget_overlaps_parent_window(const struct wzWidget *widget)
{
	if (!widget->window)
		return true;

	return WZ_RECTS_OVERLAP(wz_widget_get_absolute_rect((struct wzWidget *)widget->window), wz_widget_get_absolute_rect(widget));
}

void wz_widget_set_clip_input_to_parent(struct wzWidget *widget, bool value)
{
	WZ_ASSERT(widget);
	widget->inputNotClippedToParent = !value;
}

void wz_widget_set_internal_metadata(struct wzWidget *widget, void *metadata)
{
	WZ_ASSERT(widget);
	widget->internalMetadata = metadata;
}

void *wz_widget_get_internal_metadata(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->internalMetadata;
}

int wz_widget_get_line_height(const struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return wz_renderer_get_line_height(widget->renderer, widget->fontFace, widget->fontSize);
}

void wz_widget_measure_text(const struct wzWidget *widget, const char *text, int n, int *width, int *height)
{
	WZ_ASSERT(widget);
	wz_renderer_measure_text(widget->renderer, widget->fontFace, widget->fontSize, text, n, width, height);
}

} // namespace wz
