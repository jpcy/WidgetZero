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
#include <stdlib.h>
#include <string.h>
#include "wz_main_window.h"
#include "wz_widget.h"
#include "wz_widget_draw.h"
#include "wz_window.h"

struct wzMainWindow
{
	struct wzWidget base;

	struct wzWidget *content;

	// Centralized event handler.
	wzEventCallback handle_event;

	wzCursor cursor;

	bool isShiftKeyDown, isControlKeyDown;

	struct wzWidget **lockInputWidgetStack;

	// Lock input to this window, i.e. don't call mouse_move, mouse_button_down or mouse_button_up on any widget that isn't this window or it's descendants.
	struct wzWindow *lockInputWindow;

	struct wzWidget *keyboardFocusWidget;

	// This window is currently being moved and may be docked.
	struct wzWindow *movingWindow;

	// Hidden from the consumer.
	struct wzDummy *dockIcons[WZ_NUM_DOCK_POSITIONS];
	struct wzDummy *dockPreview;

	struct wzWindow **dockedWindows[WZ_NUM_DOCK_POSITIONS];

	// A window being dragged will be docked to this position on mouse up. Set when the cursor hovers over a dock icon.
	wzDockPosition windowDockPosition;

	// Each dock position has a tab bar which is visible when multiple windows are docked at the same position.
	struct wzTabBar *dockTabBars[WZ_NUM_DOCK_POSITIONS];

	bool ignoreDockTabBarChangedEvent;

	struct wzMenuBar *menuBar;
};

/*
================================================================================

MISC. UTILITY FUNCTIONS

================================================================================
*/

// Returns the window that the mouse cursor is hovering over. NULL if there isn't one.
static struct wzWindow *wz_main_window_get_hover_window(struct wzMainWindow *mainWindow, int mouseX, int mouseY)
{
	struct wzWindow *result;
	int drawPriority;
	bool resultIsDocked;
	int i;

	WZ_ASSERT(mainWindow);
	result = NULL;
	resultIsDocked = false;
	drawPriority = -1;

