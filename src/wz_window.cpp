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

#define WZ_WINDOW_UNDOCK_DISTANCE 16

namespace wz {

Window::Window(const std::string &title)
{
	type_ = WidgetType::Window;
	drawPriority_ = 0;
	headerHeight_ = 0;
	borderSize_ = 4;
	drag_ = WindowDrag::None;
	title_ = title;

	content_ = new Widget;
	content_->setStretch(Stretch::All);
	addChildWidget(content_);
}

int Window::getHeaderHeight() const
{
	return headerHeight_;
}

int Window::getBorderSize() const
{
	return borderSize_;
}

Rect Window::getHeaderRect() const
{
	Rect rect;
	rect.x = rect_.x + borderSize_;
	rect.y = rect_.y + borderSize_;
	rect.w = rect_.w - borderSize_ * 2;
	rect.h = headerHeight_;
	return rect;
}

void Window::setTitle(const char *title)
{
	title_ = title;
}

const char *Window::getTitle() const
{
	return title_.c_str();
}

Widget *Window::getContentWidget()
{
	return content_;
}

int Window::getDrawPriority() const
{
	return drawPriority_;
}

void Window::setDrawPriority(int drawPriority)
{
	drawPriority_ = drawPriority;
}

void Window::dock()
{
	// Save the window size before docking so it can be restored if the window is undocked later.
	sizeBeforeDocking_.w = rect_.w;
	sizeBeforeDocking_.h = rect_.h;
}

void Window::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->getType() == WidgetType::MainWindow || widget->getType() == WidgetType::Window)
		return;

	content_->addChildWidget(widget);
}

void Window::remove(Widget *widget)
{
	WZ_ASSERT(widget);
	content_->removeChildWidget(widget);
}

void Window::onRendererChanged()
{
	refreshHeaderHeightAndPadding();
}

void Window::onFontChanged(const char * /*fontFace*/, float /*fontSize*/)
{
	refreshHeaderHeightAndPadding();
	setRectDirty();
}

void Window::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		// Drag the header.
		if (WZ_POINT_IN_RECT(mouseX, mouseY, getHeaderRect()))
		{
			drag_ = WindowDrag::Header;
			mainWindow_->pushLockInputWidget(this);

			// Don't actually move the window yet if it's docked.
			if (mainWindow_->getWindowDockPosition(this) == DockPosition::None)
			{
				mainWindow_->setMovingWindow(this);
			}
			else
			{
				undockStartPosition_.x = mouseX;
				undockStartPosition_.y = mouseY;
			}

			return;
		}

		// Resize by dragging the border.
		Rect borderRects[Compass::NumPoints];
		bool mouseOverBorderRects[Compass::NumPoints];

		calculateBorderRects(borderRects);
		calculateMouseOverBorderRects(mouseX, mouseY, borderRects, mouseOverBorderRects);

		for (int i = 0; i < Compass::NumPoints; i++)
		{
			if (mouseOverBorderRects[i])
			{
				drag_ = (WindowDrag::Enum)(WindowDrag::Resize_N + i);
				resizeStartPosition_.x = mouseX;
				resizeStartPosition_.y = mouseY;
				resizeStartRect_ = rect_;
				mainWindow_->pushLockInputWidget(this);
				return;
			}
		}
	}
}

void Window::onMouseButtonUp(int mouseButton, int /*mouseX*/, int /*mouseY*/)
{
	if (mouseButton == 1)
	{
		if (drag_ == WindowDrag::Header)
		{
			mainWindow_->setMovingWindow(NULL);
		}

		drag_ = WindowDrag::None;
		mainWindow_->popLockInputWidget(this);
	}
}

