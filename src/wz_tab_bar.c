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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "wz_desktop.h"
#include "wz_widget.h"

struct wzTabBar
{
	struct wzWidget base;
	struct wzButton *selectedTab;
	struct wzButton **tabs;

	int scrollValue;
	struct wzButton *decrementButton;
	struct wzButton *incrementButton;

	wzEventCallback *tab_changed_callbacks;
};

// Ensure tab height stay the same as the tab bar.
static void wz_tab_set_rect(struct wzWidget *widget, wzRect rect)
{
	assert(widget);
	rect.h = ((struct wzTabBar *)widget->parent)->base.rect.h;
	widget->rect = rect;
}

// Show the scroll buttons if they're required, hides them if they're not.
static void wz_tab_bar_update_scroll_buttons(struct wzTabBar *tabBar)
{
	int i, totalTabWidth;
	bool wereScrollButtonsVisible, showScrollButtons;
	wzRect rect;

	assert(tabBar);

	if (!tabBar->decrementButton || !tabBar->incrementButton)
		return;

	// Total tab widths.
	totalTabWidth = 0;

	for (i = 0; i < wz_arr_len(tabBar->tabs); i++)
	{
		wzSize size = wz_widget_get_size((struct wzWidget *)tabBar->tabs[i]);
		totalTabWidth += size.w;
	}

	// Show/hide the scroll buttons and set their rects.
	wereScrollButtonsVisible = wz_widget_get_visible((struct wzWidget *)tabBar->decrementButton);
	showScrollButtons = totalTabWidth > tabBar->base.rect.w;

	rect.w = wz_widget_get_size((struct wzWidget *)tabBar->decrementButton).w;
	rect.x = tabBar->base.rect.w - rect.w * 2;
	rect.y = 0;
	rect.h = tabBar->base.rect.h;
	wz_widget_set_rect_internal((struct wzWidget *)tabBar->decrementButton, rect);
	wz_widget_set_visible((struct wzWidget *)tabBar->decrementButton, showScrollButtons);

	rect.w = wz_widget_get_size((struct wzWidget *)tabBar->incrementButton).w;
	rect.x = tabBar->base.rect.w - rect.w;
	rect.y = 0;
	rect.h = tabBar->base.rect.h;
	wz_widget_set_rect_internal((struct wzWidget *)tabBar->incrementButton, rect);
	wz_widget_set_visible((struct wzWidget *)tabBar->incrementButton, showScrollButtons);

	if (wereScrollButtonsVisible && showScrollButtons)
	{
		tabBar->scrollValue = 0;
	}
}

static void wz_tab_bar_update_tabs(struct wzTabBar *tabBar)
{
	int x, i;

	assert(tabBar);

	// Start at the left edge of the tab bar.
	x = 0;

	for (i = 0; i < wz_arr_len(tabBar->tabs); i++)
	{
		struct wzWidget *widget = (struct wzWidget *)tabBar->tabs[i];

		if (i < tabBar->scrollValue)
		{
			// Scrolled out of view, hide it.
			wz_widget_set_visible(widget, false);
			continue;
		}
		else
		{
			// Reposition and show.
			wzRect rect;
			rect.x = x;
			rect.y = 0;
			rect.w = widget->rect.w;
			rect.h = tabBar->base.rect.h;
			wz_widget_set_rect_internal(widget, rect);
			wz_widget_set_visible(widget, true);
			x += rect.w;
		}
	}
}

// Sets the scroll value, and repositions and shows/hides the tabs accordingly.
static void wz_tab_bar_set_scroll_value(struct wzTabBar *tabBar, int value)
{
	int oldValue = tabBar->scrollValue;
	tabBar->scrollValue = WZ_CLAMPED(0, value, WZ_MAX(0, wz_arr_len(tabBar->tabs) - 1));

	if (oldValue != tabBar->scrollValue)
	{
		// Value has changed.
		wz_tab_bar_update_tabs(tabBar);
	}
}

static void wz_tab_bar_invoke_tab_changed(struct wzTabBar *tabBar)
{
	wzEvent e;

	assert(tabBar);
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_CHANGED;
	e.tabBar.tabBar = tabBar;
	e.tabBar.tab = tabBar->selectedTab;
	wz_invoke_event(&e, tabBar->tab_changed_callbacks);
}

// Sets the tab bar (the button's parent) selected tab.
static void wz_tab_bar_button_pressed(wzEvent *e)
{
	assert(e);
	wz_tab_bar_select_tab((struct wzTabBar *)e->base.widget->parent, (struct wzButton *)e->base.widget);
}