	for (i = 0; i < wz_arr_len(mainWindow->base.children); i++)
	{
		struct wzWidget *widget;
		struct wzWindow *window;
		bool docked;

		widget = mainWindow->base.children[i];

		if (widget->type != WZ_TYPE_WINDOW)
			continue;

		if (!wz_widget_get_visible(widget) || !WZ_POINT_IN_RECT(mouseX, mouseY, widget->rect))
			continue;

		window = (struct wzWindow *)widget;
		docked = wz_main_window_get_window_dock_position(mainWindow, window) != WZ_DOCK_POSITION_NONE;

		// Undocked always takes priority over docked.
		if (wz_window_get_draw_priority(window) >= drawPriority || (resultIsDocked && !docked))
		{
			drawPriority = wz_window_get_draw_priority((struct wzWindow *)widget);
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
static void wz_main_window_dock_tab_bar_tab_changed(wzEvent *e)
{
	struct wzMainWindow *mainWindow;
	wzDockPosition dockPosition;
	int i;
	struct wzWindow *window;

	WZ_ASSERT(e);
	mainWindow = e->base.widget->mainWindow;

	if (mainWindow->ignoreDockTabBarChangedEvent)
		return;

	// Figure out which dock position this tab bar is at.
	dockPosition = WZ_DOCK_POSITION_NONE;

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		if (mainWindow->dockTabBars[i] == e->tabBar.tabBar)
		{
			dockPosition = i;
			break;
		}
	}

	WZ_ASSERT(dockPosition != WZ_DOCK_POSITION_NONE);

	// Get the window corresponding to the tab.
	window = (struct wzWindow *)wz_widget_get_internal_metadata((struct wzWidget *)e->tabBar.tab);

	// Internal metadata won't be set yet when first adding a tab.
	if (window == NULL)
		return;

	// Set the window to visible, hide all the other windows at this dock position.
	wz_widget_set_visible((struct wzWidget *)window, true);

	for (i = 0; i < wz_arr_len(mainWindow->dockedWindows[dockPosition]); i++)
	{
		if (mainWindow->dockedWindows[dockPosition][i] == window)
			continue;

		wz_widget_set_visible((struct wzWidget *)mainWindow->dockedWindows[dockPosition][i], false);
	}
}

static void wz_main_window_refresh_dock_tab_bar(struct wzMainWindow *mainWindow, wzDockPosition dockPosition)
{
	struct wzTabBar *tabBar;

	WZ_ASSERT(mainWindow);
	tabBar = mainWindow->dockTabBars[dockPosition];

	if (wz_arr_len(mainWindow->dockedWindows[dockPosition]) < 2)
	{
		// Hide the tab bar.
		wz_widget_set_visible((struct wzWidget *)tabBar, false);
	}
	else
	{
		wzRect windowRect;
		int tabBarHeight;
		int i;

		// Clear the tabs.
		wz_tab_bar_clear_tabs(tabBar);

		// Resize the tab bar to match the window(s) at this dock position. Just use the first window, doesn't matter which.
		windowRect = wz_widget_get_rect((struct wzWidget *)mainWindow->dockedWindows[dockPosition][0]);
		tabBarHeight = wz_widget_get_height((struct wzWidget *)tabBar);

		// Assume space has already been made for the tab bar below the window.
		wz_widget_set_rect_args_internal((struct wzWidget *)tabBar, windowRect.x, windowRect.y + windowRect.h, windowRect.w, tabBarHeight);

		// Add one tab for each window.
		for (i = 0; i < wz_arr_len(mainWindow->dockedWindows[dockPosition]); i++)
		{
			struct wzWindow *window = mainWindow->dockedWindows[dockPosition][i];

			// Create a new tab.
			struct wzButton *tab = wz_tab_bar_create_tab(tabBar);
			wz_button_set_label(tab, wz_window_get_title(window));

			// Set the tab internal metadata to the window.
			wz_widget_set_internal_metadata((struct wzWidget *)tab, window);

			// If this window is selected (visible), select the corresponding tab.
			if (wz_widget_get_visible((struct wzWidget *)window))
			{
				wz_tab_bar_select_tab(tabBar, tab);
			}
		}

		// Show the tab bar.
		wz_widget_set_visible((struct wzWidget *)tabBar, true);
	}
}

/*
================================================================================

DOCKING

================================================================================
*/

// The docked window "window" has been resized, update the rects of other windows docked at the same position.
void wz_main_window_update_docked_window_rect(struct wzMainWindow *mainWindow, struct wzWindow *window)
{
	wzRect rect;
	wzDockPosition i;
	int j, k;

	WZ_ASSERT(mainWindow);
	WZ_ASSERT(window);
	rect = wz_widget_get_rect((const struct wzWidget *)window);

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		for (j = 0; j < wz_arr_len(mainWindow->dockedWindows[i]); j++)
		{
			if (mainWindow->dockedWindows[i][j] == window)
			{
				for (k = 0; k < wz_arr_len(mainWindow->dockedWindows[i]); k++)
				{
					if (j == k)
						continue;

					wz_widget_set_rect_internal((struct wzWidget *)mainWindow->dockedWindows[i][k], rect);
				}

				// Update the tab bar too.
				wz_widget_set_rect_args_internal((struct wzWidget *)mainWindow->dockTabBars[i], rect.x, rect.y + rect.h, rect.w, wz_widget_get_height((struct wzWidget *)mainWindow->dockTabBars[i]));
				return;
			}
		}
	}
}

// Update the rects of docked windows and dock tab bars.
static void wz_main_window_update_docking_rects(struct wzMainWindow *mainWindow)
{
	wzDockPosition i;

	WZ_ASSERT(mainWindow);

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		int nWindows;
		wzRect tabBarRect, windowRect;
		int j;

		nWindows = wz_arr_len(mainWindow->dockedWindows[i]);

		if (nWindows == 0)
			continue;

		// Calculate the window rect. All windows will have the same rect, so just use the first one as a basis.
		windowRect = wz_widget_get_rect((struct wzWidget *)mainWindow->dockedWindows[i][0]);

		switch (i)
		{
		case WZ_DOCK_POSITION_NORTH:
			windowRect.w = mainWindow->base.rect.w;
			break;
		case WZ_DOCK_POSITION_SOUTH:
			windowRect.y = mainWindow->base.rect.h - windowRect.h;
			windowRect.w = mainWindow->base.rect.w;
			break;
		case WZ_DOCK_POSITION_EAST:
			windowRect.x = mainWindow->base.rect.w - windowRect.w;
			windowRect.h = mainWindow->base.rect.h;
			break;
		case WZ_DOCK_POSITION_WEST:
			windowRect.h = mainWindow->base.rect.h;
			break;
		}

		// Update tab bar rect. Adjust window rect to make space for the tab bar.
		if (nWindows > 1)
		{
			tabBarRect = wz_widget_get_rect((struct wzWidget *)mainWindow->dockTabBars[i]);

			switch (i)
			{
			case WZ_DOCK_POSITION_NORTH:
				tabBarRect.w = mainWindow->base.rect.w;
				break;
			case WZ_DOCK_POSITION_SOUTH:
				tabBarRect.y = mainWindow->base.rect.h - tabBarRect.h;
				tabBarRect.w = mainWindow->base.rect.w;
				windowRect.h -= tabBarRect.h;
				break;
			case WZ_DOCK_POSITION_EAST:
				tabBarRect.x = mainWindow->base.rect.w - windowRect.w;
				tabBarRect.y = mainWindow->base.rect.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			case WZ_DOCK_POSITION_WEST:
				tabBarRect.y = mainWindow->base.rect.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			}

			wz_widget_set_rect_internal((struct wzWidget *)mainWindow->dockTabBars[i], tabBarRect);
		}

		// Set window rects.
		for (j = 0; j < wz_arr_len(mainWindow->dockedWindows[i]); j++)
		{
			wz_widget_set_rect_internal((struct wzWidget *)mainWindow->dockedWindows[i][j], windowRect);
		}
	}
}

