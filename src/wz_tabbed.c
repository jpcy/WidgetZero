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
#include "wz_renderer.h"
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
	WZ_ASSERT(widget);
	return wz_widget_get_absolute_rect(widget);
}

static struct wzWidget *wz_tab_page_create(struct wzRenderer *renderer)
{
	struct wzWidget *page = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(page, 0, sizeof(struct wzWidget));
	page->type = WZ_TYPE_TAB_PAGE;
	page->renderer = renderer;
	page->vtable.get_children_clip_rect = wz_tab_page_get_children_clip_rect;
	return page;
}

void wz_tab_page_add(struct wzWidget *tabPage, struct wzWidget *widget)
{
	WZ_ASSERT(tabPage);
	WZ_ASSERT(widget);

	if (tabPage->type != WZ_TYPE_TAB_PAGE || widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	wz_widget_add_child_widget(tabPage, widget);
}

void wz_tab_page_remove(struct wzWidget *tabPage, struct wzWidget *widget)
{
	WZ_ASSERT(tabPage);
	WZ_ASSERT(widget);

	if (tabPage->type != WZ_TYPE_TAB_PAGE)
		return;

	wz_widget_remove_child_widget(tabPage, widget);
}

/*
================================================================================

TABBED WIDGET

================================================================================
*/

static void wz_tabbed_draw(struct wzWidget *widget, wzRect clip)
{
	int i, selectedTabIndex;
	wzRect tr; // Tab rect.
	struct wzTabbed *tabbed = (struct wzTabbed *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzTabbedStyle *style = &widget->style.tabbed;
	wzRect rect = wz_widget_get_absolute_rect(widget);
	const struct wzButton *selectedTab = wz_tab_bar_get_selected_tab(tabbed->tabBar);

	// Use the page rect.
	const int tabBarHeight = wz_widget_get_height((struct wzWidget *)tabbed->tabBar);
	rect.y += tabBarHeight;
	rect.h -= tabBarHeight;

	nvgSave(vg);
	wz_renderer_clip_to_rect_intersection(vg, clip, wz_widget_get_absolute_rect(widget));

	// Draw an outline around the selected tab and the tab page.
	nvgBeginPath(vg);

	if (wz_widget_get_visible((struct wzWidget *)selectedTab))
	{
		// Selected tab.
		tr = wz_widget_get_absolute_rect((struct wzWidget *)selectedTab);
		nvgMoveTo(vg, tr.x + 0.5f, tr.y + tr.h + 0.5f); // bl
		nvgLineTo(vg, tr.x + 0.5f, tr.y + 0.5f); // tl
		nvgLineTo(vg, tr.x + tr.w - 0.5f, tr.y + 0.5f); // tr
		nvgLineTo(vg, tr.x + tr.w - 0.5f, tr.y + tr.h + 0.5f); // br

		// The tab page.
		nvgLineTo(vg, rect.x + rect.w - 0.5f, rect.y + 0.5f); // tr
		nvgLineTo(vg, rect.x + rect.w - 0.5f, rect.y + rect.h - 0.5f); // br
		nvgLineTo(vg, rect.x + 0.5f, rect.y + rect.h - 0.5f); // bl
		nvgLineTo(vg, rect.x + 0.5f, rect.y + 0.5f); // tl
		nvgClosePath(vg);
	}
	else
	{
		nvgRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f);
	}
	
	nvgStrokeColor(vg, style->borderColor);
	nvgStroke(vg);

	// Get the selected tab index.
	for (i = 0; i < wz_arr_len(tabbed->pages); i++)
	{
		if (tabbed->pages[i].tab == selectedTab)
		{
			selectedTabIndex = i;
			break;
		}
	}

	// Draw an outline around the non-selected tabs.
	for (i = 0; i < wz_arr_len(tabbed->pages); i++)
	{
		const struct wzButton *tab = tabbed->pages[i].tab;

		if (tab == selectedTab || !wz_widget_get_visible((struct wzWidget *)tab))
			continue;

		tr = wz_widget_get_absolute_rect((struct wzWidget *)tab);
		nvgBeginPath(vg);

		// Only draw the left side if this is the leftmost tab.
		if (i == wz_tab_bar_get_scroll_value(tabbed->tabBar))
		{
			nvgMoveTo(vg, tr.x + 0.5f, tr.y + tr.h - 0.5f); // bl
			nvgLineTo(vg, tr.x + 0.5f, tr.y + 0.5f); // tl
		}
		else
		{
			nvgMoveTo(vg, tr.x + 0.5f, tr.y + 0.5f); // tl
		}

		nvgLineTo(vg, tr.x + tr.w - 0.5f, tr.y + 0.5f); // tr

		// If the selected tab is next to this tab, on the right, don't draw the right side.
		if (selectedTabIndex != i + 1)
		{
			nvgLineTo(vg, tr.x + tr.w - 0.5f, tr.y + tr.h - 0.5f); // br
		}

		nvgStrokeColor(vg, style->borderColor);
		nvgStroke(vg);
	}

	nvgRestore(vg);
}

static void wz_tabbed_destroy(struct wzWidget *widget)
{
	struct wzTabbed *tabbed;

	WZ_ASSERT(widget);
	tabbed = (struct wzTabbed *)widget;
	wz_arr_free(tabbed->pages);
}

static void wz_tabbed_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzTabbed *tabbed;
	wzSize pageSize;
	int i;

	WZ_ASSERT(widget);
	widget->rect = rect;
	tabbed = (struct wzTabbed *)widget;

	// Set the tab bar width to match.
	wz_widget_set_width_internal((struct wzWidget *)tabbed->tabBar, rect.w);

	// Resize the pages to take up the remaining space.
	pageSize.w = rect.w;
	pageSize.h = rect.h - wz_widget_get_height((struct wzWidget *)tabbed->tabBar);

	for (i = 0; i < wz_arr_len(tabbed->pages); i++)
	{
		wz_widget_set_size_internal((struct wzWidget *)tabbed->pages[i].page, pageSize);
	}
}

