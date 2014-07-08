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
#include <assert.h>
#include "wz_desktop.h"
#include "wz_widget.h"
#include "wz_widget_draw.h"
#include "wz_window.h"

struct wzDesktop
{
	struct wzWidget base;

	struct wzWidget *content;

	// Centralized event handler.
	wzEventCallback handle_event;

	wzDesktopMeasureTextCallback measure_text;
	wzDesktopTextGetPixelDeltaCallback text_get_pixel_delta;

	wzCursor cursor;

	struct wzWidget **lockInputWidgetStack;

	// Lock input to this window, i.e. don't call mouse_move, mouse_button_down or mouse_button_up on any widget that isn't this window or it's descendants.
	struct wzWindow *lockInputWindow;

	// This window is currently being moved and may be docked.
	struct wzWindow *movingWindow;

	wzDesktopDrawDockIconCallback draw_dock_icon;
	wzDesktopDrawDockPreviewCallback draw_dock_preview;

	// Hidden from the consumer.
	struct wzLabel *dockIcons[WZ_NUM_DOCK_POSITIONS];
	struct wzLabel *dockPreview;

	struct wzWindow **dockedWindows[WZ_NUM_DOCK_POSITIONS];

	// A window being dragged will be docked to this position on mouse up. Set when the cursor hovers over a dock icon.
	wzDockPosition windowDockPosition;

	// Each dock position has a tab bar which is visible when multiple windows are docked at the same position.
	struct wzTabBar *dockTabBars[WZ_NUM_DOCK_POSITIONS];

	bool ignoreDockTabBarChangedEvent;
};

/*
================================================================================

MISC. UTILITY FUNCTIONS

================================================================================
*/

// Returns the window that the mouse cursor is hovering over. NULL if there isn't one.
static struct wzWindow *wz_desktop_get_hover_window(struct wzDesktop *desktop, int mouseX, int mouseY)
{
	struct wzWindow *result;
	int drawPriority;
	int i;

	assert(desktop);
	result = NULL;
	drawPriority = -1;

