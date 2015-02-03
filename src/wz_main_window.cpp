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
#include "wz_pch.h"
#pragma hdrstop

namespace wz {

/*
================================================================================

MISC. UTILITY FUNCTIONS

================================================================================
*/

// Returns the window that the mouse cursor is hovering over. NULL if there isn't one.
static struct WindowImpl *wz_main_window_get_hover_window(struct MainWindowImpl *mainWindow, int mouseX, int mouseY)
{
	struct WindowImpl *result;
	int drawPriority;
	bool resultIsDocked;

	WZ_ASSERT(mainWindow);
	result = NULL;
	resultIsDocked = false;
	drawPriority = -1;

	for (size_t i = 0; i < mainWindow->children.size(); i++)
	{
		struct WidgetImpl *widget;
		struct WindowImpl *window;
		bool docked;

		widget = mainWindow->children[i];

		if (widget->type != WZ_TYPE_WINDOW)
			continue;

		if (!wz_widget_get_visible(widget) || !WZ_POINT_IN_RECT(mouseX, mouseY, widget->rect))
			continue;

		window = (struct WindowImpl *)widget;
		docked = mainWindow->getWindowDockPosition(window) != WZ_DOCK_POSITION_NONE;

		// Undocked always takes priority over docked.
		if (wz_window_get_draw_priority(window) >= drawPriority || (resultIsDocked && !docked))
		{
			drawPriority = wz_window_get_draw_priority((struct WindowImpl *)widget);
			resultIsDocked = docked;
			result = window;
		}
	}

	return result;
}

/*
================================================================================

DOCK TAB BARS

================================================================================
*/

// Used by all dock tab bars.
static void wz_main_window_dock_tab_bar_tab_changed(Event *e)
{
	struct MainWindowImpl *mainWindow;
	DockPosition dockPosition;
	struct WindowImpl *window;

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
	window = (struct WindowImpl *)wz_widget_get_internal_metadata(e->tabBar.tab);

	// Internal metadata won't be set yet when first adding a tab.
	if (window == NULL)
		return;

	// Set the window to visible, hide all the other windows at this dock position.
	wz_widget_set_visible(window, true);

	for (size_t i = 0; i < mainWindow->dockedWindows[dockPosition].size(); i++)
	{
		if (mainWindow->dockedWindows[dockPosition][i] == window)
			continue;

		wz_widget_set_visible(mainWindow->dockedWindows[dockPosition][i], false);
	}
}

static void wz_main_window_refresh_dock_tab_bar(struct MainWindowImpl *mainWindow, DockPosition dockPosition)
{
	struct TabBarImpl *tabBar;

	WZ_ASSERT(mainWindow);
	tabBar = mainWindow->dockTabBars[dockPosition];

	if (mainWindow->dockedWindows[dockPosition].size() < 2)
	{
		// Hide the tab bar.
		wz_widget_set_visible(tabBar, false);
	}
	else
	{
		Rect windowRect;
		int tabBarHeight;

		// Clear the tabs.
		tabBar->clearTabs();

		// Resize the tab bar to match the window(s) at this dock position. Just use the first window, doesn't matter which.
		windowRect = wz_widget_get_rect(mainWindow->dockedWindows[dockPosition][0]);
		tabBarHeight = wz_widget_get_height(tabBar);

		// Assume space has already been made for the tab bar below the window.
		wz_widget_set_rect_args_internal(tabBar, windowRect.x, windowRect.y + windowRect.h, windowRect.w, tabBarHeight);

		// Add one tab for each window.
		for (size_t i = 0; i < mainWindow->dockedWindows[dockPosition].size(); i++)
		{
			struct WindowImpl *window = mainWindow->dockedWindows[dockPosition][i];

			// Create a new tab.
			struct ButtonImpl *tab = tabBar->createTab();
			tab->setLabel(window->getTitle());

			// Set the tab internal metadata to the window.
			wz_widget_set_internal_metadata(tab, window);

			// If this window is selected (visible), select the corresponding tab.
			if (wz_widget_get_visible(window))
			{
				tabBar->selectTab(tab);
			}
		}

		// Show the tab bar.
		wz_widget_set_visible(tabBar, true);
	}
}

/*
================================================================================

DOCKING

================================================================================
*/

// Update the rects of docked windows and dock tab bars.
static void wz_main_window_update_docking_rects(struct MainWindowImpl *mainWindow)
{
	int i;

	WZ_ASSERT(mainWindow);

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		int nWindows;
		Rect tabBarRect, windowRect;
		int menuBarHeight;

		nWindows = mainWindow->dockedWindows[i].size();

		if (nWindows == 0)
			continue;

		// Calculate the window rect. All windows will have the same rect, so just use the first one as a basis.
		windowRect = wz_widget_get_rect(mainWindow->dockedWindows[i][0]);
		menuBarHeight = mainWindow->menuBar ? wz_widget_get_height(mainWindow->menuBar) : 0;

		switch (i)
		{
		case WZ_DOCK_POSITION_NORTH:
			windowRect.w = mainWindow->rect.w;
			break;
		case WZ_DOCK_POSITION_SOUTH:
			windowRect.y = mainWindow->rect.h - windowRect.h;
			windowRect.w = mainWindow->rect.w;
			break;
		case WZ_DOCK_POSITION_EAST:
			windowRect.x = mainWindow->rect.w - windowRect.w;
			windowRect.h = mainWindow->rect.h - menuBarHeight;
			break;
		case WZ_DOCK_POSITION_WEST:
			windowRect.h = mainWindow->rect.h - menuBarHeight;
			break;
		}

		// Update tab bar rect. Adjust window rect to make space for the tab bar.
		if (nWindows > 1)
		{
			tabBarRect = wz_widget_get_rect(mainWindow->dockTabBars[i]);

			switch (i)
			{
			case WZ_DOCK_POSITION_NORTH:
				tabBarRect.w = mainWindow->rect.w;
				break;
			case WZ_DOCK_POSITION_SOUTH:
				tabBarRect.y = mainWindow->rect.h - tabBarRect.h;
				tabBarRect.w = mainWindow->rect.w;
				windowRect.h -= tabBarRect.h;
				break;
			case WZ_DOCK_POSITION_EAST:
				tabBarRect.x = mainWindow->rect.w - windowRect.w;
				tabBarRect.y = mainWindow->rect.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			case WZ_DOCK_POSITION_WEST:
				tabBarRect.y = mainWindow->rect.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			}

			wz_widget_set_rect_internal(mainWindow->dockTabBars[i], tabBarRect);
		}

		// Set window rects.
		for (size_t j = 0; j < mainWindow->dockedWindows[i].size(); j++)
		{
			wz_widget_set_rect_internal(mainWindow->dockedWindows[i][j], windowRect);
		}
	}
}