static wzRect wz_main_window_calculate_dock_window_rect(struct wzMainWindow *mainWindow, wzDockPosition dockPosition, wzSize windowSize)
{
	int nDockedWindows;
	wzRect rect;
	int menuBarHeight;

	// e.g. north dock max height is mainWindow height * maxPreviewSizeMultiplier.
	const float maxPreviewSizeMultiplier = 0.3f;

	WZ_ASSERT(mainWindow);
	menuBarHeight = mainWindow->menuBar ? wz_widget_get_height((const struct wzWidget *)mainWindow->menuBar) : 0;

	// If there's already a window docked at this position, set the dock preview rect to that size.
	nDockedWindows = wz_arr_len(mainWindow->dockedWindows[dockPosition]);

	if (nDockedWindows > 0)
	{
		rect = wz_widget_get_rect((struct wzWidget *)mainWindow->dockedWindows[dockPosition][0]);

		// If there's exactly one window already docked at this position, leave room for the dock tab bar.
		if (nDockedWindows == 1)
		{
			rect.h -= wz_widget_get_height((struct wzWidget *)mainWindow->dockTabBars[dockPosition]);
		}
	}
	else
	{
		// Use the window width for east/west or height for north/south, but don't go over half of the mainWindow width/height.
		if (dockPosition == WZ_DOCK_POSITION_NORTH)
		{
			rect.x = 0;
			rect.y = menuBarHeight;
			rect.w = mainWindow->base.rect.w;
			rect.h = WZ_MIN(windowSize.h, (int)(mainWindow->base.rect.h * maxPreviewSizeMultiplier)) - menuBarHeight;
		}
		else if (dockPosition == WZ_DOCK_POSITION_SOUTH)
		{
			const int h = WZ_MIN(windowSize.h, (int)(mainWindow->base.rect.h * maxPreviewSizeMultiplier));
			rect.x = 0;
			rect.y = mainWindow->base.rect.h - h;
			rect.w = mainWindow->base.rect.w;
			rect.h = h;
		}
		else if (dockPosition == WZ_DOCK_POSITION_EAST)
		{
			const int w = WZ_MIN(windowSize.w, (int)(mainWindow->base.rect.w * maxPreviewSizeMultiplier));
			rect.x = mainWindow->base.rect.w - w;
			rect.y = menuBarHeight;
			rect.w = w;
			rect.h = mainWindow->base.rect.h - menuBarHeight;
		}
		else if (dockPosition == WZ_DOCK_POSITION_WEST)
		{
			rect.x = 0;
			rect.y = menuBarHeight;
			rect.w = WZ_MIN(windowSize.w, (int)(mainWindow->base.rect.w * maxPreviewSizeMultiplier));
			rect.h = mainWindow->base.rect.h - menuBarHeight;
		}
	}

	return rect;
}

void wz_main_window_dock_window(struct wzMainWindow *mainWindow, struct wzWindow *window, wzDockPosition dockPosition)
{
	int i;

	WZ_ASSERT(mainWindow);
	WZ_ASSERT(window);

	// Not valid, use wz_main_window_undock_window to undock.
	if (dockPosition == WZ_DOCK_POSITION_NONE)
		return;

	// Don't do anything if this window is already docked at this position.
	for (i = 0; i < wz_arr_len(mainWindow->dockedWindows[dockPosition]); i++)
	{
		if (mainWindow->dockedWindows[dockPosition][i] == window)
			return;
	}

	// Hide any other windows docked at the same position.
	for (i = 0; i < wz_arr_len(mainWindow->dockedWindows[dockPosition]); i++)
	{
		wz_widget_set_visible((struct wzWidget *)mainWindow->dockedWindows[dockPosition][i], false);
	}

	// Inform the window it is being docked.
	wz_window_dock(window);

	// Resize the window.
	wz_widget_set_rect_internal((struct wzWidget *)window, wz_main_window_calculate_dock_window_rect(mainWindow, dockPosition, wz_widget_get_size((struct wzWidget *)window)));

	// Dock the window.
	wz_arr_push(mainWindow->dockedWindows[dockPosition], window);

	// Resize the other windows docked at this position to match.
	wz_main_window_update_docked_window_rect(mainWindow, window);

	// Refresh the tab bar for this dock position.
	mainWindow->ignoreDockTabBarChangedEvent = true;
	wz_main_window_refresh_dock_tab_bar(mainWindow, dockPosition);
	mainWindow->ignoreDockTabBarChangedEvent = false;

	// Docked windows affect the mainWindow content rect, so update it.
	wz_main_window_update_content_rect(mainWindow);
}

wzDockPosition wz_main_window_get_window_dock_position(const struct wzMainWindow *mainWindow, const struct wzWindow *window)
{
	wzDockPosition i;
	int j;

	WZ_ASSERT(mainWindow);
	WZ_ASSERT(window);

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		for (j = 0; j < wz_arr_len(mainWindow->dockedWindows[i]); j++)
		{
			if (mainWindow->dockedWindows[i][j] == window)
			{
				return i;
			}
		}
	}

	return WZ_DOCK_POSITION_NONE;
}

