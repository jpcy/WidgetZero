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
#include <stdarg.h>
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

//------------------------------------------------------------------------------

struct ButtonPrivate : public wzWidget
{
	ButtonPrivate()
	{
		button = wz_button_create(NULL, NULL);
		wz_widget_set_metadata((wzWidget *)button, this);
	}

	~ButtonPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)button))
		{
			wz_widget_destroy((wzWidget *)button);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }

	wzButton *button;
};

struct CheckboxPrivate : public wzWidget
{
	CheckboxPrivate()
	{
		checkBox = wz_check_box_create(NULL);
		wz_widget_set_metadata((wzWidget *)checkBox, this);
	}

	~CheckboxPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)checkBox))
		{
			wz_widget_destroy((wzWidget *)checkBox);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)checkBox; }
	virtual wzWidget *getWidget() { return (wzWidget *)checkBox; }

	wzCheckBox *checkBox;
};

struct ComboPrivate : public wzWidget
{
	ComboPrivate()
	{
		combo = wz_combo_create(NULL, 0, 0);
		wz_widget_set_metadata((wzWidget *)combo, this);
	}

	~ComboPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)combo))
		{
			wz_widget_destroy((wzWidget *)combo);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)combo; }
	virtual wzWidget *getWidget() { return (wzWidget *)combo; }

	wzCombo *combo;
};

struct FramePrivate : public wzWidget
{
	FramePrivate()
	{
		frame = wz_frame_create();
		wzWidget *widget = (wzWidget *)frame;
		wz_widget_set_metadata(widget, this);
		wz_widget_set_size_args(widget, 200, 200);
	}

	~FramePrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)frame))
		{
			wz_widget_destroy((wzWidget *)frame);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)frame; }
	virtual wzWidget *getWidget() { return (wzWidget *)frame; }

	wzFrame *frame;
};

struct GroupBoxPrivate : public wzWidget
{
	GroupBoxPrivate()
	{
		groupBox = wz_group_box_create(NULL);
		wz_widget_set_metadata((wzWidget *)groupBox, this);
		wz_widget_set_size_args((wzWidget *)groupBox, 200, 200);
	}

	~GroupBoxPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)groupBox))
		{
			wz_widget_destroy((wzWidget *)groupBox);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)groupBox; }
	virtual wzWidget *getWidget() { return (wzWidget *)groupBox; }

	wzGroupBox *groupBox;
};

struct LabelPrivate : public wzWidget
{
	LabelPrivate()
	{
		label = wz_label_create(NULL);
		wz_widget_set_metadata((wzWidget *)label, this);
	}

	~LabelPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)label))
		{
			wz_widget_destroy((wzWidget *)label);
		}
	}
	virtual const wzWidget *getWidget() const { return (const wzWidget *)label; }
	virtual wzWidget *getWidget() { return (wzWidget *)label; }

	wzLabel *label;
};

struct ListPrivate : public wzWidget
{
	ListPrivate()
	{
		list = wz_list_create(NULL, 0, 0);
		wz_widget_set_metadata((wzWidget *)list, this);
	}

	~ListPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)list))
		{
			wz_widget_destroy((wzWidget *)list);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)list; }
	virtual wzWidget *getWidget() { return (wzWidget *)list; }

	wzList *list;
};

struct MainWindowPrivate : public wzWidget
{
	MainWindowPrivate(wzRenderer *renderer)
	{
		WZ_ASSERT(renderer);
		this->renderer = renderer;
		mainWindow = wz_main_window_create(renderer);
		wz_widget_set_metadata((wzWidget *)mainWindow, this);

		menuBar = wz_menu_bar_create();
		wz_main_window_set_menu_bar(mainWindow, menuBar);
	}

	~MainWindowPrivate()
	{
		wz_widget_destroy((wzWidget *)mainWindow);
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)mainWindow; }
	virtual wzWidget *getWidget() { return (wzWidget *)mainWindow; }

	wzMainWindow *mainWindow;
	wzMenuBar *menuBar;
	wzRenderer *renderer;
};

struct RadioButtonPrivate : public wzWidget
{
	RadioButtonPrivate()
	{
		button = wz_radio_button_create(NULL);
		wz_widget_set_metadata((wzWidget *)button, this);
	}