void Window::onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	// Set the mouse cursor.
	Rect borderRects[Compass::NumPoints];
	bool mouseOverBorderRects[Compass::NumPoints];

	calculateBorderRects(borderRects);
	calculateMouseOverBorderRects(mouseX, mouseY, borderRects, mouseOverBorderRects);

	if (mouseOverBorderRects[Compass::N] || mouseOverBorderRects[Compass::S] || drag_ == WindowDrag::Resize_N || drag_ == WindowDrag::Resize_S)
	{
		mainWindow_->setCursor(Cursor::Resize_N_S);
	}
	else if (mouseOverBorderRects[Compass::E] || mouseOverBorderRects[Compass::W] || drag_ == WindowDrag::Resize_E || drag_ == WindowDrag::Resize_W)
	{
		mainWindow_->setCursor(Cursor::Resize_E_W);
	}
	else if (mouseOverBorderRects[Compass::NE] || mouseOverBorderRects[Compass::SW] || drag_ == WindowDrag::Resize_NE || drag_ == WindowDrag::Resize_SW)
	{
		mainWindow_->setCursor(Cursor::Resize_NE_SW);
	}
	else if (mouseOverBorderRects[Compass::NW] || mouseOverBorderRects[Compass::SE] || drag_ == WindowDrag::Resize_NW || drag_ == WindowDrag::Resize_SE)
	{
		mainWindow_->setCursor(Cursor::Resize_NW_SE);
	}

	// Don't actually move the window yet if it's docked.
	const DockPosition::Enum dockPosition = mainWindow_->getWindowDockPosition(this);

	if (drag_ == WindowDrag::Header && dockPosition != DockPosition::None)
	{
		Position delta;

		delta.x = mouseX - undockStartPosition_.x;
		delta.y = mouseY - undockStartPosition_.y;

		// Undock and start moving if the mouse has moved far enough.
		if ((int)sqrtf((float)(delta.x * delta.x + delta.y * delta.y)) < WZ_WINDOW_UNDOCK_DISTANCE)
			return;

		mainWindow_->undockWindow(this);
		mainWindow_->setMovingWindow(this);

		// Re-position and resize the window.
		Rect newRect = rect_;
		newRect.x += delta.x;
		newRect.y += delta.y;
		newRect.w = WZ_MAX(200, sizeBeforeDocking_.w);
		newRect.h = WZ_MAX(200, sizeBeforeDocking_.h);

		// If the mouse cursor would be outside the window, center the window on the mouse cursor.
		if (mouseX < newRect.x + borderSize_ || mouseX > newRect.x + newRect.w - borderSize_)
		{
			newRect.x = mouseX - newRect.w / 2;
		}

		setRect(newRect);
		mainWindow_->updateContentRect();
		return;
	}

	// Calculate the minimum allowed window size.
	const Size minimumWindowSize(borderSize_ * 2, headerHeight_ + borderSize_ * 2);

	// Calculate mouse deltas for dragging. Deltas are relative to the dragging start position (mouseDeltaX and mouseDeltaY are relative to the last mouse position).
	Position resizeDelta;

	if (drag_ >= WindowDrag::Resize_N)
	{
		resizeDelta.x = mouseX - resizeStartPosition_.x;
		resizeDelta.y = mouseY - resizeStartPosition_.y;
	}

	Rect newRect = rect_;

	switch (drag_)
	{
	case WindowDrag::Header:
		newRect.x += mouseDeltaX;
		newRect.y += mouseDeltaY;
		break;
	case WindowDrag::Resize_N:
	{
		int delta = WZ_MIN(resizeDelta.y, resizeStartRect_.h - minimumWindowSize.h);
		newRect.y = resizeStartRect_.y + delta;
		newRect.h = resizeStartRect_.h - delta;
	}
	break;
	case WindowDrag::Resize_NE:
	{
		int delta = WZ_MIN(resizeDelta.y, resizeStartRect_.h - minimumWindowSize.h);
		newRect.y = resizeStartRect_.y + delta;
		newRect.w = WZ_MAX(minimumWindowSize.w, resizeStartRect_.w + resizeDelta.x);
		newRect.h = resizeStartRect_.h - delta;
	}
	break;
	case WindowDrag::Resize_E:
		newRect.w = WZ_MAX(minimumWindowSize.w, resizeStartRect_.w + resizeDelta.x);
		break;
	case WindowDrag::Resize_SE:
		newRect.w = WZ_MAX(minimumWindowSize.w, resizeStartRect_.w + resizeDelta.x);
		newRect.h = WZ_MAX(minimumWindowSize.h, resizeStartRect_.h + resizeDelta.y);
		break;
	case WindowDrag::Resize_S:
		newRect.h = WZ_MAX(minimumWindowSize.h, resizeStartRect_.h + resizeDelta.y);
		break;
	case WindowDrag::Resize_SW:
	{
		int delta = WZ_MIN(resizeDelta.x, resizeStartRect_.w - minimumWindowSize.w);
		newRect.x = resizeStartRect_.x + delta;
		newRect.w = resizeStartRect_.w - delta;
		newRect.h = WZ_MAX(minimumWindowSize.h, resizeStartRect_.h + resizeDelta.y);
	}
	break;
	case WindowDrag::Resize_W:
	{
		int delta = WZ_MIN(resizeDelta.x, resizeStartRect_.w - minimumWindowSize.w);
		newRect.x = resizeStartRect_.x + delta;
		newRect.w = resizeStartRect_.w - delta;
	}
	break;
	case WindowDrag::Resize_NW:
	{
		int deltaX, deltaY;
		deltaX = WZ_MIN(resizeDelta.x, resizeStartRect_.w - minimumWindowSize.w);
		deltaY = WZ_MIN(resizeDelta.y, resizeStartRect_.h - minimumWindowSize.h);
		newRect.x = resizeStartRect_.x + deltaX;
		newRect.y = resizeStartRect_.y + deltaY;
		newRect.w = resizeStartRect_.w - deltaX;
		newRect.h = resizeStartRect_.h - deltaY;
	}
	break;
	default:
		return; // Not dragging, don't call MainWindow::updateContentRect.
	}

	setRect(newRect);

	// Resizing a docked window: 
	if (mainWindow_->getWindowDockPosition(this) != DockPosition::None)
	{
		// Tell the mainWindow so it can resize other windows docked at the same position too.
		mainWindow_->updateDockedWindowRect(this);

		// Update the mainWindow content rect.
		mainWindow_->updateContentRect();
	}
}

