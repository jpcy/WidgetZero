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

DockIcon::DockIcon()
{
	setSizeInternal(48, 48);
}

void DockIcon::draw(Rect clip)
{
	renderer->drawDockIcon(this, clip);
}

void DockPreview::draw(Rect clip)
{
	renderer->drawDockPreview(this, clip);
}

// Used by all dock tab bars.
static void wz_main_window_dock_tab_bar_tab_changed(Event *e)
{
	MainWindow *mainWindow;
	DockPosition dockPosition;
	Window *window;

	WZ_ASSERT(e);
	mainWindow = e->base.widget->mainWindow;

	if (mainWindow->ignoreDockTabBarChangedEvent)
		return;

	// Figure out which dock position this tab bar is at.
	dockPosition = WZ_DOCK_POSITION_NONE;

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		if (mainWindow->dockTabBars[i] == e->tabBar.tabBar)
		{
			dockPosition = (DockPosition)i;
			break;
		}
	}

	WZ_ASSERT(dockPosition != WZ_DOCK_POSITION_NONE);

	// Get the window corresponding to the tab.
	window = (Window *)e->tabBar.tab->getInternalMetadata();

	// Internal metadata won't be set yet when first adding a tab.
	if (window == NULL)
		return;

	// Set the window to visible, hide all the other windows at this dock position.
	window->setVisible(true);

	for (size_t i = 0; i < mainWindow->dockedWindows[dockPosition].size(); i++)
	{
		if (mainWindow->dockedWindows[dockPosition][i] == window)
			continue;

		mainWindow->dockedWindows[dockPosition][i]->setVisible(false);
	}
}

/*
================================================================================

DRAWING

================================================================================
*/

typedef bool (*WidgetPredicate)(const Widget *);

static bool wz_widget_true(const Widget *widget)
{
	widget = widget;
	return true;
}

static bool wz_widget_is_combo_ancestor(const Widget *widget)
{
	return widget->type != WZ_TYPE_COMBO && widget->findClosestAncestor(WZ_TYPE_COMBO) != NULL;
}

static bool wz_widget_is_not_combo(const Widget *widget)
{
	return widget->type != WZ_TYPE_COMBO;
}

static bool wz_widget_is_not_window_or_combo(const Widget *widget)
{
	return widget->type != WZ_TYPE_WINDOW && widget->type != WZ_TYPE_COMBO;
}

static void wz_widget_draw_recursive(Widget *widget, Rect clip, WidgetPredicate draw_predicate, WidgetPredicate recurse_predicate)
{
	bool drawLastFound = false;

	if (!widget->getVisible())
		return;

	// Don't render the widget if it's outside its parent window.
	// Special case for combo ancestors.
	if (!widget->overlapsParentWindow() && !wz_widget_is_combo_ancestor(widget))
		return;

	if (draw_predicate(widget) && !widget->drawManually)
	{
		widget->draw(clip);
	}

	// Update clip rect.
	if (!Rect::intersect(clip, widget->getChildrenClipRect(), &clip))
	{
		// Reset to mainWindow clip rect.
		clip = widget->mainWindow->getRect();
	}

	if (!recurse_predicate(widget))
		return;

	// Recurse into children, skip children that are flagged to draw last.
	for (size_t i = 0; i < widget->children.size(); i++)
	{
		if (widget->children[i]->flags & WZ_WIDGET_FLAG_DRAW_LAST)
		{
			drawLastFound = true;
		}
		else
		{
			wz_widget_draw_recursive(widget->children[i], clip, draw_predicate, recurse_predicate);
		}
	}

	// Recurse into children that are flagged to draw last.
	if (drawLastFound)
	{
		for (size_t i = 0; i < widget->children.size(); i++)
		{
			if (widget->children[i]->flags & WZ_WIDGET_FLAG_DRAW_LAST)
			{
				wz_widget_draw_recursive(widget->children[i], clip, draw_predicate, recurse_predicate);
			}
		}
	}
}

static void wz_widget_draw(Widget *widget, WidgetPredicate draw_predicate, WidgetPredicate recurse_predicate)
{
	WZ_ASSERT(widget);
	wz_widget_draw_recursive(widget, widget->getRect(), draw_predicate, recurse_predicate);
}

