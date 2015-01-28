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
Rect::intersect is SDL_IntersectRect

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

const Border Border_zero;

WidgetChildren::~WidgetChildren()
{
#if 0
	for (size_t i = 0; i < widgets_.size(); i++)
	{
		delete widgets_[i];
	}

	widgets_.clear();

	for (size_t i = 0; i < impls_.size(); i++)
	{
		delete impls_[i];
	}

	impls_.clear();
#endif
}
	
size_t WidgetChildren::size() const
{
	return widgets_.size() + impls_.size();
}

bool WidgetChildren::empty() const
{
	return widgets_.empty() && impls_.empty();
}

const struct WidgetImpl *WidgetChildren::operator[](size_t i) const
{
	if (i < widgets_.size())
		return widgets_[i]->impl;

	return impls_[i - widgets_.size()];
}

struct WidgetImpl *WidgetChildren::operator[](size_t i)
{
	if (i < widgets_.size())
		return widgets_[i]->impl;

	return impls_[i - widgets_.size()];
}

void WidgetChildren::clear()
{
	widgets_.clear();
	impls_.clear();
}

void WidgetChildren::push_back(Widget *widget)
{
	widgets_.push_back(widget);
}

void WidgetChildren::push_back(struct WidgetImpl *widgetImpl)
{
	impls_.push_back(widgetImpl);
}

void WidgetChildren::erase(size_t i)
{
	if (i < widgets_.size())
		widgets_.erase(widgets_.begin() + i);
	else
		impls_.erase(impls_.begin() + (i - impls_.size()));
}

WidgetImpl::WidgetImpl()
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

bool Rect::intersect(const Rect A, const Rect B, Rect *result)
{
    int Amin, Amax, Bmin, Bmax;

	WZ_ASSERT(result);

    /* Special cases for empty rects */
    if (A.isEmpty() || B.isEmpty())
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

    return !result->isEmpty();
}

/*
================================================================================

PUBLIC WIDGET FUNCTIONS

================================================================================
*/

void wz_widget_destroy(struct WidgetImpl *widget)
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

struct MainWindowImpl *wz_widget_get_main_window(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->mainWindow;
}

WidgetType wz_widget_get_type(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->type;
}

bool wz_widget_is_layout(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->type == WZ_TYPE_STACK_LAYOUT;
}

void wz_widget_set_position_args(struct WidgetImpl *widget, int x, int y)
{
	Position position;
	position.x = x;
	position.y = y;
	wz_widget_set_position(widget, position);
}

void wz_widget_set_position(struct WidgetImpl *widget, Position position)
{
	Rect rect;

	WZ_ASSERT(widget);
	rect = widget->userRect;
	rect.x = position.x;
	rect.y = position.y;
	wz_widget_set_rect(widget, rect);
}

Position wz_widget_get_position(const struct WidgetImpl *widget)
{
	Rect rect;
	Position pos;

	WZ_ASSERT(widget);
	rect = wz_widget_get_rect(widget);
	pos.x = rect.x;
	pos.y = rect.y;
	return pos;
}

Position wz_widget_get_absolute_position(const struct WidgetImpl *widget)
{
	Rect rect;
	Position pos;

	WZ_ASSERT(widget);
	rect = wz_widget_get_absolute_rect(widget);
	pos.x = rect.x;
	pos.y = rect.y;
	return pos;
}

void wz_widget_set_width(struct WidgetImpl *widget, int w)
{
	Rect rect;

	WZ_ASSERT(widget);
	rect = widget->userRect;
	rect.w = w;
	wz_widget_set_rect(widget, rect);
}

void wz_widget_set_height(struct WidgetImpl *widget, int h)
{
	Rect rect;

	WZ_ASSERT(widget);
	rect = widget->userRect;
	rect.h = h;
	wz_widget_set_rect(widget, rect);
}

void wz_widget_set_size_args(struct WidgetImpl *widget, int w, int h)
{
	Size size;
	size.w = w;
	size.h = h;
	wz_widget_set_size(widget, size);
}

void wz_widget_set_size(struct WidgetImpl *widget, Size size)
{
	Rect rect;

	WZ_ASSERT(widget);
	rect = widget->userRect;
	rect.w = size.w;
	rect.h = size.h;
	wz_widget_set_rect(widget, rect);
}

int wz_widget_get_width(const struct WidgetImpl *widget)
{
	return wz_widget_get_rect(widget).w;
}

int wz_widget_get_height(const struct WidgetImpl *widget)
{
	return wz_widget_get_rect(widget).h;
}

