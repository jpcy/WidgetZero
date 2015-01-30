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

TAB PAGE WIDGET

================================================================================
*/

static Rect wz_tab_page_get_children_clip_rect(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	return wz_widget_get_absolute_rect(widget);
}

static struct WidgetImpl *wz_tab_page_create(struct wzRenderer *renderer)
{
	struct WidgetImpl *page = new struct WidgetImpl;
	page->type = WZ_TYPE_TAB_PAGE;
	page->renderer = renderer;
	page->vtable.get_children_clip_rect = wz_tab_page_get_children_clip_rect;
	return page;
}

void wz_tab_page_add(struct WidgetImpl *tabPage, struct WidgetImpl *widget)
{
	WZ_ASSERT(tabPage);
	WZ_ASSERT(widget);

	if (tabPage->type != WZ_TYPE_TAB_PAGE || widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	wz_widget_add_child_widget(tabPage, widget);
}

void wz_tab_page_remove(struct WidgetImpl *tabPage, struct WidgetImpl *widget)
{
	WZ_ASSERT(tabPage);
	WZ_ASSERT(widget);

	if (tabPage->type != WZ_TYPE_TAB_PAGE)
		return;

	wz_widget_remove_child_widget(tabPage, widget);
}

/*
================================================================================

TAB

================================================================================
*/

// Wraps tab button and page.
struct TabImpl
{
	TabImpl() : button(NULL), page(NULL) {}
	ButtonImpl *button;
	WidgetImpl *page;
};

Tab::Tab()
{
	impl = new TabImpl();
}

Tab::~Tab()
{
	delete impl;
}

Tab *Tab::setLabel(const std::string &label)
{
	impl->button->setLabel(label.c_str());
	return this;
}

Widget *Tab::add(Widget *widget)
{
	wz_tab_page_add(impl->page, widget->impl);
	return widget;
}

void Tab::remove(Widget *widget)
{
	wz_tab_page_remove(impl->page, widget->impl);
}

/*
================================================================================

TABBED WIDGET

================================================================================
*/

static void wz_tabbed_draw(struct WidgetImpl *widget, Rect clip)
{
	int selectedTabIndex;
	Rect tr; // Tab rect.
	struct TabbedImpl *tabbed = (struct TabbedImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	Rect rect = wz_widget_get_absolute_rect(widget);
	const struct ButtonImpl *selectedTab = tabbed->tabBar->getSelectedTab();

	// Use the page rect.
	const int tabBarHeight = wz_widget_get_height(tabbed->tabBar);
	rect.y += tabBarHeight;
	rect.h -= tabBarHeight;

	nvgSave(vg);
	wz_renderer_clip_to_rect_intersection(vg, clip, wz_widget_get_absolute_rect(widget));

	// Draw an outline around the selected tab and the tab page.
	nvgBeginPath(vg);

	if (wz_widget_get_visible(selectedTab))
	{
		// Selected tab.
		tr = wz_widget_get_absolute_rect(selectedTab);
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
	
	nvgStrokeColor(vg, WZ_SKIN_TABBED_BORDER_COLOR);
	nvgStroke(vg);

	// Get the selected tab index.
	for (size_t i = 0; i < tabbed->pages.size(); i++)
	{
		if (tabbed->pages[i].tab == selectedTab)
		{
			selectedTabIndex = (int)i;
			break;
		}
	}

	// Draw an outline around the non-selected tabs.
	for (size_t i = 0; i < tabbed->pages.size(); i++)
	{
		const struct ButtonImpl *tab = tabbed->pages[i].tab;

		if (tab == selectedTab || !wz_widget_get_visible(tab))
			continue;

		tr = wz_widget_get_absolute_rect(tab);
		nvgBeginPath(vg);

		// Only draw the left side if this is the leftmost tab.
		if (i == tabbed->tabBar->getScrollValue())
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

		nvgStrokeColor(vg, WZ_SKIN_TABBED_BORDER_COLOR);
		nvgStroke(vg);
	}

	nvgRestore(vg);
}

static void wz_tabbed_set_rect(struct WidgetImpl *widget, Rect rect)
{
	struct TabbedImpl *tabbed;
	Size pageSize;

	WZ_ASSERT(widget);
	widget->rect = rect;
	tabbed = (struct TabbedImpl *)widget;

	// Set the tab bar width to match.
	wz_widget_set_width_internal(tabbed->tabBar, rect.w);

	// Resize the pages to take up the remaining space.
	pageSize.w = rect.w;
	pageSize.h = rect.h - wz_widget_get_height(tabbed->tabBar);

	for (size_t i = 0; i < tabbed->pages.size(); i++)
	{
		wz_widget_set_size_internal(tabbed->pages[i].page, pageSize);
	}
}

static void wz_tabbed_tab_bar_tab_changed(Event *e)
{
	struct TabbedImpl *tabbed;

	WZ_ASSERT(e);
	tabbed = (struct TabbedImpl *)e->base.widget->parent;

	// Set the corresponding page to visible, hide all the others.
	for (size_t i = 0; i < tabbed->pages.size(); i++)
	{
		wz_widget_set_visible(tabbed->pages[i].page, tabbed->pages[i].tab == e->tabBar.tab);
	}
}

TabbedImpl::TabbedImpl()
{
	type = WZ_TYPE_TABBED;
	vtable.draw = wz_tabbed_draw;
	vtable.set_rect = wz_tabbed_set_rect;

	tabBar = new TabBarImpl;
	tabBar->addCallbackTabChanged(wz_tabbed_tab_bar_tab_changed);
	wz_widget_add_child_widget(this, tabBar);
}

void TabbedImpl::addTab(struct ButtonImpl **tab, struct WidgetImpl **page)
{
	WZ_ASSERT(tab);
	WZ_ASSERT(page);

	// Add the tab.
	*tab = tabBar->createTab();

	// Add the page widget.
	*page = wz_tab_page_create(renderer);
	wz_widget_set_visible(*page, tabBar->getSelectedTab() == *tab);
	wz_widget_add_child_widget(this, *page);

	// Set the page widget rect.
	int tabBarHeight = wz_widget_get_height(tabBar);
	wz_widget_set_rect_args_internal(*page, 0, tabBarHeight, rect.w, rect.h - tabBarHeight);

	// Add the tabbed page.
	TabbedPage newPage;
	newPage.tab = *tab;
	newPage.page = *page;
	pages.push_back(newPage);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

Tabbed::Tabbed()
{
	impl = new TabbedImpl;
}

Tabbed::~Tabbed()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

Tab *Tabbed::addTab(Tab *tab)
{
	((TabbedImpl *)impl)->addTab(&tab->impl->button, &tab->impl->page);
	return tab;
}

} // namespace wz