static Rect wz_main_window_calculate_dock_window_rect(struct MainWindowImpl *mainWindow, DockPosition dockPosition, Size windowSize)
{
	int nDockedWindows;
	Rect rect;
	int menuBarHeight;

	// e.g. north dock max height is mainWindow height * maxPreviewSizeMultiplier.
	const float maxPreviewSizeMultiplier = 0.3f;

	WZ_ASSERT(mainWindow);
	menuBarHeight = mainWindow->menuBar ? wz_widget_get_height(mainWindow->menuBar) : 0;

	// If there's already a window docked at this position, set the dock preview rect to that size.
	nDockedWindows = mainWindow->dockedWindows[dockPosition].size();

	if (nDockedWindows > 0)
	{
		rect = wz_widget_get_rect(mainWindow->dockedWindows[dockPosition][0]);

		// If there's exactly one window already docked at this position, leave room for the dock tab bar.
		if (nDockedWindows == 1)
		{
			rect.h -= wz_widget_get_height(mainWindow->dockTabBars[dockPosition]);
		}
	}
	else
	{
		// Use the window width for east/west or height for north/south, but don't go over half of the mainWindow width/height.
		if (dockPosition == WZ_DOCK_POSITION_NORTH)
		{
			rect.x = 0;
			rect.y = menuBarHeight;
			rect.w = mainWindow->rect.w;
			rect.h = WZ_MIN(windowSize.h, (int)(mainWindow->rect.h * maxPreviewSizeMultiplier)) - menuBarHeight;
		}
		else if (dockPosition == WZ_DOCK_POSITION_SOUTH)
		{
			const int h = WZ_MIN(windowSize.h, (int)(mainWindow->rect.h * maxPreviewSizeMultiplier));
			rect.x = 0;
			rect.y = mainWindow->rect.h - h;
			rect.w = mainWindow->rect.w;
			rect.h = h;
		}
		else if (dockPosition == WZ_DOCK_POSITION_EAST)
		{
			const int w = WZ_MIN(windowSize.w, (int)(mainWindow->rect.w * maxPreviewSizeMultiplier));
			rect.x = mainWindow->rect.w - w;
			rect.y = menuBarHeight;
			rect.w = w;
			rect.h = mainWindow->rect.h - menuBarHeight;
		}
		else if (dockPosition == WZ_DOCK_POSITION_WEST)
		{
			rect.x = 0;
			rect.y = menuBarHeight;
			rect.w = WZ_MIN(windowSize.w, (int)(mainWindow->rect.w * maxPreviewSizeMultiplier));
			rect.h = mainWindow->rect.h - menuBarHeight;
		}
	}

	return rect;
}

/*
================================================================================

DOCK ICON

================================================================================
*/