void wz_main_window_undock_window(struct wzMainWindow *mainWindow, struct wzWindow *window)
{
	wzDockPosition dockPosition, i;
	int j, windowIndex;
	int nDockedWindows;

	WZ_ASSERT(mainWindow);
	WZ_ASSERT(window);

	// Find the dock position for the window, and the window index.
	dockPosition = WZ_DOCK_POSITION_NONE;
	windowIndex = -1;

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		for (j = 0; j < wz_arr_len(mainWindow->dockedWindows[i]); j++)
		{
			if (mainWindow->dockedWindows[i][j] == window)
			{
				dockPosition = i;
				windowIndex = j;
				break;
			}
		}

		if (dockPosition != WZ_DOCK_POSITION_NONE)
			break;
	}

	if (windowIndex == -1)
		return;

	wz_arr_delete(mainWindow->dockedWindows[i], windowIndex);
	nDockedWindows = wz_arr_len(mainWindow->dockedWindows[dockPosition]);

	// If there are other windows docked at this position, make sure one is visible after removing this window.
	if (wz_widget_get_visible((const struct wzWidget *)window) && nDockedWindows > 0)
	{
		wz_widget_set_visible((struct wzWidget *)mainWindow->dockedWindows[dockPosition][0], true);
	}

	// Refresh the tab bar for this dock position.
	wz_main_window_refresh_dock_tab_bar(mainWindow, dockPosition);

	// If the dock tab bar is hidden, resize the windows at this dock position to reclaim the space it used.
	if (!wz_widget_get_visible((const struct wzWidget *)mainWindow->dockTabBars[dockPosition]))
	{
		for (j = 0; j < wz_arr_len(mainWindow->dockedWindows[dockPosition]); j++)
		{
			struct wzWidget *widget = (struct wzWidget *)mainWindow->dockedWindows[dockPosition][j];

			if (dockPosition == WZ_DOCK_POSITION_SOUTH)
			{
				wz_widget_set_height_internal(widget, widget->rect.h + (mainWindow->base.rect.h - (widget->rect.y + widget->rect.h)));
			}
			else if (dockPosition == WZ_DOCK_POSITION_EAST || dockPosition == WZ_DOCK_POSITION_WEST)
			{
				wz_widget_set_height_internal(widget, mainWindow->base.rect.h);
			}
		}
	}
}

/*
================================================================================

DOCK ICON

================================================================================
*/

static void wz_main_window_draw_dock_icon(struct wzWidget *widget, wzRect clip)
{
	WZ_ASSERT(widget);
	clip = clip; // Never clipped, so just ignore that parameter.
	widget->renderer->draw_dock_icon(widget->renderer, wz_widget_get_rect(widget));
}

static void wz_main_window_update_dock_icon_positions(struct wzMainWindow *mainWindow)
{
	// Push icons out this percent/100 from mainWindow edges.
	const float percent = 0.04f;
	wzSize ds, dis;
	int centerW, centerH;

	WZ_ASSERT(mainWindow);
	ds = wz_widget_get_size((struct wzWidget *)mainWindow);
	dis = wz_widget_get_size((struct wzWidget *)mainWindow->dockIcons[WZ_DOCK_POSITION_NORTH]);
	centerW = (int)(ds.w / 2.0f - dis.w / 2.0f);
	centerH = (int)(ds.h / 2.0f - dis.h / 2.0f);

	wz_widget_set_position_args_internal((struct wzWidget *)mainWindow->dockIcons[WZ_DOCK_POSITION_NORTH], centerW, (int)(ds.h * percent));
	wz_widget_set_position_args_internal((struct wzWidget *)mainWindow->dockIcons[WZ_DOCK_POSITION_SOUTH], centerW, (int)(ds.h * (1.0f - percent) - dis.h));
	wz_widget_set_position_args_internal((struct wzWidget *)mainWindow->dockIcons[WZ_DOCK_POSITION_EAST], (int)(ds.w * (1.0f - percent) - dis.h), centerH);
	wz_widget_set_position_args_internal((struct wzWidget *)mainWindow->dockIcons[WZ_DOCK_POSITION_WEST], (int)(ds.w * percent), centerH);
}

/*
================================================================================

DOCK PREVIEW

================================================================================
*/

static void wz_main_window_draw_dock_preview(struct wzWidget *widget, wzRect clip)
{
	WZ_ASSERT(widget);
	clip = clip; // Never clipped, so just ignore that parameter.
	widget->renderer->draw_dock_preview(widget->renderer, wz_widget_get_rect(widget));
}

static void wz_widget_update_dock_preview_rect(struct wzMainWindow *mainWindow, wzDockPosition dockPosition)
{
	wzSize windowSize;
	wzRect rect;

	WZ_ASSERT(mainWindow);
	WZ_ASSERT(mainWindow->movingWindow);
	windowSize = wz_widget_get_size((struct wzWidget *)mainWindow->movingWindow);
	rect = wz_main_window_calculate_dock_window_rect(mainWindow, dockPosition, windowSize);
	wz_widget_set_rect_internal((struct wzWidget *)mainWindow->dockPreview, rect);
}

static void wz_main_window_update_dock_preview_visible(struct wzMainWindow *mainWindow, int mouseX, int mouseY)
{
	wzDockPosition i;
	bool showDockPreview = false;

	WZ_ASSERT(mainWindow);

	if (!mainWindow->movingWindow)
		return;

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wzRect rect = wz_widget_get_rect((struct wzWidget *)mainWindow->dockIcons[i]);

		if (WZ_POINT_IN_RECT(mouseX, mouseY, rect))
		{
			mainWindow->windowDockPosition = i;
			wz_widget_update_dock_preview_rect(mainWindow, i);
			showDockPreview = true;
			break;
		}
	}

	wz_widget_set_visible((struct wzWidget *)mainWindow->dockPreview, showDockPreview);
}

/*
================================================================================

MENU BAR

================================================================================
*/

static void wz_main_window_update_menu_bar_rect(struct wzMainWindow *mainWindow)
{
	WZ_ASSERT(mainWindow);

	if (!mainWindow->menuBar)
		return;

	wz_widget_set_width((struct wzWidget *)mainWindow->menuBar, mainWindow->base.rect.w);
}