	~RadioButtonPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)button))
		{
			wz_widget_destroy((wzWidget *)button);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }

	wzRadioButton *button;
};

struct ScrollerPrivate : public wzWidget
{
	ScrollerPrivate(wzScrollerType type)
	{
		scroller = wz_scroller_create(type, 0, 1, 0);
		wz_widget_set_metadata((wzWidget *)scroller, this);
	}

	~ScrollerPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)scroller))
		{
			wz_widget_destroy((wzWidget *)scroller);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)scroller; }
	virtual wzWidget *getWidget() { return (wzWidget *)scroller; }

	wzScroller *scroller;
};

struct SpinnerPrivate : public wzWidget
{
	SpinnerPrivate()
	{
		spinner = wz_spinner_create();
		wz_widget_set_metadata((wzWidget *)spinner, this);
	}

	~SpinnerPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)spinner))
		{
			wz_widget_destroy((wzWidget *)spinner);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)spinner; }
	virtual wzWidget *getWidget() { return (wzWidget *)spinner; }

	wzSpinner *spinner;
};

struct StackLayoutPrivate : public wzWidget
{
	StackLayoutPrivate()
	{
		layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 0);
	}

	~StackLayoutPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)layout))
		{
			wz_widget_destroy((wzWidget *)layout);
		}
	}

	virtual const wzWidget *getWidget() const { return (wzWidget *)layout; }
	virtual wzWidget *getWidget() { return (wzWidget *)layout; }
	
	wzStackLayout *layout;
};

// Wraps tab button and page.
struct TabPrivate
{
	TabPrivate() : button(NULL), page(NULL) {}
	wzButton *button;
	wzWidget *page;
};

struct TabbedPrivate : public wzWidget
{
	TabbedPrivate()
	{
		tabbed = wz_tabbed_create();
		wz_widget_set_metadata((wzWidget *)tabbed, this);
	}

	~TabbedPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)tabbed))
		{
			wz_widget_destroy((wzWidget *)tabbed);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabbed; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabbed; }

	wzTabbed *tabbed;
};

struct TextEditPrivate : public wzWidget
{
	TextEditPrivate(bool multiline)
	{
		textEdit = wz_text_edit_create(multiline, 256);
		wz_widget_set_metadata((wzWidget *)textEdit, this);
	}

	~TextEditPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)textEdit))
		{
			wz_widget_destroy((wzWidget *)textEdit);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)textEdit; }
	virtual wzWidget *getWidget() { return (wzWidget *)textEdit; }

	wzTextEdit *textEdit;
};

struct ToggleButtonPrivate : public wzWidget
{
	ToggleButtonPrivate()
	{
		button = wz_toggle_button_create(NULL, NULL);
		wz_widget_set_metadata((wzWidget *)button, this);
	}

	~ToggleButtonPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)button))
		{
			wz_widget_destroy((wzWidget *)button);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }

	wzButton *button;
};

struct WindowPrivate : public wzWidget
{
	WindowPrivate()
	{
		window = wz_window_create(NULL);
		wz_widget_set_metadata((wzWidget *)window, this);
	}

	~WindowPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)window))
		{
			wz_widget_destroy((wzWidget *)window);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)window; }
	virtual wzWidget *getWidget() { return (wzWidget *)window; }

	wzWindow *window;
};

//------------------------------------------------------------------------------

Widget::~Widget()
{
}

wzRect Widget::getRect() const
{
	return wz_widget_get_absolute_rect(p->getWidget());
}

Widget *Widget::setPosition(int x, int y)
{ 
	wz_widget_set_position_args(p->getWidget(), x, y);
	return this;
}

Widget *Widget::setWidth(int w)
{
	wz_widget_set_width(p->getWidget(), w);
	return this;
}

Widget *Widget::setHeight(int h)
{
	wz_widget_set_height(p->getWidget(), h);
	return this;
}

Widget *Widget::setSize(int w, int h)
{
	wz_widget_set_size_args(p->getWidget(), w, h);
	return this;
}

Widget *Widget::setRect(int x, int y, int w, int h)
{
	wz_widget_set_rect_args(p->getWidget(), x, y, w, h);
	return this;
}