static void wz_main_window_draw_dock_icon(struct WidgetImpl *widget, Rect clip)
{
	NVGRenderer *r = (NVGRenderer *)widget->renderer;
	struct NVGcontext *vg = r->getContext();
	const Rect rect = wz_widget_get_rect(widget);
	clip = clip; // Never clipped, so just ignore that parameter.

	nvgSave(vg);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h, 3);
	nvgFillColor(vg, nvgRGBA(64, 64, 64, 128));
	nvgFill(vg);
	nvgRestore(vg);
}

static void wz_main_window_update_dock_icon_positions(struct MainWindowImpl *mainWindow)
{
	// Push icons out this percent/100 from mainWindow edges.
	const float percent = 0.04f;
	Size ds, dis;
	int centerW, centerH;

	WZ_ASSERT(mainWindow);
	ds = wz_widget_get_size(mainWindow);
	dis = wz_widget_get_size(mainWindow->dockIcons[WZ_DOCK_POSITION_NORTH]);
	centerW = (int)(ds.w / 2.0f - dis.w / 2.0f);
	centerH = (int)(ds.h / 2.0f - dis.h / 2.0f);

	wz_widget_set_position_args_internal(mainWindow->dockIcons[WZ_DOCK_POSITION_NORTH], centerW, (int)(ds.h * percent));
	wz_widget_set_position_args_internal(mainWindow->dockIcons[WZ_DOCK_POSITION_SOUTH], centerW, (int)(ds.h * (1.0f - percent) - dis.h));
	wz_widget_set_position_args_internal(mainWindow->dockIcons[WZ_DOCK_POSITION_EAST], (int)(ds.w * (1.0f - percent) - dis.h), centerH);
	wz_widget_set_position_args_internal(mainWindow->dockIcons[WZ_DOCK_POSITION_WEST], (int)(ds.w * percent), centerH);
}

/*
================================================================================

DOCK PREVIEW

================================================================================
*/

static void wz_main_window_draw_dock_preview(struct WidgetImpl *widget, Rect clip)
{
	NVGRenderer *r = (NVGRenderer *)widget->renderer;
	struct NVGcontext *vg = r->getContext();
	clip = clip; // Never clipped, so just ignore that parameter.

	nvgSave(vg);
	r->drawFilledRect(wz_widget_get_rect(widget), WZ_SKIN_MAIN_WINDOW_DOCK_PREVIEW_COLOR);
	nvgRestore(vg);
}

static void wz_widget_update_dock_preview_rect(struct MainWindowImpl *mainWindow, DockPosition dockPosition)
{
	Size windowSize;
	Rect rect;

	WZ_ASSERT(mainWindow);
	WZ_ASSERT(mainWindow->movingWindow);
	windowSize = wz_widget_get_size(mainWindow->movingWindow);
	rect = wz_main_window_calculate_dock_window_rect(mainWindow, dockPosition, windowSize);
	wz_widget_set_rect_internal(mainWindow->dockPreview, rect);
}

static void wz_main_window_update_dock_preview_visible(struct MainWindowImpl *mainWindow, int mouseX, int mouseY)
{
	int i;
	bool showDockPreview = false;

	WZ_ASSERT(mainWindow);

	if (!mainWindow->movingWindow)
		return;

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		Rect rect = wz_widget_get_rect(mainWindow->dockIcons[i]);

		if (WZ_POINT_IN_RECT(mouseX, mouseY, rect))
		{
			mainWindow->windowDockPosition = (DockPosition)i;
			wz_widget_update_dock_preview_rect(mainWindow, (DockPosition)i);
			showDockPreview = true;
			break;
		}
	}

	wz_widget_set_visible(mainWindow->dockPreview, showDockPreview);
}

/*
================================================================================

MENU BAR

================================================================================
*/

static void wz_main_window_update_menu_bar_rect(struct MainWindowImpl *mainWindow)
{
	WZ_ASSERT(mainWindow);

	if (!mainWindow->menuBar)
		return;

	wz_widget_set_width(mainWindow->menuBar, mainWindow->rect.w);
}

/*
================================================================================

DRAW PRIORITY

================================================================================
*/

static int wz_compare_window_draw_priorities(const void *a, const void *b)
{
	return wz_window_get_draw_priority((const struct WindowImpl *)a) - wz_window_get_draw_priority((const struct WindowImpl *)b);
}

// top can be NULL
static void wz_main_window_update_window_draw_priorities(struct MainWindowImpl *mainWindow, struct WindowImpl *top)
{
	struct WindowImpl *windows[WZ_MAX_WINDOWS];
	int nWindows;

	WZ_ASSERT(mainWindow);

	// Get a list of windows (excluding top).
	nWindows = 0;

	for (size_t i = 0; i < (int)mainWindow->children.size(); i++)
	{
		struct WidgetImpl *widget = mainWindow->children[i];
	
		if (widget->type == WZ_TYPE_WINDOW && widget != top)
		{
			windows[nWindows] = (struct WindowImpl *)widget;
			nWindows++;
		}
	}

	// Sort them in ascending order by draw priority.
	qsort(windows, nWindows, sizeof(struct WindowImpl *), wz_compare_window_draw_priorities);

	// Assign each window a new draw priority, starting at 0 and ascending by 1.
	int i;

	for (i = 0; i < nWindows; i++)
	{
		wz_window_set_draw_priority(windows[i], i);
	}

	// Give the top window the highest priority.
	if (top)
	{
		wz_window_set_draw_priority(top, i);
	}
}

