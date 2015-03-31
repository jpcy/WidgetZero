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

#if !defined(NDEBUG) && defined(_MSC_VER)
#include <Windows.h>
#endif

#ifndef _NDEBUG
static const char *widgetTypeNames[] =
{
	"Widget",
	"MainWindow",
	"Window",
	"Button",
	"CheckBox",
	"Combo",
	"DockIcon",
	"Frame",
	"GroupBox",
	"Label",
	"List",
	"MenuBar",
	"MenuBarButton",
	"RadioButton",
	"Scroller",
	"Spinner",
	"StackLayout",
	"TabBar",
	"TabPage",
	"Tabbed",
	"TextEdit"
};
#endif

namespace wz {

Widget::Widget()
{
	type_ = WidgetType::Widget;
	stretch_ = Stretch::None;
	stretchWidthScale_ = 0;
	stretchHeightScale_ = 0;
	align_ = Align::None;
	metadata_ = NULL;
	flags_ = WidgetFlags::MeasureDirty | WidgetFlags::RectDirty;
	hover_ = false;
	visible_ = true;
	ignore_ = false;
	overlap_ = false;
	drawManually_ = false;
	inputClippedToParent_ = true;
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

bool Widget::isLayoutWidget() const
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
	if (rect != userRect_)
	{
		userRect_ = rect;
		setRectDirty();
		debugPrintf("user rect set (%i %i %i %i)", rect.x, rect.y, rect.w, rect.h);
	}
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
		const Border parentPadding = parent_->getPadding();

		rect.x += parentPosition.x + parentPadding.left;
		rect.y += parentPosition.y + parentPadding.top;
	}

	return rect;
}

void Widget::setMargin(Border margin)
{
	margin_ = margin;
	setRectDirty();
}

void Widget::setMargin(int top, int right, int bottom, int left)
{
	setMargin(Border(top, right, bottom, left));
}

Border Widget::getMargin() const
{
	return margin_;
}

void Widget::setPadding(Border padding)
{
	padding_ = padding;
}

void Widget::setPadding(int top, int right, int bottom, int left)
{
	setPadding(Border(top, right, bottom, left));
}

Border Widget::getPadding() const
{
	return padding_;
}

void Widget::setStretch(Stretch::Enum stretch)
{
	stretch_ = stretch;
	setRectDirty();
}

int Widget::getStretch() const
{
	return stretch_;
}

void Widget::setStretchScale(float width, float height)
{
	stretchWidthScale_ = width;
	stretchHeightScale_ = height;
	setRectDirty();
}

float Widget::getStretchWidthScale() const
{
	return stretchWidthScale_;
}

float Widget::getStretchHeightScale() const
{
	return stretchHeightScale_;
}

void Widget::setAlign(Align::Enum align)
{
	align_ = align;
	setRectDirty();
}

int Widget::getAlign() const
{
	return align_;
}

void Widget::setFontFace(const char *fontFace)
{
	strcpy(fontFace_, fontFace);
	setMeasureDirty();
	onFontChanged(fontFace_, fontSize_);
}

const char *Widget::getFontFace() const
{
	return fontFace_;
}

void Widget::setFontSize(float fontSize)
{
	fontSize_ = fontSize;
	setMeasureDirty();
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
	setMeasureDirty();
	onFontChanged(fontFace_, fontSize_);
}

bool Widget::getHover() const
{
	return hover_;
}

void Widget::setVisible(bool visible)
{
	if (visible_ == visible)
		return;

	visible_ = visible;
	setRectDirty();
	onVisibilityChanged();
}

