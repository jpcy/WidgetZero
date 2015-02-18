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
	drag_ = WZ_DRAG_NONE;
	title_ = title;

	content_ = new Widget;
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
	refreshHeaderHeight();
	refreshRect();
}

void Window::onFontChanged(const char *fontFace, float fontSize)
{
	refreshHeaderHeight();
	refreshRect();
}

void Window::onRectChanged()
{
	Rect contentRect;
	contentRect.x = borderSize_;
	contentRect.y = borderSize_ + headerHeight_;
	contentRect.w = rect_.w - borderSize_ * 2;
	contentRect.h = rect_.h - (headerHeight_ + borderSize_ * 2);
	content_->setRectInternal(contentRect);
}

void Window::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		// Drag the header.
		if (WZ_POINT_IN_RECT(mouseX, mouseY, getHeaderRect()))
		{
			drag_ = WZ_DRAG_HEADER;
			mainWindow_->pushLockInputWidget(this);

			// Don't actually move the window yet if it's docked.
			if (mainWindow_->getWindowDockPosition(this) == WZ_DOCK_POSITION_NONE)
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
		Rect borderRects[WZ_NUM_COMPASS_POINTS];
		bool mouseOverBorderRects[WZ_NUM_COMPASS_POINTS];

		calculateBorderRects(borderRects);
		calculateMouseOverBorderRects(mouseX, mouseY, borderRects, mouseOverBorderRects);

		for (int i = 0; i < WZ_NUM_COMPASS_POINTS; i++)
		{
			if (mouseOverBorderRects[i])
			{
				drag_ = (WindowDrag)(WZ_DRAG_RESIZE_N + i);
				resizeStartPosition_.x = mouseX;
				resizeStartPosition_.y = mouseY;
				resizeStartRect_ = rect_;
				mainWindow_->pushLockInputWidget(this);
				return;
			}
		}
	}
}

void Window::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		if (drag_ == WZ_DRAG_HEADER)
		{
			mainWindow_->setMovingWindow(NULL);
		}

		drag_ = WZ_DRAG_NONE;
		mainWindow_->popLockInputWidget(this);
	}
}