/*
================================================================================

MOUSE BUTTON DOWN

================================================================================
*/

static void wz_widget_mouse_button_down_recursive(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	WZ_ASSERT(widget);

	if (!wz_widget_get_visible(widget))
		return;

	if (widget->vtable.mouse_button_down)
	{
		widget->vtable.mouse_button_down(widget, mouseButton, mouseX, mouseY);
	}

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		if (widget->children[i]->hover)
		{
			wz_widget_mouse_button_down_recursive(widget->children[i], mouseButton, mouseX, mouseY);
		}
	}
}

/*
================================================================================

MOUSE BUTTON UP

================================================================================
*/

static void wz_widget_mouse_button_up_recursive(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	WZ_ASSERT(widget);

	if (!wz_widget_get_visible(widget))
		return;

	if (widget->vtable.mouse_button_up)
	{
		widget->vtable.mouse_button_up(widget, mouseButton, mouseX, mouseY);
	}

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		wz_widget_mouse_button_up_recursive(widget->children[i], mouseButton, mouseX, mouseY);
	}
}

/*
================================================================================

MOUSE MOVE

================================================================================
*/

// Clear widget hover on everything but ignoreWindow and it's children.
static void wz_widget_clear_hover_recursive(struct WindowImpl *ignoreWindow, struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);

	if (widget == ignoreWindow)
		return;

	if (widget->hover)
	{
		// Stop hovering.
		widget->hover = false;

		if (widget->vtable.mouse_hover_off)
		{
			widget->vtable.mouse_hover_off(widget);
		}
	}

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		wz_widget_clear_hover_recursive(ignoreWindow, widget->children[i]);
	}
}

// Sets WidgetImpl.ignore
static void wz_widget_ignore_overlapping_children(struct WidgetImpl *widget, int mouseX, int mouseY)
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

			if (!wz_widget_get_visible(widget->children[j]))
				continue;

			// If the mouse cursor is in the intersection of the two widget rects.
			rect1 = wz_widget_get_absolute_rect(widget->children[i]);
			rect2 = wz_widget_get_absolute_rect(widget->children[j]);

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

// If window is not NULL, only call mouse_move in widgets that are children of the window and the window itself.
static void wz_widget_mouse_move_recursive(struct WindowImpl *window, struct WidgetImpl *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	Rect rect;
	bool hoverWindow;
	bool hoverParent;
	bool widgetIsChildOfWindow;
	bool oldHover;

	WZ_ASSERT(widget);

	if (!wz_widget_get_visible(widget))
		return;

	// Don't process mouse move if the widget is ignored.
	if (widget->ignore)
	{
		if (widget->hover)
		{
			// Stop hovering.
			widget->hover = false;

			if (widget->vtable.mouse_hover_off)
			{
				widget->vtable.mouse_hover_off(widget);
			}
		}

		return;
	}

	// Determine whether the mouse is hovering over the widget's parent window.
	// Special case for combo dropdown list poking outside the window.
	if (widget->window && !(widget->parent && widget->parent->type == WZ_TYPE_COMBO))
	{
		hoverWindow = WZ_POINT_IN_RECT(mouseX, mouseY, wz_widget_get_absolute_rect(wz_window_get_content_widget(widget->window)));
	}
	else
	{
		hoverWindow = true;
	}

	// Determine whether the mouse is hovering over the widget's parent.
	if (!widget->inputNotClippedToParent && widget->parent && widget->parent != widget->mainWindow && widget->parent != widget->window)
	{
		hoverParent = WZ_POINT_IN_RECT(mouseX, mouseY, wz_widget_get_absolute_rect(widget->parent));
	}
	else
	{
		hoverParent = true;
	}

	// Or the window itself.
	widgetIsChildOfWindow = !window || (window && (widget->window == window || widget == window));

	// Set widget hover.
	oldHover = widget->hover;
	rect = wz_widget_get_absolute_rect(widget);
	widget->hover = widgetIsChildOfWindow && hoverWindow && hoverParent && WZ_POINT_IN_RECT(mouseX, mouseY, rect);

	// Run callbacks if hover has changed.
	if (!oldHover && widget->hover && widget->vtable.mouse_hover_on)
	{
		widget->vtable.mouse_hover_on(widget);
	}
	else if (oldHover && !widget->hover && widget->vtable.mouse_hover_off)
	{
		widget->vtable.mouse_hover_off(widget);
	}

	// Run mouse move callback.
	if (widget->vtable.mouse_move)
	{
		// If the mouse is hovering over the widget, or if input is locked to the widget.
		if (widget->hover || (widgetIsChildOfWindow && !widget->mainWindow->lockInputWidgetStack.empty() && widget == widget->mainWindow->lockInputWidgetStack.back()))
		{
			widget->vtable.mouse_move(widget, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		}
	}

	wz_widget_ignore_overlapping_children(widget, mouseX, mouseY);

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		wz_widget_mouse_move_recursive(window, widget->children[i], mouseX, mouseY, mouseDeltaX, mouseDeltaY);
	}
}