Widget *Widget::setStretch(int stretch)
{
	wz_widget_set_stretch(p->getWidget(), stretch);
	return this;
}

Widget *Widget::setAlign(int align)
{
	wz_widget_set_align(p->getWidget(), align);
	return this;
}

Widget *Widget::setMargin(int margin)
{
	wz_widget_set_margin_args(p->getWidget(), margin, margin, margin, margin);
	return this;
}

Widget *Widget::setMargin(int top, int right, int bottom, int left)
{
	wz_widget_set_margin_args(p->getWidget(), top, right, bottom, left);
	return this;
}

Widget *Widget::setMargin(wzBorder margin)
{
	wz_widget_set_margin(p->getWidget(), margin);
	return this;
}

Widget *Widget::setFontFace(const std::string &fontFace)
{
	wz_widget_set_font_face(p->getWidget(), fontFace.c_str());
	return this;
}

Widget *Widget::setFontSize(float fontSize)
{
	wz_widget_set_font_size(p->getWidget(), fontSize);
	return this;
}

Widget *Widget::setFont(const std::string &fontFace, float fontSize)
{
	wz_widget_set_font(p->getWidget(), fontFace.c_str(), fontSize);
	return this;
}

Widget *Widget::setVisible(bool visible)
{
	wz_widget_set_visible(p->getWidget(), visible);
	return this;
}

Widget *Widget::addEventHandler(IEventHandler *eventHandler)
{
	p->eventHandlers.push_back(eventHandler);
	return this;
}

//------------------------------------------------------------------------------

Button::Button()
{
	p = new ButtonPrivate();
}

Button::Button(const std::string &label, const std::string &icon)
{
	p = new ButtonPrivate();

	if (!label.empty())
		setLabel(label);

	if (!icon.empty())
		setIcon(icon);
}

Button::~Button()
{
	delete p;
}

wzBorder Button::getPadding() const
{
	return wz_button_get_padding((const wzButton *)p->getWidget());
}

Button *Button::setPadding(wzBorder padding)
{
	wz_button_set_padding((wzButton *)p->getWidget(), padding);
	return this;
}

Button *Button::setPadding(int top, int right, int bottom, int left)
{
	wzBorder padding;
	padding.top = top;
	padding.right = right;
	padding.bottom = bottom;
	padding.left = left;
	wz_button_set_padding((wzButton *)p->getWidget(), padding);
	return this;
}

const char *Button::getIcon() const
{
	return wz_button_get_icon((const wzButton *)p->getWidget());
}

Button *Button::setIcon(const std::string &icon)
{
	wz_button_set_icon((wzButton *)p->getWidget(), icon.c_str());
	return this;
}

const char *Button::getLabel() const
{
	return wz_button_get_label((const wzButton *)p->getWidget());
}

Button *Button::setLabel(const std::string &label)
{
	wz_button_set_label((wzButton *)p->getWidget(), label.c_str());
	return this;
}

//------------------------------------------------------------------------------

Checkbox::Checkbox()
{
	p = new CheckboxPrivate();
}

Checkbox::Checkbox(const std::string &label)
{
	p = new CheckboxPrivate();
	setLabel(label);
}

Checkbox::~Checkbox()
{
	delete p;
}

const char *Checkbox::getLabel() const
{
	return wz_check_box_get_label((const wzCheckBox *)p->getWidget());
}

Checkbox *Checkbox::setLabel(const std::string &label)
{
	wz_check_box_set_label((wzCheckBox *)p->getWidget(), label.c_str());
	return this;
}

Checkbox *Checkbox::bindValue(bool *value)
{
	wz_check_box_bind_value((wzCheckBox *)p->getWidget(), value);
	return this;
}

//------------------------------------------------------------------------------

Combo::Combo()
{
	p = new ComboPrivate();
}

Combo::~Combo()
{
	delete p;
}

Combo *Combo::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	wzList *list = wz_combo_get_list((wzCombo *)p->getWidget());
	wz_list_set_item_data(list, itemData);
	wz_list_set_item_stride(list, itemStride);
	wz_list_set_num_items(list, nItems);
	return this;
}

//------------------------------------------------------------------------------

