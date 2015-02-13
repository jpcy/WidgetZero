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

// rects parameter size should be WZ_NUM_COMPASS_POINTS
static void wz_window_calculate_border_rects(Window *window, Rect *rects)
{
	WZ_ASSERT(window);
	WZ_ASSERT(rects);

	rects[WZ_COMPASS_N] = window->rect;
	rects[WZ_COMPASS_N].x += window->borderSize;
	rects[WZ_COMPASS_N].w -= window->borderSize * 2;
	rects[WZ_COMPASS_N].h = window->borderSize;

	rects[WZ_COMPASS_NE] = window->rect;
	rects[WZ_COMPASS_NE].x += rects[WZ_COMPASS_NE].w - window->borderSize;
	rects[WZ_COMPASS_NE].w = window->borderSize;
	rects[WZ_COMPASS_NE].h = window->borderSize;

	rects[WZ_COMPASS_E] = window->rect;
	rects[WZ_COMPASS_E].x += rects[WZ_COMPASS_E].w - window->borderSize;
	rects[WZ_COMPASS_E].y += window->borderSize;
	rects[WZ_COMPASS_E].w = window->borderSize;
	rects[WZ_COMPASS_E].h -= window->borderSize * 2;

	rects[WZ_COMPASS_SE] = window->rect;
	rects[WZ_COMPASS_SE].x += rects[WZ_COMPASS_SE].w - window->borderSize;
	rects[WZ_COMPASS_SE].y += rects[WZ_COMPASS_SE].h - window->borderSize;
	rects[WZ_COMPASS_SE].w = window->borderSize;
	rects[WZ_COMPASS_SE].h = window->borderSize;

	rects[WZ_COMPASS_S] = window->rect;
	rects[WZ_COMPASS_S].x += window->borderSize;
	rects[WZ_COMPASS_S].y += rects[WZ_COMPASS_S].h - window->borderSize;
	rects[WZ_COMPASS_S].w -= window->borderSize * 2;
	rects[WZ_COMPASS_S].h = window->borderSize;

	rects[WZ_COMPASS_SW] = window->rect;
	rects[WZ_COMPASS_SW].y += rects[WZ_COMPASS_SW].h - window->borderSize;
	rects[WZ_COMPASS_SW].w = window->borderSize;
	rects[WZ_COMPASS_SW].h = window->borderSize;

	rects[WZ_COMPASS_W] = window->rect;
	rects[WZ_COMPASS_W].y += window->borderSize;
	rects[WZ_COMPASS_W].w = window->borderSize;
	rects[WZ_COMPASS_W].h -= window->borderSize * 2;

	rects[WZ_COMPASS_NW] = window->rect;
	rects[WZ_COMPASS_NW].w = window->borderSize;
	rects[WZ_COMPASS_NW].h = window->borderSize;
}

// borderRects and mouseOverBorderRects parameter sizes should be WZ_NUM_COMPASS_POINTS
static void wz_window_calculate_mouse_over_border_rects(Window *window, int mouseX, int mouseY, Rect *borderRects, bool *mouseOverBorderRects)
{
	DockPosition dockPosition;

	WZ_ASSERT(window);
	dockPosition = window->mainWindow->getWindowDockPosition(window);

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

Window::Window(const std::string &title)
{
	type = WZ_TYPE_WINDOW;
	drawPriority = 0;
	headerHeight = 0;
	borderSize = 4;
	drag = WZ_DRAG_NONE;

	this->title = title;

	content = new Widget;
	addChildWidget(content);
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
	contentRect.x = borderSize;
	contentRect.y = borderSize + headerHeight;
	contentRect.w = rect.w - borderSize * 2;
	contentRect.h = rect.h - (headerHeight + borderSize * 2);
	content->setRectInternal(contentRect);
}

void Window::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		// Drag the header.
		if (WZ_POINT_IN_RECT(mouseX, mouseY, getHeaderRect()))
		{
			drag = WZ_DRAG_HEADER;
			mainWindow->pushLockInputWidget(this);

			// Don't actually move the window yet if it's docked.
			if (mainWindow->getWindowDockPosition(this) == WZ_DOCK_POSITION_NONE)
			{
				mainWindow->setMovingWindow(this);
			}
			else
			{
				undockStartPosition.x = mouseX;
				undockStartPosition.y = mouseY;
			}

			return;
		}

		// Resize by dragging the border.
		Rect borderRects[WZ_NUM_COMPASS_POINTS];
		bool mouseOverBorderRects[WZ_NUM_COMPASS_POINTS];

		wz_window_calculate_border_rects(this, borderRects);
		wz_window_calculate_mouse_over_border_rects(this, mouseX, mouseY, borderRects, mouseOverBorderRects);

		for (int i = 0; i < WZ_NUM_COMPASS_POINTS; i++)
		{
			if (mouseOverBorderRects[i])
			{
				drag = (WindowDrag)(WZ_DRAG_RESIZE_N + i);
				resizeStartPosition.x = mouseX;
				resizeStartPosition.y = mouseY;
				resizeStartRect = rect;
				mainWindow->pushLockInputWidget(this);
				return;
			}
		}
	}
}