Rect Window::getChildrenClipRect() const
{
	// Use the content rect.
	return content_->getAbsoluteRect();
}

void Window::draw(Rect clip)
{
	renderer_->drawWindow(this, clip);
}

Size Window::measure()
{
	return renderer_->measureWindow(this);
}

void Window::refreshHeaderHeightAndPadding()
{
	headerHeight_ = getLineHeight() + 6; // Padding.
	padding_ = Border(borderSize_ + headerHeight_, borderSize_, borderSize_, borderSize_);
}

void Window::calculateBorderRects(Rect *rects)
{
	WZ_ASSERT(rects);

	rects[Compass::N] = rect_;
	rects[Compass::N].x += borderSize_;
	rects[Compass::N].w -= borderSize_ * 2;
	rects[Compass::N].h = borderSize_;

	rects[Compass::NE] = rect_;
	rects[Compass::NE].x += rects[Compass::NE].w - borderSize_;
	rects[Compass::NE].w = borderSize_;
	rects[Compass::NE].h = borderSize_;

	rects[Compass::E] = rect_;
	rects[Compass::E].x += rects[Compass::E].w - borderSize_;
	rects[Compass::E].y += borderSize_;
	rects[Compass::E].w = borderSize_;
	rects[Compass::E].h -= borderSize_ * 2;

	rects[Compass::SE] = rect_;
	rects[Compass::SE].x += rects[Compass::SE].w - borderSize_;
	rects[Compass::SE].y += rects[Compass::SE].h - borderSize_;
	rects[Compass::SE].w = borderSize_;
	rects[Compass::SE].h = borderSize_;

	rects[Compass::S] = rect_;
	rects[Compass::S].x += borderSize_;
	rects[Compass::S].y += rects[Compass::S].h - borderSize_;
	rects[Compass::S].w -= borderSize_ * 2;
	rects[Compass::S].h = borderSize_;

	rects[Compass::SW] = rect_;
	rects[Compass::SW].y += rects[Compass::SW].h - borderSize_;
	rects[Compass::SW].w = borderSize_;
	rects[Compass::SW].h = borderSize_;

	rects[Compass::W] = rect_;
	rects[Compass::W].y += borderSize_;
	rects[Compass::W].w = borderSize_;
	rects[Compass::W].h -= borderSize_ * 2;

	rects[Compass::NW] = rect_;
	rects[Compass::NW].w = borderSize_;
	rects[Compass::NW].h = borderSize_;
}

void Window::calculateMouseOverBorderRects(int mouseX, int mouseY, Rect *borderRects, bool *mouseOverBorderRects)
{
	DockPosition::Enum dockPosition = mainWindow_->getWindowDockPosition(this);

	// Take into account dockPositioning, e.g. north dockPositioned window can only be resized south.
	mouseOverBorderRects[Compass::N] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[Compass::N]) && (dockPosition == DockPosition::None || dockPosition == DockPosition::South));
	mouseOverBorderRects[Compass::NE] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[Compass::NE]) && dockPosition == DockPosition::None);
	mouseOverBorderRects[Compass::E] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[Compass::E]) && (dockPosition == DockPosition::None || dockPosition == DockPosition::West));
	mouseOverBorderRects[Compass::SE] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[Compass::SE]) && dockPosition == DockPosition::None);
	mouseOverBorderRects[Compass::S] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[Compass::S]) && (dockPosition == DockPosition::None || dockPosition == DockPosition::North));
	mouseOverBorderRects[Compass::SW] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[Compass::SW]) && dockPosition == DockPosition::None);
	mouseOverBorderRects[Compass::W] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[Compass::W]) && (dockPosition == DockPosition::None || dockPosition == DockPosition::East));
	mouseOverBorderRects[Compass::NW] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[Compass::NW]) && dockPosition == DockPosition::None);
}

} // namespace wz