/*
================================================================================

DRAW PRIORITY

================================================================================
*/

static int wz_compare_window_draw_priorities(const void *a, const void *b)
{
	return wz_window_get_draw_priority((const struct wzWindow *)a) - wz_window_get_draw_priority((const struct wzWindow *)b);
}

// top can be NULL
static void wz_main_window_update_window_draw_priorities(struct wzMainWindow *mainWindow, struct wzWindow *top)
{
	struct wzWindow *windows[WZ_MAX_WINDOWS];
	int nWindows;
	int i;

	WZ_ASSERT(mainWindow);

	// Get a list of windows (excluding top).
	nWindows = 0;

	for (i = 0; i < wz_arr_len(mainWindow->base.children); i++)
	{
		struct wzWidget *widget = mainWindow->base.children[i];
	
		if (widget->type == WZ_TYPE_WINDOW && widget != (struct wzWidget *)top)
		{
			windows[nWindows] = (struct wzWindow *)widget;
			nWindows++;
		}
	}

	// Sort them in ascending order by draw priority.
	qsort(windows, nWindows, sizeof(struct wzWindow *), wz_compare_window_draw_priorities);

	// Assign each window a new draw priority, starting at 0 and ascending by 1.
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

static void wz_widget_mouse_button_down_recursive(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	int i;

	WZ_ASSERT(widget);

	if (!wz_widget_get_visible(widget))
		return;

	if (widget->vtable.mouse_button_down)
	{
		widget->vtable.mouse_button_down(widget, mouseButton, mouseX, mouseY);
	}

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		if (widget->children[i]->hover)
		{
			wz_widget_mouse_button_down_recursive(widget->children[i], mouseButton, mouseX, mouseY);
		}
	}
}

void wz_main_window_mouse_button_down(struct wzMainWindow *mainWindow, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	WZ_ASSERT(mainWindow);

	// Clear keyboard focus widget.
	mainWindow->keyboardFocusWidget = NULL;

	mainWindow->lockInputWindow = wz_main_window_get_hover_window(mainWindow, mouseX, mouseY);

	if (wz_arr_len(mainWindow->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = mainWindow->lockInputWidgetStack[wz_arr_lastn(mainWindow->lockInputWidgetStack)];
	}
	else if (mainWindow->lockInputWindow)
	{
		wz_main_window_update_window_draw_priorities(mainWindow, mainWindow->lockInputWindow);
		widget = (struct wzWidget *)mainWindow->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)mainWindow;
	}

	wz_widget_mouse_button_down_recursive(widget, mouseButton, mouseX, mouseY);

	// Need a special case for dock icons.
	wz_main_window_update_dock_preview_visible(mainWindow, mouseX, mouseY);
}

/*
================================================================================

MOUSE BUTTON UP

================================================================================
*/

static void wz_widget_mouse_button_up_recursive(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	int i;

	WZ_ASSERT(widget);

	if (!wz_widget_get_visible(widget))
		return;

	if (widget->vtable.mouse_button_up)
	{
		widget->vtable.mouse_button_up(widget, mouseButton, mouseX, mouseY);
	}

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_mouse_button_up_recursive(widget->children[i], mouseButton, mouseX, mouseY);
	}
}

void wz_main_window_mouse_button_up(struct wzMainWindow *mainWindow, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	WZ_ASSERT(mainWindow);

	// Need a special case for dock icons.
	if (mainWindow->movingWindow)
	{
		// If the dock preview is visible, movingWindow can be docked.
		if (wz_widget_get_visible((struct wzWidget *)mainWindow->dockPreview))
		{
			wz_main_window_dock_window(mainWindow, mainWindow->movingWindow, mainWindow->windowDockPosition);
		}

		wz_widget_set_visible((struct wzWidget *)mainWindow->dockPreview, false);
		mainWindow->movingWindow = NULL;
	}

	if (wz_arr_len(mainWindow->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = mainWindow->lockInputWidgetStack[wz_arr_lastn(mainWindow->lockInputWidgetStack)];
	}
	else if (mainWindow->lockInputWindow)
	{
		widget = (struct wzWidget *)mainWindow->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)mainWindow;
	}

	wz_widget_mouse_button_up_recursive(widget, mouseButton, mouseX, mouseY);
}

/*
================================================================================

MOUSE MOVE

================================================================================
*/

// Clear widget hover on everything but ignoreWindow and it's children.
static void wz_widget_clear_hover_recursive(struct wzWindow *ignoreWindow, struct wzWidget *widget)
{
	int i;

	WZ_ASSERT(widget);

	if (widget == (struct wzWidget *)ignoreWindow)
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

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_clear_hover_recursive(ignoreWindow, widget->children[i]);
	}
}

// Sets wzWidget.ignore
static void wz_widget_ignore_overlapping_children(struct wzWidget *widget, int mouseX, int mouseY)
{
	int i, j;

	WZ_ASSERT(widget);

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		widget->children[i]->ignore = false;
	}

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		for (j = 0; j < wz_arr_len(widget->children); j++)
		{
			wzRect rect1, rect2, intersection;

			if (i == j)
				continue;

			if (!wz_widget_get_visible(widget->children[j]))
				continue;

			// If the mouse cursor is in the intersection of the two widget rects.
			rect1 = wz_widget_get_absolute_rect(widget->children[i]);
			rect2 = wz_widget_get_absolute_rect(widget->children[j]);

			if (wz_intersect_rects(rect1, rect2, &intersection) && WZ_POINT_IN_RECT(mouseX, mouseY, intersection))
			{
				// Ignore the one with lower draw priority.
				if (widget->children[i]->drawPriority < widget->children[j]->drawPriority)
				{
					widget->children[i]->ignore = true;
					break;
				}
			}
		}
	}
}