void Window::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		if (drag == WZ_DRAG_HEADER)
		{
			mainWindow->setMovingWindow(NULL);
		}

		drag = WZ_DRAG_NONE;
		mainWindow->popLockInputWidget(this);
	}
}

void Window::onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	// Set the mouse cursor.
	Rect borderRects[WZ_NUM_COMPASS_POINTS];
	bool mouseOverBorderRects[WZ_NUM_COMPASS_POINTS];

	wz_window_calculate_border_rects(this, borderRects);
	wz_window_calculate_mouse_over_border_rects(this, mouseX, mouseY, borderRects, mouseOverBorderRects);

	if (mouseOverBorderRects[WZ_COMPASS_N] || mouseOverBorderRects[WZ_COMPASS_S] || drag == WZ_DRAG_RESIZE_N || drag == WZ_DRAG_RESIZE_S)
	{
		mainWindow->setCursor(WZ_CURSOR_RESIZE_N_S);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_E] || mouseOverBorderRects[WZ_COMPASS_W] || drag == WZ_DRAG_RESIZE_E || drag == WZ_DRAG_RESIZE_W)
	{
		mainWindow->setCursor(WZ_CURSOR_RESIZE_E_W);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NE] || mouseOverBorderRects[WZ_COMPASS_SW] || drag == WZ_DRAG_RESIZE_NE || drag == WZ_DRAG_RESIZE_SW)
	{
		mainWindow->setCursor(WZ_CURSOR_RESIZE_NE_SW);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NW] || mouseOverBorderRects[WZ_COMPASS_SE] || drag == WZ_DRAG_RESIZE_NW || drag == WZ_DRAG_RESIZE_SE)
	{
		mainWindow->setCursor(WZ_CURSOR_RESIZE_NW_SE);
	}

	// Don't actually move the window yet if it's docked.
	const DockPosition dockPosition = mainWindow->getWindowDockPosition(this);

	if (drag == WZ_DRAG_HEADER && dockPosition != WZ_DOCK_POSITION_NONE)
	{
		Position delta;

		delta.x = mouseX - undockStartPosition.x;
		delta.y = mouseY - undockStartPosition.y;

		// Undock and start moving if the mouse has moved far enough.
		if ((int)sqrtf((float)(delta.x * delta.x + delta.y * delta.y)) < WZ_WINDOW_UNDOCK_DISTANCE)
			return;

		mainWindow->undockWindow(this);
		mainWindow->setMovingWindow(this);

		// Re-position and resize the window.
		Rect newRect = rect;
		newRect.x += delta.x;
		newRect.y += delta.y;
		newRect.w = WZ_MAX(200, sizeBeforeDocking.w);
		newRect.h = WZ_MAX(200, sizeBeforeDocking.h);

		// If the mouse cursor would be outside the window, center the window on the mouse cursor.
		if (mouseX < newRect.x + borderSize || mouseX > newRect.x + newRect.w - borderSize)
		{
			newRect.x = mouseX - newRect.w / 2;
		}

		setRectInternal(newRect);
		mainWindow->updateContentRect();
	}

	// Calculate the minimum allowed window size.
	const Size minimumWindowSize(borderSize * 2, headerHeight + borderSize * 2);

	// Calculate mouse deltas for dragging. Deltas are relative to the dragging start position (mouseDeltaX and mouseDeltaY are relative to the last mouse position).
	Position resizeDelta;

	if (drag >= WZ_DRAG_RESIZE_N)
	{
		resizeDelta.x = mouseX - resizeStartPosition.x;
		resizeDelta.y = mouseY - resizeStartPosition.y;
	}

	Rect newRect = rect;

	switch (drag)
	{
	case WZ_DRAG_HEADER:
		newRect.x += mouseDeltaX;
		newRect.y += mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_N:
		{
			int delta = WZ_MIN(resizeDelta.y, resizeStartRect.h - minimumWindowSize.h);
			newRect.y = resizeStartRect.y + delta;
			newRect.h = resizeStartRect.h - delta;
		}
		break;
	case WZ_DRAG_RESIZE_NE:
		{
			int delta = WZ_MIN(resizeDelta.y, resizeStartRect.h - minimumWindowSize.h);
			newRect.y = resizeStartRect.y + delta;
			newRect.w = WZ_MAX(minimumWindowSize.w, resizeStartRect.w + resizeDelta.x);
			newRect.h = resizeStartRect.h - delta;
		}
		break;
	case WZ_DRAG_RESIZE_E:
		newRect.w = WZ_MAX(minimumWindowSize.w, resizeStartRect.w + resizeDelta.x);
		break;
	case WZ_DRAG_RESIZE_SE:
		newRect.w = WZ_MAX(minimumWindowSize.w, resizeStartRect.w + resizeDelta.x);
		newRect.h = WZ_MAX(minimumWindowSize.h, resizeStartRect.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_S:
		newRect.h = WZ_MAX(minimumWindowSize.h, resizeStartRect.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_SW:
		{
			int delta = WZ_MIN(resizeDelta.x, resizeStartRect.w - minimumWindowSize.w);
			newRect.x = resizeStartRect.x + delta;
			newRect.w = resizeStartRect.w - delta;
			newRect.h = WZ_MAX(minimumWindowSize.h, resizeStartRect.h + resizeDelta.y);
		}
		break;
	case WZ_DRAG_RESIZE_W:
		{
			int delta = WZ_MIN(resizeDelta.x, resizeStartRect.w - minimumWindowSize.w);
			newRect.x = resizeStartRect.x + delta;
			newRect.w = resizeStartRect.w - delta;
		}
		break;
	case WZ_DRAG_RESIZE_NW:
		{
			int deltaX, deltaY;
			deltaX = WZ_MIN(resizeDelta.x, resizeStartRect.w - minimumWindowSize.w);
			deltaY = WZ_MIN(resizeDelta.y, resizeStartRect.h - minimumWindowSize.h);
			newRect.x = resizeStartRect.x + deltaX;
			newRect.y = resizeStartRect.y + deltaY;
			newRect.w = resizeStartRect.w - deltaX;
			newRect.h = resizeStartRect.h - deltaY;
		}
		break;
	default:
		return; // Not dragging, don't call MainWindow::updateContentRect.
	}

	setRectInternal(newRect);

	// Resizing a docked window: 
	if (mainWindow->getWindowDockPosition(this) != WZ_DOCK_POSITION_NONE)
	{
		// Tell the mainWindow so it can resize other windows docked at the same position too.
		mainWindow->updateDockedWindowRect(this);

		// Update the mainWindow content rect.
		mainWindow->updateContentRect();
	}
}

Rect Window::getChildrenClipRect() const
{
	// Use the content rect.
	return content->getAbsoluteRect();
}

void Window::draw(Rect clip)
{
	renderer->drawWindow(this, clip);
}

Size Window::measure()
{
	return renderer->measureWindow(this);
}

int Window::getHeaderHeight() const
{
	return headerHeight;
}

int Window::getBorderSize() const
{
	return borderSize;
}

Rect Window::getHeaderRect() const
{
	Rect rect;
	rect.x = this->rect.x + borderSize;
	rect.y = this->rect.y + borderSize;
	rect.w = this->rect.w - borderSize * 2;
	rect.h = headerHeight;
	return rect;
}

void Window::setTitle(const char *title)
{
	this->title = title;
}

const char *Window::getTitle() const
{
	return title.c_str();
}

Widget *Window::getContentWidget()
{
	return content;
}

int Window::getDrawPriority() const
{
	return drawPriority;
}

void Window::setDrawPriority(int drawPriority)
{
	this->drawPriority = drawPriority;
}

void Window::dock()
{
	// Save the window size before docking so it can be restored if the window is undocked later.
	sizeBeforeDocking.w = rect.w;
	sizeBeforeDocking.h = rect.h;
}

void Window::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	content->addChildWidget(widget);
}

void Window::remove(Widget *widget)
{
	WZ_ASSERT(widget);
	content->removeChildWidget(widget);
}

void Window::refreshHeaderHeight()
{
	headerHeight = getLineHeight() + 6; // Padding.
}

} // namespace wz