static int wz_compare_window_draw_priorities_docked(const void *a, const void *b)
{
	const Window *window1, *window2;
	bool window1Docked, window2Docked;

	window1 = *((const Window **)a);
	window2 = *((const Window **)b);
	window1Docked = window1->mainWindow->getWindowDockPosition(window1) != WZ_DOCK_POSITION_NONE;
	window2Docked = window2->mainWindow->getWindowDockPosition(window2) != WZ_DOCK_POSITION_NONE;

	if (window1Docked && !window2Docked)
	{
		return -1;
	}
	else if (window2Docked && !window1Docked)
	{
		return 1;
	}

	return window1->getDrawPriority() - window2->getDrawPriority();
}

static void wz_widget_draw_if_visible(Widget *widget)
{
	if (widget->getVisible())
	{
		Rect clip;
		clip.x = clip.y = clip.w = clip.h = 0;
		widget->draw(clip);
	}
}

MainWindow::MainWindow(IRenderer *renderer)
{
	type = WZ_TYPE_MAIN_WINDOW;
	handle_event = NULL;
	cursor = WZ_CURSOR_DEFAULT;
	isShiftKeyDown_ = isControlKeyDown_ = false;
	lockInputWindow = NULL;
	keyboardFocusWidget = NULL;
	movingWindow = NULL;
	windowDockPosition = WZ_DOCK_POSITION_NONE;
	ignoreDockTabBarChangedEvent = false;
	menuBar = NULL;

	this->renderer = renderer;
	mainWindow = this;
	isTextCursorVisible_ = true;

	// Create content widget.
	content = new Widget;
	content->mainWindow = this;
	addChildWidget(content);

	// Create dock icon widgets.
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		dockIcons[i] = new DockIcon;
		dockIcons[i]->setVisible(false);
		dockIcons[i]->setDrawManually(true);
		addChildWidget(dockIcons[i]);
	}

	updateDockIconPositions();

	// Create dock preview widget.
	dockPreview = new DockPreview;
	dockPreview->setDrawManually(true);
	dockPreview->setVisible(false);
	addChildWidget(dockPreview);

	// Create dock tab bars.
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		dockTabBars[i] = new TabBar;
		dockTabBars[i]->setVisible(false);
		addChildWidget(dockTabBars[i]);
		dockTabBars[i]->addCallbackTabChanged(wz_main_window_dock_tab_bar_tab_changed);
	}

	// Create menu bar.
	menuBar = new MenuBar;
	setMenuBar(menuBar);
}

void MainWindow::createMenuButton(const std::string &label)
{
	MenuBarButton *button = menuBar->createButton();
	button->setLabel(label.c_str());
}

void MainWindow::onRectChanged()
{
	// Match the menu bar width to the main window width.
	if (menuBar)
	{
		menuBar->setWidth(rect.w);
	}

	updateDockIconPositions();
	updateDockingRects();
	updateContentRect();
}

bool MainWindow::isShiftKeyDown() const
{
	return isShiftKeyDown_;
}

bool MainWindow::isControlKeyDown() const
{
	return isControlKeyDown_;
}

void MainWindow::setEventCallback(EventCallback callback)
{
	handle_event = callback;
}

static void MouseButtonDownRecursive(Widget *widget, int mouseButton, int mouseX, int mouseY)
{
	WZ_ASSERT(widget);

	if (!widget->getVisible())
		return;

	widget->onMouseButtonDown(mouseButton, mouseX, mouseY);

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		if (widget->children[i]->hover)
		{
			MouseButtonDownRecursive(widget->children[i], mouseButton, mouseX, mouseY);
		}
	}
}

void MainWindow::mouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	// Clear keyboard focus widget.
	keyboardFocusWidget = NULL;

	lockInputWindow = getHoverWindow(mouseX, mouseY);
	Widget *widget = this;

	if (!mainWindow->lockInputWidgetStack.empty())
	{
		// Lock input to the top/last item on the stack.
		widget = lockInputWidgetStack.back();
	}
	else if (mainWindow->lockInputWindow)
	{
		updateWindowDrawPriorities(lockInputWindow);
		widget = lockInputWindow;
	}

	MouseButtonDownRecursive(widget, mouseButton, mouseX, mouseY);

	// Need a special case for dock icons.
	updateDockPreviewVisible(mouseX, mouseY);
}

static void MouseButtonUpRecursive(Widget *widget, int mouseButton, int mouseX, int mouseY)
{
	WZ_ASSERT(widget);

	if (!widget->getVisible())
		return;

	widget->onMouseButtonUp(mouseButton, mouseX, mouseY);

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		MouseButtonUpRecursive(widget->children[i], mouseButton, mouseX, mouseY);
	}
}