	for (i = 0; i < wz_arr_len(desktop->base.children); i++)
	{
		struct wzWidget *widget = desktop->base.children[i];

		if (widget->type == WZ_TYPE_WINDOW && wz_widget_get_visible(widget) && WZ_POINT_IN_RECT(mouseX, mouseY, widget->rect) && wz_window_get_draw_priority((struct wzWindow *)widget) >= drawPriority)
		{
			drawPriority = wz_window_get_draw_priority((struct wzWindow *)widget);
			result = (struct wzWindow *)widget;
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
static void wz_desktop_dock_tab_bar_tab_changed(wzEvent e)
{
	struct wzDesktop *desktop;
	wzDockPosition dockPosition;
	int i;
	struct wzWindow *window;

	desktop = e.base.widget->desktop;

	if (desktop->ignoreDockTabBarChangedEvent)
		return;

	// Figure out which dock position this tab bar is at.
	dockPosition = WZ_DOCK_POSITION_NONE;

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		if (desktop->dockTabBars[i] == e.tabBar.tabBar)
		{
			dockPosition = i;
			break;
		}
	}

	assert(dockPosition != WZ_DOCK_POSITION_NONE);

	// Get the window corresponding to the tab.
	window = (struct wzWindow *)wz_widget_get_internal_metadata((struct wzWidget *)e.tabBar.tab);

	// Set the window to visible, hide all the other windows at this dock position.
	wz_widget_set_visible((struct wzWidget *)window, true);

	for (i = 0; i < wz_arr_len(desktop->dockedWindows[dockPosition]); i++)
	{
		if (desktop->dockedWindows[dockPosition][i] == window)
			continue;

		wz_widget_set_visible((struct wzWidget *)desktop->dockedWindows[dockPosition][i], false);
	}
}

void wz_desktop_set_dock_tab_bar(struct wzDesktop *desktop, wzDockPosition dockPosition, struct wzTabBar *tabBar)
{
	assert(desktop);
	assert(tabBar);

	if (desktop->dockTabBars[dockPosition])
		return;

	desktop->dockTabBars[dockPosition] = tabBar;
	wz_widget_set_visible((struct wzWidget *)tabBar, false);
	wz_widget_set_draw_priority((struct wzWidget *)tabBar, WZ_DRAW_PRIORITY_DOCK_TAB_BAR);
	wz_widget_add_child_widget_internal((struct wzWidget *)desktop, (struct wzWidget *)tabBar);
	wz_tab_bar_add_callback_tab_changed(tabBar, wz_desktop_dock_tab_bar_tab_changed);

	// Override scroll button draw priority.
	wz_widget_set_draw_priority((struct wzWidget *)wz_tab_bar_get_decrement_button(tabBar), WZ_DRAW_PRIORITY_DOCK_TAB_BAR_SCROLL_BUTTON);
	wz_widget_set_draw_priority((struct wzWidget *)wz_tab_bar_get_increment_button(tabBar), WZ_DRAW_PRIORITY_DOCK_TAB_BAR_SCROLL_BUTTON);
}

struct wzWindow *wz_desktop_get_dock_tab_window(struct wzDesktop *desktop, struct wzButton *tab)
{
	assert(desktop);
	assert(tab);
	return (struct wzWindow *)wz_widget_get_internal_metadata((struct wzWidget *)tab);
}

static void wz_desktop_refresh_dock_tab_bar(struct wzDesktop *desktop, wzDockPosition dockPosition)
{
	struct wzTabBar *tabBar;

	assert(desktop);
	tabBar = desktop->dockTabBars[dockPosition];

	if (wz_arr_len(desktop->dockedWindows[dockPosition]) < 2)
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
		windowRect = wz_widget_get_rect((struct wzWidget *)desktop->dockedWindows[dockPosition][0]);
		tabBarHeight = wz_widget_get_height((struct wzWidget *)tabBar);

		// Assume space has already been made for the tab bar below the window.
		wz_widget_set_rect_args((struct wzWidget *)tabBar, windowRect.x, windowRect.y + windowRect.h, windowRect.w, tabBarHeight);

		// Add one tab for each window.
		for (i = 0; i < wz_arr_len(desktop->dockedWindows[dockPosition]); i++)
		{
			// Create the tab before adding it to the tab bar (i.e. use wz_tab_bar_add_existing_tab instead of wz_tab_bar_add_tab), so we have the opportunity to set tab internal metadata before adding the tab invokes a WZ_EVENT_TAB_BAR_TAB_ADDED event.
			struct wzButton *tab = wz_button_create(desktop);

			// Set the tab internal metadata to the window, so the consumer can, for example, set the tab label to the window title. See wz_desktop_get_dock_tab_window.
			wz_widget_set_internal_metadata((struct wzWidget *)tab, desktop->dockedWindows[dockPosition][i]);

			wz_tab_bar_add_existing_tab(tabBar, tab);

			// If this window is selected (visible), select the corresponding tab.
			if (wz_widget_get_visible((struct wzWidget *)desktop->dockedWindows[dockPosition][i]))
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
void wz_desktop_update_docked_window_rect(struct wzDesktop *desktop, struct wzWindow *window)
{
	wzRect rect;
	wzDockPosition i;
	int j, k;

	assert(desktop);
	assert(window);
	rect = wz_widget_get_rect((const struct wzWidget *)window);

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		for (j = 0; j < wz_arr_len(desktop->dockedWindows[i]); j++)
		{
			if (desktop->dockedWindows[i][j] == window)
			{
				for (k = 0; k < wz_arr_len(desktop->dockedWindows[i]); k++)
				{
					if (j == k)
						continue;

					wz_widget_set_rect((struct wzWidget *)desktop->dockedWindows[i][k], rect);
				}

				// Update the tab bar too.
				wz_widget_set_rect_args((struct wzWidget *)desktop->dockTabBars[i], rect.x, rect.y + rect.h, rect.w, wz_widget_get_height((struct wzWidget *)desktop->dockTabBars[i]));
				return;
			}
		}
	}
}

// Update the rects of docked windows and dock tab bars.
static void wz_desktop_update_docking_rects(struct wzDesktop *desktop)
{
	wzDockPosition i;

	assert(desktop);

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		int nWindows;
		wzRect tabBarRect, windowRect;
		int j;

		nWindows = wz_arr_len(desktop->dockedWindows[i]);

		if (nWindows == 0)
			continue;

		// Calculate the window rect. All windows will have the same rect, so just use the first one as a basis.
		windowRect = wz_widget_get_rect((struct wzWidget *)desktop->dockedWindows[i][0]);

		switch (i)
		{
		case WZ_DOCK_POSITION_NORTH:
			windowRect.w = desktop->base.rect.w;
			break;
		case WZ_DOCK_POSITION_SOUTH:
			windowRect.y = desktop->base.rect.h - windowRect.h;
			windowRect.w = desktop->base.rect.w;
			break;
		case WZ_DOCK_POSITION_EAST:
			windowRect.x = desktop->base.rect.w - windowRect.w;
			windowRect.h = desktop->base.rect.h;
			break;
		case WZ_DOCK_POSITION_WEST:
			windowRect.h = desktop->base.rect.h;
			break;
		}

		// Update tab bar rect. Adjust window rect to make space for the tab bar.
		if (nWindows > 1)
		{
			tabBarRect = wz_widget_get_rect((struct wzWidget *)desktop->dockTabBars[i]);

			switch (i)
			{
			case WZ_DOCK_POSITION_NORTH:
				tabBarRect.w = desktop->base.rect.w;
				break;
			case WZ_DOCK_POSITION_SOUTH:
				tabBarRect.y = desktop->base.rect.h - tabBarRect.h;
				tabBarRect.w = desktop->base.rect.w;
				windowRect.h -= tabBarRect.h;
				break;
			case WZ_DOCK_POSITION_EAST:
				tabBarRect.x = desktop->base.rect.w - windowRect.w;
				tabBarRect.y = desktop->base.rect.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			case WZ_DOCK_POSITION_WEST:
				tabBarRect.y = desktop->base.rect.h - tabBarRect.h;
				windowRect.h -= tabBarRect.h;
				break;
			}

			wz_widget_set_rect((struct wzWidget *)desktop->dockTabBars[i], tabBarRect);
		}

		// Set window rects.
		for (j = 0; j < wz_arr_len(desktop->dockedWindows[i]); j++)
		{
			wz_widget_set_rect((struct wzWidget *)desktop->dockedWindows[i][j], windowRect);
		}
	}
}

static void wz_desktop_dock_window(struct wzDesktop *desktop, struct wzWindow *window)
{
	int i;

	assert(desktop);
	assert(window);

	// Hide any other windows docked at the same position.
	for (i = 0; i < wz_arr_len(desktop->dockedWindows[desktop->windowDockPosition]); i++)
	{
		wz_widget_set_visible((struct wzWidget *)desktop->dockedWindows[desktop->windowDockPosition][i], false);
	}

	// Dock the window.
	wz_arr_push(desktop->dockedWindows[desktop->windowDockPosition], window);

	// Inform the window it is being docked.
	wz_window_dock(window);

	// Resize the window to match the dock preview.
	wz_widget_set_rect((struct wzWidget *)window, wz_widget_get_rect((struct wzWidget *)desktop->dockPreview));

	// Resize the other windows docked at this position to match.
	wz_desktop_update_docked_window_rect(desktop, window);

	// Refresh the tab bar for this dock position.
	desktop->ignoreDockTabBarChangedEvent = true;
	wz_desktop_refresh_dock_tab_bar(desktop, desktop->windowDockPosition);
	desktop->ignoreDockTabBarChangedEvent = false;

	// Docked windows affect the desktop content rect, so update it.
	wz_desktop_update_content_rect(desktop);
}

wzDockPosition wz_desktop_get_window_dock_position(const struct wzDesktop *desktop, const struct wzWindow *window)
{
	wzDockPosition i;
	int j;

	assert(desktop);
	assert(window);

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		for (j = 0; j < wz_arr_len(desktop->dockedWindows[i]); j++)
		{
			if (desktop->dockedWindows[i][j] == window)
			{
				return i;
			}
		}
	}

	return WZ_DOCK_POSITION_NONE;
}

void wz_desktop_undock_window(struct wzDesktop *desktop, struct wzWindow *window)
{
	wzDockPosition dockPosition, i;
	int j, windowIndex;
	int nDockedWindows;

	assert(desktop);
	assert(window);

	// Find the dock position for the window, and the window index.
	dockPosition = WZ_DOCK_POSITION_NONE;
	windowIndex = -1;

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		for (j = 0; j < wz_arr_len(desktop->dockedWindows[i]); j++)
		{
			if (desktop->dockedWindows[i][j] == window)
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

	wz_arr_delete(desktop->dockedWindows[i], windowIndex);
	nDockedWindows = wz_arr_len(desktop->dockedWindows[dockPosition]);

	// If there are other windows docked at this position, make sure one is visible after removing this window.
	if (wz_widget_get_visible((const struct wzWidget *)window) && nDockedWindows > 0)
	{
		wz_widget_set_visible((struct wzWidget *)desktop->dockedWindows[dockPosition][0], true);
	}

	// Refresh the tab bar for this dock position.
	wz_desktop_refresh_dock_tab_bar(desktop, dockPosition);

	// If the dock tab bar is hidden, resize the windows at this dock position to reclaim the space it used.
	if (!wz_widget_get_visible((const struct wzWidget *)desktop->dockTabBars[dockPosition]))
	{
		for (j = 0; j < wz_arr_len(desktop->dockedWindows[dockPosition]); j++)
		{
			struct wzWidget *widget = (struct wzWidget *)desktop->dockedWindows[dockPosition][j];

			if (dockPosition == WZ_DOCK_POSITION_SOUTH)
			{
				wz_widget_set_height(widget, widget->rect.h + (desktop->base.rect.h - (widget->rect.y + widget->rect.h)));
			}
			else if (dockPosition == WZ_DOCK_POSITION_EAST || dockPosition == WZ_DOCK_POSITION_WEST)
			{
				wz_widget_set_height(widget, desktop->base.rect.h);
			}
		}
	}
}

/*
================================================================================

DOCK ICON

================================================================================
*/

static void wz_desktop_draw_dock_icon(struct wzWidget *widget, wzRect clip)
{
	assert(widget);
	clip = clip; // Never clipped, so just ignore that parameter.

	if (widget->desktop->draw_dock_icon)
	{
		int i;

		for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
		{
			widget->desktop->draw_dock_icon(wz_widget_get_rect((struct wzWidget *)widget->desktop->dockIcons[i]), wz_widget_get_metadata((struct wzWidget *)widget->desktop->dockIcons[i]));
		}
	}
}

static void wz_desktop_update_dock_icon_positions(struct wzDesktop *desktop)
{
	// Push icons out this percent/100 from desktop edges.
	const float percent = 0.04f;
	wzSize ds, dis;
	int centerW, centerH;

	assert(desktop);
	ds = wz_widget_get_size((struct wzWidget *)desktop);
	dis = wz_widget_get_size((struct wzWidget *)desktop->dockIcons[WZ_DOCK_POSITION_NORTH]);
	centerW = (int)(ds.w / 2.0f - dis.w / 2.0f);
	centerH = (int)(ds.h / 2.0f - dis.h / 2.0f);

	wz_widget_set_position_args((struct wzWidget *)desktop->dockIcons[WZ_DOCK_POSITION_NORTH], centerW, (int)(ds.h * percent));
	wz_widget_set_position_args((struct wzWidget *)desktop->dockIcons[WZ_DOCK_POSITION_SOUTH], centerW, (int)(ds.h * (1.0f - percent) - dis.h));
	wz_widget_set_position_args((struct wzWidget *)desktop->dockIcons[WZ_DOCK_POSITION_EAST], (int)(ds.w * (1.0f - percent) - dis.h), centerH);
	wz_widget_set_position_args((struct wzWidget *)desktop->dockIcons[WZ_DOCK_POSITION_WEST], (int)(ds.w * percent), centerH);
}

void wz_desktop_set_draw_dock_icon_callback(struct wzDesktop *desktop, wzDesktopDrawDockIconCallback callback, void *metadata)
{
	int i;

	assert(desktop);
	desktop->draw_dock_icon = callback;

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wz_widget_set_metadata((struct wzWidget *)desktop->dockIcons[i], metadata);
	}
}

void wz_desktop_set_dock_icon_size(struct wzDesktop *desktop, wzSize size)
{
	wz_desktop_set_dock_icon_size_args(desktop, size.w, size.h);
}

void wz_desktop_set_dock_icon_size_args(struct wzDesktop *desktop, int w, int h)
{
	int i;

	assert(desktop);

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wz_widget_set_size_args((struct wzWidget *)desktop->dockIcons[i], w, h);
	}

