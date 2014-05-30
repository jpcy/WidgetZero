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
};

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
}

static void wz_tab_bar_destroy(struct wzWidget *widget)
{
	assert(widget);
	wz_arr_free(((struct wzTabBar *)widget)->tabs);
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

struct wzTabBar *wz_tab_bar_create(struct wzDesktop *desktop)
{
	struct wzTabBar *tabBar;

	assert(desktop);
	tabBar = (struct wzTabBar *)malloc(sizeof(struct wzTabBar));
	memset(tabBar, 0, sizeof(struct wzTabBar));
	tabBar->base.type = WZ_TYPE_TAB_BAR;
	tabBar->base.desktop = desktop;
	tabBar->base.vtable.destroy = wz_tab_bar_destroy;
	tabBar->base.vtable.set_rect = wz_tab_bar_set_rect;
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
	}

	return button;
}