/*
================================================================================

MOUSE WHEEL MOVE

================================================================================
*/

static void wz_widget_mouse_wheel_move_recursive(struct WidgetImpl *widget, int x, int y)
{
	WZ_ASSERT(widget);

	if (!wz_widget_get_visible(widget))
		return;

	if (widget->vtable.mouse_wheel_move)
	{
		widget->vtable.mouse_wheel_move(widget, x, y);
	}

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		if (widget->children[i]->hover)
		{
			wz_widget_mouse_wheel_move_recursive(widget->children[i], x, y);
		}
	}
}

/*
================================================================================

KEY DOWN AND UP

================================================================================
*/

static void wz_main_window_key(struct MainWindowImpl *mainWindow, Key key, bool down)
{
	struct WidgetImpl *widget;

	WZ_ASSERT(mainWindow);
	widget = mainWindow->keyboardFocusWidget;

	if (!widget || !wz_widget_get_visible(widget))
		return;

	if (down && widget->vtable.key_down)
	{
		widget->vtable.key_down(widget, key);
	}
	else if (!down && widget->vtable.key_up)
	{
		widget->vtable.key_up(widget, key);
	}
}

/*
================================================================================

DRAWING

================================================================================
*/

typedef bool (*WidgetImplPredicate)(const struct WidgetImpl *);

static bool wz_widget_true(const struct WidgetImpl *widget)
{
	widget = widget;
	return true;
}

static bool wz_widget_is_combo_ancestor(const struct WidgetImpl *widget)
{
	return widget->type != WZ_TYPE_COMBO && wz_widget_find_closest_ancestor(widget, WZ_TYPE_COMBO) != NULL;
}

static bool wz_widget_is_not_combo(const struct WidgetImpl *widget)
{
	return widget->type != WZ_TYPE_COMBO;
}

static bool wz_widget_is_not_window_or_combo(const struct WidgetImpl *widget)
{
	return widget->type != WZ_TYPE_WINDOW && widget->type != WZ_TYPE_COMBO;
}

static void wz_widget_draw_recursive(struct WidgetImpl *widget, Rect clip, WidgetImplPredicate draw_predicate, WidgetImplPredicate recurse_predicate)
{
	bool drawLastFound = false;

	if (!wz_widget_get_visible(widget))
		return;

	// Don't render the widget if it's outside its parent window.
	// Special case for combo ancestors.
	if (!wz_widget_overlaps_parent_window(widget) && !wz_widget_is_combo_ancestor(widget))
		return;

	if (draw_predicate(widget) && !widget->drawManually)
	{
		if (widget->vtable.draw)
		{
			widget->vtable.draw(widget, clip);
		}
		else
		{
			widget->draw(clip);
		}
	}

	// Update clip rect.
	if (widget->vtable.get_children_clip_rect)
	{
		if (!Rect::intersect(clip, widget->vtable.get_children_clip_rect(widget), &clip))
		{
			// Reset to mainWindow clip rect.
			clip = wz_widget_get_rect(widget->mainWindow);
		}
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

static void wz_widget_draw(struct WidgetImpl *widget, WidgetImplPredicate draw_predicate, WidgetImplPredicate recurse_predicate)
{
	WZ_ASSERT(widget);
	wz_widget_draw_recursive(widget, wz_widget_get_rect(widget), draw_predicate, recurse_predicate);
}

static int wz_compare_window_draw_priorities_docked(const void *a, const void *b)
{
	const struct WindowImpl *window1, *window2;
	bool window1Docked, window2Docked;

	window1 = *((const struct WindowImpl **)a);
	window2 = *((const struct WindowImpl **)b);
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

	return wz_window_get_draw_priority(window1) - wz_window_get_draw_priority(window2);
}

static void wz_widget_draw_if_visible(struct WidgetImpl *widget)
{
	if (wz_widget_get_visible(widget))
	{
		Rect clip;
		clip.x = clip.y = clip.w = clip.h = 0;

		if (widget->vtable.draw)
		{
			widget->vtable.draw(widget, clip);
		}
		else
		{
			widget->draw(clip);
		}
	}
}

static void wz_main_window_set_rect(struct WidgetImpl *widget, Rect rect)
{
	struct MainWindowImpl *mainWindow;

	WZ_ASSERT(widget);
	mainWindow = (struct MainWindowImpl *)widget;
	mainWindow->rect.w = rect.w;
	mainWindow->rect.h = rect.h;

	wz_main_window_update_menu_bar_rect(mainWindow);
	wz_main_window_update_dock_icon_positions(mainWindow);
	wz_main_window_update_docking_rects(mainWindow);
	mainWindow->updateContentRect();
}

MainWindowImpl::MainWindowImpl(IRenderer *renderer)
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
	vtable.set_rect = wz_main_window_set_rect;
	isTextCursorVisible_ = true;

	// Create content widget.
	content = new struct WidgetImpl;
	content->mainWindow = this;
	wz_widget_add_child_widget(this, content);

	// Create dock icon widgets.
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		dockIcons[i] = new WidgetImpl;
		struct WidgetImpl *widget = dockIcons[i];
		wz_widget_set_draw_callback(widget, wz_main_window_draw_dock_icon);
		wz_widget_set_size_args_internal(dockIcons[i], 48, 48);
		wz_widget_set_visible(widget, false);
		wz_widget_set_draw_manually(widget, true);
		wz_widget_add_child_widget(this, widget);
	}

	wz_main_window_update_dock_icon_positions(this);

	// Create dock preview widget.
	dockPreview = new WidgetImpl;
	struct WidgetImpl *widget = dockPreview;
	wz_widget_set_draw_manually(widget, true);
	wz_widget_set_draw_callback(widget, wz_main_window_draw_dock_preview);
	wz_widget_set_visible(widget, false);
	wz_widget_add_child_widget(this, widget);

	// Create dock tab bars.
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		dockTabBars[i] = new TabBarImpl;
		wz_widget_set_visible(dockTabBars[i], false);
		wz_widget_add_child_widget(this, dockTabBars[i]);
		dockTabBars[i]->addCallbackTabChanged(wz_main_window_dock_tab_bar_tab_changed);
	}

	// Create menu bar.
	menuBar = new MenuBarImpl;
	setMenuBar(menuBar);
}