Frame::Frame()
{
	p = new FramePrivate();
}

Frame::~Frame()
{
	delete p;
}

Widget *Frame::add(Widget *widget)
{
	wz_frame_add((wzFrame *)p->getWidget(), widget->p->getWidget());
	return widget;
}

void Frame::remove(Widget *widget)
{
	wz_frame_remove((wzFrame *)p->getWidget(), widget->p->getWidget());
}

//------------------------------------------------------------------------------

GroupBox::GroupBox()
{
	p = new GroupBoxPrivate();
}

GroupBox::GroupBox(const std::string &label)
{
	p = new GroupBoxPrivate();
	setLabel(label);
}

GroupBox::~GroupBox()
{
	delete p;
}

const char *GroupBox::getLabel() const
{
	return wz_group_box_get_label((wzGroupBox *)p->getWidget());
}

GroupBox *GroupBox::setLabel(const std::string &label)
{
	wz_group_box_set_label((wzGroupBox *)p->getWidget(), label.c_str());
	return this;
}

Widget *GroupBox::add(Widget *widget)
{
	wz_group_box_add((wzGroupBox *)p->getWidget(), widget->p->getWidget());
	return widget;
}

void GroupBox::remove(Widget *widget)
{
	wz_group_box_remove((wzGroupBox *)p->getWidget(), widget->p->getWidget());
}

//------------------------------------------------------------------------------

Label::Label()
{
	p = new LabelPrivate();
}

Label::Label(const std::string &text)
{
	p = new LabelPrivate();
	setText(text.c_str());
}

Label::~Label()
{
	delete p;
}

Label *Label::setText(const char *format, ...)
{
	static char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	
	wz_label_set_text((wzLabel *)p->getWidget(), buffer);
	return this;
}

Label *Label::setTextColor(float r, float g, float b, float a)
{
	NVGcolor color;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	wz_label_set_text_color((wzLabel *)p->getWidget(), color);
	return this;
}

Label *Label::setMultiline(bool multiline)
{
	wz_label_set_multiline((wzLabel *)p->getWidget(), multiline);
	return this;
}

//------------------------------------------------------------------------------

List::List()
{
	p = new ListPrivate();
}

List::~List()
{
	delete p;
}

List *List::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	wz_list_set_item_data((wzList *)p->getWidget(), itemData);
	wz_list_set_item_stride((wzList *)p->getWidget(), itemStride);
	wz_list_set_num_items((wzList *)p->getWidget(), nItems);
	return this;
}

List *List::setSelectedItem(int index)
{
	wz_list_set_selected_item((wzList *)p->getWidget(), index);
	return this;
}

List *List::setItemHeight(int height)
{
	wz_list_set_item_height((wzList *)p->getWidget(), height);
	return this;
}

List *List::setDrawItemCallback(wzDrawListItemCallback callback)
{
	wz_list_set_draw_item_callback((wzList *)p->getWidget(), callback);
	return this;
}

//------------------------------------------------------------------------------

MainWindow::MainWindow(wzRenderer *renderer)
{
	p = new MainWindowPrivate(renderer);
}

MainWindow::~MainWindow()
{
	delete p;
}

int MainWindow::getWidth() const
{
	return wz_widget_get_width((const wzWidget *)p->mainWindow);
}

int MainWindow::getHeight() const
{
	return wz_widget_get_height((const wzWidget *)p->mainWindow);
}

void MainWindow::setSize(int w, int h)
{
	wz_widget_set_size_args((wzWidget *)p->mainWindow, w, h);
}

void MainWindow::mouseMove(int x, int y, int dx, int dy)
{
	wz_main_window_mouse_move(p->mainWindow, x, y, dx, dy);
}

void MainWindow::mouseButtonDown(int button, int x, int y)
{
	wz_main_window_mouse_button_down(p->mainWindow, button, x, y);
}

void MainWindow::mouseButtonUp(int button, int x, int y)
{
	wz_main_window_mouse_button_up(p->mainWindow, button, x, y);
}

void MainWindow::mouseWheelMove(int x, int y)
{
	wz_main_window_mouse_wheel_move(p->mainWindow, x, y);
}