void MainWindow::mouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	// Need a special case for dock icons.
	if (movingWindow)
	{
		// If the dock preview is visible, movingWindow can be docked.
		if (dockPreview->getVisible())
		{
			dockWindow(movingWindow, windowDockPosition);
		}

		dockPreview->setVisible(false);
		movingWindow = NULL;
	}

	Widget *widget = this;

	if (!lockInputWidgetStack.empty())
	{
		// Lock input to the top/last item on the stack.
		widget = lockInputWidgetStack.back();
	}
	else if (lockInputWindow)
	{
		widget = lockInputWindow;
	}

	MouseButtonUpRecursive(widget, mouseButton, mouseX, mouseY);
}

// Clear widget hover on everything but ignoreWindow and it's children.
static void ClearHoverRecursive(Window *ignoreWindow, Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget == ignoreWindow)
		return;

	if (widget->hover)
	{
		// Stop hovering.
		widget->hover = false;
		widget->onMouseHoverOff();
	}

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		ClearHoverRecursive(ignoreWindow, widget->children[i]);
	}
}

// Sets Widget.ignore
static void IgnoreOverlappingChildren(Widget *widget, int mouseX, int mouseY)
{
	WZ_ASSERT(widget);

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		widget->children[i]->ignore = false;
	}

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		for (size_t j = 0; j < widget->children.size(); j++)
		{
			Rect rect1, rect2, intersection;

			if (i == j)
				continue;

			if (!widget->children[j]->getVisible())
				continue;

			// If the mouse cursor is in the intersection of the two widget rects.
			rect1 = widget->children[i]->getAbsoluteRect();
			rect2 = widget->children[j]->getAbsoluteRect();

			if (Rect::intersect(rect1, rect2, &intersection) && WZ_POINT_IN_RECT(mouseX, mouseY, intersection))
			{
				// Ignore the one that isn't set to overlap.
				if (widget->children[i]->overlap)
				{
					widget->children[j]->ignore = true;
				}
				else if (widget->children[j]->overlap)
				{
					widget->children[i]->ignore = true;
				}
			}
		}
	}
}

// If window is not NULL, only call onMouseMove in widgets that are children of the window and the window itself.
static void MouseMoveRecursive(Window *window, Widget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	Rect rect;
	bool hoverWindow;
	bool hoverParent;
	bool widgetIsChildOfWindow;
	bool oldHover;

	WZ_ASSERT(widget);

	if (!widget->getVisible())
		return;

	// Don't process mouse move if the widget is ignored.
	if (widget->ignore)
	{
		if (widget->hover)
		{
			// Stop hovering.
			widget->hover = false;
			widget->onMouseHoverOff();
		}

		return;
	}

	// Determine whether the mouse is hovering over the widget's parent window.
	// Special case for combo dropdown list poking outside the window.
	if (widget->window && !(widget->parent && widget->parent->type == WZ_TYPE_COMBO))
	{
		hoverWindow = WZ_POINT_IN_RECT(mouseX, mouseY, widget->window->getContentWidget()->getAbsoluteRect());
	}
	else
	{
		hoverWindow = true;
	}

	// Determine whether the mouse is hovering over the widget's parent.
	if (!widget->inputNotClippedToParent && widget->parent && widget->parent != widget->mainWindow && widget->parent != widget->window)
	{
		hoverParent = WZ_POINT_IN_RECT(mouseX, mouseY, widget->parent->getAbsoluteRect());
	}
	else
	{
		hoverParent = true;
	}

	// Or the window itself.
	widgetIsChildOfWindow = !window || (window && (widget->window == window || widget == window));

	// Set widget hover.
	oldHover = widget->hover;
	rect = widget->getAbsoluteRect();
	widget->hover = widgetIsChildOfWindow && hoverWindow && hoverParent && WZ_POINT_IN_RECT(mouseX, mouseY, rect);

	// Run callbacks if hover has changed.
	if (!oldHover && widget->hover)
	{
		widget->onMouseHoverOn();
	}
	else if (oldHover && !widget->hover)
	{
		widget->onMouseHoverOff();
	}

	// Run mouse move if the mouse is hovering over the widget, or if input is locked to the widget.
	if (widget->hover || (widgetIsChildOfWindow && !widget->mainWindow->lockInputWidgetStack.empty() && widget == widget->mainWindow->lockInputWidgetStack.back()))
	{
		widget->onMouseMove(mouseX, mouseY, mouseDeltaX, mouseDeltaY);
	}

	IgnoreOverlappingChildren(widget, mouseX, mouseY);

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		MouseMoveRecursive(window, widget->children[i], mouseX, mouseY, mouseDeltaX, mouseDeltaY);
	}
}

