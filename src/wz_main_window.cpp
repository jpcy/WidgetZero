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

class WindowTabButton : public TabButton
{
public:
	WindowTabButton(Window *window) : TabButton(window->getTitle())
	{
		this->window = window;
	}

	Window *window;
};

DockIcon::DockIcon()
{
	type_ = WidgetType::DockIcon;
	setSize(48, 48);
}

void DockIcon::draw(Rect clip)
{
	renderer_->drawDockIcon(this, clip);
}

void DockPreview::draw(Rect clip)
{
	renderer_->drawDockPreview(this, clip);
}

MainWindow::MainWindow(IRenderer *renderer, MainWindowFlags::Enum flags)
{
	type_ = WidgetType::MainWindow;
	cursor_ = Cursor::Default;
	isShiftKeyDown_ = isControlKeyDown_ = false;
	lockInputWindow_ = NULL;
	keyboardFocusWidget_ = NULL;
	movingWindow_ = NULL;
	windowDockPosition_ = DockPosition::None;
	ignoreDockTabBarChangedEvent_ = false;
	menuBar_ = NULL;
	renderer_ = renderer;
	flags_ = flags | MainWindowFlags::AnyWidgetMeasureDirty | MainWindowFlags::AnyWidgetRectDirty;
	mainWindow_ = this;
	isTextCursorVisible_ = true;

	// Create content widget.
	content_ = new Widget;
	content_->mainWindow_ = this;
	addChildWidget(content_);

	if (isDockingEnabled())
	{
		// Create dock icon widgets.
		for (int i = 0; i < DockPosition::NumDockPositions; i++)
		{
			dockIcons_[i] = new DockIcon;
			dockIcons_[i]->setVisible(false);
			dockIcons_[i]->setDrawManually(true);
			addChildWidget(dockIcons_[i]);
		}

		// Create dock preview widget.
		dockPreview_ = new DockPreview;
		dockPreview_->setDrawManually(true);
		dockPreview_->setVisible(false);
		addChildWidget(dockPreview_);

		// Create dock tab bars.
		for (int i = 0; i < DockPosition::NumDockPositions; i++)
		{
			dockTabBars_[i] = new TabBar;
			dockTabBars_[i]->setVisible(false);
			addChildWidget(dockTabBars_[i]);
			dockTabBars_[i]->addEventHandler(EventType::TabBarTabChanged, this, &MainWindow::onDockTabBarTabChanged);
		}
	}

	if (isMenuEnabled())
	{
		// Create menu bar.
		menuBar_ = new MenuBar;
		menuBar_->setStretch(Stretch::Width);
		addChildWidget(menuBar_);
	}
}

bool MainWindow::isDockingEnabled() const
{
	return (flags_ & MainWindowFlags::DockingEnabled) == MainWindowFlags::DockingEnabled;
}

bool MainWindow::isMenuEnabled() const
{
	return (flags_ & MainWindowFlags::MenuEnabled) == MainWindowFlags::MenuEnabled;
}

void MainWindow::createMenuButton(const std::string &label)
{
	if (!isMenuEnabled())
	{
		WZ_ASSERT("menu not enabled" && false);
		return;
	}

	MenuBarButton *button = menuBar_->createButton();
	button->setLabel(label.c_str());
}

bool MainWindow::isShiftKeyDown() const
{
	return isShiftKeyDown_;
}

bool MainWindow::isControlKeyDown() const
{
	return isControlKeyDown_;
}

void MainWindow::mouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	doMeasureAndLayoutPasses();

	// Clear keyboard focus widget.
	keyboardFocusWidget_ = NULL;

	lockInputWindow_ = getHoverWindow(mouseX, mouseY);
	Widget *widget = this;

	if (!lockInputWidgetStack_.empty())
	{
		// Lock input to the top/last item on the stack.
		widget = lockInputWidgetStack_.back();
	}
	else if (lockInputWindow_)
	{
		updateWindowDrawPriorities(lockInputWindow_);
		widget = lockInputWindow_;
	}

	mouseButtonDownRecursive(widget, mouseButton, mouseX, mouseY);

	// Need a special case for dock icons.
	updateDockPreviewVisible(mouseX, mouseY);
}