// If window is not NULL, only call mouse_move in widgets that are children of the window and the window itself.
static void wz_widget_mouse_move_recursive(struct wzWindow *window, struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	wzRect rect;
	bool hoverWindow;
	bool hoverParent;
	bool widgetIsChildOfWindow;
	bool oldHover;
	int i;

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
	// Don't do this if the inherited widget draw priority is higher than window draw priority.
	if (widget->window && wz_widget_calculate_inherited_draw_priority(widget) < WZ_DRAW_PRIORITY_WINDOW)
	{
		hoverWindow = WZ_POINT_IN_RECT(mouseX, mouseY, wz_widget_get_absolute_rect(wz_window_get_content_widget(widget->window)));
	}
	else
	{
		hoverWindow = true;
	}

	// Determine whether the mouse is hovering over the widget's parent.
	if (!widget->inputNotClippedToParent && widget->parent && widget->parent != (struct wzWidget *)widget->mainWindow && widget->parent != (struct wzWidget *)widget->window)
	{
		hoverParent = WZ_POINT_IN_RECT(mouseX, mouseY, wz_widget_get_absolute_rect(widget->parent));
	}
	else
	{
		hoverParent = true;
	}

	// Or the window itself.
	widgetIsChildOfWindow = !window || (window && (widget->window == window || widget == (struct wzWidget *)window));

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
		if (widget->hover || (widgetIsChildOfWindow && wz_arr_len(widget->mainWindow->lockInputWidgetStack) > 0 && widget == widget->mainWindow->lockInputWidgetStack[wz_arr_lastn(widget->mainWindow->lockInputWidgetStack)]))
		{
			widget->vtable.mouse_move(widget, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		}
	}

	wz_widget_ignore_overlapping_children(widget, mouseX, mouseY);

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_mouse_move_recursive(window, widget->children[i], mouseX, mouseY, mouseDeltaX, mouseDeltaY);
	}
}