static void wz_tab_bar_destroy(struct wzWidget *widget)
{
	struct wzTabBar *tabBar;

	assert(widget);
	tabBar = (struct wzTabBar *)widget;
	wz_arr_free(tabBar->tabs);
	wz_arr_free(tabBar->tab_changed_callbacks);
}

static void wz_tab_bar_set_rect(struct wzWidget *widget, wzRect rect)
{
	int i;

	assert(widget);
	widget->rect = rect;

	// Set button heights to match.
	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_set_height_internal(widget->children[i], rect.h);
	}

	wz_tab_bar_update_tabs((struct wzTabBar *)widget);
	wz_tab_bar_update_scroll_buttons((struct wzTabBar *)widget);
}

static wzRect wz_tab_bar_get_children_clip_rect(struct wzWidget *widget)
{
	return wz_widget_get_absolute_rect(widget);
}

static void wz_tab_bar_decrement_button_clicked(wzEvent *e)
{
	struct wzTabBar *tabBar;

	assert(e);
	assert(e->base.widget);
	assert(e->base.widget->parent);
	tabBar = (struct wzTabBar *)e->base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue - 1);
}

static void wz_tab_bar_increment_button_clicked(wzEvent *e)
{
	struct wzTabBar *tabBar;

	assert(e);
	assert(e->base.widget);
	assert(e->base.widget->parent);
	tabBar = (struct wzTabBar *)e->base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue + 1);
}

struct wzTabBar *wz_tab_bar_create(struct wzButton *decrementButton, struct wzButton *incrementButton)
{
	struct wzTabBar *tabBar;
	int defaultScrollButtonWidth;

	assert(decrementButton);
	assert(incrementButton);

	tabBar = (struct wzTabBar *)malloc(sizeof(struct wzTabBar));
	memset(tabBar, 0, sizeof(struct wzTabBar));
	tabBar->base.type = WZ_TYPE_TAB_BAR;
	tabBar->base.vtable.destroy = wz_tab_bar_destroy;
	tabBar->base.vtable.set_rect = wz_tab_bar_set_rect;
	tabBar->base.vtable.get_children_clip_rect = wz_tab_bar_get_children_clip_rect;

	// Custom draw priority so the scroll buttons always overlap the tabs.
	defaultScrollButtonWidth = 14;
	tabBar->decrementButton = decrementButton;
	wz_button_add_callback_clicked(tabBar->decrementButton, wz_tab_bar_decrement_button_clicked);
	wz_widget_add_child_widget_internal((struct wzWidget *)tabBar, (struct wzWidget *)tabBar->decrementButton);
	wz_widget_set_width_internal((struct wzWidget *)tabBar->decrementButton, defaultScrollButtonWidth);
	wz_widget_set_visible((struct wzWidget *)tabBar->decrementButton, false);
	wz_widget_set_draw_priority((struct wzWidget *)tabBar->decrementButton, WZ_DRAW_PRIORITY_TAB_BAR_SCROLL_BUTTON);

	tabBar->incrementButton = incrementButton;
	wz_button_add_callback_clicked(tabBar->incrementButton, wz_tab_bar_increment_button_clicked);
	wz_widget_add_child_widget_internal((struct wzWidget *)tabBar, (struct wzWidget *)tabBar->incrementButton);
	wz_widget_set_width_internal((struct wzWidget *)tabBar->incrementButton, defaultScrollButtonWidth);
	wz_widget_set_visible((struct wzWidget *)tabBar->incrementButton, false);
	wz_widget_set_draw_priority((struct wzWidget *)tabBar->incrementButton, WZ_DRAW_PRIORITY_TAB_BAR_SCROLL_BUTTON);

	return tabBar;
}

void wz_tab_bar_add_tab(struct wzTabBar *tabBar, struct wzButton *tab)
{
	wzRect rect;
	int i;
	wzEvent e;

	assert(tabBar);
	assert(tab);

	// Position to the right of the last tab.
	rect.x = rect.y = 0;
	rect.w = 50; // Default width.
	rect.h = tabBar->base.rect.h;

	for (i = 0; i < wz_arr_len(tabBar->tabs); i++)
	{
		wzSize size = wz_widget_get_size((struct wzWidget *)tabBar->tabs[i]);
		rect.x += size.w;
	}

	((struct wzWidget *)tab)->vtable.set_rect = wz_tab_set_rect;
	wz_button_add_callback_pressed(tab, wz_tab_bar_button_pressed);
	wz_button_set_click_behavior(tab, WZ_BUTTON_CLICK_BEHAVIOR_DOWN);
	wz_button_set_set_behavior(tab, WZ_BUTTON_SET_BEHAVIOR_STICKY);
	wz_widget_add_child_widget_internal((struct wzWidget *)tabBar, (struct wzWidget *)tab);
	wz_arr_push(tabBar->tabs, tab);
	wz_widget_set_rect_internal((struct wzWidget *)tab, rect);

	// Select the first tab added.
	if (!tabBar->selectedTab)
	{
		wz_button_set(tab, true);
		tabBar->selectedTab = tab;
		wz_tab_bar_invoke_tab_changed(tabBar);
	}

	wz_tab_bar_update_scroll_buttons(tabBar);

	// Invoke the tab added event.
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_ADDED;
	e.tabBar.tabBar = tabBar;
	e.tabBar.tab = tab;
	wz_invoke_event(&e, NULL);
}