bool MainWindowImpl::isShiftKeyDown() const
{
	return isShiftKeyDown_;
}

bool MainWindowImpl::isControlKeyDown() const
{
	return isControlKeyDown_;
}

void MainWindowImpl::setEventCallback(EventCallback callback)
{
	handle_event = callback;
}

void MainWindowImpl::mouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	// Clear keyboard focus widget.
	keyboardFocusWidget = NULL;

	lockInputWindow = wz_main_window_get_hover_window(this, mouseX, mouseY);
	struct WidgetImpl *widget = this;;

	if (!mainWindow->lockInputWidgetStack.empty())
	{
		// Lock input to the top/last item on the stack.
		widget = lockInputWidgetStack.back();
	}
	else if (mainWindow->lockInputWindow)
	{
		wz_main_window_update_window_draw_priorities(this, lockInputWindow);
		widget = lockInputWindow;
	}

	wz_widget_mouse_button_down_recursive(widget, mouseButton, mouseX, mouseY);

	// Need a special case for dock icons.
	wz_main_window_update_dock_preview_visible(this, mouseX, mouseY);
}

void MainWindowImpl::mouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	// Need a special case for dock icons.
	if (movingWindow)
	{
		// If the dock preview is visible, movingWindow can be docked.
		if (wz_widget_get_visible(dockPreview))
		{
			dockWindow(movingWindow, windowDockPosition);
		}

		wz_widget_set_visible(dockPreview, false);
		movingWindow = NULL;
	}

	struct WidgetImpl *widget = this;

	if (!lockInputWidgetStack.empty())
	{
		// Lock input to the top/last item on the stack.
		widget = lockInputWidgetStack.back();
	}
	else if (lockInputWindow)
	{
		widget = lockInputWindow;
	}

	wz_widget_mouse_button_up_recursive(widget, mouseButton, mouseX, mouseY);
}

void MainWindowImpl::mouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	// Reset the mouse cursor to default.
	cursor = WZ_CURSOR_DEFAULT;

	// Need a special case for dock icons.
	wz_main_window_update_dock_preview_visible(this, mouseX, mouseY);

	if (!lockInputWidgetStack.empty())
	{
		// Lock input to the top/last item on the stack.
		wz_widget_mouse_move_recursive(NULL, lockInputWidgetStack.back(), mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		return;
	}

	lockInputWindow = wz_main_window_get_hover_window(this, mouseX, mouseY);

	// Clear hover on everything but the lockInputWindow and it's children.
	wz_widget_clear_hover_recursive(lockInputWindow, this);

	wz_widget_mouse_move_recursive(lockInputWindow, this, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
}

void MainWindowImpl::mouseWheelMove(int x, int y)
{
	struct WidgetImpl *widget = this;

	if (!lockInputWidgetStack.empty())
	{
		// Lock input to the top/last item on the stack.
		widget = lockInputWidgetStack.back();
	}
	else if (lockInputWindow)
	{
		widget = lockInputWindow;
	}

	wz_widget_mouse_wheel_move_recursive(widget, x, y);
}

void MainWindowImpl::keyDown(Key key)
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

	wz_main_window_key(this, key, true);
}