void MainWindow::mouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	doMeasureAndLayoutPasses();

	// Need a special case for dock icons.
	if (isDockingEnabled() && movingWindow_)
	{
		// If the dock preview is visible, movingWindow can be docked.
		if (dockPreview_->isVisible())
		{
			dockWindow(movingWindow_, windowDockPosition_);
		}

		dockPreview_->setVisible(false);
		movingWindow_ = NULL;
	}

	Widget *widget = this;

	if (!lockInputWidgetStack_.empty())
	{
		// Lock input to the top/last item on the stack.
		widget = lockInputWidgetStack_.back();
	}
	else if (lockInputWindow_)
	{
		widget = lockInputWindow_;
	}

	mouseButtonUpRecursive(widget, mouseButton, mouseX, mouseY);
}

void MainWindow::mouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	doMeasureAndLayoutPasses();

	// Reset the mouse cursor to default.
	cursor_ = Cursor::Default;

	// Need a special case for dock icons.
	updateDockPreviewVisible(mouseX, mouseY);

	if (!lockInputWidgetStack_.empty())
	{
		// Lock input to the top/last item on the stack.
		mouseMoveRecursive(NULL, lockInputWidgetStack_.back(), mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		return;
	}

	lockInputWindow_ = getHoverWindow(mouseX, mouseY);

	// Clear hover on everything but the lockInputWindow and it's children.
	clearHoverRecursive(lockInputWindow_, this);

	mouseMoveRecursive(lockInputWindow_, this, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
}

void MainWindow::mouseWheelMove(int x, int y)
{
	doMeasureAndLayoutPasses();

	Widget *widget = this;

	if (!lockInputWidgetStack_.empty())
	{
		// Lock input to the top/last item on the stack.
		widget = lockInputWidgetStack_.back();
	}
	else if (lockInputWindow_)
	{
		widget = lockInputWindow_;
	}

	mouseWheelMoveRecursive(widget, x, y);
}

void MainWindow::keyDelta(Key::Enum key, bool down)
{
	doMeasureAndLayoutPasses();

	Widget *widget = keyboardFocusWidget_;

	if (!widget || !widget->isVisible())
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

void MainWindow::keyDown(Key::Enum key)
{
	if (WZ_KEY_MOD_OFF(key) == Key::Unknown)
		return;

	doMeasureAndLayoutPasses();

	if (WZ_KEY_MOD_OFF(key) == Key::LeftShift || WZ_KEY_MOD_OFF(key) == Key::RightShift)
	{
		isShiftKeyDown_ = true;
	}
	else if (WZ_KEY_MOD_OFF(key) == Key::LeftControl || WZ_KEY_MOD_OFF(key) == Key::RightControl)
	{
		isControlKeyDown_ = true;
	}

	keyDelta(key, true);
}

void MainWindow::keyUp(Key::Enum key)
{
	if (WZ_KEY_MOD_OFF(key) == Key::Unknown)
		return;

	doMeasureAndLayoutPasses();

	if (WZ_KEY_MOD_OFF(key) == Key::LeftShift || WZ_KEY_MOD_OFF(key) == Key::RightShift)
	{
		isShiftKeyDown_ = false;
	}
	else if (WZ_KEY_MOD_OFF(key) == Key::LeftControl || WZ_KEY_MOD_OFF(key) == Key::RightControl)
	{
		isControlKeyDown_ = false;
	}

	keyDelta(key, false);
}

void MainWindow::textInput(const char *text)
{
	Widget *widget = keyboardFocusWidget_;

	if (!widget || !widget->isVisible())
		return;

	doMeasureAndLayoutPasses();
	widget->onTextInput(text);
}

static int compare_window_draw_priorities_docked(const void *a, const void *b)
{
	const Window *window1, *window2;
	bool window1Docked, window2Docked;

	window1 = *((const Window **)a);
	window2 = *((const Window **)b);
	window1Docked = window1->getMainWindow()->getWindowDockPosition(window1) != DockPosition::None;
	window2Docked = window2->getMainWindow()->getWindowDockPosition(window2) != DockPosition::None;

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

static bool IsWidgetTrue(const Widget * /*widget*/)
{
	return true;
}

static bool IsWidgetComboAncestor(const Widget *widget)
{
	return widget->getType() != WidgetType::Combo && widget->findClosestAncestor(WidgetType::Combo) != NULL;
}

static bool IsWidgetNotCombo(const Widget *widget)
{
	return widget->getType() != WidgetType::Combo;
}

static bool IsWidgetNotWindowOrCombo(const Widget *widget)
{
	return widget->getType() != WidgetType::Window && widget->getType() != WidgetType::Combo;
}

void MainWindow::draw()
{
	doMeasureAndLayoutPasses();

	// Draw the main window (not really) and ancestors. Don't recurse into windows or combos.
	drawWidget(this, IsWidgetTrue, IsWidgetNotWindowOrCombo);

	// Get a list of windows (excluding top).
	Window *windows[WZ_MAX_WINDOWS];
	int nWindows = 0;

	for (size_t i = 0; i < children_.size(); i++)
	{
		if (children_[i]->getType() == WidgetType::Window)
		{
			windows[nWindows] = (Window *)children_[i];
			nWindows++;
		}
	}

	// Sort them in ascending order by draw priority.
	qsort(windows, nWindows, sizeof(Window *), compare_window_draw_priorities_docked);

	// For each window, draw the window and all ancestors. Don't recurse into combos.
	for (int i = 0; i < nWindows; i++)
	{
		Widget *widget = windows[i];

		if (!widget->isVisible())
			continue;

		widget->draw(rect_);
		drawWidget(widget, IsWidgetTrue, IsWidgetNotCombo);
	}

	// Draw combo box dropdown lists.
	drawWidget(this, IsWidgetComboAncestor, IsWidgetTrue);

	if (isDockingEnabled())
	{
		// Draw dock preview.
		dockPreview_->drawIfVisible();

		// Draw dock icons.
		for (int i = 0; i < DockPosition::NumDockPositions; i++)
		{
			dockIcons_[i]->drawIfVisible();
		}
	}
}

void MainWindow::drawFrame()
{
	renderer_->beginFrame(rect_.w, rect_.h);
	draw();
	renderer_->endFrame();
}

void MainWindow::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->getType() == WidgetType::MainWindow)
		return;

	// Special case for windows: add directly, not to the content widget.
	if (widget->getType() == WidgetType::Window)
	{
		addChildWidget(widget);
	}
	else
	{
		content_->addChildWidget(widget);
	}
}

void MainWindow::remove(Widget *widget)
{
	WZ_ASSERT(widget);

	// Special case for windows: remove directly, not from the content widget.
	if (widget->getType() == WidgetType::Window)
	{
		removeChildWidget(widget);
	}
	else
	{
		content_->removeChildWidget(widget);
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

void MainWindow::setCursor(Cursor::Enum cursor)
{
	cursor_ = cursor;
}

Cursor::Enum MainWindow::getCursor() const
{
	return cursor_;
}

const Widget *MainWindow::getKeyboardFocusWidget() const
{
	return keyboardFocusWidget_;
}

void MainWindow::setKeyboardFocusWidget(Widget *widget)
{
	keyboardFocusWidget_ = widget;
}

DockPosition::Enum MainWindow::getWindowDockPosition(const Window *window) const
{
	WZ_ASSERT(window);
	WZ_ASSERT(isDockingEnabled());

	for (int i = 0; i < DockPosition::NumDockPositions; i++)
	{
		for (size_t j = 0; j < dockedWindows_[i].size(); j++)
		{
			if (dockedWindows_[i][j] == window)
			{
				return (DockPosition::Enum)i;
			}
		}
	}

	return DockPosition::None;
}

void MainWindow::dockWindow(Window *window, DockPosition::Enum dockPosition)
{
	WZ_ASSERT(window);

	// Don't do anything if docking is disabled.
	if (!isDockingEnabled())
		return;

	// Do a layout pass so this mainwindow and the window rects are up to date.
	doMeasureAndLayoutPasses();

	// Not valid, use undockWindow to undock.
	if (dockPosition == DockPosition::None)
		return;

	// Don't do anything if this window is already docked at this position.
	for (size_t i = 0; i < dockedWindows_[dockPosition].size(); i++)
	{
		if (dockedWindows_[dockPosition][i] == window)
			return;
	}

	// Hide any other windows docked at the same position.
	for (size_t i = 0; i < dockedWindows_[dockPosition].size(); i++)
	{
		dockedWindows_[dockPosition][i]->setVisible(false);
	}

	// Inform the window it is being docked.
	window->dock();

	// Resize the window.
	window->setRect(calculateDockWindowRect(dockPosition, window->getSize()));

	// Dock the window.
	dockedWindows_[dockPosition].push_back(window);

	// Resize the other windows docked at this position to match.
	updateDockedWindowRect(window);

	// Refresh the tab bar for this dock position.
	ignoreDockTabBarChangedEvent_ = true;
	refreshDockTabBar(dockPosition);
	ignoreDockTabBarChangedEvent_ = false;

	// Docked windows affect the main window content rect, so update it.
	updateContentRect();
}

void MainWindow::undockWindow(Window *window)
{
	WZ_ASSERT(window);

	// Don't do anything if docking is disabled.
	if (!isDockingEnabled())
		return;

	// Find the dock position for the window, and the window index.
	DockPosition::Enum dockPosition = DockPosition::None;
	int windowIndex = -1;

	for (int i = 0; i < DockPosition::NumDockPositions; i++)
	{
		for (size_t j = 0; j < dockedWindows_[i].size(); j++)
		{
			if (dockedWindows_[i][j] == window)
			{
				dockPosition = (DockPosition::Enum)i;
				windowIndex = j;
				break;
			}
		}

		if (dockPosition != DockPosition::None)
			break;
	}

	if (windowIndex == -1)
		return;

	dockedWindows_[dockPosition].erase(dockedWindows_[dockPosition].begin() + windowIndex);
	int nDockedWindows = dockedWindows_[dockPosition].size();

	// If there are other windows docked at this position, make sure one is visible after removing this window.
	if (window->isVisible() && nDockedWindows > 0)
	{
		dockedWindows_[dockPosition][0]->setVisible(true);
	}

	// Refresh the tab bar for this dock position.
	refreshDockTabBar(dockPosition);

	// If the dock tab bar is hidden, resize the windows at this dock position to reclaim the space it used.
	if (!dockTabBars_[dockPosition]->isVisible())
	{
		for (size_t j = 0; j < dockedWindows_[dockPosition].size(); j++)
		{
			Widget *widget = dockedWindows_[dockPosition][j];

			if (dockPosition == DockPosition::South)
			{
				widget->setHeight(widget->getHeight() + (rect_.h - (widget->getPosition().y + widget->getHeight())));
			}
			else if (dockPosition == DockPosition::East || dockPosition == DockPosition::West)
			{
				widget->setHeight(rect_.h);
			}
		}
	}
}

void MainWindow::updateDockedWindowRect(Window *window)
{
	WZ_ASSERT(window);

	// Don't do anything if docking is disabled.
	if (!isDockingEnabled())
		return;

	Rect rect = window->getRect();

	for (int i = 0; i < DockPosition::NumDockPositions; i++)
	{
		for (size_t j = 0; j < dockedWindows_[i].size(); j++)
		{
			if (dockedWindows_[i][j] == window)
			{
				for (size_t k = 0; k < dockedWindows_[i].size(); k++)
				{
					if (j == k)
						continue;

					dockedWindows_[i][k]->setRect(rect);
				}

				// Update the tab bar too.
				dockTabBars_[i]->setRect(rect.x, rect.y + rect.h, rect.w, dockTabBars_[i]->getHeight());
				return;
			}
		}
	}
}

void MainWindow::pushLockInputWidget(Widget *widget)
{
	lockInputWidgetStack_.push_back(widget);
}

void MainWindow::popLockInputWidget(Widget *widget)
{
	// Only pop if the widget is on the top of the stack.
	if (lockInputWidgetStack_.empty())
		return;

	if (widget == lockInputWidgetStack_.back())
	{	
		lockInputWidgetStack_.pop_back();
	}
}

void MainWindow::setMovingWindow(Window *window)
{
	movingWindow_ = window;

	if (isDockingEnabled())
	{
		// Show the dock icons if movingWindow is not NULL.
		for (int i = 0; i < DockPosition::NumDockPositions; i++)
		{
			dockIcons_[i]->setVisible(movingWindow_ != NULL);
		}
	}
}

void MainWindow::updateContentRect()
{
	Rect rect = rect_;

	// Adjust the content rect based on the menu bar height.
	if (isMenuEnabled())
	{
		const int h = menuBar_->getUserOrMeasuredSize().h;
		rect.y += h;
		rect.h -= h;
	}

	// Adjust the content rect based on docked windows.
	for (int i = 0; i < DockPosition::NumDockPositions; i++)
	{
		Rect windowRect;

		if (dockedWindows_[i].empty())
			continue;

		windowRect = dockedWindows_[i][0]->getRect();

		switch ((DockPosition::Enum)i)
		{
		case DockPosition::North:
			rect.y += windowRect.h;
			rect.h -= windowRect.h;
			break;
		case DockPosition::South:
			rect.h -= windowRect.h;
			break;
		case DockPosition::East:
			rect.w -= windowRect.w;
			break;
		case DockPosition::West:
			rect.x += windowRect.w;
			rect.w -= windowRect.w;
			break;
		}
	}

	content_->setRect(rect);
}

void MainWindow::setAnyWidgetMeasureDirty(bool value)
{
	if (value)
	{
		flags_ = flags_ | MainWindowFlags::AnyWidgetMeasureDirty;
	}
	else
	{
		flags_ = MainWindowFlags::Enum(flags_ & ~MainWindowFlags::AnyWidgetMeasureDirty);
	}
}

void MainWindow::setAnyWidgetRectDirty(bool value)
{
	if (value)
	{
		flags_ = flags_ | MainWindowFlags::AnyWidgetRectDirty;
	}
	else
	{
		flags_ = MainWindowFlags::Enum(flags_ & ~MainWindowFlags::AnyWidgetRectDirty);
	}
}

void MainWindow::onRectChanged()
{
	updateDockIconPositions();
	updateDockingRects();
	updateContentRect();
}

void MainWindow::doMeasureAndLayoutPasses()
{
	if (flags_ & MainWindowFlags::AnyWidgetMeasureDirty)
	{
		debugPrintf("***** BEGIN MEASURE PASS *****");
		doMeasurePassRecursive(isMeasureDirty());
		setAnyWidgetMeasureDirty(false);
		debugPrintf("***** END MEASURE PASS *****");
	}

	if (flags_ & MainWindowFlags::AnyWidgetRectDirty)
	{
		debugPrintf("***** BEGIN LAYOUT PASS *****");
		doLayoutPassRecursive(isRectDirty());
		setAnyWidgetRectDirty(false);
		debugPrintf("***** END LAYOUT PASS *****");
	}
}

void MainWindow::mouseButtonDownRecursive(Widget *widget, int mouseButton, int mouseX, int mouseY)
{
	WZ_ASSERT(widget);

	if (!widget->isVisible())
		return;

	widget->onMouseButtonDown(mouseButton, mouseX, mouseY);

	for (size_t i = 0; i < widget->children_.size(); i++)
	{
		if (widget->children_[i]->getHover())
		{
			mouseButtonDownRecursive(widget->children_[i], mouseButton, mouseX, mouseY);
		}
	}
}

void MainWindow::mouseButtonUpRecursive(Widget *widget, int mouseButton, int mouseX, int mouseY)
{
	WZ_ASSERT(widget);

	if (!widget->isVisible())
		return;

	widget->onMouseButtonUp(mouseButton, mouseX, mouseY);

	for (size_t i = 0; i < widget->children_.size(); i++)
	{
		mouseButtonUpRecursive(widget->children_[i], mouseButton, mouseX, mouseY);
	}
}

void MainWindow::clearHoverRecursive(Window *ignoreWindow, Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget == ignoreWindow)
		return;

	if (widget->getHover())
	{
		// Stop hovering.
		widget->hover_ = false;
		widget->onMouseHoverOff();
	}

	for (size_t i = 0; i < widget->children_.size(); i++)
	{
		clearHoverRecursive(ignoreWindow, widget->children_[i]);
	}
}

void MainWindow::ignoreOverlappingChildren(Widget *widget, int mouseX, int mouseY)
{
	WZ_ASSERT(widget);

	for (size_t i = 0; i < widget->children_.size(); i++)
	{
		widget->children_[i]->ignore_ = false;
	}

	for (size_t i = 0; i < widget->children_.size(); i++)
	{
		for (size_t j = 0; j < widget->children_.size(); j++)
		{
			Rect rect1, rect2, intersection;

			if (i == j)
				continue;

			if (!widget->children_[j]->isVisible())
				continue;

			// If the mouse cursor is in the intersection of the two widget rects.
			rect1 = widget->children_[i]->getAbsoluteRect();
			rect2 = widget->children_[j]->getAbsoluteRect();

			if (Rect::intersect(rect1, rect2, &intersection) && WZ_POINT_IN_RECT(mouseX, mouseY, intersection))
			{
				// Ignore the one that isn't set to overlap.
				if (widget->children_[i]->overlap_)
				{
					widget->children_[j]->ignore_ = true;
				}
				else if (widget->children_[j]->overlap_)
				{
					widget->children_[i]->ignore_ = true;
				}
			}
		}
	}
}

void MainWindow::mouseMoveRecursive(Window *window, Widget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	Rect rect;
	bool hoverWindow;
	bool hoverParent;
	bool widgetIsChildOfWindow;
	bool oldHover;

	WZ_ASSERT(widget);

	if (!widget->isVisible())
		return;

	// Don't process mouse move if the widget is ignored.
	if (widget->ignore_)
	{
		if (widget->getHover())
		{
			// Stop hovering.
			widget->hover_ = false;
			widget->onMouseHoverOff();
		}

		return;
	}

	// Determine whether the mouse is hovering over the widget's parent window.
	// Special case for combo dropdown list poking outside the window.
	if (widget->window_ && widget->findClosestAncestor(WidgetType::Combo) == NULL)
	{
		hoverWindow = WZ_POINT_IN_RECT(mouseX, mouseY, widget->window_->getContentWidget()->getAbsoluteRect());
	}
	else
	{
		hoverWindow = true;
	}

	// Determine whether the mouse is hovering over the widget's parent.
	if (widget->inputClippedToParent_ && widget->parent_ && widget->parent_ != this && widget->parent_ != widget->window_)
	{
		hoverParent = WZ_POINT_IN_RECT(mouseX, mouseY, widget->parent_->getAbsoluteRect());
	}
	else
	{
		hoverParent = true;
	}

	// Or the window itself.
	widgetIsChildOfWindow = !window || (window && (widget->window_ == window || widget == window));

	// Set widget hover.
	oldHover = widget->getHover();
	rect = widget->getAbsoluteRect();
	widget->hover_ = widgetIsChildOfWindow && hoverWindow && hoverParent && WZ_POINT_IN_RECT(mouseX, mouseY, rect);

	// Run callbacks if hover has changed.
	if (!oldHover && widget->getHover())
	{
		widget->onMouseHoverOn();
	}
	else if (oldHover && !widget->getHover())
	{
		widget->onMouseHoverOff();
	}

	// Run mouse move if the mouse is hovering over the widget, or if input is locked to the widget.
	if (widget->getHover() || (widgetIsChildOfWindow && !lockInputWidgetStack_.empty() && widget == lockInputWidgetStack_.back()))
	{
		widget->onMouseMove(mouseX, mouseY, mouseDeltaX, mouseDeltaY);
	}

	ignoreOverlappingChildren(widget, mouseX, mouseY);

	for (size_t i = 0; i < widget->children_.size(); i++)
	{
		mouseMoveRecursive(window, widget->children_[i], mouseX, mouseY, mouseDeltaX, mouseDeltaY);
	}
}

void MainWindow::mouseWheelMoveRecursive(Widget *widget, int x, int y)
{
	WZ_ASSERT(widget);

	if (!widget->isVisible())
		return;

	widget->onMouseWheelMove(x, y);

	for (size_t i = 0; i < widget->children_.size(); i++)
	{
		if (widget->children_[i]->getHover())
		{
			mouseWheelMoveRecursive(widget->children_[i], x, y);
		}
	}
}

void MainWindow::drawWidgetRecursive(Widget *widget, Rect clip, WidgetPredicate drawPredicate, WidgetPredicate recursePredicate)
{
	bool drawLastFound = false;

	if (!widget->isVisible())
		return;

	// Don't render the widget if it's outside its parent window.
	// Special case for combo ancestors.
	if (!widget->overlapsParentWindow() && !IsWidgetComboAncestor(widget))
		return;

	if (drawPredicate(widget) && !widget->drawManually_)
	{
		widget->draw(clip);
	}

	// Update clip rect.
	if (!Rect::intersect(clip, widget->getChildrenClipRect(), &clip))
	{
		// Reset to main window clip rect.
		clip = rect_;
	}

	if (!recursePredicate(widget))
		return;

	// Recurse into children, skip children that are flagged to draw last.
	for (size_t i = 0; i < widget->children_.size(); i++)
	{
		if (widget->children_[i]->getDrawLast())
		{
			drawLastFound = true;
		}
		else
		{
			drawWidgetRecursive(widget->children_[i], clip, drawPredicate, recursePredicate);
		}
	}

	// Recurse into children that are flagged to draw last.
	if (drawLastFound)
	{
		for (size_t i = 0; i < widget->children_.size(); i++)
		{
			if (widget->children_[i]->getDrawLast())
			{
				drawWidgetRecursive(widget->children_[i], clip, drawPredicate, recursePredicate);
			}
		}
	}
}

void MainWindow::drawWidget(Widget *widget, WidgetPredicate drawPredicate, WidgetPredicate recursePredicate)
{
	WZ_ASSERT(widget);
	drawWidgetRecursive(widget, widget->getRect(), drawPredicate, recursePredicate);
}

Window *MainWindow::getHoverWindow(int mouseX, int mouseY)
{
	Window *result = NULL;
	bool resultIsDocked = false;
	int drawPriority = -1;

	for (size_t i = 0; i < children_.size(); i++)
	{
		Widget *widget;
		Window *window;
		bool docked;

		widget = children_[i];

		if (widget->getType() != WidgetType::Window)
			continue;

		if (!widget->isVisible() || !WZ_POINT_IN_RECT(mouseX, mouseY, widget->getRect()))
			continue;

		window = (Window *)widget;
		docked = getWindowDockPosition(window) != DockPosition::None;

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

void MainWindow::onDockTabBarTabChanged(Event e)
{
	if (ignoreDockTabBarChangedEvent_)
		return;

	// Figure out which dock position this tab bar is at.
	DockPosition::Enum dockPosition = DockPosition::None;

	for (int i = 0; i < DockPosition::NumDockPositions; i++)
	{
		if (dockTabBars_[i] == e.tabBar.tabBar)
		{
			dockPosition = (DockPosition::Enum)i;
			break;
		}
	}

	WZ_ASSERT(dockPosition != DockPosition::None);

	// Get the window corresponding to the tab.
	Window *window = ((WindowTabButton *)e.tabBar.tab)->window;

	// Internal metadata won't be set yet when first adding a tab.
	if (window == NULL)
		return;

	// Set the window to visible, hide all the other windows at this dock position.
	window->setVisible(true);

	for (size_t i = 0; i < dockedWindows_[dockPosition].size(); i++)
	{
		if (dockedWindows_[dockPosition][i] == window)
			continue;

		dockedWindows_[dockPosition][i]->setVisible(false);
	}
}

void MainWindow::refreshDockTabBar(DockPosition::Enum dockPosition)
{
	if (!isDockingEnabled())
		return;

	TabBar *tabBar = dockTabBars_[dockPosition];

	if (dockedWindows_[dockPosition].size() < 2)
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
		windowRect = dockedWindows_[dockPosition][0]->getRect();
		tabBarHeight = tabBar->getHeight();

		// Assume space has already been made for the tab bar below the window.
		tabBar->setRect(windowRect.x, windowRect.y + windowRect.h, windowRect.w, tabBarHeight);

		// Add one tab for each window.
		for (size_t i = 0; i < dockedWindows_[dockPosition].size(); i++)
		{
			Window *window = dockedWindows_[dockPosition][i];

			// Create a new tab.
			WindowTabButton *tab = new WindowTabButton(window);
			tabBar->addTab(tab);

			// If this window is selected (visible), select the corresponding tab.
			if (window->isVisible())
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
	if (!isDockingEnabled())
		return;

	for (int i = 0; i < DockPosition::NumDockPositions; i++)
	{
		int nWindows = dockedWindows_[i].size();

		if (nWindows == 0)
			continue;

		// Calculate the window rect. All windows will have the same rect, so just use the first one as a basis.
		Rect windowRect = dockedWindows_[i][0]->getRect();
		int menuBarHeight = isMenuEnabled() ? menuBar_->getHeight() : 0;

		switch (i)
		{
		case DockPosition::North:
			windowRect.w = rect_.w;
			break;
		case DockPosition::South:
			windowRect.y = rect_.h - windowRect.h;
			windowRect.w = rect_.w;
			break;
		case DockPosition::East:
			windowRect.x = rect_.w - windowRect.w;
			windowRect.h = rect_.h - menuBarHeight;
			break;
		case DockPosition::West:
			windowRect.h = rect_.h - menuBarHeight;
			break;
		}

		// Update tab bar rect. Adjust window rect to make space for the tab bar.
		if (nWindows > 1)
		{
			Rect tabBarRect = dockTabBars_[i]->getRect();

			switch (i)
			{
			case DockPosition::North:
				tabBarRect.w = rect_.w;
				break;
			case DockPosition::South:
				tabBarRect.y = rect_.h - tabBarRect.h;
				tabBarRect.w = rect_.w;
				windowRect.h -= tabBarRect.h;
				break;
			case DockPosition::East:
				tabBarRect.x = rect_.w - windowRect.w;
				tabBarRect.y = rect_.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			case DockPosition::West:
				tabBarRect.y = rect_.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			}

			dockTabBars_[i]->setRect(tabBarRect);
		}

		// Set window rects.
		for (size_t j = 0; j < dockedWindows_[i].size(); j++)
		{
			dockedWindows_[i][j]->setRect(windowRect);
		}
	}
}

Rect MainWindow::calculateDockWindowRect(DockPosition::Enum dockPosition, Size windowSize)
{
	// e.g. north dock max height is main window height * maxPreviewSizeMultiplier.
	const float maxPreviewSizeMultiplier = 0.3f;

	int menuBarHeight = isMenuEnabled() ? menuBar_->getHeight() : 0;

	// If there's already a window docked at this position, set the dock preview rect to that size.
	int nDockedWindows = dockedWindows_[dockPosition].size();
	Rect rect;

	if (nDockedWindows > 0)
	{
		rect = dockedWindows_[dockPosition][0]->getRect();

		// If there's exactly one window already docked at this position, leave room for the dock tab bar.
		if (nDockedWindows == 1)
		{
			rect.h -= dockTabBars_[dockPosition]->getHeight();
		}
	}
	else
	{
		// Use the window width for east/west or height for north/south, but don't go over half of the main window width/height.
		if (dockPosition == DockPosition::North)
		{
			rect.x = 1;
			rect.y = menuBarHeight;
			rect.w = rect_.w - 1;
			rect.h = WZ_MIN(windowSize.h, (int)(rect_.h * maxPreviewSizeMultiplier)) - menuBarHeight;
		}
		else if (dockPosition == DockPosition::South)
		{
			const int h = WZ_MIN(windowSize.h, (int)(rect_.h * maxPreviewSizeMultiplier));
			rect.x = 1;
			rect.y = rect_.h - h;
			rect.w = rect_.w - 1;
			rect.h = h - 1;
		}
		else if (dockPosition == DockPosition::East)
		{
			const int w = WZ_MIN(windowSize.w, (int)(rect_.w * maxPreviewSizeMultiplier));
			rect.x = rect_.w - w;
			rect.y = menuBarHeight;
			rect.w = w;
			rect.h = rect_.h - menuBarHeight - 1;
		}
		else if (dockPosition == DockPosition::West)
		{
			rect.x = 1;
			rect.y = menuBarHeight;
			rect.w = WZ_MIN(windowSize.w, (int)(rect_.w * maxPreviewSizeMultiplier));
			rect.h = rect_.h - menuBarHeight - 1;
		}
	}

	return rect;
}

void MainWindow::updateDockIconPositions()
{
	if (!isDockingEnabled())
		return;

	// Push icons out this percent/100 from main window edges.
	const float percent = 0.04f;

	Size ds = getSize();
	Size dis = dockIcons_[DockPosition::North]->getUserOrMeasuredSize();
	int centerW = (int)(ds.w / 2.0f - dis.w / 2.0f);
	int centerH = (int)(ds.h / 2.0f - dis.h / 2.0f);

	dockIcons_[DockPosition::North]->setPosition(centerW, (int)(ds.h * percent));
	dockIcons_[DockPosition::South]->setPosition(centerW, (int)(ds.h * (1.0f - percent) - dis.h));
	dockIcons_[DockPosition::East]->setPosition((int)(ds.w * (1.0f - percent) - dis.h), centerH);
	dockIcons_[DockPosition::West]->setPosition((int)(ds.w * percent), centerH);
}

void MainWindow::updateDockPreviewRect(DockPosition::Enum dockPosition)
{
	WZ_ASSERT(movingWindow_);

	if (!isDockingEnabled())
		return;

	const Size windowSize = movingWindow_->getSize();
	const Rect rect = calculateDockWindowRect(dockPosition, windowSize);
	dockPreview_->setRect(rect);
}

void MainWindow::updateDockPreviewVisible(int mouseX, int mouseY)
{
	if (!isDockingEnabled() || !movingWindow_)
		return;

	bool showDockPreview = false;

	for (int i = 0; i < DockPosition::NumDockPositions; i++)
	{
		const Rect rect = dockIcons_[i]->getRect();

		if (WZ_POINT_IN_RECT(mouseX, mouseY, rect))
		{
			windowDockPosition_ = (DockPosition::Enum)i;
			updateDockPreviewRect((DockPosition::Enum)i);
			showDockPreview = true;
			break;
		}
	}

	dockPreview_->setVisible(showDockPreview);
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

	for (size_t i = 0; i < (int)children_.size(); i++)
	{
		if (children_[i]->getType() == WidgetType::Window && children_[i] != top)
		{
			windows[nWindows] = (Window *)children_[i];
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
