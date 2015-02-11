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
#include "wz_internal.h"
#pragma hdrstop

#define WZ_WINDOW_UNDOCK_DISTANCE 16

namespace wz {

// rects parameter size should be WZ_NUM_COMPASS_POINTS
static void wz_window_calculate_border_rects(struct WindowImpl *window, Rect *rects)
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
static void wz_window_calculate_mouse_over_border_rects(struct WindowImpl *window, int mouseX, int mouseY, Rect *borderRects, bool *mouseOverBorderRects)
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

static void wz_window_mouse_button_down(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct WindowImpl *window;
	
	WZ_ASSERT(widget);
	window = (struct WindowImpl *)widget;

	if (mouseButton == 1)
	{
		Rect borderRects[WZ_NUM_COMPASS_POINTS];
		bool mouseOverBorderRects[WZ_NUM_COMPASS_POINTS];
		int i;

		// Drag the header.
		if (WZ_POINT_IN_RECT(mouseX, mouseY, window->getHeaderRect()))
		{
			window->drag = WZ_DRAG_HEADER;
			widget->mainWindow->pushLockInputWidget(widget);

			// Don't actually move the window yet if it's docked.
			if (window->mainWindow->getWindowDockPosition(window) == WZ_DOCK_POSITION_NONE)
			{
				widget->mainWindow->setMovingWindow(window);
			}
			else
			{
				window->undockStartPosition.x = mouseX;
				window->undockStartPosition.y = mouseY;
			}

			return;
		}

		// Resize by dragging the border.
		wz_window_calculate_border_rects(window, borderRects);
		wz_window_calculate_mouse_over_border_rects(window, mouseX, mouseY, borderRects, mouseOverBorderRects);

		for (i = 0; i < WZ_NUM_COMPASS_POINTS; i++)
		{
			if (mouseOverBorderRects[i])
			{
				window->drag = (WindowDrag)(WZ_DRAG_RESIZE_N + i);
				window->resizeStartPosition.x = mouseX;
				window->resizeStartPosition.y = mouseY;
				window->resizeStartRect = window->rect;
				widget->mainWindow->pushLockInputWidget(widget);
				return;
			}
		}
	}
}

static void wz_window_mouse_button_up(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct WindowImpl *window;

	WZ_ASSERT(widget);
	window = (struct WindowImpl *)widget;

	if (mouseButton == 1)
	{
		if (window->drag == WZ_DRAG_HEADER)
		{
			widget->mainWindow->setMovingWindow(NULL);
		}

		window->drag = WZ_DRAG_NONE;
		widget->mainWindow->popLockInputWidget(widget);
	}
}

static void wz_window_mouse_move(struct WidgetImpl *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct WindowImpl *window;
	Rect borderRects[WZ_NUM_COMPASS_POINTS];
	bool mouseOverBorderRects[WZ_NUM_COMPASS_POINTS];
	DockPosition dockPosition;
	Size minimumWindowSize;
	Position resizeDelta;
	Rect rect;

	WZ_ASSERT(widget);
	window = (struct WindowImpl *)widget;

	// Set the mouse cursor.
	wz_window_calculate_border_rects(window, borderRects);
	wz_window_calculate_mouse_over_border_rects(window, mouseX, mouseY, borderRects, mouseOverBorderRects);

	if (mouseOverBorderRects[WZ_COMPASS_N] || mouseOverBorderRects[WZ_COMPASS_S] || window->drag == WZ_DRAG_RESIZE_N || window->drag == WZ_DRAG_RESIZE_S)
	{
		widget->mainWindow->setCursor(WZ_CURSOR_RESIZE_N_S);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_E] || mouseOverBorderRects[WZ_COMPASS_W] || window->drag == WZ_DRAG_RESIZE_E || window->drag == WZ_DRAG_RESIZE_W)
	{
		widget->mainWindow->setCursor(WZ_CURSOR_RESIZE_E_W);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NE] || mouseOverBorderRects[WZ_COMPASS_SW] || window->drag == WZ_DRAG_RESIZE_NE || window->drag == WZ_DRAG_RESIZE_SW)
	{
		widget->mainWindow->setCursor(WZ_CURSOR_RESIZE_NE_SW);
	}
	else if (mouseOverBorderRects[WZ_COMPASS_NW] || mouseOverBorderRects[WZ_COMPASS_SE] || window->drag == WZ_DRAG_RESIZE_NW || window->drag == WZ_DRAG_RESIZE_SE)
	{
		widget->mainWindow->setCursor(WZ_CURSOR_RESIZE_NW_SE);
	}

	// Don't actually move the window yet if it's docked.
	dockPosition = widget->mainWindow->getWindowDockPosition(window);

	if (window->drag == WZ_DRAG_HEADER && dockPosition != WZ_DOCK_POSITION_NONE)
	{
		Position delta;

		delta.x = mouseX - window->undockStartPosition.x;
		delta.y = mouseY - window->undockStartPosition.y;

		// Undock and start moving if the mouse has moved far enough.
		if ((int)sqrtf((float)(delta.x * delta.x + delta.y * delta.y)) < WZ_WINDOW_UNDOCK_DISTANCE)
			return;

		widget->mainWindow->undockWindow(window);
		widget->mainWindow->setMovingWindow(window);

		// Re-position and resize the window.
		rect = widget->getRect();
		rect.x += delta.x;
		rect.y += delta.y;
		rect.w = WZ_MAX(200, window->sizeBeforeDocking.w);
		rect.h = WZ_MAX(200, window->sizeBeforeDocking.h);

		// If the mouse cursor would be outside the window, center the window on the mouse cursor.
		if (mouseX < rect.x + window->borderSize || mouseX > rect.x + rect.w - window->borderSize)
		{
			rect.x = mouseX - rect.w / 2;
		}

		widget->setRectInternal(rect);
		widget->mainWindow->updateContentRect();
	}

	// Calculate the minimum allowed window size.
	minimumWindowSize.w = window->borderSize * 2;
	minimumWindowSize.h = window->headerHeight + window->borderSize * 2;

	// Calculate mouse deltas for dragging. Deltas are relative to the dragging start position (mouseDeltaX and mouseDeltaY are relative to the last mouse position).
	if (window->drag >= WZ_DRAG_RESIZE_N)
	{
		resizeDelta.x = mouseX - window->resizeStartPosition.x;
		resizeDelta.y = mouseY - window->resizeStartPosition.y;
	}

	rect = widget->getRect();

	switch (window->drag)
	{
	case WZ_DRAG_HEADER:
		rect.x += mouseDeltaX;
		rect.y += mouseDeltaY;
		break;
	case WZ_DRAG_RESIZE_N:
		{
			int delta = WZ_MIN(resizeDelta.y, window->resizeStartRect.h - minimumWindowSize.h);
			rect.y = window->resizeStartRect.y + delta;
			rect.h = window->resizeStartRect.h - delta;
		}
		break;
	case WZ_DRAG_RESIZE_NE:
		{
			int delta = WZ_MIN(resizeDelta.y, window->resizeStartRect.h - minimumWindowSize.h);
			rect.y = window->resizeStartRect.y + delta;
			rect.w = WZ_MAX(minimumWindowSize.w, window->resizeStartRect.w + resizeDelta.x);
			rect.h = window->resizeStartRect.h - delta;
		}
		break;
	case WZ_DRAG_RESIZE_E:
		rect.w = WZ_MAX(minimumWindowSize.w, window->resizeStartRect.w + resizeDelta.x);
		break;
	case WZ_DRAG_RESIZE_SE:
		rect.w = WZ_MAX(minimumWindowSize.w, window->resizeStartRect.w + resizeDelta.x);
		rect.h = WZ_MAX(minimumWindowSize.h, window->resizeStartRect.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_S:
		rect.h = WZ_MAX(minimumWindowSize.h, window->resizeStartRect.h + resizeDelta.y);
		break;
	case WZ_DRAG_RESIZE_SW:
		{
			int delta = WZ_MIN(resizeDelta.x, window->resizeStartRect.w - minimumWindowSize.w);
			rect.x = window->resizeStartRect.x + delta;
			rect.w = window->resizeStartRect.w - delta;
			rect.h = WZ_MAX(minimumWindowSize.h, window->resizeStartRect.h + resizeDelta.y);
		}
		break;
	case WZ_DRAG_RESIZE_W:
		{
			int delta = WZ_MIN(resizeDelta.x, window->resizeStartRect.w - minimumWindowSize.w);
			rect.x = window->resizeStartRect.x + delta;
			rect.w = window->resizeStartRect.w - delta;
		}
		break;
	case WZ_DRAG_RESIZE_NW:
		{
			int deltaX, deltaY;
			deltaX = WZ_MIN(resizeDelta.x, window->resizeStartRect.w - minimumWindowSize.w);
			deltaY = WZ_MIN(resizeDelta.y, window->resizeStartRect.h - minimumWindowSize.h);
			rect.x = window->resizeStartRect.x + deltaX;
			rect.y = window->resizeStartRect.y + deltaY;
			rect.w = window->resizeStartRect.w - deltaX;
			rect.h = window->resizeStartRect.h - deltaY;
		}
		break;
	default:
		return; // Not dragging, don't call MainWindowImpl::updateContentRect.
	}

	widget->setRectInternal(rect);

	// Resizing a docked window: 
	if (widget->mainWindow->getWindowDockPosition(window) != WZ_DOCK_POSITION_NONE)
	{
		// Tell the mainWindow so it can resize other windows docked at the same position too.
		widget->mainWindow->updateDockedWindowRect(window);

		// Update the mainWindow content rect.
		widget->mainWindow->updateContentRect();
	}
}

