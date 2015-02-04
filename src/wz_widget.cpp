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
#include "wz_pch.h"
#pragma hdrstop

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
		impls_.erase(impls_.begin() + (i - widgets_.size()));
}

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

static void wz_widget_set_renderer(struct WidgetImpl *widget, IRenderer *renderer)
{
	IRenderer *oldRenderer;

	WZ_ASSERT(widget);
	oldRenderer = widget->renderer;
	widget->renderer = renderer;

	if (oldRenderer != widget->renderer && widget->vtable.renderer_changed)
	{
		widget->vtable.renderer_changed(widget);
	}
}

// Do this recursively, since it's possible to setup a widget heirarchy *before* adding the root widget via WidgetImpl::addChildWidget.
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
			wz_widget_set_renderer(child, mainWindow->renderer);
		}

		wz_widget_set_main_window_and_window_recursive(child, mainWindow, window);
	}
}

static void wz_widget_measure_and_resize_recursive(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	widget->resizeToMeasured();

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		wz_widget_measure_and_resize_recursive(widget->children[i]);
	}
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

WidgetType WidgetImpl::getType() const
{
	return type;
}

bool WidgetImpl::isLayout() const
{
	return type == WZ_TYPE_STACK_LAYOUT;
}

struct MainWindowImpl *WidgetImpl::getMainWindow()
{
	return mainWindow;
}

void WidgetImpl::setPosition(int x, int y)
{
	setPosition(Position(x, y));
}

void WidgetImpl::setPosition(Position position)
{
	Rect rect = userRect;
	rect.x = position.x;
	rect.y = position.y;
	setRect(rect);
}

Position WidgetImpl::getPosition() const
{
	const Rect rect = getRect();
	return Position(rect.x, rect.y);
}

Position WidgetImpl::getAbsolutePosition() const
{
	const Rect rect = getAbsoluteRect();
	return Position(rect.x, rect.y);
}

void WidgetImpl::setWidth(int w)
{
	Rect rect = userRect;
	rect.w = w;
	setRect(rect);
}

void WidgetImpl::setHeight(int h)
{
	Rect rect = userRect;
	rect.h = h;
	setRect(rect);
}

void WidgetImpl::setSize(int w, int h)
{
	setSize(Size(w, h));
}

void WidgetImpl::setSize(Size size)
{
	Rect rect = userRect;
	rect.w = size.w;
	rect.h = size.h;
	setRect(rect);
}

int WidgetImpl::getWidth() const
{
	return rect.w;
}

int WidgetImpl::getHeight() const
{
	return rect.h;
}

Size WidgetImpl::getSize() const
{
	return Size(rect.w, rect.h);
}

void WidgetImpl::setRect(int x, int y, int w, int h)
{
	setRect(Rect(x, y, w, h));
}

void WidgetImpl::setRect(Rect rect)
{
	userRect = rect;
	setRectInternal(rect);
}

Rect WidgetImpl::getRect() const
{
	return rect;
}

Rect WidgetImpl::getAbsoluteRect() const
{
	Rect rect = this->rect;

	if (parent)
	{
		const Position parentPosition = parent->getAbsolutePosition();
		rect.x += parentPosition.x;
		rect.y += parentPosition.y;
	}

	return rect;
}

void WidgetImpl::setMargin(Border margin)
{
	this->margin = margin;
	refreshRect();
}

void WidgetImpl::setMargin(int top, int right, int bottom, int left)
{
	setMargin(Border(top, right, bottom, left));
}

void WidgetImpl::setUniformMargin(int value)
{
	setMargin(Border(value, value, value, value));
}

Border WidgetImpl::getMargin() const
{
	return margin;
}

void WidgetImpl::setStretch(int stretch)
{
	this->stretch = stretch;

	// If the parent is a layout widget, refresh it.
	if (parent && parent->isLayout())
	{
		parent->refreshRect();
	}
}

int WidgetImpl::getStretch() const
{
	return stretch;
}

void WidgetImpl::setStretchScale(float width, float height)
{
	stretchWidthScale = width;
	stretchHeightScale = height;
}

float WidgetImpl::getStretchWidthScale() const
{
	return stretchWidthScale;
}

float WidgetImpl::getStretchHeightScale() const
{
	return stretchHeightScale;
}

void WidgetImpl::setAlign(int align)
{
	this->align = align;

	// If the parent is a layout widget, refresh it.
	if (parent && parent->isLayout())
	{
		parent->refreshRect();
	}
}

int WidgetImpl::getAlign() const
{
	return align;
}

void WidgetImpl::setFontFace(const char *fontFace)
{
	strcpy(this->fontFace, fontFace);
	resizeToMeasured();

	if (vtable.font_changed)
	{
		vtable.font_changed(this, fontFace, fontSize);
	}
}

const char *WidgetImpl::getFontFace() const
{
	return fontFace;
}

void WidgetImpl::setFontSize(float fontSize)
{
	this->fontSize = fontSize;
	resizeToMeasured();

	if (vtable.font_changed)
	{
		vtable.font_changed(this, fontFace, fontSize);
	}
}