void MainWindow::keyDown(wzKey key)
{
	wz_main_window_key_down(p->mainWindow, key);
}

void MainWindow::keyUp(wzKey key)
{
	wz_main_window_key_up(p->mainWindow, key);
}

void MainWindow::textInput(const char *text)
{
	wz_main_window_text_input(p->mainWindow, text);
}

void MainWindow::draw()
{	
	wz_main_window_draw(p->mainWindow);
}

void MainWindow::drawFrame()
{	
	wz_main_window_draw_frame(p->mainWindow);
}

void MainWindow::toggleTextCursor()
{
	wz_main_window_toggle_text_cursor(p->mainWindow);
}

wzCursor MainWindow::getCursor() const
{
	return wz_main_window_get_cursor(p->mainWindow);
}

Widget *MainWindow::add(Widget *widget)
{
	wz_main_window_add(p->mainWindow, widget->p->getWidget());
	return widget;
}

void MainWindow::remove(Widget *widget)
{
	wz_main_window_remove(p->mainWindow, widget->p->getWidget());
}

void MainWindow::createMenuButton(const std::string &label)
{
	wzMenuBarButton *button = wz_menu_bar_create_button(p->menuBar);
	wz_menu_bar_button_set_label(button, label.c_str());
}

void MainWindow::dockWindow(Window *window, wzDockPosition dockPosition)
{
	wz_main_window_dock_window(p->mainWindow, (wzWindow *)window->p->getWidget(), dockPosition);
}

//------------------------------------------------------------------------------

RadioButton::RadioButton()
{
	p = new RadioButtonPrivate();
}

RadioButton::RadioButton(const std::string &label)
{
	p = new RadioButtonPrivate();
	setLabel(label);
}

RadioButton::~RadioButton()
{
	delete p;
}

const char *RadioButton::getLabel() const
{
	return wz_radio_button_get_label((const wzRadioButton *)p->getWidget());
}

RadioButton *RadioButton::setLabel(const std::string &label)
{
	wz_radio_button_set_label((wzRadioButton *)p->getWidget(), label.c_str());
	return this;
}

//------------------------------------------------------------------------------

Scroller::Scroller(wzScrollerType type)
{
	p = new ScrollerPrivate(type);
}

Scroller::~Scroller()
{
	delete p;
}

Scroller *Scroller::setValue(int value)
{
	wz_scroller_set_value(((ScrollerPrivate *)p)->scroller, value);
	return this;
}

Scroller *Scroller::setStepValue(int stepValue)
{
	wz_scroller_set_step_value(((ScrollerPrivate *)p)->scroller, stepValue);
	return this;
}

Scroller *Scroller::setMaxValue(int maxValue)
{
	wz_scroller_set_max_value(((ScrollerPrivate *)p)->scroller, maxValue);
	return this;
}

int Scroller::getValue() const
{
	return wz_scroller_get_value(((ScrollerPrivate *)p)->scroller);
}

//------------------------------------------------------------------------------

Spinner::Spinner()
{
	p = new SpinnerPrivate();
}

Spinner::~Spinner()
{
	delete p;
}

Spinner *Spinner::setValue(int value)
{
	wz_spinner_set_value(((SpinnerPrivate *)p)->spinner, value);
	return this;
}

int Spinner::getValue() const
{
	return wz_spinner_get_value(((SpinnerPrivate *)p)->spinner);
}

//------------------------------------------------------------------------------

StackLayout::StackLayout()
{
	p = new StackLayoutPrivate();
}

StackLayout::StackLayout(wzStackLayoutDirection direction)
{
	p = new StackLayoutPrivate();
	setDirection(direction);
}

StackLayout::~StackLayout()
{
	delete p;
}

StackLayout *StackLayout::setDirection(wzStackLayoutDirection direction)
{
	wz_stack_layout_set_direction(((StackLayoutPrivate *)p)->layout, direction);
	return this;
}

StackLayout *StackLayout::setSpacing(int spacing)
{
	wz_stack_layout_set_spacing(((StackLayoutPrivate *)p)->layout, spacing);
	return this;
}

int StackLayout::getSpacing() const
{
	return wz_stack_layout_get_spacing(((StackLayoutPrivate *)p)->layout);
}

