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
#include "wz_widget.h"

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

/*
================================================================================

TAB PAGE WIDGET

================================================================================
*/

static wzRect wz_tab_page_get_children_clip_rect(struct wzWidget *widget)
{
	return wz_widget_get_absolute_rect(widget);
}

struct wzWidget *wz_tab_page_create()
{
	struct wzWidget *page = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(page, 0, sizeof(struct wzWidget));
	page->type = WZ_TYPE_TAB_PAGE;
	page->vtable.get_children_clip_rect = wz_tab_page_get_children_clip_rect;
	return page;
}

/*
================================================================================

TABBED WIDGET

================================================================================
*/

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
	wzSize pageSize;
	int i;

	assert(widget);
	widget->rect = rect;
	tabbed = (struct wzTabbed *)widget;

	// Set the tab bar width to match.
	wz_widget_set_width((struct wzWidget *)tabbed->tabBar, rect.w);

	// Resize the pages to take up the remaining space.
	pageSize.w = rect.w;
	pageSize.h = rect.h - wz_widget_get_height((struct wzWidget *)tabbed->tabBar);

	for (i = 0; i < wz_arr_len(tabbed->pages); i++)
	{
		wz_widget_set_size((struct wzWidget *)tabbed->pages[i].page, pageSize);
	}
}

static void wz_tabbed_tab_bar_tab_changed(wzEvent *e)
{
	struct wzTabbed *tabbed;
	int i;

	assert(e);
	tabbed = (struct wzTabbed *)e->base.widget->parent;

	// Set the corresponding page to visible, hide all the others.
	for (i = 0; i < wz_arr_len(tabbed->pages); i++)
	{
		wz_widget_set_visible(tabbed->pages[i].page, tabbed->pages[i].tab == e->tabBar.tab);
	}
}

struct wzTabbed *wz_tabbed_create()
{
	struct wzTabbed *tabbed;

	tabbed = (struct wzTabbed *)malloc(sizeof(struct wzTabbed));
	memset(tabbed, 0, sizeof(struct wzTabbed));
	tabbed->base.type = WZ_TYPE_TABBED;
	tabbed->base.vtable.destroy = wz_tabbed_destroy;
	tabbed->base.vtable.set_rect = wz_tabbed_set_rect;
	return tabbed;
}

void wz_tabbed_add_tab(struct wzTabbed *tabbed, struct wzButton *tab, struct wzWidget *page)
{
	int tabBarHeight;
	wzTabbedPage newPage;

	assert(tabbed);
	assert(tab);
	assert(page);

	// Add the tab.
	wz_tab_bar_add_tab(tabbed->tabBar, tab);

	// Add the page widget.
	wz_widget_set_visible(page, wz_tab_bar_get_selected_tab(tabbed->tabBar) == tab);
	wz_widget_add_child_widget_internal((struct wzWidget *)tabbed, page);

	// Set the page widget rect.
	tabBarHeight = wz_widget_get_height((struct wzWidget *)tabbed->tabBar);
	wz_widget_set_rect_args(page, 0, tabBarHeight, tabbed->base.rect.w, tabbed->base.rect.h - tabBarHeight);

	// Add the tabbed page.
	newPage.tab = tab;
	newPage.page = page;
	wz_arr_push(tabbed->pages, newPage);
}

void wz_tabbed_set_tab_bar(struct wzTabbed *tabbed, struct wzTabBar *tabBar)
{
	assert(tabbed);
	assert(tabBar);

	if (tabbed->tabBar)
		return;

	tabbed->tabBar = tabBar;
	wz_tab_bar_add_callback_tab_changed(tabbed->tabBar, wz_tabbed_tab_bar_tab_changed);
	wz_widget_add_child_widget_internal((struct wzWidget *)tabbed, (struct wzWidget *)tabbed->tabBar);
}