float WidgetImpl::getFontSize() const
{
	return fontSize;
}

void WidgetImpl::setFont(const char *fontFace, float fontSize)
{
	strcpy(this->fontFace, fontFace);
	this->fontSize = fontSize;
	resizeToMeasured();

	if (vtable.font_changed)
	{
		vtable.font_changed(this, fontFace, fontSize);
	}
}

bool WidgetImpl::getHover() const
{
	return hover;
}

void WidgetImpl::setVisible(bool visible)
{
	if (vtable.set_visible)
	{
		vtable.set_visible(this, visible);
	}
	else
	{
		hidden = !visible;
	}
}

bool WidgetImpl::getVisible() const
{
	return !hidden;
}

bool WidgetImpl::hasKeyboardFocus() const
{
	return mainWindow->getKeyboardFocusWidget() == this;
}

void WidgetImpl::setMetadata(void *metadata)
{
	this->metadata = metadata;
}

void *WidgetImpl::getMetadata()
{
	return metadata;
}

void WidgetImpl::setDrawCallback(WidgetDrawCallback draw)
{
	vtable.draw = draw;
}

void WidgetImpl::setMeasureCallback(WidgetMeasureCallback measure)
{
	vtable.measure = measure;
}

void WidgetImpl::resizeToMeasured()
{
	if (!renderer)
		return;

	Size size;

	if (vtable.measure)
	{
		size = vtable.measure(this);
	}
	else
	{
		size = measure();
	}

	// The explicitly set size overrides the measured size.
	if (userRect.w != 0)
	{
		size.w = userRect.w;
	}

	if (userRect.h != 0)
	{
		size.h = userRect.h;
	}

	// Keep the current size if 0.
	if (size.w == 0)
	{
		size.w = getWidth();
	}

	if (size.h == 0)
	{
		size.h = getHeight();
	}
		
	// Set the size.
	setSizeInternal(size);
}

void WidgetImpl::addChildWidget(struct WidgetImpl *child)
{
	WZ_ASSERT(child);

	// Set mainWindow.
	child->mainWindow = wz_widget_find_main_window(this);

	// Find the closest ancestor window.
	child->window = (struct WindowImpl *)findClosestAncestor(WZ_TYPE_WINDOW);

	// Set the renderer.
	if (child->mainWindow)
	{
		wz_widget_set_renderer(child, child->mainWindow->renderer);
	}

	// Set children mainWindow, window and renderer.
	wz_widget_set_main_window_and_window_recursive(child, child->mainWindow, child->type == WZ_TYPE_WINDOW ? (struct WindowImpl *)child : child->window);

	child->parent = this;
	children.push_back(child);

	// Resize the widget and children to their measured sizes.
	wz_widget_measure_and_resize_recursive(child);
	child->refreshRect();

	if (child->vtable.added)
	{
		child->vtable.added(this, child);
	}
}

void WidgetImpl::removeChildWidget(struct WidgetImpl *child)
{
	int deleteIndex;

	WZ_ASSERT(child);

	// Ensure the child is actually a child of this widget before destroying it.
	deleteIndex = -1;

	for (size_t i = 0; i < children.size(); i++)
	{
		if (children[i] == child)
		{
			deleteIndex = (int)i;
			break;
		}
	}

	if (deleteIndex == -1)
		return;

	children.erase(deleteIndex);

	// The child is no longer connected to the widget hierarchy, so reset some state.
	child->mainWindow = NULL;
	child->parent = NULL;
	child->window = NULL;
}

void WidgetImpl::destroyChildWidget(struct WidgetImpl *child)
{
	const size_t n = children.size();
	removeChildWidget(child);

	// Don't destroy if the child wasn't removed. Happens if it is not really a child, see remove_child_widget.
	if (n == children.size())
		return;

	wz_widget_destroy(child);
}

void WidgetImpl::setPositionInternal(int x, int y)
{
	Position position;
	position.x = x;
	position.y = y;
	setPositionInternal(position);
}

void WidgetImpl::setPositionInternal(Position position)
{
	Rect rect = this->rect;
	rect.x = position.x;
	rect.y = position.y;
	setRectInternal(rect);
}

void WidgetImpl::setWidthInternal(int w)
{
	Rect rect = this->rect;
	rect.w = w;
	setRectInternal(rect);
}

void WidgetImpl::setHeightInternal(int h)
{
	Rect rect = this->rect;
	rect.h = h;
	setRectInternal(rect);
}

void WidgetImpl::setSizeInternal(int w, int h)
{
	setSizeInternal(Size(w, h));
}

void WidgetImpl::setSizeInternal(Size size)
{
	Rect rect = this->rect;
	rect.w = size.w;
	rect.h = size.h;
	setRectInternal(rect);
}

void WidgetImpl::setRectInternal(int x, int y, int w, int h)
{
	setRectInternal(Rect(x, y, w, h));
}