void MainWindowImpl::keyUp(Key key)
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

	wz_main_window_key(this, key, false);
}

void MainWindowImpl::textInput(const char *text)
{
	struct WidgetImpl *widget = keyboardFocusWidget;

	if (!widget || !wz_widget_get_visible(widget))
		return;

	if (widget->vtable.text_input)
	{
		widget->vtable.text_input(widget, text);
	}
}

void MainWindowImpl::draw()
{
	// Draw the main window (not really, vtable.draw is NULL) and ancestors. Don't recurse into windows or combos.
	wz_widget_draw(this, wz_widget_true, wz_widget_is_not_window_or_combo);

	// Get a list of windows (excluding top).
	struct WindowImpl *windows[WZ_MAX_WINDOWS];
	int nWindows = 0;

	for (size_t i = 0; i < children.size(); i++)
	{
		if (children[i]->type == WZ_TYPE_WINDOW)
		{
			windows[nWindows] = (struct WindowImpl *)children[i];
			nWindows++;
		}
	}

	// Sort them in ascending order by draw priority.
	qsort(windows, nWindows, sizeof(struct WindowImpl *), wz_compare_window_draw_priorities_docked);

	// For each window, draw the window and all ancestors. Don't recurse into combos.
	for (int i = 0; i < nWindows; i++)
	{
		struct WidgetImpl *widget = windows[i];

		if (!wz_widget_get_visible(widget))
			continue;

		if (widget->vtable.draw)
		{
			widget->vtable.draw(widget, rect);
		}
		else
		{
			widget->draw(rect);
		}

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

void MainWindowImpl::drawFrame()
{
	NVGRenderer *r = (NVGRenderer *)renderer;
	struct NVGcontext *vg = r->getContext();
	nvgBeginFrame(vg, rect.w, rect.h, 1);
	draw();
	nvgEndFrame(vg);
}

void MainWindowImpl::setMenuBar(struct MenuBarImpl *menuBar)
{
	this->menuBar = menuBar;
	wz_widget_set_width(menuBar, mainWindow->rect.w);
	wz_widget_add_child_widget(this, menuBar);
}

void MainWindowImpl::add(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW)
		return;

	// Special case for windows: add directly, not to the content widget.
	if (wz_widget_get_type(widget) == WZ_TYPE_WINDOW)
	{
		wz_widget_add_child_widget(this, widget);
	}
	else
	{
		wz_widget_add_child_widget(content, widget);
	}
}

void MainWindowImpl::remove(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);

	// Special case for windows: remove directly, not from the content widget.
	if (wz_widget_get_type(widget) == WZ_TYPE_WINDOW)
	{
		wz_widget_remove_child_widget(this, widget);
	}
	else
	{
		wz_widget_remove_child_widget(content, widget);
	}
}

bool MainWindowImpl::isTextCursorVisible() const
{
	return isTextCursorVisible_;
}

void MainWindowImpl::toggleTextCursor()
{
	isTextCursorVisible_ = !isTextCursorVisible_;
}

Cursor MainWindowImpl::getCursor() const
{
	return cursor;
}

const struct WidgetImpl *MainWindowImpl::getKeyboardFocusWidget() const
{
	return keyboardFocusWidget;
}

void MainWindowImpl::setKeyboardFocusWidget(struct WidgetImpl *widget)
{
	keyboardFocusWidget = widget;
}

DockPosition MainWindowImpl::getWindowDockPosition(const struct WindowImpl *window) const
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

void MainWindowImpl::dockWindow(struct WindowImpl *window, DockPosition dockPosition)
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
		wz_widget_set_visible(dockedWindows[dockPosition][i], false);
	}

	// Inform the window it is being docked.
	wz_window_dock(window);

	// Resize the window.
	wz_widget_set_rect_internal(window, wz_main_window_calculate_dock_window_rect(this, dockPosition, wz_widget_get_size(window)));

	// Dock the window.
	dockedWindows[dockPosition].push_back(window);

	// Resize the other windows docked at this position to match.
	updateDockedWindowRect(window);

	// Refresh the tab bar for this dock position.
	ignoreDockTabBarChangedEvent = true;
	wz_main_window_refresh_dock_tab_bar(this, dockPosition);
	ignoreDockTabBarChangedEvent = false;

	// Docked windows affect the mainWindow content rect, so update it.
	updateContentRect();
}