static Rect wz_window_get_children_clip_rect(struct WidgetImpl *widget)
{
	struct WindowImpl *window;

	WZ_ASSERT(widget);
	window = (struct WindowImpl *)widget;
	return window->content->getAbsoluteRect();
}

static void wz_window_set_rect(struct WidgetImpl *widget, Rect rect)
{
	struct WindowImpl *window;
	Rect contentRect;

	WZ_ASSERT(widget);
	window = (struct WindowImpl *)widget;
	window->rect = rect;

	contentRect.x = window->borderSize;
	contentRect.y = window->borderSize + window->headerHeight;
	contentRect.w = rect.w - window->borderSize * 2;
	contentRect.h = rect.h - (window->headerHeight + window->borderSize * 2);

	window->content->setRectInternal(contentRect);
}

WindowImpl::WindowImpl(const std::string &title)
{
	type = WZ_TYPE_WINDOW;
	drawPriority = 0;
	headerHeight = 0;
	borderSize = 4;
	drag = WZ_DRAG_NONE;

	vtable.mouse_button_down = wz_window_mouse_button_down;
	vtable.mouse_button_up = wz_window_mouse_button_up;
	vtable.mouse_move = wz_window_mouse_move;
	vtable.get_children_clip_rect = wz_window_get_children_clip_rect;
	vtable.set_rect = wz_window_set_rect;
	this->title = title;

	content = new struct WidgetImpl;
	addChildWidget(content);
}