bool Widget::isVisible() const
{
	return visible_;
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

	// If the widget is dirty, set the dirty flags on the main window.
	if (child->mainWindow_)
	{
		if (child->isMeasureDirty())
		{
			child->mainWindow_->setAnyWidgetMeasureDirty();
		}

		if (child->isRectDirty())
		{
			child->mainWindow_->setAnyWidgetRectDirty();
		}
	}

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

bool Widget::isRectDirty() const
{
	return (flags_ & WidgetFlags::RectDirty) != 0;
}

bool Widget::isMeasureDirty() const
{
	return (flags_ & WidgetFlags::MeasureDirty) != 0;
}

Rect Widget::getUserRect() const
{
	return userRect_;
}

Size Widget::getMeasuredSize() const
{
	return measuredSize_;
}

Size Widget::getUserOrMeasuredSize() const
{
	return Size(userRect_.w != 0 ? userRect_.w : measuredSize_.w, userRect_.h != 0 ? userRect_.h : measuredSize_.h);
}

void Widget::setRectInternal(Rect rect)
{
	if (rect == rect_)
		return;

	rect_ = rect;
	debugPrintf("internal rect set (%i %i %i %i)", rect_.x, rect_.y, rect_.w, rect_.h);
	onRectChanged();
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
		flags_ = flags_ | WidgetFlags::DrawLast;
	}
	else
	{
		flags_ = WidgetFlags::Enum(flags_ & ~WidgetFlags::DrawLast);
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
	inputClippedToParent_ = value;
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

void Widget::doLayout() {}

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

void Widget::setMeasureDirty(bool value)
{
	if (value)
	{
		flags_ = flags_ | WidgetFlags::MeasureDirty;

		// A parent layout will need re-measuring too.
		if (parent_ && parent_->isLayoutWidget())
		{
			parent_->setMeasureDirty();
		}

		// Let the main window know that one of the widgets in its heirarchy need re-measuring.
		if (mainWindow_)
		{
			mainWindow_->setAnyWidgetMeasureDirty();
		}

		// If we need to re-measure, we also need to recalculate the rect.
		setRectDirty();
	}
	else
	{
		flags_ = WidgetFlags::Enum(flags_ & ~WidgetFlags::MeasureDirty);
	}
}

void Widget::setRectDirty(bool value)
{
	if (value)
	{
		flags_ = flags_ | WidgetFlags::RectDirty;

		// A parent layout will need it's rect to be recalculated too.
		if (parent_ && parent_->isLayoutWidget())
		{
			parent_->setRectDirty();
		}

		// Let the main window know that one of the widgets in its heirarchy needs its rect recalculated.
		if (mainWindow_)
		{
			mainWindow_->setAnyWidgetRectDirty();
		}
	}
	else
	{
		flags_ = WidgetFlags::Enum(flags_ & ~WidgetFlags::RectDirty);
	}
}

void Widget::doMeasurePassRecursive(bool dirty)
{
	for (size_t i = 0; i < children_.size(); i++)
	{
		children_[i]->doMeasurePassRecursive(dirty || children_[i]->isMeasureDirty());
	}

	if (dirty)
	{
		measuredSize_ = measure();
		setMeasureDirty(false);
	}
}

void Widget::doLayoutPassRecursive(bool dirty)
{
	if (!visible_)
		return;

	if (dirty && (!parent_ || !parent_->isLayoutWidget()))
	{
		// Calculate new size.
		const Rect parentRect = parent_ ? parent_->getRect() : Rect();
		const Border parentPadding = parent_ ? parent_->getPadding() : Border();
		Rect newRect = rect_;

		if (userRect_.w != 0)
		{
			// Width is user set.
			newRect.w = userRect_.w;
		}
		else if (parent_ && (stretch_ & Stretch::Width) != 0)
		{
			// Width is stretched to parent rect.
			const float scale = (stretchWidthScale_ < 0.01f) ? 1 : stretchWidthScale_;
			newRect.w = (int)(parentRect.w * scale) - (margin_.left + margin_.right) - (parentPadding.left + parentPadding.right);
		}
		else
		{
			// Width is measured (default).
			newRect.w = measuredSize_.w;
		}

		if (userRect_.h != 0)
		{
			// Height is user set.
			newRect.h = userRect_.h;
		}
		else if (parent_ && (stretch_ & Stretch::Height) != 0)
		{
			// Height is stretched to parent rect.
			const float scale = (stretchHeightScale_ < 0.01f) ? 1 : stretchHeightScale_;
			newRect.h = (int)(parentRect.h * scale) - (margin_.top + margin_.bottom) - (parentPadding.top + parentPadding.bottom);
		}
		else
		{
			// Height is measured (default).
			newRect.h = measuredSize_.h;
		}

		// Calculate new position.
		if (userRect_.x != 0)
		{
			// x is user set.
			newRect.x = userRect_.x;
		}
		else if (parent_ && ((align_ & (Align::Left | Align::Center | Align::Right)) != 0))
		{
			// Align to parent rect.
			if ((align_ & Align::Left) != 0)
			{
				newRect.x = margin_.left;
			}
			else if ((align_ & Align::Center) != 0)
			{
				newRect.x = margin_.left + (int)((parentRect.w - margin_.right) / 2.0f - newRect.w / 2.0f);
			}
			else if ((align_ & Align::Right) != 0)
			{
				newRect.x = parentRect.w - margin_.right - newRect.w;
			}
		}
		else
		{
			// x is set to the left margin (default).
			newRect.x = margin_.left;
		}

		if (userRect_.y != 0)
		{
			// y is user set.
			newRect.y = userRect_.y;
		}
		else if (parent_ && ((align_ & (Align::Top | Align::Middle | Align::Bottom)) != 0))
		{
			// Align to parent rect.
			if ((align_ & Align::Top) != 0)
			{
				newRect.y = margin_.top;
			}
			else if ((align_ & Align::Middle) != 0)
			{
				newRect.y = margin_.top + (int)((parentRect.h - margin_.bottom) / 2.0f - newRect.h / 2.0f);
			}
			else if ((align_ & Align::Bottom) != 0)
			{
				newRect.y = parentRect.h - margin_.bottom - newRect.h;
			}
		}
		else
		{
			// y is set to the left margin (default).
			newRect.y = margin_.top;
		}

		// Apply the new rect.
		setRectInternal(newRect);
	}

	// Done, rect is no longer dirty.
	setRectDirty(false);

	// Have layout widgets run their layout logic.
	if (isLayoutWidget())
	{
		doLayout();
	}

	// Recurse into children.
	for (size_t i = 0; i < children_.size(); i++)
	{
		children_[i]->doLayoutPassRecursive(dirty || children_[i]->isRectDirty());
	}
}

void Widget::drawIfVisible()
{
	if (isVisible())
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

#ifdef NDEBUG
void Widget::debugPrintf(const char *, ...) const
{
}
#else
static void DebugVPrint(const char *format, va_list args)
{
	char buffer[128];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

#ifdef _MSC_VER
	OutputDebugString(buffer);
#else
	printf("%s", buffer);	
#endif
}

static void DebugPrintf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	DebugVPrint(format, args);
}

void Widget::debugPrintf(const char *format, ...) const
{
	debugPrintWidgetDetailsRecursive();

	DebugPrintf(": ");

	va_list args;
	va_start(args, format);
	DebugVPrint(format, args);

	DebugPrintf("\n");
}

void Widget::debugPrintWidgetDetailsRecursive() const
{
	if (parent_)
	{
		parent_->debugPrintWidgetDetailsRecursive();
		DebugPrintf(" -> ");
	}

	DebugPrintf(widgetTypeNames[(int)type_]);

	// Try to print a description for the widget too.
	if (type_ == WidgetType::Window && ((Window *)this)->getTitle()[0])
	{
		DebugPrintf(" \"%s\"", ((Window *)this)->getTitle());
	}
	else if ((type_ == WidgetType::Button || type_ == WidgetType::CheckBox || type_ == WidgetType::RadioButton) && ((Button *)this)->getLabel()[0])
	{
		DebugPrintf(" \"%s\"", ((Button *)this)->getLabel());
	}
	else if (type_ == WidgetType::GroupBox && ((GroupBox *)this)->getLabel()[0])
	{
		DebugPrintf(" \"%s\"", ((GroupBox *)this)->getLabel());
	}
	else if (type_ == WidgetType::Label && !((Label *)this)->getMultiline())
	{
		DebugPrintf(" \"%s\"", ((Label *)this)->getText());
	}
	else if (type_ == WidgetType::MenuBarButton && ((MenuBarButton *)this)->getLabel()[0])
	{
		DebugPrintf(" \"%s\"", ((MenuBarButton *)this)->getLabel());
	}
}
#endif

} // namespace wz