void MainWindowImpl::undockWindow(struct WindowImpl *window)
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
	if (wz_widget_get_visible(window) && nDockedWindows > 0)
	{
		wz_widget_set_visible(dockedWindows[dockPosition][0], true);
	}

	// Refresh the tab bar for this dock position.
	wz_main_window_refresh_dock_tab_bar(this, dockPosition);

	// If the dock tab bar is hidden, resize the windows at this dock position to reclaim the space it used.
	if (!wz_widget_get_visible(dockTabBars[dockPosition]))
	{
		for (size_t j = 0; j < dockedWindows[dockPosition].size(); j++)
		{
			struct WidgetImpl *widget = dockedWindows[dockPosition][j];

			if (dockPosition == WZ_DOCK_POSITION_SOUTH)
			{
				wz_widget_set_height_internal(widget, widget->rect.h + (rect.h - (widget->rect.y + widget->rect.h)));
			}
			else if (dockPosition == WZ_DOCK_POSITION_EAST || dockPosition == WZ_DOCK_POSITION_WEST)
			{
				wz_widget_set_height_internal(widget, rect.h);
			}
		}
	}
}

void MainWindowImpl::updateDockedWindowRect(struct WindowImpl *window)
{
	WZ_ASSERT(window);
	Rect rect = wz_widget_get_rect(window);

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

					wz_widget_set_rect_internal(dockedWindows[i][k], rect);
				}

				// Update the tab bar too.
				wz_widget_set_rect_args_internal(dockTabBars[i], rect.x, rect.y + rect.h, rect.w, wz_widget_get_height(dockTabBars[i]));
				return;
			}
		}
	}
}

void MainWindowImpl::pushLockInputWidget(struct WidgetImpl *widget)
{
	lockInputWidgetStack.push_back(widget);
}

void MainWindowImpl::popLockInputWidget(struct WidgetImpl *widget)
{
	// Only pop if the widget is on the top of the stack.
	if (lockInputWidgetStack.empty())
		return;

	if (widget == lockInputWidgetStack.back())
	{	
		lockInputWidgetStack.pop_back();
	}
}

void MainWindowImpl::setMovingWindow(struct WindowImpl *window)
{
	movingWindow = window;

	// Show the dock icons if movingWindow is not NULL.
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wz_widget_set_visible(dockIcons[i], movingWindow != NULL);
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

void MainWindowImpl::updateContentRect()
{
	Rect rect = this->rect;

	// Adjust the content rect based on the menu bar height.
	if (menuBar)
	{
		const int h = wz_widget_get_height(menuBar);
		rect.y += h;
		rect.h -= h;
	}

	// Adjust the content rect based on docked windows.
	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		Rect windowRect;

		if (dockedWindows[i].empty())
			continue;

		windowRect = wz_widget_get_rect(dockedWindows[i][0]);

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

	wz_widget_set_rect_internal(content, rect);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

MainWindow::MainWindow(IRenderer *renderer)
{
	WZ_ASSERT(renderer);
	impl = new MainWindowImpl(renderer);
}

MainWindow::~MainWindow()
{
	wz_widget_destroy(impl);
}

int MainWindow::getWidth() const
{
	return wz_widget_get_width(impl);
}

int MainWindow::getHeight() const
{
	return wz_widget_get_height(impl);
}

void MainWindow::setSize(int w, int h)
{
	wz_widget_set_size_args(impl, w, h);
}

void MainWindow::mouseMove(int x, int y, int dx, int dy)
{
	impl->mouseMove(x, y, dx, dy);
}

void MainWindow::mouseButtonDown(int button, int x, int y)
{
	impl->mouseButtonDown(button, x, y);
}

void MainWindow::mouseButtonUp(int button, int x, int y)
{
	impl->mouseButtonUp(button, x, y);
}

void MainWindow::mouseWheelMove(int x, int y)
{
	impl->mouseWheelMove(x, y);
}

void MainWindow::keyDown(Key key)
{
	impl->keyDown(key);
}

void MainWindow::keyUp(Key key)
{
	impl->keyUp(key);
}

void MainWindow::textInput(const char *text)
{
	impl->textInput(text);
}

void MainWindow::draw()
{	
	impl->draw();
}

void MainWindow::drawFrame()
{	
	impl->drawFrame();
}

void MainWindow::toggleTextCursor()
{
	impl->toggleTextCursor();
}

Cursor MainWindow::getCursor() const
{
	return impl->getCursor();
}

void MainWindowImpl::setCursor(Cursor cursor)
{
	this->cursor = cursor;
}

Widget *MainWindow::add(Widget *widget)
{
	impl->add(widget->impl);
	return widget;
}

void MainWindow::remove(Widget *widget)
{
	impl->remove(widget->impl);
}

void MainWindow::createMenuButton(const std::string &label)
{
	MenuBarButtonImpl *button = impl->menuBar->createButton();
	button->setLabel(label.c_str());
}

void MainWindow::dockWindow(Window *window, DockPosition dockPosition)
{
	impl->dockWindow((WindowImpl *)window->impl, dockPosition);
}

} // namespace wz