void WindowImpl::onRendererChanged()
{
	refreshHeaderHeight();
	refreshRect();
}

void WindowImpl::onFontChanged(const char *fontFace, float fontSize)
{
	refreshHeaderHeight();
	refreshRect();
}

void WindowImpl::draw(Rect clip)
{
	renderer->drawWindow(this, clip);
}

Size WindowImpl::measure()
{
	return renderer->measureWindow(this);
}

int WindowImpl::getHeaderHeight() const
{
	return headerHeight;
}

int WindowImpl::getBorderSize() const
{
	return borderSize;
}

Rect WindowImpl::getHeaderRect() const
{
	Rect rect;
	rect.x = this->rect.x + borderSize;
	rect.y = this->rect.y + borderSize;
	rect.w = this->rect.w - borderSize * 2;
	rect.h = headerHeight;
	return rect;
}

void WindowImpl::setTitle(const char *title)
{
	this->title = title;
}

const char *WindowImpl::getTitle() const
{
	return title.c_str();
}

void WindowImpl::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->getImpl()->getType() == WZ_TYPE_MAIN_WINDOW || widget->getImpl()->getType() == WZ_TYPE_WINDOW)
		return;

	content->addChildWidget(widget);
}

void WindowImpl::add(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	content->addChildWidget(widget);
}

void WindowImpl::remove(Widget *widget)
{
	WZ_ASSERT(widget);
	content->removeChildWidget(widget);
}

void WindowImpl::remove(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	content->removeChildWidget(widget);
}

void WindowImpl::refreshHeaderHeight()
{
	headerHeight = getLineHeight() + 6; // Padding.
}

struct WidgetImpl *wz_window_get_content_widget(struct WindowImpl *window)
{
	WZ_ASSERT(window);
	return window->content;
}

// Save the window size before docking so it can be restored if the window is undocked later.
void wz_window_dock(struct WindowImpl *window)
{
	WZ_ASSERT(window);
	window->sizeBeforeDocking.w = window->rect.w;
	window->sizeBeforeDocking.h = window->rect.h;
}

int wz_window_get_draw_priority(const struct WindowImpl *window)
{
	WZ_ASSERT(window);
	return window->drawPriority;
}

void wz_window_set_draw_priority(struct WindowImpl *window, int drawPriority)
{
	WZ_ASSERT(window);
	window->drawPriority = drawPriority;
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

Window::Window()
{
	impl.reset(new WindowImpl(NULL));
}

Window::Window(const std::string &title)
{
	impl.reset(new WindowImpl(title));
}

Window::~Window()
{
}

const char *Window::getTitle() const
{
	return getImpl()->getTitle();
}

Window *Window::setTitle(const std::string &title)
{
	getImpl()->setTitle(title.c_str());
	return this;
}

Widget *Window::add(Widget *widget)
{
	getImpl()->add(widget);
	return widget;
}

void Window::remove(Widget *widget)
{
	getImpl()->remove(widget);
}

WindowImpl *Window::getImpl()
{
	return (WindowImpl *)impl.get();
}

const WindowImpl *Window::getImpl() const
{
	return (const WindowImpl *)impl.get();
}

} // namespace wz