void Window::onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	// Set the mouse cursor.
	Rect borderRects[WZ_NUM_COMPASS_POINTS];
	bool mouseOverBorderRects[WZ_NUM_COMPASS_POINTS];

	calculateBorderRects(borderRects);
	calculateMouseOverBorderRects(mouseX, mouseY, borderRects, mouseOverBorderRects);

	if (mouseOverBorderRects[WZ_COMPASS_N] || mouseOverBorderRects[WZ_COMPASS_S] || drag_ == WZ_DRAG_RESIZE_N || drag_ == WZ_DRAG_RESIZE_S)
	{
		mainWindow_->setCursor(WZ_CURSOR_RESIZE_N_S);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_E] || mouseOverBorderRects[WZ_COMPASS_W] || drag_ == WZ_DRAG_RESIZE_E || drag_ == WZ_DRAG_RESIZE_W)
	{
		mainWindow_->setCursor(WZ_CURSOR_RESIZE_E_W);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NE] || mouseOverBorderRects[WZ_COMPASS_SW] || drag_ == WZ_DRAG_RESIZE_NE || drag_ == WZ_DRAG_RESIZE_SW)
	{
		mainWindow_->setCursor(WZ_CURSOR_RESIZE_NE_SW);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NW] || mouseOverBorderRects[WZ_COMPASS_SE] || drag_ == WZ_DRAG_RESIZE_NW || drag_ == WZ_DRAG_RESIZE_SE)
	{
		mainWindow_->setCursor(WZ_CURSOR_RESIZE_NW_SE);
	}

	// Don't actually move the window yet if it's docked.
	const DockPosition dockPosition = mainWindow_->getWindowDockPosition(this);

	if (drag_ == WZ_DRAG_HEADER && dockPosition != WZ_DOCK_POSITION_NONE)
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

		setRectInternal(newRect);
		mainWindow_->updateContentRect();
	}

	// Calculate the minimum allowed window size.
	const Size minimumWindowSize(borderSize_ * 2, headerHeight_ + borderSize_ * 2);

	// Calculate mouse deltas for dragging. Deltas are relative to the dragging start position (mouseDeltaX and mouseDeltaY are relative to the last mouse position).
	Position resizeDelta;

	if (drag_ >= WZ_DRAG_RESIZE_N)
	{
		resizeDelta.x = mouseX - resizeStartPosition_.x;
		resizeDelta.y = mouseY - resizeStartPosition_.y;
	}

	Rect newRect = rect_;

	switch (drag_)
	{
	case WZ_DRAG_HEADER:
		newRect.x += mouseDeltaX;
		newRect.y += mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_N:
	{
		int delta = WZ_MIN(resizeDelta.y, resizeStartRect_.h - minimumWindowSize.h);
		newRect.y = resizeStartRect_.y + delta;
		newRect.h = resizeStartRect_.h - delta;
	}
	break;
	case WZ_DRAG_RESIZE_NE:
	{
		int delta = WZ_MIN(resizeDelta.y, resizeStartRect_.h - minimumWindowSize.h);
		newRect.y = resizeStartRect_.y + delta;
		newRect.w = WZ_MAX(minimumWindowSize.w, resizeStartRect_.w + resizeDelta.x);
		newRect.h = resizeStartRect_.h - delta;
	}
	break;
	case WZ_DRAG_RESIZE_E:
		newRect.w = WZ_MAX(minimumWindowSize.w, resizeStartRect_.w + resizeDelta.x);
		break;
	case WZ_DRAG_RESIZE_SE:
		newRect.w = WZ_MAX(minimumWindowSize.w, resizeStartRect_.w + resizeDelta.x);
		newRect.h = WZ_MAX(minimumWindowSize.h, resizeStartRect_.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_S:
		newRect.h = WZ_MAX(minimumWindowSize.h, resizeStartRect_.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_SW:
	{
		int delta = WZ_MIN(resizeDelta.x, resizeStartRect_.w - minimumWindowSize.w);
		newRect.x = resizeStartRect_.x + delta;
		newRect.w = resizeStartRect_.w - delta;
		newRect.h = WZ_MAX(minimumWindowSize.h, resizeStartRect_.h + resizeDelta.y);
	}
	break;
	case WZ_DRAG_RESIZE_W:
	{
		int delta = WZ_MIN(resizeDelta.x, resizeStartRect_.w - minimumWindowSize.w);
		newRect.x = resizeStartRect_.x + delta;
		newRect.w = resizeStartRect_.w - delta;
	}
	break;
	case WZ_DRAG_RESIZE_NW:
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

	setRectInternal(newRect);

	// Resizing a docked window: 
	if (mainWindow_->getWindowDockPosition(this) != WZ_DOCK_POSITION_NONE)
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

void Window::refreshHeaderHeight()
{
	headerHeight_ = getLineHeight() + 6; // Padding.
}

void Window::calculateBorderRects(Rect *rects)
{
	WZ_ASSERT(rects);

	rects[WZ_COMPASS_N] = rect_;
	rects[WZ_COMPASS_N].x += borderSize_;
	rects[WZ_COMPASS_N].w -= borderSize_ * 2;
	rects[WZ_COMPASS_N].h = borderSize_;

	rects[WZ_COMPASS_NE] = rect_;
	rects[WZ_COMPASS_NE].x += rects[WZ_COMPASS_NE].w - borderSize_;
	rects[WZ_COMPASS_NE].w = borderSize_;
	rects[WZ_COMPASS_NE].h = borderSize_;

	rects[WZ_COMPASS_E] = rect_;
	rects[WZ_COMPASS_E].x += rects[WZ_COMPASS_E].w - borderSize_;
	rects[WZ_COMPASS_E].y += borderSize_;
	rects[WZ_COMPASS_E].w = borderSize_;
	rects[WZ_COMPASS_E].h -= borderSize_ * 2;

	rects[WZ_COMPASS_SE] = rect_;
	rects[WZ_COMPASS_SE].x += rects[WZ_COMPASS_SE].w - borderSize_;
	rects[WZ_COMPASS_SE].y += rects[WZ_COMPASS_SE].h - borderSize_;
	rects[WZ_COMPASS_SE].w = borderSize_;
	rects[WZ_COMPASS_SE].h = borderSize_;

	rects[WZ_COMPASS_S] = rect_;
	rects[WZ_COMPASS_S].x += borderSize_;
	rects[WZ_COMPASS_S].y += rects[WZ_COMPASS_S].h - borderSize_;
	rects[WZ_COMPASS_S].w -= borderSize_ * 2;
	rects[WZ_COMPASS_S].h = borderSize_;

	rects[WZ_COMPASS_SW] = rect_;
	rects[WZ_COMPASS_SW].y += rects[WZ_COMPASS_SW].h - borderSize_;
	rects[WZ_COMPASS_SW].w = borderSize_;
	rects[WZ_COMPASS_SW].h = borderSize_;

	rects[WZ_COMPASS_W] = rect_;
	rects[WZ_COMPASS_W].y += borderSize_;
	rects[WZ_COMPASS_W].w = borderSize_;
	rects[WZ_COMPASS_W].h -= borderSize_ * 2;

	rects[WZ_COMPASS_NW] = rect_;
	rects[WZ_COMPASS_NW].w = borderSize_;
	rects[WZ_COMPASS_NW].h = borderSize_;
}

void Window::calculateMouseOverBorderRects(int mouseX, int mouseY, Rect *borderRects, bool *mouseOverBorderRects)
{
	DockPosition dockPosition = mainWindow_->getWindowDockPosition(this);

	// Take into account dockPositioning, e.g. north dockPositioned window can only be resized south.
	mouseOverBorderRects[WZ_COMPASS_N] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_N]) && (dockPosition == WZ_DOCK_POSITION_NONE || dockPosition == WZ_DOCK_POSITION_SOUTH));
	mouseOverBorderRects[WZ_COMPASS_NE] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_NE]) && dockPosition == WZ_DOCK_POSITION_NONE);
	mouseOverBorderRects[WZ_COMPASS_E] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_E]) && (dockPosition == WZ_DOCK_POSITION_NONE || dockPosition == WZ_DOCK_POSITION_WEST));
	mouseOverBorderRects[WZ_COMPASS_SE] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_SE]) && dockPosition == WZ_DOCK_POSITION_NONE);
	mouseOverBorderRects[WZ_COMPASS_S] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_S]) && (dockPosition == WZ_DOCK_POSITION_NONE || dockPosition == WZ_DOCK_POSITION_NORTH));
	mouseOverBorderRects[WZ_COMPASS_SW] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_SW]) && dockPosition == WZ_DOCK_POSITION_NONE);
	mouseOverBorderRects[WZ_COMPASS_W] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_W]) && (dockPosition == WZ_DOCK_POSITION_NONE || dockPosition == WZ_DOCK_POSITION_EAST));
	mouseOverBorderRects[WZ_COMPASS_NW] = (WZ_POINT_IN_RECT(mouseX, mouseY, borderRects[WZ_COMPASS_NW]) && dockPosition == WZ_DOCK_POSITION_NONE);
}

} // namespace wz