Size wz_widget_get_size(const struct WidgetImpl *widget)
{
	Rect rect;
	Size size;

	WZ_ASSERT(widget);
	rect = wz_widget_get_rect(widget);
	size.w = rect.w;
	size.h = rect.h;
	return size;
}

void wz_widget_set_rect_args(struct WidgetImpl *widget, int x, int y, int w, int h)
{
	Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	wz_widget_set_rect(widget, rect);
}

void wz_widget_set_rect(struct WidgetImpl *widget, Rect rect)
{
	WZ_ASSERT(widget);
	widget->userRect = rect;
	wz_widget_set_rect_internal(widget, rect);
}

Rect wz_widget_get_rect(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->rect;
}

Rect wz_widget_get_absolute_rect(const struct WidgetImpl *widget)
{
	Rect rect;

	WZ_ASSERT(widget);
	rect = wz_widget_get_rect(widget);

	if (widget->parent)
	{
		const Position parentPosition = wz_widget_get_absolute_position(widget->parent);
		rect.x += parentPosition.x;
		rect.y += parentPosition.y;
	}

	return rect;
}

void wz_widget_set_margin(struct WidgetImpl *widget, Border margin)
{
	WZ_ASSERT(widget);
	widget->margin = margin;
	wz_widget_refresh_rect(widget);
}

void wz_widget_set_margin_args(struct WidgetImpl *widget, int top, int right, int bottom, int left)
{
	Border margin;
	margin.top = top;
	margin.right = right;
	margin.bottom = bottom;
	margin.left = left;
	wz_widget_set_margin(widget, margin);
}

void wz_widget_set_margin_uniform(struct WidgetImpl *widget, int value)
{
	Border margin;
	margin.top = margin.right = margin.bottom = margin.left = value;
	wz_widget_set_margin(widget, margin);
}

Border wz_widget_get_margin(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->margin;
}

void wz_widget_set_stretch(struct WidgetImpl *widget, int stretch)
{
	WZ_ASSERT(widget);
	widget->stretch = stretch;

	// If the parent is a layout widget, refresh it.
	if (widget->parent && wz_widget_is_layout(widget->parent))
	{
		wz_widget_refresh_rect(widget->parent);
	}
}

int wz_widget_get_stretch(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->stretch;
}

void wz_widget_set_stretch_scale(struct WidgetImpl *widget, float width, float height)
{
	WZ_ASSERT(widget);
	widget->stretchWidthScale = width;
	widget->stretchHeightScale = height;
}

float wz_widget_get_stretch_width_scale(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->stretchWidthScale;
}

float wz_widget_get_stretch_height_scale(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->stretchHeightScale;
}

void wz_widget_set_align(struct WidgetImpl *widget, int align)
{
	WZ_ASSERT(widget);
	widget->align = align;

	// If the parent is a layout widget, refresh it.
	if (widget->parent && wz_widget_is_layout(widget->parent))
	{
		wz_widget_refresh_rect(widget->parent);
	}
}

int wz_widget_get_align(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->align;
}

void wz_widget_set_font_face(struct WidgetImpl *widget, const char *fontFace)
{
	WZ_ASSERT(widget);
	strcpy(widget->fontFace, fontFace);
	wz_widget_resize_to_measured(widget);

	if (widget->vtable.font_changed)
	{
		widget->vtable.font_changed(widget, widget->fontFace, widget->fontSize);
	}
}

const char *wz_widget_get_font_face(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->fontFace;
}

void wz_widget_set_font_size(struct WidgetImpl *widget, float fontSize)
{
	WZ_ASSERT(widget);
	widget->fontSize = fontSize;
	wz_widget_resize_to_measured(widget);

	if (widget->vtable.font_changed)
	{
		widget->vtable.font_changed(widget, widget->fontFace, widget->fontSize);
	}
}

float wz_widget_get_font_size(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->fontSize;
}

void wz_widget_set_font(struct WidgetImpl *widget, const char *fontFace, float fontSize)
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

bool wz_widget_get_hover(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->hover;
}

void wz_widget_set_visible(struct WidgetImpl *widget, bool visible)
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

bool wz_widget_get_visible(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return !widget->hidden;
}

bool wz_widget_has_keyboard_focus(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return wz_main_window_get_keyboard_focus_widget(widget->mainWindow) == widget;
}

void wz_widget_set_metadata(struct WidgetImpl *widget, void *metadata)
{
	WZ_ASSERT(widget);
	widget->metadata = metadata;
}

void *wz_widget_get_metadata(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->metadata;
}

void wz_widget_set_draw_callback(struct WidgetImpl *widget, WidgetDrawCallback draw)
{
	WZ_ASSERT(widget);
	widget->vtable.draw = draw;
}