	wz_desktop_update_dock_icon_positions(desktop);
}

/*
================================================================================

DOCK PREVIEW

================================================================================
*/

static void wz_desktop_draw_dock_preview(struct wzWidget *widget, wzRect clip)
{
	assert(widget);
	clip = clip; // Never clipped, so just ignore that parameter.

	if (widget->desktop->draw_dock_preview)
	{
		widget->desktop->draw_dock_preview(wz_widget_get_rect((struct wzWidget *)widget->desktop->dockPreview), wz_widget_get_metadata((struct wzWidget *)widget->desktop->dockPreview));
	}
}

static void wz_widget_update_dock_preview_rect(struct wzDesktop *desktop, wzDockPosition dockPosition)
{
	int nDockedWindows;

	// e.g. north dock max height is desktop height * maxPreviewSizeMultiplier.
	const float maxPreviewSizeMultiplier = 0.3f;

	assert(desktop);
	assert(desktop->movingWindow);

	// If there's already a window docked at this position, set the dock preview rect to that size.
	nDockedWindows = wz_arr_len(desktop->dockedWindows[dockPosition]);

	if (nDockedWindows > 0)
	{
		wzRect rect = wz_widget_get_rect((struct wzWidget *)desktop->dockedWindows[dockPosition][0]);

		// If there's exactly one window already docked at this position, leave room for the dock tab bar.
		if (nDockedWindows == 1)
		{
			rect.h -= wz_widget_get_height((struct wzWidget *)desktop->dockTabBars[dockPosition]);
		}

		wz_widget_set_rect((struct wzWidget *)desktop->dockPreview, rect);
	}
	else
	{
		wzSize windowSize;
		wzRect rect;

		// Use the window width for east/west or height for north/south, but don't go over half of the desktop width/height.
		windowSize = wz_widget_get_size((struct wzWidget *)desktop->movingWindow);

		if (dockPosition == WZ_DOCK_POSITION_NORTH)
		{
			rect.x = 0;
			rect.y = 0;
			rect.w = desktop->base.rect.w;
			rect.h = WZ_MIN(windowSize.h, (int)(desktop->base.rect.h * maxPreviewSizeMultiplier));
		}
		else if (dockPosition == WZ_DOCK_POSITION_SOUTH)
		{
			const int h = WZ_MIN(windowSize.h, (int)(desktop->base.rect.h * maxPreviewSizeMultiplier));
			rect.x = 0;
			rect.y = desktop->base.rect.h - h;
			rect.w = desktop->base.rect.w;
			rect.h = h;
		}
		else if (dockPosition == WZ_DOCK_POSITION_EAST)
		{
			const int w = WZ_MIN(windowSize.w, (int)(desktop->base.rect.w * maxPreviewSizeMultiplier));
			rect.x = desktop->base.rect.w - w;
			rect.y = 0;
			rect.w = w;
			rect.h = desktop->base.rect.h;
		}
		else if (dockPosition == WZ_DOCK_POSITION_WEST)
		{
			rect.x = 0;
			rect.y = 0;
			rect.w = WZ_MIN(windowSize.w, (int)(desktop->base.rect.w * maxPreviewSizeMultiplier));
			rect.h = desktop->base.rect.h;
		}

		wz_widget_set_rect((struct wzWidget *)desktop->dockPreview, rect);
	}
}