void MainWindow::mouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	// Reset the mouse cursor to default.
	cursor = WZ_CURSOR_DEFAULT;

	// Need a special case for dock icons.
	updateDockPreviewVisible(mouseX, mouseY);

	if (!lockInputWidgetStack.empty())
	{
		// Lock input to the top/last item on the stack.
		MouseMoveRecursive(NULL, lockInputWidgetStack.back(), mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		return;
	}

	lockInputWindow = getHoverWindow(mouseX, mouseY);

	// Clear hover on everything but the lockInputWindow and it's children.
	ClearHoverRecursive(lockInputWindow, this);

	MouseMoveRecursive(lockInputWindow, this, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
}

static void MouseWheelMoveRecursive(Widget *widget, int x, int y)
{
	WZ_ASSERT(widget);

	if (!widget->getVisible())
		return;

	widget->onMouseWheelMove(x, y);

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		if (widget->children[i]->hover)
		{
			MouseWheelMoveRecursive(widget->children[i], x, y);
		}
	}
}

void MainWindow::mouseWheelMove(int x, int y)
{
	Widget *widget = this;

	if (!lockInputWidgetStack.empty())
	{
		// Lock input to the top/last item on the stack.
		widget = lockInputWidgetStack.back();
	}
	else if (lockInputWindow)
	{
		widget = lockInputWindow;
	}

	MouseWheelMoveRecursive(widget, x, y);
}

void MainWindow::keyDelta(Key key, bool down)
{
	Widget *widget = keyboardFocusWidget;

	if (!widget || !widget->getVisible())
		return;

	if (down)
	{
		widget->onKeyDown(key);
	}
	else if (!down)
	{
		widget->onKeyUp(key);
	}
}

void MainWindow::keyDown(Key key)
{
	if (WZ_KEY_MOD_OFF(key) == WZ_KEY_UNKNOWN)
		return;

	if (WZ_KEY_MOD_OFF(key) == WZ_KEY_LSHIFT || WZ_KEY_MOD_OFF(key) == WZ_KEY_RSHIFT)
	{
		isShiftKeyDown_ = true;
	}
	else if (WZ_KEY_MOD_OFF(key) == WZ_KEY_LCONTROL || WZ_KEY_MOD_OFF(key) == WZ_KEY_RCONTROL)
	{
		isControlKeyDown_ = true;
	}

	keyDelta(key, true);
}

void MainWindow::keyUp(Key key)
{
	if (WZ_KEY_MOD_OFF(key) == WZ_KEY_UNKNOWN)
		return;

	if (WZ_KEY_MOD_OFF(key) == WZ_KEY_LSHIFT || WZ_KEY_MOD_OFF(key) == WZ_KEY_RSHIFT)
	{
		isShiftKeyDown_ = false;
	}
	else if (WZ_KEY_MOD_OFF(key) == WZ_KEY_LCONTROL || WZ_KEY_MOD_OFF(key) == WZ_KEY_RCONTROL)
	{
		isControlKeyDown_ = false;
	}

	keyDelta(key, false);
}

void MainWindow::textInput(const char *text)
{
	Widget *widget = keyboardFocusWidget;

	if (!widget || !widget->getVisible())
		return;

	widget->onTextInput(text);
}

void MainWindow::draw()
{
	// Draw the main window (not really) and ancestors. Don't recurse into windows or combos.
	wz_widget_draw(this, wz_widget_true, wz_widget_is_not_window_or_combo);

	// Get a list of windows (excluding top).
	Window *windows[WZ_MAX_WINDOWS];
	int nWindows = 0;

	for (size_t i = 0; i < children.size(); i++)
	{
		if (children[i]->type == WZ_TYPE_WINDOW)
		{
			windows[nWindows] = (Window *)children[i];
			nWindows++;
		}
	}

	// Sort them in ascending order by draw priority.
	qsort(windows, nWindows, sizeof(Window *), wz_compare_window_draw_priorities_docked);

	// For each window, draw the window and all ancestors. Don't recurse into combos.
	for (int i = 0; i < nWindows; i++)
	{
		Widget *widget = windows[i];

		if (!widget->getVisible())
			continue;

		widget->draw(rect);
		wz_widget_draw(widget, wz_widget_true, wz_widget_is_not_combo);
	}

	// Draw combo box dropdown lists.
	wz_widget_draw(this, wz_widget_is_combo_ancestor, wz_widget_true);

	// Draw dock preview.
	wz_widget_draw_if_visible(dockPreview);

	// Draw dock icons.
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wz_widget_draw_if_visible(dockIcons[i]);
	}
}