void wz_widget_set_measure_callback(struct WidgetImpl *widget, WidgetMeasureCallback measure)
{
	WZ_ASSERT(widget);
	widget->vtable.measure = measure;
}

static struct MainWindowImpl *wz_widget_find_main_window(struct WidgetImpl *widget)
{
	for (;;)
	{
		if (!widget)
			break;

		if (widget->type == WZ_TYPE_MAIN_WINDOW)
			return (struct MainWindowImpl *)widget;

		widget = widget->parent;
	}

	return NULL;
}

static void wz_widget_set_renderer(struct WidgetImpl *widget, struct wzRenderer *renderer)
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
static void wz_widget_set_main_window_and_window_recursive(struct WidgetImpl *widget, struct MainWindowImpl *mainWindow, struct WindowImpl *window)
{
	WZ_ASSERT(widget);

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		struct WidgetImpl *child = widget->children[i];
		child->mainWindow = mainWindow;
		child->window = window;

		// Set the renderer too.
		if (mainWindow)
		{
			wz_widget_set_renderer(child, ((struct WidgetImpl *)mainWindow)->renderer);
		}

		wz_widget_set_main_window_and_window_recursive(child, mainWindow, window);
	}
}

bool wz_widget_is_descendant_of(struct WidgetImpl *widget, WidgetType type)
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

struct WidgetImpl *wz_widget_get_parent(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->parent;
}

struct WindowImpl *wz_widget_get_parent_window(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->window;
}

/*
================================================================================

WIDGET MEASURING

================================================================================
*/