void wz_main_window_mouse_move(struct wzMainWindow *mainWindow, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	WZ_ASSERT(mainWindow);

	// Reset the mouse cursor to default.
	mainWindow->cursor = WZ_CURSOR_DEFAULT;

	// Need a special case for dock icons.
	wz_main_window_update_dock_preview_visible(mainWindow, mouseX, mouseY);

	if (wz_arr_len(mainWindow->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		wz_widget_mouse_move_recursive(NULL, mainWindow->lockInputWidgetStack[wz_arr_lastn(mainWindow->lockInputWidgetStack)], mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		return;
	}

	mainWindow->lockInputWindow = wz_main_window_get_hover_window(mainWindow, mouseX, mouseY);

	// Clear hover on everything but the lockInputWindow and it's children.
	wz_widget_clear_hover_recursive(mainWindow->lockInputWindow, (struct wzWidget *)mainWindow);

	wz_widget_mouse_move_recursive(mainWindow->lockInputWindow, (struct wzWidget *)mainWindow, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
}

/*
================================================================================

MOUSE WHEEL MOVE

================================================================================
*/

static void wz_widget_mouse_wheel_move_recursive(struct wzWidget *widget, int x, int y)
{
	int i;

	WZ_ASSERT(widget);

	if (!wz_widget_get_visible(widget))
		return;

	if (widget->vtable.mouse_wheel_move)
	{
		widget->vtable.mouse_wheel_move(widget, x, y);
	}

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		if (widget->children[i]->hover)
		{
			wz_widget_mouse_wheel_move_recursive(widget->children[i], x, y);
		}
	}
}

void wz_main_window_mouse_wheel_move(struct wzMainWindow *mainWindow, int x, int y)
{
	struct wzWidget *widget;

	WZ_ASSERT(mainWindow);

	if (wz_arr_len(mainWindow->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = mainWindow->lockInputWidgetStack[wz_arr_lastn(mainWindow->lockInputWidgetStack)];
	}
	else if (mainWindow->lockInputWindow)
	{
		widget = (struct wzWidget *)mainWindow->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)mainWindow;
	}

	wz_widget_mouse_wheel_move_recursive(widget, x, y);
}

/*
================================================================================

KEY DOWN AND UP

================================================================================
*/

static void wz_main_window_key(struct wzMainWindow *mainWindow, wzKey key, bool down)
{
	struct wzWidget *widget;

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

void wz_main_window_key_down(struct wzMainWindow *mainWindow, wzKey key)
{
	if (WZ_KEY_MOD_OFF(key) == WZ_KEY_UNKNOWN)
		return;

	if (WZ_KEY_MOD_OFF(key) == WZ_KEY_LSHIFT || WZ_KEY_MOD_OFF(key) == WZ_KEY_RSHIFT)
	{
		mainWindow->isShiftKeyDown = true;
	}
	else if (WZ_KEY_MOD_OFF(key) == WZ_KEY_LCONTROL || WZ_KEY_MOD_OFF(key) == WZ_KEY_RCONTROL)
	{
		mainWindow->isControlKeyDown = true;
	}

	wz_main_window_key(mainWindow, key, true);
}

void wz_main_window_key_up(struct wzMainWindow *mainWindow, wzKey key)
{
	if (WZ_KEY_MOD_OFF(key) == WZ_KEY_UNKNOWN)
		return;

	if (WZ_KEY_MOD_OFF(key) == WZ_KEY_LSHIFT || WZ_KEY_MOD_OFF(key) == WZ_KEY_RSHIFT)
	{
		mainWindow->isShiftKeyDown = false;
	}
	else if (WZ_KEY_MOD_OFF(key) == WZ_KEY_LCONTROL || WZ_KEY_MOD_OFF(key) == WZ_KEY_RCONTROL)
	{
		mainWindow->isControlKeyDown = false;
	}

	wz_main_window_key(mainWindow, key, false);
}

/*
================================================================================

TEXT INPUT

================================================================================
*/

void wz_main_window_text_input(struct wzMainWindow *mainWindow, const char *text)
{
	struct wzWidget *widget;

	WZ_ASSERT(mainWindow);
	widget = mainWindow->keyboardFocusWidget;

	if (!widget || !wz_widget_get_visible(widget))
		return;

	if (widget->vtable.text_input)
	{
		widget->vtable.text_input(widget, text);
	}
}

/*
================================================================================

DRAWING

================================================================================
*/

void wz_main_window_draw(struct wzMainWindow *mainWindow)
{
	mainWindow->base.renderer->draw_main_window(mainWindow->base.renderer, mainWindow);

	wz_widget_draw_main_window(mainWindow);
}

/*
================================================================================

MISC.

================================================================================
*/

static void wz_main_window_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzMainWindow *mainWindow;

	WZ_ASSERT(widget);
	mainWindow = (struct wzMainWindow *)widget;
	mainWindow->base.rect.w = rect.w;
	mainWindow->base.rect.h = rect.h;

	wz_main_window_update_menu_bar_rect(mainWindow);
	wz_main_window_update_dock_icon_positions(mainWindow);
	wz_main_window_update_docking_rects(mainWindow);
	wz_main_window_update_content_rect(mainWindow);
}

struct wzMainWindow *wz_main_window_create(struct wzRenderer *renderer)
{
	struct wzMainWindow *mainWindow;
	int i;
	struct wzWidget *widget;

	mainWindow = (struct wzMainWindow *)malloc(sizeof(struct wzMainWindow));
	memset(mainWindow, 0, sizeof(struct wzMainWindow));
	mainWindow->base.type = WZ_TYPE_MAIN_WINDOW;
	mainWindow->base.renderer = renderer;
	mainWindow->base.mainWindow = mainWindow;
	mainWindow->base.vtable.set_rect = wz_main_window_set_rect;

	// Create content widget.
	mainWindow->content = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(mainWindow->content, 0, sizeof(struct wzWidget));
	mainWindow->content->mainWindow = mainWindow;
	wz_widget_add_child_widget((struct wzWidget *)mainWindow, mainWindow->content);

	// Create dock icon widgets.
	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		mainWindow->dockIcons[i] = wz_dummy_create();
		widget = (struct wzWidget *)mainWindow->dockIcons[i];
		wz_widget_set_draw_priority(widget, WZ_DRAW_PRIORITY_DOCK_ICON);
		wz_widget_set_measure_callback(widget, NULL);
		wz_widget_set_draw_callback(widget, wz_main_window_draw_dock_icon);
		wz_widget_set_size_internal((struct wzWidget *)mainWindow->dockIcons[i], renderer->get_dock_icon_size(renderer));
		wz_widget_set_visible(widget, false);
		wz_widget_add_child_widget((struct wzWidget *)mainWindow, widget);
	}

	wz_main_window_update_dock_icon_positions(mainWindow);

	// Create dock preview widget.
	mainWindow->dockPreview = wz_dummy_create();
	widget = (struct wzWidget *)mainWindow->dockPreview;
	wz_widget_set_draw_priority(widget, WZ_DRAW_PRIORITY_DOCK_PREVIEW);
	wz_widget_set_draw_callback(widget, wz_main_window_draw_dock_preview);
	wz_widget_set_visible(widget, false);
	wz_widget_add_child_widget((struct wzWidget *)mainWindow, widget);

	// Create dock tab bars.
	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		struct wzTabBar *tabBar = wz_tab_bar_create();
		mainWindow->dockTabBars[i] = tabBar;
		wz_widget_set_visible((struct wzWidget *)tabBar, false);
		wz_widget_set_draw_priority((struct wzWidget *)tabBar, WZ_DRAW_PRIORITY_DOCK_TAB_BAR);
		wz_widget_add_child_widget((struct wzWidget *)mainWindow, (struct wzWidget *)tabBar);
		wz_tab_bar_add_callback_tab_changed(tabBar, wz_main_window_dock_tab_bar_tab_changed);

		// Override scroll button draw priority.
		wz_widget_set_draw_priority((struct wzWidget *)wz_tab_bar_get_decrement_button(tabBar), WZ_DRAW_PRIORITY_DOCK_TAB_BAR_SCROLL_BUTTON);
		wz_widget_set_draw_priority((struct wzWidget *)wz_tab_bar_get_increment_button(tabBar), WZ_DRAW_PRIORITY_DOCK_TAB_BAR_SCROLL_BUTTON);
	}

	return mainWindow;
}