void MainWindow::drawFrame()
{
	NVGRenderer *r = (NVGRenderer *)renderer;
	NVGcontext *vg = r->getContext();
	nvgBeginFrame(vg, rect.w, rect.h, 1);
	draw();
	nvgEndFrame(vg);
}

void MainWindow::setMenuBar(MenuBar *menuBar)
{
	this->menuBar = menuBar;
	menuBar->setWidth(mainWindow->rect.w);
	addChildWidget(menuBar);
}

void MainWindow::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW)
		return;

	// Special case for windows: add directly, not to the content widget.
	if (widget->type == WZ_TYPE_WINDOW)
	{
		addChildWidget(widget);
	}
	else
	{
		content->addChildWidget(widget);
	}
}

void MainWindow::remove(Widget *widget)
{
	WZ_ASSERT(widget);

	// Special case for windows: remove directly, not from the content widget.
	if (widget->type == WZ_TYPE_WINDOW)
	{
		removeChildWidget(widget);
	}
	else
	{
		content->removeChildWidget(widget);
	}
}

bool MainWindow::isTextCursorVisible() const
{
	return isTextCursorVisible_;
}

void MainWindow::toggleTextCursor()
{
	isTextCursorVisible_ = !isTextCursorVisible_;
}

void MainWindow::setCursor(Cursor cursor)
{
	this->cursor = cursor;
}

Cursor MainWindow::getCursor() const
{
	return cursor;
}

const Widget *MainWindow::getKeyboardFocusWidget() const
{
	return keyboardFocusWidget;
}

void MainWindow::setKeyboardFocusWidget(Widget *widget)
{
	keyboardFocusWidget = widget;
}

DockPosition MainWindow::getWindowDockPosition(const Window *window) const
{
	WZ_ASSERT(window);

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		for (size_t j = 0; j < dockedWindows[i].size(); j++)
		{
			if (dockedWindows[i][j] == window)
			{
				return (DockPosition)i;
			}
		}
	}

	return WZ_DOCK_POSITION_NONE;
}

void MainWindow::dockWindow(Window *window, DockPosition dockPosition)
{
	WZ_ASSERT(window);

	// Not valid, use undockWindow to undock.
	if (dockPosition == WZ_DOCK_POSITION_NONE)
		return;

	// Don't do anything if this window is already docked at this position.
	for (size_t i = 0; i < dockedWindows[dockPosition].size(); i++)
	{
		if (dockedWindows[dockPosition][i] == window)
			return;
	}

	// Hide any other windows docked at the same position.
	for (size_t i = 0; i < dockedWindows[dockPosition].size(); i++)
	{
		dockedWindows[dockPosition][i]->setVisible(false);
	}

	// Inform the window it is being docked.
	window->dock();

	// Resize the window.
	window->setRectInternal(calculateDockWindowRect(dockPosition, window->getSize()));

	// Dock the window.
	dockedWindows[dockPosition].push_back(window);

	// Resize the other windows docked at this position to match.
	updateDockedWindowRect(window);

	// Refresh the tab bar for this dock position.
	ignoreDockTabBarChangedEvent = true;
	refreshDockTabBar(dockPosition);
	ignoreDockTabBarChangedEvent = false;

	// Docked windows affect the mainWindow content rect, so update it.
	updateContentRect();
}

void MainWindow::undockWindow(Window *window)
{
	WZ_ASSERT(window);

	// Find the dock position for the window, and the window index.
	DockPosition dockPosition = WZ_DOCK_POSITION_NONE;
	int windowIndex = -1;

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		for (size_t j = 0; j < dockedWindows[i].size(); j++)
		{
			if (dockedWindows[i][j] == window)
			{
				dockPosition = (DockPosition)i;
				windowIndex = j;
				break;
			}
		}

		if (dockPosition != WZ_DOCK_POSITION_NONE)
			break;
	}

	if (windowIndex == -1)
		return;

	dockedWindows[dockPosition].erase(dockedWindows[dockPosition].begin() + windowIndex);
	int nDockedWindows = dockedWindows[dockPosition].size();

	// If there are other windows docked at this position, make sure one is visible after removing this window.
	if (window->getVisible() && nDockedWindows > 0)
	{
		dockedWindows[dockPosition][0]->setVisible(true);
	}

	// Refresh the tab bar for this dock position.
	refreshDockTabBar(dockPosition);

	// If the dock tab bar is hidden, resize the windows at this dock position to reclaim the space it used.
	if (!dockTabBars[dockPosition]->getVisible())
	{
		for (size_t j = 0; j < dockedWindows[dockPosition].size(); j++)
		{
			Widget *widget = dockedWindows[dockPosition][j];

			if (dockPosition == WZ_DOCK_POSITION_SOUTH)
			{
				widget->setHeightInternal(widget->rect.h + (rect.h - (widget->rect.y + widget->rect.h)));
			}
			else if (dockPosition == WZ_DOCK_POSITION_EAST || dockPosition == WZ_DOCK_POSITION_WEST)
			{
				widget->setHeightInternal(rect.h);
			}
		}
	}
}

