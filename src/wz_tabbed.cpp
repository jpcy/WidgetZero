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
#include "wz_internal.h"
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
	return widget->getAbsoluteRect();
}

TabPage::TabPage()
{
	type = WZ_TYPE_TAB_PAGE;
	vtable.get_children_clip_rect = wz_tab_page_get_children_clip_rect;
}

void TabPage::add(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	addChildWidget(widget);
}

void TabPage::remove(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	removeChildWidget(widget);
}

/*
================================================================================

TAB PUBLIC INTERFACE

================================================================================
*/

// Wraps tab button and page.
struct TabImpl
{
	TabImpl() : button(NULL), page(NULL) {}
	ButtonImpl *button;
	TabPage *page;
};

Tab::Tab()
{
	impl.reset(new TabImpl());
}

Tab::~Tab()
{
}

Tab *Tab::setLabel(const std::string &label)
{
	impl->button->setLabel(label.c_str());
	return this;
}

Widget *Tab::add(Widget *widget)
{
	impl->page->add(widget->getImpl());
	return widget;
}

void Tab::remove(Widget *widget)
{
	impl->page->remove(widget->getImpl());
}

/*
================================================================================

TABBED WIDGET

================================================================================
*/

static void wz_tabbed_tab_bar_tab_changed(Event *e)
{
	struct TabbedImpl *tabbed;

	WZ_ASSERT(e);
	tabbed = (struct TabbedImpl *)e->base.widget->parent;

	// Set the corresponding page to visible, hide all the others.
	for (size_t i = 0; i < tabbed->pages.size(); i++)
	{
		tabbed->pages[i].page->setVisible(tabbed->pages[i].tab == e->tabBar.tab);
	}
}

TabbedImpl::TabbedImpl()
{
	type = WZ_TYPE_TABBED;

	tabBar = new TabBarImpl;
	tabBar->addCallbackTabChanged(wz_tabbed_tab_bar_tab_changed);
	addChildWidget(tabBar);
}

void TabbedImpl::onRectChanged()
{
	// Set the tab bar width to match.
	tabBar->setWidthInternal(rect.w);

	// Resize the pages to take up the remaining space.
	Size pageSize;
	pageSize.w = rect.w;
	pageSize.h = rect.h - tabBar->getHeight();

	for (size_t i = 0; i < pages.size(); i++)
	{
		pages[i].page->setSizeInternal(pageSize);
	}
}

void TabbedImpl::draw(Rect clip)
{
	renderer->drawTabbed(this, clip);
}

Size TabbedImpl::measure()
{
	return renderer->measureTabbed(this);
}

void TabbedImpl::addTab(struct ButtonImpl **tab, struct TabPage **page)
{
	WZ_ASSERT(tab);
	WZ_ASSERT(page);

	// Add the tab.
	*tab = tabBar->createTab();

	// Add the page widget.
	*page = new TabPage();
	(*page)->setVisible(tabBar->getSelectedTab() == *tab);
	addChildWidget(*page);

	// Set the page widget rect.
	int tabBarHeight = tabBar->getHeight();
	(*page)->setRectInternal(0, tabBarHeight, rect.w, rect.h - tabBarHeight);

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
	impl.reset(new TabbedImpl);
}

Tabbed::~Tabbed()
{
}

Tab *Tabbed::addTab(Tab *tab)
{
	getImpl()->addTab(&tab->impl->button, &tab->impl->page);
	return tab;
}

TabbedImpl *Tabbed::getImpl()
{
	return (TabbedImpl *)impl.get();
}

const TabbedImpl *Tabbed::getImpl() const
{
	return (const TabbedImpl *)impl.get();
}

} // namespace wz
