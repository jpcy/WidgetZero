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
#include "wz.h"
#pragma hdrstop
#include "wz_renderer_nanovg.h"

namespace wz {

Widget::Widget()
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
}

Widget::~Widget()
{
	// Delete event handlers.
	for (size_t i = 0; i < eventHandlers.size(); i++)
	{
		delete eventHandlers[i];
	}
}

Widget *Widget::addEventHandler(IEventHandler *eventHandler)
{
	eventHandlers.push_back(eventHandler);
	return this;
}

WidgetType Widget::getType() const
{
	return type;
}

bool Widget::isLayout() const
{
	return type == WZ_TYPE_STACK_LAYOUT;
}

MainWindow *Widget::getMainWindow()
{
	return mainWindow;
}

void Widget::setPosition(int x, int y)
{
	setPosition(Position(x, y));
}

void Widget::setPosition(Position position)
{
	Rect rect = userRect;
	rect.x = position.x;
	rect.y = position.y;
	setRect(rect);
}

Position Widget::getPosition() const
{
	const Rect rect = getRect();
	return Position(rect.x, rect.y);
}

Position Widget::getAbsolutePosition() const
{
	const Rect rect = getAbsoluteRect();
	return Position(rect.x, rect.y);
}

void Widget::setWidth(int w)
{
	Rect rect = userRect;
	rect.w = w;
	setRect(rect);
}

void Widget::setHeight(int h)
{
	Rect rect = userRect;
	rect.h = h;
	setRect(rect);
}

void Widget::setSize(int w, int h)
{
	setSize(Size(w, h));
}

void Widget::setSize(Size size)
{
	Rect rect = userRect;
	rect.w = size.w;
	rect.h = size.h;
	setRect(rect);
}

int Widget::getWidth() const
{
	return rect.w;
}

int Widget::getHeight() const
{
	return rect.h;
}

Size Widget::getSize() const
{
	return Size(rect.w, rect.h);
}

void Widget::setRect(int x, int y, int w, int h)
{
	setRect(Rect(x, y, w, h));
}

void Widget::setRect(Rect rect)
{
	userRect = rect;
	setRectInternal(rect);
}

Rect Widget::getRect() const
{
	return rect;
}

Rect Widget::getAbsoluteRect() const
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

void Widget::setMargin(Border margin)
{
	this->margin = margin;
	refreshRect();
}

void Widget::setMargin(int top, int right, int bottom, int left)
{
	setMargin(Border(top, right, bottom, left));
}

void Widget::setUniformMargin(int value)
{
	setMargin(Border(value, value, value, value));
}

Border Widget::getMargin() const
{
	return margin;
}

void Widget::setStretch(int stretch)
{
	this->stretch = stretch;

	// If the parent is a layout widget, refresh it.
	if (parent && parent->isLayout())
	{
		parent->refreshRect();
	}
}

int Widget::getStretch() const
{
	return stretch;
}

void Widget::setStretchScale(float width, float height)
{
	stretchWidthScale = width;
	stretchHeightScale = height;
}

float Widget::getStretchWidthScale() const
{
	return stretchWidthScale;
}

float Widget::getStretchHeightScale() const
{
	return stretchHeightScale;
}

void Widget::setAlign(int align)
{
	this->align = align;

	// If the parent is a layout widget, refresh it.
	if (parent && parent->isLayout())
	{
		parent->refreshRect();
	}
}

int Widget::getAlign() const
{
	return align;
}

void Widget::setFontFace(const char *fontFace)
{
	strcpy(this->fontFace, fontFace);
	resizeToMeasured();
	onFontChanged(fontFace, fontSize);
}

const char *Widget::getFontFace() const
{
	return fontFace;
}

void Widget::setFontSize(float fontSize)
{
	this->fontSize = fontSize;
	resizeToMeasured();
	onFontChanged(fontFace, fontSize);
}

float Widget::getFontSize() const
{
	return fontSize;
}

void Widget::setFont(const char *fontFace, float fontSize)
{
	strcpy(this->fontFace, fontFace);
	this->fontSize = fontSize;
	resizeToMeasured();
	onFontChanged(fontFace, fontSize);
}

bool Widget::getHover() const
{
	return hover;
}

void Widget::setVisible(bool visible)
{
	hidden = !visible;
	onVisibilityChanged();
}

bool Widget::getVisible() const
{
	return !hidden;
}

bool Widget::hasKeyboardFocus() const
{
	return mainWindow->getKeyboardFocusWidget() == this;
}

void Widget::setMetadata(void *metadata)
{
	this->metadata = metadata;
}

void *Widget::getMetadata()
{
	return metadata;
}