void wz_main_window_set_event_callback(struct wzMainWindow *mainWindow, wzEventCallback callback)
{
	WZ_ASSERT(mainWindow);
	mainWindow->handle_event = callback;
}

void wz_main_window_set_menu_bar(struct wzMainWindow *mainWindow, struct wzMenuBar *menuBar)
{
	WZ_ASSERT(mainWindow);
	mainWindow->menuBar = menuBar;
	wz_widget_set_width((struct wzWidget *)menuBar, mainWindow->base.rect.w);
	wz_widget_add_child_widget((struct wzWidget *)mainWindow, (struct wzWidget *)menuBar);
}

void wz_main_window_add(struct wzMainWindow *mainWindow, struct wzWidget *widget)
{
	WZ_ASSERT(mainWindow);
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW)
		return;

	// Special case for windows: add directly, not to the content widget.
	if (wz_widget_get_type(widget) == WZ_TYPE_WINDOW)
	{
		wz_widget_add_child_widget((struct wzWidget *)mainWindow, widget);
	}
	else
	{
		wz_widget_add_child_widget(mainWindow->content, widget);
	}
}

void wz_main_window_remove(struct wzMainWindow *mainWindow, struct wzWidget *widget)
{
	WZ_ASSERT(mainWindow);
	WZ_ASSERT(widget);

	// Special case for windows: remove directly, not from the content widget.
	if (wz_widget_get_type(widget) == WZ_TYPE_WINDOW)
	{
		wz_widget_remove_child_widget((struct wzWidget *)mainWindow, widget);
	}
	else
	{
		wz_widget_remove_child_widget(mainWindow->content, widget);
	}
}

wzCursor wz_main_window_get_cursor(const struct wzMainWindow *mainWindow)
{
	WZ_ASSERT(mainWindow);
	return mainWindow->cursor;
}

void wz_main_window_set_cursor(struct wzMainWindow *mainWindow, wzCursor cursor)
{
	WZ_ASSERT(mainWindow);
	mainWindow->cursor = cursor;
}

const struct wzWidget *wz_main_window_get_keyboard_focus_widget(const struct wzMainWindow *mainWindow)
{
	WZ_ASSERT(mainWindow);
	return mainWindow->keyboardFocusWidget;
}

void wz_main_window_set_keyboard_focus_widget(struct wzMainWindow *mainWindow, struct wzWidget *widget)
{
	WZ_ASSERT(mainWindow);
	mainWindow->keyboardFocusWidget = widget;
}

bool wz_main_window_is_shift_key_down(const struct wzMainWindow *mainWindow)
{
	WZ_ASSERT(mainWindow);
	return mainWindow->isShiftKeyDown;
}

bool wz_main_window_is_control_key_down(const struct wzMainWindow *mainWindow)
{
	WZ_ASSERT(mainWindow);
	return mainWindow->isControlKeyDown;
}

void wz_main_window_push_lock_input_widget(struct wzMainWindow *mainWindow, struct wzWidget *widget)
{
	WZ_ASSERT(mainWindow);
	wz_arr_push(mainWindow->lockInputWidgetStack, widget);
}

void wz_main_window_pop_lock_input_widget(struct wzMainWindow *mainWindow, struct wzWidget *widget)
{
	// Only pop if the widget is on the top of the stack.
	if (wz_arr_len(mainWindow->lockInputWidgetStack) == 0)
		return;

	if (widget == mainWindow->lockInputWidgetStack[wz_arr_lastn(mainWindow->lockInputWidgetStack)])
	{	
		wz_arr_pop(mainWindow->lockInputWidgetStack);
	}
}

void wz_main_window_set_moving_window(struct wzMainWindow *mainWindow, struct wzWindow *window)
{
	int i;

	WZ_ASSERT(mainWindow);
	mainWindow->movingWindow = window;

	// Show the dock icons if movingWindow is not NULL.
	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wz_widget_set_visible((struct wzWidget *)mainWindow->dockIcons[i], mainWindow->movingWindow != NULL);
	}
}

void wz_invoke_event(wzEvent *e, wzEventCallback *callbacks)
{
	int i;

	WZ_ASSERT(e);

	if (e->base.widget->mainWindow && e->base.widget->mainWindow->handle_event)
	{
		e->base.widget->mainWindow->handle_event(e);
	}

	for (i = 0; i < wz_arr_len(callbacks); i++)
	{
		callbacks[i](e);
	}
}

void wz_main_window_update_content_rect(struct wzMainWindow *mainWindow)
{
	wzRect rect;
	wzDockPosition i;

	WZ_ASSERT(mainWindow);
	rect = mainWindow->base.rect;

	// Adjust the content rect based on the menu bar height.
	if (mainWindow->menuBar)
	{
		const int h = wz_widget_get_height((struct wzWidget *)mainWindow->menuBar);
		rect.y += h;
		rect.h -= h;
	}

	// Adjust the content rect based on docked windows.
	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wzRect windowRect;

		if (wz_arr_len(mainWindow->dockedWindows[i]) == 0)
			continue;

		windowRect = wz_widget_get_rect((struct wzWidget *)mainWindow->dockedWindows[i][0]);

		switch (i)
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

	wz_widget_set_rect_internal(mainWindow->content, rect);
}