Widget *StackLayout::add(Widget *widget)
{
	wz_stack_layout_add((wzStackLayout *)p->getWidget(), widget->p->getWidget());
	return widget;
}

void StackLayout::remove(Widget *widget)
{
	wz_stack_layout_remove((wzStackLayout *)p->getWidget(), widget->p->getWidget());
}

//------------------------------------------------------------------------------

Tab::Tab()
{
	p = new TabPrivate();
}

Tab::~Tab()
{
	delete p;
}

Tab *Tab::setLabel(const std::string &label)
{
	wz_button_set_label(p->button, label.c_str());
	return this;
}

Widget *Tab::add(Widget *widget)
{
	wz_tab_page_add(p->page, widget->p->getWidget());
	return widget;
}

void Tab::remove(Widget *widget)
{
	wz_tab_page_remove(p->page, widget->p->getWidget());
}

//------------------------------------------------------------------------------

Tabbed::Tabbed()
{
	p = new TabbedPrivate();
}

Tabbed::~Tabbed()
{
	delete p;
}

Tab *Tabbed::addTab(Tab *tab)
{
	wz_tabbed_add_tab((wzTabbed *)p->getWidget(), &tab->p->button, &tab->p->page);
	return tab;
}

//------------------------------------------------------------------------------

TextEdit::TextEdit(bool multiline)
{
	p = new TextEditPrivate(multiline);
}

TextEdit::TextEdit(const std::string &text, bool multiline)
{
	p = new TextEditPrivate(multiline);
	setText(text);
}

TextEdit::~TextEdit()
{
	delete p;
}

TextEdit *TextEdit::setText(const std::string &text)
{
	wz_text_edit_set_text((wzTextEdit *)p->getWidget(), text.c_str());
	return this;
}

//------------------------------------------------------------------------------

ToggleButton::ToggleButton()
{
	p = new ToggleButtonPrivate();
}

ToggleButton::ToggleButton(const std::string &label, const std::string &icon)
{
	p = new ToggleButtonPrivate();

	if (!label.empty())
		setLabel(label);

	if (!icon.empty())
		setIcon(icon);
}

ToggleButton::~ToggleButton()
{
	delete p;
}

wzBorder ToggleButton::getPadding() const
{
	return wz_button_get_padding((const wzButton *)p->getWidget());
}

ToggleButton *ToggleButton::setPadding(wzBorder padding)
{
	wz_button_set_padding((wzButton *)p->getWidget(), padding);
	return this;
}

ToggleButton *ToggleButton::setPadding(int top, int right, int bottom, int left)
{
	wzBorder padding;
	padding.top = top;
	padding.right = right;
	padding.bottom = bottom;
	padding.left = left;
	wz_button_set_padding((wzButton *)p->getWidget(), padding);
	return this;
}

const char *ToggleButton::getIcon() const
{
	return wz_button_get_icon((const wzButton *)p->getWidget());
}

ToggleButton *ToggleButton::setIcon(const std::string &icon)
{
	wz_button_set_icon((wzButton *)p->getWidget(), icon.c_str());
	return this;
}

const char *ToggleButton::getLabel() const
{
	return wz_button_get_label((const wzButton *)p->getWidget());
}

ToggleButton *ToggleButton::setLabel(const std::string &label)
{
	wz_button_set_label((wzButton *)p->getWidget(), label.c_str());
	return this;
}

//------------------------------------------------------------------------------

Window::Window()
{
	p = new WindowPrivate();
}

Window::Window(const std::string &title)
{
	p = new WindowPrivate();
	setTitle(title);
}

Window::~Window()
{
	delete p;
}

const char *Window::getTitle() const
{
	return wz_window_get_title((wzWindow *)p->getWidget());
}

Window *Window::setTitle(const std::string &title)
{
	wz_window_set_title((wzWindow *)p->getWidget(), title.c_str());
	return this;
}

Widget *Window::add(Widget *widget)
{
	wz_window_add((wzWindow *)p->getWidget(), widget->p->getWidget());
	return widget;
}

void Window::remove(Widget *widget)
{
	wz_window_remove((wzWindow *)p->getWidget(), widget->p->getWidget());
}

} // namespace wz
