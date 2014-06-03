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

typedef struct
{
	struct wzButton *tab;
	struct wzWidget *page;
}
wzTabbedPage;

struct wzTabbed
{
	struct wzWidget base;
	struct wzTabBar *tabBar;
	wzTabbedPage *pages;
};

static void wz_tabbed_destroy(struct wzWidget *widget)
{
	struct wzTabbed *tabbed;

	assert(widget);
	tabbed = (struct wzTabbed *)widget;
	wz_arr_free(tabbed->pages);
}

static void wz_tabbed_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzTabbed *tabbed;
	wzRect tempRect;
	int i;

	assert(widget);
	widget->rect = rect;
	tabbed = (struct wzTabbed *)widget;

	// Move the tab bar to the top left of rect, set its width to match.
	tempRect = rect;
	tempRect.h = wz_widget_get_rect((struct wzWidget *)tabbed->tabBar).h;
	wz_widget_set_rect((struct wzWidget *)tabbed->tabBar, tempRect);

	// Move/resize the pages to take up the remaining space.
	tempRect.y += tempRect.h;
	tempRect.h = rect.h - tempRect.h;

	for (i = 0; i < wz_arr_len(tabbed->pages); i++)
	{
		wz_widget_set_rect((struct wzWidget *)tabbed->pages[i].page, tempRect);
	}
}

static void wz_tabbed_tab_bar_tab_changed(wzEvent e)
{
	struct wzTabbed *tabbed;
	int i;

	tabbed = (struct wzTabbed *)e.base.widget->parent;

	// Set the corresponding page to visible, hide all the others.
	for (i = 0; i < wz_arr_len(tabbed->pages); i++)
	{
		wz_widget_set_visible(tabbed->pages[i].page, tabbed->pages[i].tab == e.tabBar.selectedTab);
	}
}

struct wzTabbed *wz_tabbed_create(struct wzDesktop *desktop)
{
	struct wzTabbed *tabbed;

	assert(desktop);
	tabbed = (struct wzTabbed *)malloc(sizeof(struct wzTabbed));
	memset(tabbed, 0, sizeof(struct wzTabbed));
	tabbed->base.type = WZ_TYPE_TABBED;
	tabbed->base.desktop = desktop;
	tabbed->base.vtable.destroy = wz_tabbed_destroy;
	tabbed->base.vtable.set_rect = wz_tabbed_set_rect;

	// Create the tab bar.
	tabbed->tabBar = wz_tab_bar_create(desktop);
	wz_tab_bar_add_callback_tab_changed(tabbed->tabBar, wz_tabbed_tab_bar_tab_changed);
	wz_widget_add_child_widget((struct wzWidget *)tabbed, (struct wzWidget *)tabbed->tabBar);

	return tabbed;
}

void wz_tabbed_add_tab(struct wzTabbed *tabbed, struct wzButton **tab, struct wzWidget **page)
{
	wzTabbedPage newPage;
	int tabBarHeight;
	wzRect rect;

	assert(tabbed);

	// Add the tab.
	newPage.tab = wz_tab_bar_add_tab(tabbed->tabBar);

	// Create the page widget.
	newPage.page = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(newPage.page, 0, sizeof(struct wzWidget));
	newPage.page->type = WZ_TYPE_TAB_PAGE;
	newPage.page->desktop = tabbed->base.desktop;
	wz_widget_set_visible(newPage.page, wz_tab_bar_get_selected_tab(tabbed->tabBar) == newPage.tab);
	wz_widget_add_child_widget((struct wzWidget *)tabbed, newPage.page);

	// Set the page widget rect.
	rect = tabbed->base.rect;
	tabBarHeight = wz_widget_get_rect((struct wzWidget *)tabbed->tabBar).h;
	rect.y += tabBarHeight;
	rect.h -= tabBarHeight;
	wz_widget_set_rect(newPage.page, rect);

	// Add the page.
	wz_arr_push(tabbed->pages, newPage);

	// Set return values.
	if (tab)
	{
		*tab = newPage.tab;
	}

	if (page)
	{
		*page = newPage.page;
	}
}

struct wzTabBar *wz_tabbed_get_tab_bar(struct wzTabbed *tabbed)
{
	assert(tabbed);
	return tabbed->tabBar;
}