void MainWindow::updateDockedWindowRect(Window *window)
{
	WZ_ASSERT(window);
	Rect rect = window->getRect();

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		for (size_t j = 0; j < dockedWindows[i].size(); j++)
		{
			if (dockedWindows[i][j] == window)
			{
				for (size_t k = 0; k < dockedWindows[i].size(); k++)
				{
					if (j == k)
						continue;

					dockedWindows[i][k]->setRectInternal(rect);
				}

				// Update the tab bar too.
				dockTabBars[i]->setRectInternal(rect.x, rect.y + rect.h, rect.w, dockTabBars[i]->getHeight());
				return;
			}
		}
	}
}

void MainWindow::pushLockInputWidget(Widget *widget)
{
	lockInputWidgetStack.push_back(widget);
}

void MainWindow::popLockInputWidget(Widget *widget)
{
	// Only pop if the widget is on the top of the stack.
	if (lockInputWidgetStack.empty())
		return;

	if (widget == lockInputWidgetStack.back())
	{	
		lockInputWidgetStack.pop_back();
	}
}

void MainWindow::setMovingWindow(Window *window)
{
	movingWindow = window;

	// Show the dock icons if movingWindow is not NULL.
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		dockIcons[i]->setVisible(movingWindow != NULL);
	}
}

void wz_invoke_event(Event *e)
{
	WZ_ASSERT(e);

	for (size_t i = 0; i < e->base.widget->eventHandlers.size(); i++)
	{
		if (e->base.widget->eventHandlers[i]->eventType == e->base.type)
		{
			e->base.widget->eventHandlers[i]->call(e);
		}
	}

	if (e->base.widget->mainWindow && e->base.widget->mainWindow->handle_event)
	{
		e->base.widget->mainWindow->handle_event(e);
	}
}

void wz_invoke_event(Event *e, const std::vector<EventCallback> &callbacks)
{
	WZ_ASSERT(e);
	wz_invoke_event(e);

	for (size_t i = 0; i < callbacks.size(); i++)
	{
		callbacks[i](e);
	}
}

void MainWindow::updateContentRect()
{
	Rect rect = this->rect;

	// Adjust the content rect based on the menu bar height.
	if (menuBar)
	{
		const int h = menuBar->getHeight();
		rect.y += h;
		rect.h -= h;
	}

	// Adjust the content rect based on docked windows.
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		Rect windowRect;

		if (dockedWindows[i].empty())
			continue;

		windowRect = dockedWindows[i][0]->getRect();

		switch ((DockPosition)i)
		{
		case WZ_DOCK_POSITION_NORTH:
			rect.y += windowRect.h;
			rect.h -= windowRect.h;
			break;
		case WZ_DOCK_POSITION_SOUTH:
			rect.h -= windowRect.h;
			break;
		case WZ_DOCK_POSITION_EAST:
			rect.w -= windowRect.w;
			break;
		case WZ_DOCK_POSITION_WEST:
			rect.x += windowRect.w;
			rect.w -= windowRect.w;
			break;
		}
	}

	content->setRectInternal(rect);
}

Window *MainWindow::getHoverWindow(int mouseX, int mouseY)
{
	Window *result = NULL;
	bool resultIsDocked = false;
	int drawPriority = -1;

	for (size_t i = 0; i < children.size(); i++)
	{
		Widget *widget;
		Window *window;
		bool docked;

		widget = children[i];

		if (widget->type != WZ_TYPE_WINDOW)
			continue;

		if (!widget->getVisible() || !WZ_POINT_IN_RECT(mouseX, mouseY, widget->rect))
			continue;

		window = (Window *)widget;
		docked = getWindowDockPosition(window) != WZ_DOCK_POSITION_NONE;

		// Undocked always takes priority over docked.
		if (window->getDrawPriority() >= drawPriority || (resultIsDocked && !docked))
		{
			drawPriority = ((Window *)widget)->getDrawPriority();
			resultIsDocked = docked;
			result = window;
		}
	}

	return result;
}