void wz_tab_bar_remove_tab(struct wzTabBar *tabBar, struct wzButton *tab)
{
	int i, deleteIndex;
	wzEvent e;

	assert(tabBar);
	assert(tab);
	deleteIndex = -1;

	for (i = 0; i < wz_arr_len(tabBar->tabs); i++)
	{
		if (tabBar->tabs[i] == tab)
		{
			deleteIndex = i;
			break;
		}
	}

	if (deleteIndex == -1)
		return;

	// Invoke the tab removed event.
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_REMOVED;
	e.tabBar.tabBar = tabBar;
	e.tabBar.tab = tabBar->tabs[deleteIndex];
	wz_invoke_event(&e, NULL);

	// Remove our references to the tab.
	wz_arr_delete(tabBar->tabs, deleteIndex);
	wz_widget_remove_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)tab);

	// Invoke the destroy widget event.
	e.destroy.type = WZ_EVENT_DESTROY_WIDGET;
	e.destroy.parent = (struct wzWidget *)tabBar;
	e.destroy.widget = (struct wzWidget *)tab;
	wz_invoke_event(&e, NULL);
}

void wz_tab_bar_clear_tabs(struct wzTabBar *tabBar)
{
	int i;

	assert(tabBar);

	for (i = 0; i < wz_arr_len(tabBar->tabs); i++)
	{
		wzEvent e;

		// Invoke the tab removed event.
		e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_REMOVED;
		e.tabBar.tabBar = tabBar;
		e.tabBar.tab = tabBar->tabs[i];
		wz_invoke_event(&e, NULL);

		wz_widget_remove_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)tabBar->tabs[i]);

		// Invoke the destroy widget event.
		e.destroy.type = WZ_EVENT_DESTROY_WIDGET;
		e.destroy.parent = (struct wzWidget *)tabBar;
		e.destroy.widget = (struct wzWidget *)tabBar->tabs[i];
		wz_invoke_event(&e, NULL);
	}

	wz_arr_deleten(tabBar->tabs, 0, wz_arr_len(tabBar->tabs));
	tabBar->selectedTab = NULL;
	tabBar->scrollValue = 0;
	wz_widget_set_visible((struct wzWidget *)tabBar->decrementButton, false);
	wz_widget_set_visible((struct wzWidget *)tabBar->incrementButton, false);
}

struct wzButton *wz_tab_bar_get_decrement_button(struct wzTabBar *tabBar)
{
	assert(tabBar);
	return tabBar->decrementButton;
}

struct wzButton *wz_tab_bar_get_increment_button(struct wzTabBar *tabBar)
{
	assert(tabBar);
	return tabBar->incrementButton;
}

struct wzButton *wz_tab_bar_get_selected_tab(struct wzTabBar *tabBar)
{
	assert(tabBar);
	return tabBar->selectedTab;
}

void wz_tab_bar_select_tab(struct wzTabBar *tabBar, struct wzButton *tab)
{
	int i;

	assert(tabBar);
	assert(tab);

	if (tabBar->selectedTab == tab)
		return; // Already selected.

	wz_button_set(tab, true);
	tabBar->selectedTab = tab;

	// Unset all the other tab bar buttons.
	for (i = 0; i < wz_arr_len(tabBar->tabs); i++)
	{
		if (tabBar->tabs[i] != tabBar->selectedTab)
		{
			wz_button_set(tabBar->tabs[i], false);
		}
	}
	
	wz_tab_bar_invoke_tab_changed(tabBar);
}

void wz_tab_bar_add_callback_tab_changed(struct wzTabBar *tabBar, wzEventCallback callback)
{
	assert(tabBar);
	wz_arr_push(tabBar->tab_changed_callbacks, callback);
}