void Widget::resizeToMeasured()
{
	if (!renderer)
		return;

	Size size = measure();

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

void Widget::addChildWidget(Widget *child)
{
	WZ_ASSERT(child);
	children.push_back(child);
	child->parent = this;

	// Set the main window to the ancestor main window.
	child->mainWindow = findMainWindow();

	// Set the renderer.
	if (child->mainWindow)
	{
		child->setRenderer(child->mainWindow->renderer);
	}

	// Set window to the closest ancestor window.
	child->window = (Window *)findClosestAncestor(WZ_TYPE_WINDOW);

	// Set children mainWindow, window and renderer.
	child->setMainWindowAndWindowRecursive(child->mainWindow, child->type == WZ_TYPE_WINDOW ? (Window *)child : child->window);

	// Resize the widget and children to their measured sizes.
	child->resizeToMeasuredRecursive();
	child->refreshRect();

	// Inform the child it now has a parent.
	child->onParented(this);
}

void Widget::removeChildWidget(Widget *child)
{
	WZ_ASSERT(child);

	// Remove from children.
	int removeIndex = -1;

	for (size_t i = 0; i < children.size(); i++)
	{
		if (children[i] == child)
		{
			removeIndex = (int)i;
			break;
		}
	}

	if (removeIndex != -1)
	{
		children.erase(children.begin() + removeIndex);
	}

	// The child is no longer connected to the widget hierarchy, so reset some state.
	child->mainWindow = NULL;
	child->parent = NULL;
	child->window = NULL;
}

void Widget::destroyChildWidget(Widget *child)
{
	const size_t n = children.size();
	removeChildWidget(child);

	// Don't destroy if the child wasn't removed. Happens if it is not really a child, see remove_child_widget.
	if (n == children.size())
		return;

	delete child;
}

void Widget::setPositionInternal(int x, int y)
{
	Position position;
	position.x = x;
	position.y = y;
	setPositionInternal(position);
}

void Widget::setPositionInternal(Position position)
{
	Rect rect = this->rect;
	rect.x = position.x;
	rect.y = position.y;
	setRectInternal(rect);
}

void Widget::setWidthInternal(int w)
{
	Rect rect = this->rect;
	rect.w = w;
	setRectInternal(rect);
}

void Widget::setHeightInternal(int h)
{
	Rect rect = this->rect;
	rect.h = h;
	setRectInternal(rect);
}

void Widget::setSizeInternal(int w, int h)
{
	setSizeInternal(Size(w, h));
}

void Widget::setSizeInternal(Size size)
{
	Rect rect = this->rect;
	rect.w = size.w;
	rect.h = size.h;
	setRectInternal(rect);
}

void Widget::setRectInternal(int x, int y, int w, int h)
{
	setRectInternal(Rect(x, y, w, h));
}

void Widget::setRectInternalRecursive(Rect rect)
{
	Rect oldRect = this->rect;

	// Apply alignment and stretching.
	rect = calculateAlignedStretchedRect(rect);

	this->rect = rect;
	onRectChanged();

	// Don't recurse if the rect hasn't changed.
	if (oldRect.x != rect.x || oldRect.y != rect.y || oldRect.w != rect.w || oldRect.h != rect.h)
	{
		for (size_t i = 0; i < children.size(); i++)
		{
			children[i]->setRectInternalRecursive(children[i]->rect);
		}
	}
}

void Widget::setRectInternal(Rect rect)
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

void Widget::refreshRect()
{
	setRectInternal(getRect());
}

const Widget *Widget::findClosestAncestor(WidgetType type) const
{
	const Widget *temp = this;

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

Widget *Widget::findClosestAncestor(WidgetType type)
{
	Widget *temp = this;

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

void Widget::setDrawManually(bool value)
{
	drawManually = value;
}

void Widget::setDrawLast(bool value)
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

void Widget::setOverlap(bool value)
{
	overlap = value;
}

bool Widget::overlapsParentWindow() const
{
	if (!window)
		return true;

	return WZ_RECTS_OVERLAP(window->getAbsoluteRect(), getAbsoluteRect());
}

void Widget::setClipInputToParent(bool value)
{
	inputNotClippedToParent = !value;
}

void Widget::setInternalMetadata(void *metadata)
{
	internalMetadata = metadata;
}

void *Widget::getInternalMetadata()
{
	return internalMetadata;
}

int Widget::getLineHeight() const
{
	NVGRenderer *r = (NVGRenderer *)renderer;
	return r->getLineHeight(fontFace, fontSize);
}

void Widget::measureText(const char *text, int n, int *width, int *height) const
{
	NVGRenderer *r = (NVGRenderer *)renderer;
	return r->measureText(fontFace, fontSize, text, n, width, height);
}

LineBreakResult Widget::lineBreakText(const char *text, int n, int lineWidth) const
{
	NVGRenderer *r = (NVGRenderer *)renderer;
	return r->lineBreakText(fontFace, fontSize, text, n, lineWidth);
}

void Widget::drawIfVisible()
{
	if (getVisible())
	{
		Rect clip;
		clip.x = clip.y = clip.w = clip.h = 0;
		draw(clip);
	}
}

void Widget::invokeEvent(Event e)
{
	// Call method pointer event handlers.
	for (size_t i = 0; i < eventHandlers.size(); i++)
	{
		if (eventHandlers[i]->eventType == e.base.type)
		{
			eventHandlers[i]->call(e);
		}
	}

	// Call the centralized event handler.
	if (mainWindow && mainWindow->handle_event)
	{
		mainWindow->handle_event(e);
	}
}

void Widget::invokeEvent(Event e, const std::vector<EventCallback> &callbacks)
{
	invokeEvent(e);

	for (size_t i = 0; i < callbacks.size(); i++)
	{
		callbacks[i](e);
	}
}

Rect Widget::calculateAlignedStretchedRect(Rect rect) const
{
	// Can't align or stretch to parent rect if there is no parent.
	if (!parent)
		return rect;

	// Don't align or stretch if this widget is a child of a layout. The layout will handle the logic in that case.
	if (parent && parent->isLayout())
		return rect;

	Rect parentRect = parent->getRect();

	// Handle stretching.
	if ((stretch & WZ_STRETCH_WIDTH) != 0)
	{
		const float scale = (stretchWidthScale < 0.01f) ? 1 : stretchWidthScale;

		rect.x = margin.left;
		rect.w = (int)(parentRect.w * scale) - (margin.left + margin.right);
	}

	if ((stretch & WZ_STRETCH_HEIGHT) != 0)
	{
		const float scale = (stretchHeightScale < 0.01f) ? 1 : stretchHeightScale;

		rect.y = margin.top;
		rect.h = (int)(parentRect.h * scale) - (margin.top + margin.bottom);
	}

	// Handle horizontal alignment.
	if ((align & WZ_ALIGN_LEFT) != 0)
	{
		rect.x = margin.left;
	}
	else if ((align & WZ_ALIGN_CENTER) != 0)
	{
		rect.x = margin.left + (int)((parentRect.w - margin.right) / 2.0f - rect.w / 2.0f);
	}
	else if ((align & WZ_ALIGN_RIGHT) != 0)
	{
		rect.x = parentRect.w - margin.right - rect.w;
	}

	// Handle vertical alignment.
	if ((align & WZ_ALIGN_TOP) != 0)
	{
		rect.y = margin.top;
	}
	else if ((align & WZ_ALIGN_MIDDLE) != 0)
	{
		rect.y = margin.top + (int)((parentRect.h - margin.bottom) / 2.0f - rect.h / 2.0f);
	}
	else if ((align & WZ_ALIGN_BOTTOM) != 0)
	{
		rect.y = parentRect.h - margin.bottom - rect.h;
	}

	return rect;
}

MainWindow *Widget::findMainWindow()
{
	Widget *widget = this;

	for (;;)
	{
		if (!widget)
			break;

		if (widget->type == WZ_TYPE_MAIN_WINDOW)
			return (MainWindow *)widget;

		widget = widget->parent;
	}

	return NULL;
}

void Widget::setRenderer(IRenderer *renderer)
{
	IRenderer *oldRenderer = this->renderer;
	this->renderer = renderer;

	if (oldRenderer != this->renderer)
	{
		onRendererChanged();
	}
}

// Do this recursively, since it's possible to setup a widget heirarchy *before* adding the root widget via Widget::addChildWidget.
// Example: scroller does this with it's button children.
void Widget::setMainWindowAndWindowRecursive(MainWindow *mainWindow, Window *window)
{
	for (size_t i = 0; i < children.size(); i++)
	{
		Widget *child = children[i];
		child->mainWindow = mainWindow;
		child->window = window;

		// Set the renderer too.
		if (mainWindow)
		{
			child->setRenderer(mainWindow->renderer);
		}

		child->setMainWindowAndWindowRecursive(mainWindow, window);
	}
}

void Widget::resizeToMeasuredRecursive()
{
	resizeToMeasured();

	for (size_t i = 0; i < children.size(); i++)
	{
		children[i]->resizeToMeasuredRecursive();
	}
}

} // namespace wz