void MainWindow::refreshDockTabBar(DockPosition dockPosition)
{
	TabBar *tabBar = dockTabBars[dockPosition];

	if (dockedWindows[dockPosition].size() < 2)
	{
		// Hide the tab bar.
		tabBar->setVisible(false);
	}
	else
	{
		Rect windowRect;
		int tabBarHeight;

		// Clear the tabs.
		tabBar->clearTabs();

		// Resize the tab bar to match the window(s) at this dock position. Just use the first window, doesn't matter which.
		windowRect = dockedWindows[dockPosition][0]->getRect();
		tabBarHeight = tabBar->getHeight();

		// Assume space has already been made for the tab bar below the window.
		tabBar->setRectInternal(windowRect.x, windowRect.y + windowRect.h, windowRect.w, tabBarHeight);

		// Add one tab for each window.
		for (size_t i = 0; i < dockedWindows[dockPosition].size(); i++)
		{
			Window *window = dockedWindows[dockPosition][i];

			// Create a new tab.
			TabButton *tab = tabBar->createTab();
			tab->setLabel(window->getTitle());

			// Set the tab internal metadata to the window.
			tab->setInternalMetadata(window);

			// If this window is selected (visible), select the corresponding tab.
			if (window->getVisible())
			{
				tabBar->selectTab(tab);
			}
		}

		// Show the tab bar.
		tabBar->setVisible(true);
	}
}

// Update the rects of docked windows and dock tab bars.
void MainWindow::updateDockingRects()
{
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		int nWindows = dockedWindows[i].size();

		if (nWindows == 0)
			continue;

		// Calculate the window rect. All windows will have the same rect, so just use the first one as a basis.
		Rect windowRect = dockedWindows[i][0]->getRect();
		int menuBarHeight = menuBar ? menuBar->getHeight() : 0;

		switch (i)
		{
		case WZ_DOCK_POSITION_NORTH:
			windowRect.w = rect.w;
			break;
		case WZ_DOCK_POSITION_SOUTH:
			windowRect.y = rect.h - windowRect.h;
			windowRect.w = rect.w;
			break;
		case WZ_DOCK_POSITION_EAST:
			windowRect.x = rect.w - windowRect.w;
			windowRect.h = rect.h - menuBarHeight;
			break;
		case WZ_DOCK_POSITION_WEST:
			windowRect.h = rect.h - menuBarHeight;
			break;
		}

		// Update tab bar rect. Adjust window rect to make space for the tab bar.
		if (nWindows > 1)
		{
			Rect tabBarRect = dockTabBars[i]->getRect();

			switch (i)
			{
			case WZ_DOCK_POSITION_NORTH:
				tabBarRect.w = rect.w;
				break;
			case WZ_DOCK_POSITION_SOUTH:
				tabBarRect.y = rect.h - tabBarRect.h;
				tabBarRect.w = rect.w;
				windowRect.h -= tabBarRect.h;
				break;
			case WZ_DOCK_POSITION_EAST:
				tabBarRect.x = rect.w - windowRect.w;
				tabBarRect.y = rect.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			case WZ_DOCK_POSITION_WEST:
				tabBarRect.y = rect.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			}

			dockTabBars[i]->setRectInternal(tabBarRect);
		}

		// Set window rects.
		for (size_t j = 0; j < dockedWindows[i].size(); j++)
		{
			dockedWindows[i][j]->setRectInternal(windowRect);
		}
	}
}