void wz_widget_resize_to_measured(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);

	if (widget->renderer && widget->vtable.measure)
	{
		Size size = widget->vtable.measure(widget);

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

static void wz_widget_measure_and_resize_recursive(struct WidgetImpl *widget)
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

void wz_widget_add_child_widget(struct WidgetImpl *widget, struct WidgetImpl *child)
{
	WZ_ASSERT(widget);
	WZ_ASSERT(child);

	// Set mainWindow.
	child->mainWindow = wz_widget_find_main_window(widget);

	// Find the closest ancestor window.
	child->window = (struct WindowImpl *)wz_widget_find_closest_ancestor(widget, WZ_TYPE_WINDOW);

	// Set the renderer.
	if (child->mainWindow)
	{
		wz_widget_set_renderer(child, ((struct WidgetImpl *)child->mainWindow)->renderer);
	}

	// Set children mainWindow, window and renderer.
	wz_widget_set_main_window_and_window_recursive(child, child->mainWindow, child->type == WZ_TYPE_WINDOW ? (struct WindowImpl *)child : child->window);

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

void wz_widget_remove_child_widget(struct WidgetImpl *widget, struct WidgetImpl *child)
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

	widget->children.erase(deleteIndex);

	// The child is no longer connected to the widget hierarchy, so reset some state.
	child->mainWindow = NULL;
	child->parent = NULL;
	child->window = NULL;
}

void wz_widget_destroy_child_widget(struct WidgetImpl *widget, struct WidgetImpl *child)
{
	WZ_ASSERT(widget);
	const size_t n = widget->children.size();
	wz_widget_remove_child_widget(widget, child);

	// Don't destroy if the child wasn't removed. Happens if it is not really a child, see wz_widget_remove_child_widget.
	if (n == widget->children.size())
		return;

	wz_widget_destroy(child);
}

void wz_widget_set_position_args_internal(struct WidgetImpl *widget, int x, int y)
{
	Position position;
	position.x = x;
	position.y = y;
	wz_widget_set_position_internal(widget, position);
}

void wz_widget_set_position_internal(struct WidgetImpl *widget, Position position)
{
	Rect rect;

	WZ_ASSERT(widget);
	rect = widget->rect;
	rect.x = position.x;
	rect.y = position.y;
	wz_widget_set_rect_internal(widget, rect);
}

void wz_widget_set_width_internal(struct WidgetImpl *widget, int w)
{
	Rect rect;

	WZ_ASSERT(widget);
	rect = widget->rect;
	rect.w = w;
	wz_widget_set_rect_internal(widget, rect);
}

void wz_widget_set_height_internal(struct WidgetImpl *widget, int h)
{
	Rect rect;

	WZ_ASSERT(widget);
	rect = widget->rect;
	rect.h = h;
	wz_widget_set_rect_internal(widget, rect);
}

void wz_widget_set_size_args_internal(struct WidgetImpl *widget, int w, int h)
{
	Size size;
	size.w = w;
	size.h = h;
	wz_widget_set_size_internal(widget, size);
}

void wz_widget_set_size_internal(struct WidgetImpl *widget, Size size)
{
	Rect rect;

	WZ_ASSERT(widget);
	rect = widget->rect;
	rect.w = size.w;
	rect.h = size.h;
	wz_widget_set_rect_internal(widget, rect);
}

void wz_widget_set_rect_args_internal(struct WidgetImpl *widget, int x, int y, int w, int h)
{
	Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	wz_widget_set_rect_internal(widget, rect);
}

static void wz_widget_set_rect_internal_recursive(struct WidgetImpl *widget, Rect rect)
{
	Rect oldRect;

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

void wz_widget_set_rect_internal(struct WidgetImpl *widget, Rect rect)
{
	Rect oldRect;

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

void wz_widget_refresh_rect(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	wz_widget_set_rect_internal(widget, wz_widget_get_rect(widget));
}

struct WidgetImpl *wz_widget_find_closest_ancestor(const struct WidgetImpl *widget, WidgetType type)
{
	struct WidgetImpl *temp = (struct WidgetImpl *)widget;

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

void wz_widget_set_draw_manually(struct WidgetImpl *widget, bool value)
{
	widget->drawManually = value;
}

void wz_widget_set_draw_last(struct WidgetImpl *widget, bool value)
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

void wz_widget_set_overlap(struct WidgetImpl *widget, bool value)
{
	widget->overlap = value;
}

bool wz_widget_overlaps_parent_window(const struct WidgetImpl *widget)
{
	if (!widget->window)
		return true;

	return WZ_RECTS_OVERLAP(wz_widget_get_absolute_rect((struct WidgetImpl *)widget->window), wz_widget_get_absolute_rect(widget));
}

void wz_widget_set_clip_input_to_parent(struct WidgetImpl *widget, bool value)
{
	WZ_ASSERT(widget);
	widget->inputNotClippedToParent = !value;
}

void wz_widget_set_internal_metadata(struct WidgetImpl *widget, void *metadata)
{
	WZ_ASSERT(widget);
	widget->internalMetadata = metadata;
}

void *wz_widget_get_internal_metadata(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return widget->internalMetadata;
}

int wz_widget_get_line_height(const struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return wz_renderer_get_line_height(widget->renderer, widget->fontFace, widget->fontSize);
}

void wz_widget_measure_text(const struct WidgetImpl *widget, const char *text, int n, int *width, int *height)
{
	WZ_ASSERT(widget);
	wz_renderer_measure_text(widget->renderer, widget->fontFace, widget->fontSize, text, n, width, height);
}

//------------------------------------------------------------------------------

Widget::~Widget()
{
}

Rect Widget::getRect() const
{
	return wz_widget_get_absolute_rect(impl);
}

Widget *Widget::setPosition(int x, int y)
{ 
	wz_widget_set_position_args(impl, x, y);
	return this;
}

Widget *Widget::setWidth(int w)
{
	wz_widget_set_width(impl, w);
	return this;
}

Widget *Widget::setHeight(int h)
{
	wz_widget_set_height(impl, h);
	return this;
}

Widget *Widget::setSize(int w, int h)
{
	wz_widget_set_size_args(impl, w, h);
	return this;
}

Widget *Widget::setRect(int x, int y, int w, int h)
{
	wz_widget_set_rect_args(impl, x, y, w, h);
	return this;
}

Widget *Widget::setStretch(int stretch)
{
	wz_widget_set_stretch(impl, stretch);
	return this;
}

Widget *Widget::setAlign(int align)
{
	wz_widget_set_align(impl, align);
	return this;
}

Widget *Widget::setMargin(int margin)
{
	wz_widget_set_margin_args(impl, margin, margin, margin, margin);
	return this;
}

Widget *Widget::setMargin(int top, int right, int bottom, int left)
{
	wz_widget_set_margin_args(impl, top, right, bottom, left);
	return this;
}

Widget *Widget::setMargin(Border margin)
{
	wz_widget_set_margin(impl, margin);
	return this;
}

Widget *Widget::setFontFace(const std::string &fontFace)
{
	wz_widget_set_font_face(impl, fontFace.c_str());
	return this;
}

Widget *Widget::setFontSize(float fontSize)
{
	wz_widget_set_font_size(impl, fontSize);
	return this;
}

Widget *Widget::setFont(const std::string &fontFace, float fontSize)
{
	wz_widget_set_font(impl, fontFace.c_str(), fontSize);
	return this;
}

Widget *Widget::setVisible(bool visible)
{
	wz_widget_set_visible(impl, visible);
	return this;
}

Widget *Widget::addEventHandler(IEventHandler *eventHandler)
{
	impl->eventHandlers.push_back(eventHandler);
	return this;
}

} // namespace wz