static void wz_desktop_update_dock_preview_visible(struct wzDesktop *desktop, int mouseX, int mouseY)
{
	wzDockPosition i;
	bool showDockPreview = false;

	assert(desktop);

	if (!desktop->movingWindow)
		return;

	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wzRect rect = wz_widget_get_rect((struct wzWidget *)desktop->dockIcons[i]);

		if (WZ_POINT_IN_RECT(mouseX, mouseY, rect))
		{
			desktop->windowDockPosition = i;
			wz_widget_update_dock_preview_rect(desktop, i);
			showDockPreview = true;
			break;
		}
	}

	wz_widget_set_visible((struct wzWidget *)desktop->dockPreview, showDockPreview);
}

void wz_desktop_set_draw_dock_preview_callback(struct wzDesktop *desktop, wzDesktopDrawDockPreviewCallback callback, void *metadata)
{
	assert(desktop);
	desktop->draw_dock_preview = callback;
	wz_widget_set_metadata((struct wzWidget *)desktop->dockPreview, metadata);
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
static void wz_desktop_update_window_draw_priorities(struct wzDesktop *desktop, struct wzWindow *top)
{
	struct wzWindow *windows[WZ_MAX_WINDOWS];
	int nWindows;
	int i;

	assert(desktop);

	// Get a list of windows (excluding top).
	nWindows = 0;

	for (i = 0; i < wz_arr_len(desktop->base.children); i++)
	{
		struct wzWidget *widget = desktop->base.children[i];
	
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

	assert(widget);

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

void wz_desktop_mouse_button_down(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	assert(desktop);

	desktop->lockInputWindow = wz_desktop_get_hover_window(desktop, mouseX, mouseY);

	if (wz_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[wz_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		wz_desktop_update_window_draw_priorities(desktop, desktop->lockInputWindow);
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_mouse_button_down_recursive(widget, mouseButton, mouseX, mouseY);

	// Need a special case for dock icons.
	wz_desktop_update_dock_preview_visible(desktop, mouseX, mouseY);
}

/*
================================================================================

MOUSE BUTTON UP

================================================================================
*/

static void wz_widget_mouse_button_up_recursive(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	int i;

	assert(widget);

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

void wz_desktop_mouse_button_up(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	assert(desktop);

	// Need a special case for dock icons.
	if (desktop->movingWindow)
	{
		// If the dock preview is visible, movingWindow can be docked.
		if (wz_widget_get_visible((struct wzWidget *)desktop->dockPreview))
		{
			wz_desktop_dock_window(desktop, desktop->movingWindow);
		}

		wz_widget_set_visible((struct wzWidget *)desktop->dockPreview, false);
		desktop->movingWindow = NULL;
	}

	if (wz_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[wz_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_mouse_button_up_recursive(widget, mouseButton, mouseX, mouseY);
}

/*
================================================================================

MOUSE MOVE

================================================================================
*/

// Sets wzWidget.ignore
static void wz_widget_ignore_overlapping_children(struct wzWidget *widget, int mouseX, int mouseY)
{
	int i, j;

	assert(widget);

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

	assert(widget);

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
		hoverWindow = WZ_POINT_IN_RECT(mouseX, mouseY, wz_widget_get_absolute_rect(wz_widget_get_content_widget((struct wzWidget *)widget->window)));
	}
	else
	{
		hoverWindow = true;
	}

	// Determine whether the mouse is hovering over the widget's parent.
	if (!widget->inputNotClippedToParent && widget->parent && widget->parent != (struct wzWidget *)widget->desktop && widget->parent != (struct wzWidget *)widget->window)
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
		if (widget->hover || (widgetIsChildOfWindow && wz_arr_len(widget->desktop->lockInputWidgetStack) > 0 && widget == widget->desktop->lockInputWidgetStack[wz_arr_lastn(widget->desktop->lockInputWidgetStack)]))
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

void wz_desktop_mouse_move(struct wzDesktop *desktop, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	assert(desktop);

	// Reset the mouse cursor to default.
	desktop->cursor = WZ_CURSOR_DEFAULT;

	// Need a special case for dock icons.
	wz_desktop_update_dock_preview_visible(desktop, mouseX, mouseY);

	if (wz_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		wz_widget_mouse_move_recursive(NULL, desktop->lockInputWidgetStack[wz_arr_lastn(desktop->lockInputWidgetStack)], mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		return;
	}

	desktop->lockInputWindow = wz_desktop_get_hover_window(desktop, mouseX, mouseY);
	wz_widget_mouse_move_recursive(desktop->lockInputWindow, (struct wzWidget *)desktop, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
}

/*
================================================================================

MOUSE WHEEL MOVE

================================================================================
*/

static void wz_widget_mouse_wheel_move_recursive(struct wzWidget *widget, int x, int y)
{
	int i;

	assert(widget);

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

void wz_desktop_mouse_wheel_move(struct wzDesktop *desktop, int x, int y)
{
	struct wzWidget *widget;

	assert(desktop);

	if (wz_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[wz_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_mouse_wheel_move_recursive(widget, x, y);
}

/*
================================================================================

KEY DOWN AND UP

================================================================================
*/

static void wz_widget_key_recursive(struct wzWidget *widget, wzKey key, bool down)
{
	int i;

	assert(widget);

	if (!wz_widget_get_visible(widget))
		return;

	if (down && widget->vtable.key_down)
	{
		widget->vtable.key_down(widget, key);
	}
	else if (!down && widget->vtable.key_up)
	{
		widget->vtable.key_up(widget, key);
	}

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		if (widget->children[i]->hover)
		{
			wz_widget_key_recursive(widget->children[i], key, down);
		}
	}
}

static void wz_desktop_key(struct wzDesktop *desktop, wzKey key, bool down)
{
	struct wzWidget *widget;

	assert(desktop);

	if (wz_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[wz_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_key_recursive(widget, key, down);
}

void wz_desktop_key_down(struct wzDesktop *desktop, wzKey key)
{
	if (key == WZ_KEY_UNKNOWN)
		return;

	wz_desktop_key(desktop, key, true);
}

void wz_desktop_key_up(struct wzDesktop *desktop, wzKey key)
{
	if (key == WZ_KEY_UNKNOWN)
		return;

	wz_desktop_key(desktop, key, false);
}

/*
================================================================================

TEXT INPUT

================================================================================
*/

static void wz_widget_text_input_recursive(struct wzWidget *widget, const char *text)
{
	int i;

	assert(widget);

	if (!wz_widget_get_visible(widget))
		return;

	if (widget->vtable.text_input)
	{
		widget->vtable.text_input(widget, text);
	}

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		if (widget->children[i]->hover)
		{
			wz_widget_text_input_recursive(widget->children[i], text);
		}
	}
}

void wz_desktop_text_input(struct wzDesktop *desktop, const char *text)
{
	struct wzWidget *widget;

	assert(desktop);

	if (wz_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[wz_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_text_input_recursive(widget, text);
}

/*
================================================================================

DRAWING

================================================================================
*/

void wz_desktop_draw(struct wzDesktop *desktop)
{
	wz_widget_draw_desktop(desktop);
}

/*
================================================================================

MISC.

================================================================================
*/

static void wz_desktop_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzDesktop *desktop;

	assert(widget);
	desktop = (struct wzDesktop *)widget;
	desktop->base.rect.w = rect.w;
	desktop->base.rect.h = rect.h;
	wz_desktop_update_dock_icon_positions(desktop);
	wz_desktop_update_docking_rects(desktop);
	wz_desktop_update_content_rect(desktop);
}

static struct wzWidget *wz_desktop_get_content_widget(struct wzWidget *widget)
{
	assert(widget);
	return ((struct wzDesktop *)widget)->content;
}

void wz_desktop_set_measure_text_callback(struct wzDesktop *desktop, wzDesktopMeasureTextCallback callback)
{
	assert(desktop);
	desktop->measure_text = callback;
}

void wz_desktop_set_text_get_pixel_delta_callback(struct wzDesktop *desktop, wzDesktopTextGetPixelDeltaCallback callback)
{
	assert(desktop);
	desktop->text_get_pixel_delta = callback;
}

struct wzDesktop *wz_desktop_create()
{
	struct wzDesktop *desktop;
	int i;
	struct wzWidget *widget;

	desktop = (struct wzDesktop *)malloc(sizeof(struct wzDesktop));
	memset(desktop, 0, sizeof(struct wzDesktop));
	desktop->base.type = WZ_TYPE_DESKTOP;
	desktop->base.desktop = desktop;
	desktop->base.vtable.set_rect = wz_desktop_set_rect;
	desktop->base.vtable.get_content_widget = wz_desktop_get_content_widget;

	// Create content widget.
	desktop->content = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(desktop->content, 0, sizeof(struct wzWidget));
	desktop->content->desktop = desktop;
	wz_widget_add_child_widget_internal((struct wzWidget *)desktop, desktop->content);

	// Create dock icon widgets.
	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		desktop->dockIcons[i] = wz_label_create(desktop);
		widget = (struct wzWidget *)desktop->dockIcons[i];
		wz_widget_set_draw_priority(widget, WZ_DRAW_PRIORITY_DOCK_ICON);
		wz_widget_set_draw_function(widget, wz_desktop_draw_dock_icon);
		wz_widget_set_visible(widget, false);
		wz_widget_add_child_widget_internal((struct wzWidget *)desktop, widget);
	}

	wz_desktop_set_dock_icon_size_args(desktop, 48, 48);
	wz_desktop_update_dock_icon_positions(desktop);

	// Create dock preview widget.
	desktop->dockPreview = wz_label_create(desktop);
	widget = (struct wzWidget *)desktop->dockPreview;
	wz_widget_set_draw_priority(widget, WZ_DRAW_PRIORITY_DOCK_PREVIEW);
	wz_widget_set_draw_function(widget, wz_desktop_draw_dock_preview);
	wz_widget_set_visible(widget, false);
	wz_widget_add_child_widget_internal((struct wzWidget *)desktop, widget);

	return desktop;
}

void wz_desktop_set_event_callback(struct wzDesktop *desktop, wzEventCallback callback)
{
	assert(desktop);
	desktop->handle_event = callback;
}

wzCursor wz_desktop_get_cursor(const struct wzDesktop *desktop)
{
	assert(desktop);
	return desktop->cursor;
}

void wz_desktop_set_cursor(struct wzDesktop *desktop, wzCursor cursor)
{
	assert(desktop);
	desktop->cursor = cursor;
}

void wz_desktop_push_lock_input_widget(struct wzDesktop *desktop, struct wzWidget *widget)
{
	assert(desktop);
	wz_arr_push(desktop->lockInputWidgetStack, widget);
}

void wz_desktop_pop_lock_input_widget(struct wzDesktop *desktop, struct wzWidget *widget)
{
	// Only pop if the widget is on the top of the stack.
	if (wz_arr_len(desktop->lockInputWidgetStack) == 0)
		return;

	if (widget == desktop->lockInputWidgetStack[wz_arr_lastn(desktop->lockInputWidgetStack)])
	{	
		wz_arr_pop(desktop->lockInputWidgetStack);
	}
}

void wz_desktop_set_moving_window(struct wzDesktop *desktop, struct wzWindow *window)
{
	int i;

	assert(desktop);
	desktop->movingWindow = window;

	// Show the dock icons if movingWindow is not NULL.
	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wz_widget_set_visible((struct wzWidget *)desktop->dockIcons[i], desktop->movingWindow != NULL);
	}
}

void wz_invoke_event(wzEvent e, wzEventCallback *callbacks)
{
	int i;

	if (e.base.widget->desktop && e.base.widget->desktop->handle_event)
	{
		e.base.widget->desktop->handle_event(e);
	}

	for (i = 0; i < wz_arr_len(callbacks); i++)
	{
		callbacks[i](e);
	}
}

void wz_desktop_update_content_rect(struct wzDesktop *desktop)
{
	wzRect rect;
	wzDockPosition i;

	assert(desktop);
	rect = desktop->base.rect;

	// Adjust the content rect based on docked windows.
	for (i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		wzRect windowRect;

		if (wz_arr_len(desktop->dockedWindows[i]) == 0)
			continue;

		windowRect = wz_widget_get_rect((struct wzWidget *)desktop->dockedWindows[i][0]);

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

	wz_widget_set_rect(desktop->content, rect);
}

void wz_desktop_measure_text(struct wzDesktop *desktop, const char *text, int n, int *width, int *height)
{
	desktop->measure_text(desktop, text, n, width, height);
}

int wz_desktop_text_get_pixel_delta(struct wzDesktop *desktop, const char *text, int index)
{
	return desktop->text_get_pixel_delta(desktop, text, index);
}
