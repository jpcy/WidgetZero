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

namespace wz {

Widget::Widget()
{
	type_ = WidgetType::Widget;
	stretch_ = 0;
	stretchWidthScale_ = 0;
	stretchHeightScale_ = 0;
	align_ = 0;
	internalMetadata_ = NULL;
	metadata_ = NULL;
	flags_ = 0;
	hover_ = false;
	hidden_ = false;
	ignore_ = false;
	overlap_ = false;
	drawManually_ = false;
	inputNotClippedToParent_ = false;
	fontSize_ = 0;
	fontFace_[0] = NULL;
	renderer_ = NULL;
	mainWindow_ = NULL;
	window_ = NULL;
	parent_ = NULL;
}

Widget::~Widget()
{
	// Delete event handlers.
	for (size_t i = 0; i < eventHandlers_.size(); i++)
	{
		delete eventHandlers_[i];
	}
}

WidgetType::Enum Widget::getType() const
{
	return type_;
}

bool Widget::isLayout() const
{
	return type_ == WidgetType::StackLayout;
}

const MainWindow *Widget::getMainWindow() const
{
	return mainWindow_;
}

MainWindow *Widget::getMainWindow()
{
	return mainWindow_;
}

void Widget::setPosition(int x, int y)
{
	setPosition(Position(x, y));
}

void Widget::setPosition(Position position)
{
	Rect rect = userRect_;
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
	Rect rect = userRect_;
	rect.w = w;
	setRect(rect);
}

void Widget::setHeight(int h)
{
	Rect rect = userRect_;
	rect.h = h;
	setRect(rect);
}

void Widget::setSize(int w, int h)
{
	setSize(Size(w, h));
}

void Widget::setSize(Size size)
{
	Rect rect = userRect_;
	rect.w = size.w;
	rect.h = size.h;
	setRect(rect);
}

int Widget::getWidth() const
{
	return rect_.w;
}

int Widget::getHeight() const
{
	return rect_.h;
}

Size Widget::getSize() const
{
	return Size(rect_.w, rect_.h);
}

void Widget::setRect(int x, int y, int w, int h)
{
	setRect(Rect(x, y, w, h));
}

void Widget::setRect(Rect rect)
{
	userRect_ = rect;
	setRectInternal(rect);
}

Rect Widget::getRect() const
{
	return rect_;
}

Rect Widget::getAbsoluteRect() const
{
	Rect rect = rect_;

	if (parent_)
	{
		const Position parentPosition = parent_->getAbsolutePosition();
		rect.x += parentPosition.x;
		rect.y += parentPosition.y;
	}

	return rect;
}

void Widget::setMargin(Border margin)
{
	margin_ = margin;
	refreshRect();
}

void Widget::setMargin(int top, int right, int bottom, int left)
{
	setMargin(Border(top, right, bottom, left));
}

Border Widget::getMargin() const
{
	return margin_;
}

void Widget::setStretch(int stretch)
{
	stretch_ = stretch;

	// If the parent is a layout widget, refresh it.
	if (parent_ && parent_->isLayout())
	{
		parent_->refreshRect();
	}
}

int Widget::getStretch() const
{
	return stretch_;
}

void Widget::setStretchScale(float width, float height)
{
	stretchWidthScale_ = width;
	stretchHeightScale_ = height;
}

float Widget::getStretchWidthScale() const
{
	return stretchWidthScale_;
}

float Widget::getStretchHeightScale() const
{
	return stretchHeightScale_;
}

void Widget::setAlign(int align)
{
	align_ = align;

	// If the parent is a layout widget, refresh it.
	if (parent_ && parent_->isLayout())
	{
		parent_->refreshRect();
	}
}

int Widget::getAlign() const
{
	return align_;
}

void Widget::setFontFace(const char *fontFace)
{
	strcpy(fontFace_, fontFace);
	resizeToMeasured();
	onFontChanged(fontFace_, fontSize_);
}

const char *Widget::getFontFace() const
{
	return fontFace_;
}

void Widget::setFontSize(float fontSize)
{
	fontSize_ = fontSize;
	resizeToMeasured();
	onFontChanged(fontFace_, fontSize_);
}

float Widget::getFontSize() const
{
	return fontSize_;
}

void Widget::setFont(const char *fontFace, float fontSize)
{
	strcpy(fontFace_, fontFace);
	fontSize_ = fontSize;
	resizeToMeasured();
	onFontChanged(fontFace_, fontSize_);
}

bool Widget::getHover() const
{
	return hover_;
}

void Widget::setVisible(bool visible)
{
	if (hidden_ == !visible)
		return;

	hidden_ = !visible;

	// If the parent is a layout widget, it may need refreshing.
	if (parent_ && parent_->isLayout())
	{
		parent_->refreshRect();
	}

	onVisibilityChanged();
}

bool Widget::getVisible() const
{
	return !hidden_;
}

bool Widget::hasKeyboardFocus() const
{
	return mainWindow_->getKeyboardFocusWidget() == this;
}

const Widget *Widget::getParent() const
{
	return parent_;
}

Widget *Widget::getParent()
{
	return parent_;
}

const std::vector<Widget *> &Widget::getChildren() const
{
	return children_;
}

void Widget::setMetadata(void *metadata)
{
	metadata_ = metadata;
}

void *Widget::getMetadata()
{
	return metadata_;
}

void Widget::addChildWidget(Widget *child)
{
	WZ_ASSERT(child);
	children_.push_back(child);
	child->parent_ = this;

	// Set the main window to the ancestor main window.
	child->mainWindow_ = findMainWindow();

	// Set the renderer.
	if (child->mainWindow_)
	{
		child->setRenderer(child->mainWindow_->renderer_);
	}

	// Set window to the closest ancestor window.
	child->window_ = (Window *)findClosestAncestor(WidgetType::Window);

	// Set children mainWindow, window and renderer.
	child->setMainWindowAndWindowRecursive(child->mainWindow_, child->type_ == WidgetType::Window ? (Window *)child : child->window_);

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

	for (size_t i = 0; i < children_.size(); i++)
	{
		if (children_[i] == child)
		{
			removeIndex = (int)i;
			break;
		}
	}

	if (removeIndex != -1)
	{
		children_.erase(children_.begin() + removeIndex);
	}

	// The child is no longer connected to the widget hierarchy, so reset some state.
	child->mainWindow_ = NULL;
	child->parent_ = NULL;
	child->window_ = NULL;
}

void Widget::destroyChildWidget(Widget *child)
{
	const size_t n = children_.size();
	removeChildWidget(child);

	// Don't destroy if the child wasn't removed. Happens if it is not really a child, see remove_child_widget.
	if (n == children_.size())
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
	Rect rect = rect_;
	rect.x = position.x;
	rect.y = position.y;
	setRectInternal(rect);
}

void Widget::setWidthInternal(int w)
{
	Rect rect = rect_;
	rect.w = w;
	setRectInternal(rect);
}

void Widget::setHeightInternal(int h)
{
	Rect rect = rect_;
	rect.h = h;
	setRectInternal(rect);
}

void Widget::setSizeInternal(int w, int h)
{
	setSizeInternal(Size(w, h));
}

void Widget::setSizeInternal(Size size)
{
	Rect rect = rect_;
	rect.w = size.w;
	rect.h = size.h;
	setRectInternal(rect);
}

void Widget::setRectInternal(int x, int y, int w, int h)
{
	setRectInternal(Rect(x, y, w, h));
}

void Widget::setRectInternal(Rect rect)
{
	Rect oldRect = rect_;
	setRectInternalRecursive(rect);	

	// If the parent is a layout widget, it may need refreshing.
	if (parent_ && parent_->isLayout())
	{
		// Refresh if the width or height has changed.
		if (rect_.w != oldRect.w || rect_.h != oldRect.h)
		{
			parent_->refreshRect();
		}
	}
}

const Widget *Widget::findClosestAncestor(WidgetType::Enum type) const
{
	const Widget *temp = this;

	for (;;)
	{
		if (temp == NULL)
			break;

		if (temp->type_ == type)
		{
			return temp;
		}

		temp = temp->parent_;
	}

	return NULL;
}

Widget *Widget::findClosestAncestor(WidgetType::Enum type)
{
	Widget *temp = this;

	for (;;)
	{
		if (temp == NULL)
			break;

		if (temp->type_ == type)
		{
			return temp;
		}

		temp = temp->parent_;
	}

	return NULL;
}

void Widget::setDrawManually(bool value)
{
	drawManually_ = value;
}

void Widget::setDrawLast(bool value)
{
	if (value)
	{
		flags_ |= WidgetFlags::DrawLast;
	}
	else
	{
		flags_ &= ~WidgetFlags::DrawLast;
	}
}

bool Widget::getDrawLast() const
{
	return (flags_ & WidgetFlags::DrawLast) == WidgetFlags::DrawLast;
}

void Widget::setOverlap(bool value)
{
	overlap_ = value;
}

bool Widget::overlapsParentWindow() const
{
	if (!window_)
		return true;

	return WZ_RECTS_OVERLAP(window_->getAbsoluteRect(), getAbsoluteRect());
}

void Widget::setClipInputToParent(bool value)
{
	inputNotClippedToParent_ = !value;
}

Widget *Widget::addEventHandler(IEventHandler *eventHandler)
{
	eventHandlers_.push_back(eventHandler);
	return this;
}

int Widget::getLineHeight() const
{
	return renderer_->getLineHeight(fontFace_, fontSize_);
}

void Widget::measureText(const char *text, int n, int *width, int *height) const
{
	return renderer_->measureText(fontFace_, fontSize_, text, n, width, height);
}

LineBreakResult Widget::lineBreakText(const char *text, int n, int lineWidth) const
{
	return renderer_->lineBreakText(fontFace_, fontSize_, text, n, lineWidth);
}

void Widget::onParented(Widget * /*parent*/) {}

void Widget::onRendererChanged() {}

void Widget::onFontChanged(const char * /*fontFace*/, float /*fontSize*/) {}

void Widget::onVisibilityChanged() {}

void Widget::onRectChanged() {}

void Widget::onMouseButtonDown(int /*mouseButton*/, int /*mouseX*/, int /*mouseY*/) {}

void Widget::onMouseButtonUp(int /*mouseButton*/, int /*mouseX*/, int /*mouseY*/) {}

void Widget::onMouseMove(int /*mouseX*/, int /*mouseY*/, int /*mouseDeltaX*/, int /*mouseDeltaY*/) {}

void Widget::onMouseWheelMove(int /*x*/, int /*y*/) {}

void Widget::onMouseHoverOn() {}

void Widget::onMouseHoverOff() {}

void Widget::onKeyDown(Key::Enum /*key*/) {}

void Widget::onKeyUp(Key::Enum /*key*/) {}

void Widget::onTextInput(const char * /*text*/) {}

Rect Widget::getChildrenClipRect() const
{
	return getAbsoluteRect();
}

void Widget::draw(Rect /*clip*/) {}

Size Widget::measure()
{
	return Size();
}

void Widget::setInternalMetadata(void *metadata)
{
	internalMetadata_ = metadata;
}

void *Widget::getInternalMetadata()
{
	return internalMetadata_;
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
	for (size_t i = 0; i < eventHandlers_.size(); i++)
	{
		if (eventHandlers_[i]->eventType == e.base.type)
		{
			eventHandlers_[i]->call(e);
		}
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
	if (!parent_)
		return rect;

	// Don't align or stretch if this widget is a child of a layout. The layout will handle the logic in that case.
	if (parent_ && parent_->isLayout())
		return rect;

	Rect parentRect = parent_->getRect();

	// Handle stretching.
	if ((stretch_ & Stretch::Width) != 0)
	{
		const float scale = (stretchWidthScale_ < 0.01f) ? 1 : stretchWidthScale_;

		rect.x = margin_.left;
		rect.w = (int)(parentRect.w * scale) - (margin_.left + margin_.right);
	}

	if ((stretch_ & Stretch::Height) != 0)
	{
		const float scale = (stretchHeightScale_ < 0.01f) ? 1 : stretchHeightScale_;

		rect.y = margin_.top;
		rect.h = (int)(parentRect.h * scale) - (margin_.top + margin_.bottom);
	}

	// Handle horizontal alignment.
	if ((align_ & Align::Left) != 0)
	{
		rect.x = margin_.left;
	}
	else if ((align_ & Align::Center) != 0)
	{
		rect.x = margin_.left + (int)((parentRect.w - margin_.right) / 2.0f - rect.w / 2.0f);
	}
	else if ((align_ & Align::Right) != 0)
	{
		rect.x = parentRect.w - margin_.right - rect.w;
	}

	// Handle vertical alignment.
	if ((align_ & Align::Top) != 0)
	{
		rect.y = margin_.top;
	}
	else if ((align_ & Align::Middle) != 0)
	{
		rect.y = margin_.top + (int)((parentRect.h - margin_.bottom) / 2.0f - rect.h / 2.0f);
	}
	else if ((align_ & Align::Bottom) != 0)
	{
		rect.y = parentRect.h - margin_.bottom - rect.h;
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

		if (widget->type_ == WidgetType::MainWindow)
			return (MainWindow *)widget;

		widget = widget->parent_;
	}

	return NULL;
}

void Widget::setRenderer(IRenderer *renderer)
{
	IRenderer *oldRenderer = renderer_;
	renderer_ = renderer;

	if (oldRenderer != renderer_)
	{
		onRendererChanged();
	}
}

// Do this recursively, since it's possible to setup a widget heirarchy *before* adding the root widget via Widget::addChildWidget.
// Example: scroller does this with it's button children.
void Widget::setMainWindowAndWindowRecursive(MainWindow *mainWindow, Window *window)
{
	for (size_t i = 0; i < children_.size(); i++)
	{
		Widget *child = children_[i];
		child->mainWindow_ = mainWindow;
		child->window_ = window;

		// Set the renderer too.
		if (mainWindow)
		{
			child->setRenderer(mainWindow->renderer_);
		}

		child->setMainWindowAndWindowRecursive(mainWindow, window);
	}
}

void Widget::setRectInternalRecursive(Rect rect)
{
	Rect oldRect = rect_;

	// Apply alignment and stretching.
	rect = calculateAlignedStretchedRect(rect);

	rect_ = rect;
	onRectChanged();

	// Don't recurse if the rect hasn't changed.
	if (oldRect.x != rect.x || oldRect.y != rect.y || oldRect.w != rect.w || oldRect.h != rect.h)
	{
		for (size_t i = 0; i < children_.size(); i++)
		{
			children_[i]->setRectInternalRecursive(children_[i]->rect_);
		}
	}
}

void Widget::refreshRect()
{
	setRectInternal(getRect());
}

void Widget::resizeToMeasured()
{
	if (!renderer_)
		return;

	Size size = measure();

	// The explicitly set size overrides the measured size.
	if (userRect_.w != 0)
	{
		size.w = userRect_.w;
	}

	if (userRect_.h != 0)
	{
		size.h = userRect_.h;
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

void Widget::resizeToMeasuredRecursive()
{
	resizeToMeasured();

	for (size_t i = 0; i < children_.size(); i++)
	{
		children_[i]->resizeToMeasuredRecursive();
	}
}

} // namespace wz