static void wz_tabbed_tab_bar_tab_changed(wzEvent *e)
{
	struct wzTabbed *tabbed;
	int i;

	WZ_ASSERT(e);
	tabbed = (struct wzTabbed *)e->base.widget->parent;

	// Set the corresponding page to visible, hide all the others.
	for (i = 0; i < wz_arr_len(tabbed->pages); i++)
	{
		wz_widget_set_visible(tabbed->pages[i].page, tabbed->pages[i].tab == e->tabBar.tab);
	}
}

struct wzTabbed *wz_tabbed_create()
{
	struct wzTabbed *tabbed = (struct wzTabbed *)malloc(sizeof(struct wzTabbed));
	wzTabbedStyle *style = &tabbed->base.style.tabbed;

	memset(tabbed, 0, sizeof(struct wzTabbed));
	tabbed->base.type = WZ_TYPE_TABBED;
	tabbed->base.vtable.destroy = wz_tabbed_destroy;
	tabbed->base.vtable.draw = wz_tabbed_draw;
	tabbed->base.vtable.set_rect = wz_tabbed_set_rect;

	tabbed->tabBar = wz_tab_bar_create();
	wz_tab_bar_add_callback_tab_changed(tabbed->tabBar, wz_tabbed_tab_bar_tab_changed);
	wz_widget_add_child_widget((struct wzWidget *)tabbed, (struct wzWidget *)tabbed->tabBar);

	style->bgColor = nvgRGB(80, 80, 80);
	style->borderColor = WZ_STYLE_DARK_BORDER_COLOR;

	return tabbed;
}

void wz_tabbed_add_tab(struct wzTabbed *tabbed, struct wzButton **tab, struct wzWidget **page)
{
	int tabBarHeight;
	wzTabbedPage newPage;

	WZ_ASSERT(tabbed);
	WZ_ASSERT(tab);
	WZ_ASSERT(page);

	// Add the tab.
	*tab = wz_tab_bar_create_tab(tabbed->tabBar);

	// Add the page widget.
	*page = wz_tab_page_create(tabbed->base.renderer);
	wz_widget_set_visible(*page, wz_tab_bar_get_selected_tab(tabbed->tabBar) == *tab);
	wz_widget_add_child_widget((struct wzWidget *)tabbed, *page);

	// Set the page widget rect.
	tabBarHeight = wz_widget_get_height((struct wzWidget *)tabbed->tabBar);
	wz_widget_set_rect_args_internal(*page, 0, tabBarHeight, tabbed->base.rect.w, tabbed->base.rect.h - tabBarHeight);

	// Add the tabbed page.
	newPage.tab = *tab;
	newPage.page = *page;
	wz_arr_push(tabbed->pages, newPage);
}
