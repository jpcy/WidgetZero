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
#include "wz_internal.h"

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

// Show the scroll buttons if they're required, hides them if they're not.
static void wz_tab_bar_update_scroll_buttons(struct wzTabBar *tabBar)
{
	int i, totalTabWidth;
	bool wereScrollButtonsVisible, showScrollButtons;
	wzRect rect;

	assert(tabBar);

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

	rect = tabBar->base.rect;
	rect.w = wz_widget_get_size((struct wzWidget *)tabBar->decrementButton).w;
	rect.x += tabBar->base.rect.w - rect.w * 2;
	wz_widget_set_rect((struct wzWidget *)tabBar->decrementButton, rect);
	wz_widget_set_visible((struct wzWidget *)tabBar->decrementButton, showScrollButtons);

	rect = tabBar->base.rect;
	rect.w = wz_widget_get_size((struct wzWidget *)tabBar->incrementButton).w;
	rect.x += tabBar->base.rect.w - rect.w;
	wz_widget_set_rect((struct wzWidget *)tabBar->incrementButton, rect);
	wz_widget_set_visible((struct wzWidget *)tabBar->incrementButton, showScrollButtons);

	if (wereScrollButtonsVisible && showScrollButtons)
	{
		tabBar->scrollValue = 0;
	}
}

// Sets the scroll value, and repositions and shows/hides the tabs accordingly.
static void wz_tab_bar_set_scroll_value(struct wzTabBar *tabBar, int value)
{
	int x, i;

	tabBar->scrollValue = WZ_CLAMPED(0, value, WZ_MAX(0, wz_arr_len(tabBar->tabs) - 1));

	// Start at the left edge of the tab bar.
	x = tabBar->base.rect.x;

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
			rect = wz_widget_get_absolute_rect(widget);
			rect.x = x;
			wz_widget_set_rect(widget, rect);
			wz_widget_set_visible(widget, true);
			x += rect.w;
		}
	}
}

static void wz_tab_bar_invoke_tab_changed(struct wzTabBar *tabBar)
{
	wzEvent e;

	assert(tabBar);
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_CHANGED;
	e.tabBar.tabBar = tabBar;
	e.tabBar.selectedTab = tabBar->selectedTab;
	wz_invoke_event(e, tabBar->tab_changed_callbacks);
}

// Sets the tab bar (the button's parent) selected tab.
static void wz_tab_bar_button_pressed(wzEvent e)
{
	struct wzTabBar *tabBar;
	int i;

	assert(e.base.widget);
	assert(e.base.widget->parent);
	tabBar = (struct wzTabBar *)e.base.widget->parent;
	tabBar->selectedTab = (struct wzButton *)e.base.widget;

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
		wz_widget_set_height(widget->children[i], rect.h);
	}
}

static void wz_tab_bar_decrement_button_clicked(wzEvent e)
{
	struct wzTabBar *tabBar;

	assert(e.base.widget);
	assert(e.base.widget->parent);
	tabBar = (struct wzTabBar *)e.base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue - 1);
}

static void wz_tab_bar_increment_button_clicked(wzEvent e)
{
	struct wzTabBar *tabBar;

	assert(e.base.widget);
	assert(e.base.widget->parent);
	tabBar = (struct wzTabBar *)e.base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue + 1);
}

struct wzTabBar *wz_tab_bar_create(struct wzDesktop *desktop)
{
	struct wzTabBar *tabBar;
	const int defaultScrollButtonWidth = 14;

	assert(desktop);
	tabBar = (struct wzTabBar *)malloc(sizeof(struct wzTabBar));
	memset(tabBar, 0, sizeof(struct wzTabBar));
	tabBar->base.type = WZ_TYPE_TAB_BAR;
	tabBar->base.desktop = desktop;
	tabBar->base.vtable.destroy = wz_tab_bar_destroy;
	tabBar->base.vtable.set_rect = wz_tab_bar_set_rect;

	// Create scroll buttons.
	// Set the draw priority above default so the scroll buttons always overlap the tabs.
	tabBar->decrementButton = wz_button_create(desktop);
	wz_button_add_callback_clicked(tabBar->decrementButton, wz_tab_bar_decrement_button_clicked);
	wz_widget_add_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)tabBar->decrementButton);
	wz_widget_set_width((struct wzWidget *)tabBar->decrementButton, defaultScrollButtonWidth);
	wz_widget_set_visible((struct wzWidget *)tabBar->decrementButton, false);
	wz_widget_set_draw_priority((struct wzWidget *)tabBar->decrementButton, WZ_DRAW_PRIORITY_WIDGET_CUSTOM_START);
	
	tabBar->incrementButton = wz_button_create(desktop);
	wz_button_add_callback_clicked(tabBar->incrementButton, wz_tab_bar_increment_button_clicked);
	wz_widget_add_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)tabBar->incrementButton);
	wz_widget_set_width((struct wzWidget *)tabBar->incrementButton, defaultScrollButtonWidth);
	wz_widget_set_visible((struct wzWidget *)tabBar->incrementButton, false);
	wz_widget_set_draw_priority((struct wzWidget *)tabBar->incrementButton, WZ_DRAW_PRIORITY_WIDGET_CUSTOM_START);

	return tabBar;
}

struct wzButton *wz_tab_bar_add_tab(struct wzTabBar *tabBar)
{
	struct wzButton *button;
	wzRect rect;
	int i;

	assert(tabBar);
	button = wz_button_create(tabBar->base.desktop);
	wz_button_add_callback_pressed(button, wz_tab_bar_button_pressed);
	wz_button_set_click_behavior(button, WZ_BUTTON_CLICK_BEHAVIOR_DOWN);
	wz_button_set_set_behavior(button, WZ_BUTTON_SET_BEHAVIOR_STICKY);
	wz_widget_add_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)button);
	wz_arr_push(tabBar->tabs, button);

	// Position to the right of the last tab.
	rect = tabBar->base.rect;
	rect.w = 50; // Default width.

	for (i = 0; i < wz_arr_len(tabBar->tabs); i++)
	{
		wzSize size = wz_widget_get_size((struct wzWidget *)tabBar->tabs[i]);
		rect.x += size.w;
	}

	wz_widget_set_rect((struct wzWidget *)button, rect);

	// Select the first tab added.
	if (!tabBar->selectedTab)
	{
		wz_button_set(button, true);
		tabBar->selectedTab = button;
		wz_tab_bar_invoke_tab_changed(tabBar);
	}

	wz_tab_bar_update_scroll_buttons(tabBar);
	return button;
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

void wz_tab_bar_add_callback_tab_changed(struct wzTabBar *tabBar, wzEventCallback callback)
{
	assert(tabBar);
	wz_arr_push(tabBar->tab_changed_callbacks, callback);
}