void WidgetImpl::setRectInternalRecursive(Rect rect)
{
	Rect oldRect = this->rect;

	// Apply alignment and stretching.
	rect = wz_widget_calculate_aligned_stretched_rect(this, rect);

	if (vtable.set_rect)
	{
		vtable.set_rect(this, rect);
	}
	else
	{
		this->rect = rect;
	}

	// Don't recurse if the rect hasn't changed.
	if (oldRect.x != rect.x || oldRect.y != rect.y || oldRect.w != rect.w || oldRect.h != rect.h)
	{
		for (size_t i = 0; i < children.size(); i++)
		{
			children[i]->setRectInternalRecursive(children[i]->rect);
		}
	}
}

void WidgetImpl::setRectInternal(Rect rect)
{
	Rect oldRect = this->rect;
	setRectInternalRecursive(rect);	

	// If the parent is a layout widget, it may need refreshing.
	if (parent && parent->isLayout())
	{
		// Refresh if the width or height has changed.
		if (this->rect.w != oldRect.w || this->rect.h != oldRect.h)
		{
			parent->refreshRect();
		}
	}
}

void WidgetImpl::refreshRect()
{
	setRectInternal(getRect());
}

const struct WidgetImpl *WidgetImpl::findClosestAncestor(WidgetType type) const
{
	const struct WidgetImpl *temp = this;

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

struct WidgetImpl *WidgetImpl::findClosestAncestor(WidgetType type)
{
	struct WidgetImpl *temp = this;

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

void WidgetImpl::setDrawManually(bool value)
{
	drawManually = value;
}

void WidgetImpl::setDrawLast(bool value)
{
	if (value)
	{
		flags |= WZ_WIDGET_FLAG_DRAW_LAST;
	}
	else
	{
		flags &= ~WZ_WIDGET_FLAG_DRAW_LAST;
	}
}

void WidgetImpl::setOverlap(bool value)
{
	overlap = value;
}

bool WidgetImpl::overlapsParentWindow() const
{
	if (!window)
		return true;

	return WZ_RECTS_OVERLAP(window->getAbsoluteRect(), getAbsoluteRect());
}

void WidgetImpl::setClipInputToParent(bool value)
{
	inputNotClippedToParent = !value;
}

void WidgetImpl::setInternalMetadata(void *metadata)
{
	internalMetadata = metadata;
}

void *WidgetImpl::getInternalMetadata()
{
	return internalMetadata;
}

int WidgetImpl::getLineHeight() const
{
	NVGRenderer *r = (NVGRenderer *)renderer;
	return r->getLineHeight(fontFace, fontSize);
}

void WidgetImpl::measureText(const char *text, int n, int *width, int *height) const
{
	NVGRenderer *r = (NVGRenderer *)renderer;
	return r->measureText(fontFace, fontSize, text, n, width, height);
}

LineBreakResult WidgetImpl::lineBreakText(const char *text, int n, int lineWidth) const
{
	NVGRenderer *r = (NVGRenderer *)renderer;
	return r->lineBreakText(fontFace, fontSize, text, n, lineWidth);
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

//------------------------------------------------------------------------------

Widget::~Widget()
{
}

Rect Widget::getRect() const
{
	return impl->getAbsoluteRect();
}

Widget *Widget::setPosition(int x, int y)
{ 
	impl->setPosition(x, y);
	return this;
}

Widget *Widget::setWidth(int w)
{
	impl->setWidth(w);
	return this;
}

Widget *Widget::setHeight(int h)
{
	impl->setHeight(h);
	return this;
}

Widget *Widget::setSize(int w, int h)
{
	impl->setSize(w, h);
	return this;
}

Widget *Widget::setRect(int x, int y, int w, int h)
{
	impl->setRect(x, y, w, h);
	return this;
}

Widget *Widget::setStretch(int stretch)
{
	impl->setStretch(stretch);
	return this;
}

Widget *Widget::setAlign(int align)
{
	impl->setAlign(align);
	return this;
}

Widget *Widget::setMargin(int margin)
{
	impl->setUniformMargin(margin);
	return this;
}

Widget *Widget::setMargin(int top, int right, int bottom, int left)
{
	impl->setMargin(top, right, bottom, left);
	return this;
}

Widget *Widget::setMargin(Border margin)
{
	impl->setMargin(margin);
	return this;
}

Widget *Widget::setFontFace(const std::string &fontFace)
{
	impl->setFontFace(fontFace.c_str());
	return this;
}

Widget *Widget::setFontSize(float fontSize)
{
	impl->setFontSize(fontSize);
	return this;
}

Widget *Widget::setFont(const std::string &fontFace, float fontSize)
{
	impl->setFont(fontFace.c_str(), fontSize);
	return this;
}

Widget *Widget::setVisible(bool visible)
{
	impl->setVisible(visible);
	return this;
}

Widget *Widget::addEventHandler(IEventHandler *eventHandler)
{
	impl->eventHandlers.push_back(eventHandler);
	return this;
}

} // namespace wz