Rect MainWindow::calculateDockWindowRect(DockPosition dockPosition, Size windowSize)
{
	// e.g. north dock max height is main window height * maxPreviewSizeMultiplier.
	const float maxPreviewSizeMultiplier = 0.3f;

	int menuBarHeight = menuBar ? menuBar->getHeight() : 0;

	// If there's already a window docked at this position, set the dock preview rect to that size.
	int nDockedWindows = dockedWindows[dockPosition].size();
	Rect rect;

	if (nDockedWindows > 0)
	{
		rect = dockedWindows[dockPosition][0]->getRect();

		// If there's exactly one window already docked at this position, leave room for the dock tab bar.
		if (nDockedWindows == 1)
		{
			rect.h -= dockTabBars[dockPosition]->getHeight();
		}
	}
	else
	{
		// Use the window width for east/west or height for north/south, but don't go over half of the main window width/height.
		if (dockPosition == WZ_DOCK_POSITION_NORTH)
		{
			rect.x = 0;
			rect.y = menuBarHeight;
			rect.w = this->rect.w;
			rect.h = WZ_MIN(windowSize.h, (int)(this->rect.h * maxPreviewSizeMultiplier)) - menuBarHeight;
		}
		else if (dockPosition == WZ_DOCK_POSITION_SOUTH)
		{
			const int h = WZ_MIN(windowSize.h, (int)(this->rect.h * maxPreviewSizeMultiplier));
			rect.x = 0;
			rect.y = this->rect.h - h;
			rect.w = this->rect.w;
			rect.h = h;
		}
		else if (dockPosition == WZ_DOCK_POSITION_EAST)
		{
			const int w = WZ_MIN(windowSize.w, (int)(this->rect.w * maxPreviewSizeMultiplier));
			rect.x = this->rect.w - w;
			rect.y = menuBarHeight;
			rect.w = w;
			rect.h = this->rect.h - menuBarHeight;
		}
		else if (dockPosition == WZ_DOCK_POSITION_WEST)
		{
			rect.x = 0;
			rect.y = menuBarHeight;
			rect.w = WZ_MIN(windowSize.w, (int)(this->rect.w * maxPreviewSizeMultiplier));
			rect.h = this->rect.h - menuBarHeight;
		}
	}

	return rect;
}

void MainWindow::updateDockIconPositions()
{
	// Push icons out this percent/100 from mainWindow edges.
	const float percent = 0.04f;

	Size ds = getSize();
	Size dis = dockIcons[WZ_DOCK_POSITION_NORTH]->getSize();
	int centerW = (int)(ds.w / 2.0f - dis.w / 2.0f);
	int centerH = (int)(ds.h / 2.0f - dis.h / 2.0f);

	dockIcons[WZ_DOCK_POSITION_NORTH]->setPositionInternal(centerW, (int)(ds.h * percent));
	dockIcons[WZ_DOCK_POSITION_SOUTH]->setPositionInternal(centerW, (int)(ds.h * (1.0f - percent) - dis.h));
	dockIcons[WZ_DOCK_POSITION_EAST]->setPositionInternal((int)(ds.w * (1.0f - percent) - dis.h), centerH);
	dockIcons[WZ_DOCK_POSITION_WEST]->setPositionInternal((int)(ds.w * percent), centerH);
}

void MainWindow::updateDockPreviewRect(DockPosition dockPosition)
{
	WZ_ASSERT(movingWindow);
	const Size windowSize = movingWindow->getSize();
	const Rect rect = calculateDockWindowRect(dockPosition, windowSize);
	dockPreview->setRectInternal(rect);
}

void MainWindow::updateDockPreviewVisible(int mouseX, int mouseY)
{
	if (!movingWindow)
		return;

	bool showDockPreview = false;

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		const Rect rect = dockIcons[i]->getRect();

		if (WZ_POINT_IN_RECT(mouseX, mouseY, rect))
		{
			windowDockPosition = (DockPosition)i;
			updateDockPreviewRect((DockPosition)i);
			showDockPreview = true;
			break;
		}
	}

	dockPreview->setVisible(showDockPreview);
}

static int CompareWindowDrawPriorities(const void *a, const void *b)
{
	return ((const Window *)a)->getDrawPriority() - ((const Window *)b)->getDrawPriority();
}

void MainWindow::updateWindowDrawPriorities(Window *top)
{
	// Get a list of windows (excluding top).
	int nWindows = 0;
	Window *windows[WZ_MAX_WINDOWS];

	for (size_t i = 0; i < (int)children.size(); i++)
	{
		if (children[i]->type == WZ_TYPE_WINDOW && children[i] != top)
		{
			windows[nWindows] = (Window *)children[i];
			nWindows++;
		}
	}

	// Sort them in ascending order by draw priority.
	qsort(windows, nWindows, sizeof(Window *), CompareWindowDrawPriorities);

	// Assign each window a new draw priority, starting at 0 and ascending by 1.
	int i;

	for (i = 0; i < nWindows; i++)
	{
		windows[i]->setDrawPriority(i);
	}

	// Give the top window the highest priority.
	if (top)
	{
		top->setDrawPriority(i);
	}
}

} // namespace wz
