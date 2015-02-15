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
#include "wz.h"
#pragma hdrstop

namespace wz {

/*
================================================================================

TAB PAGE WIDGET

================================================================================
*/

TabPage::TabPage()
{
	type = WZ_TYPE_TAB_PAGE;
}

void TabPage::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	addChildWidget(widget);
}

void TabPage::remove(Widget *widget)
{
	WZ_ASSERT(widget);
	removeChildWidget(widget);
}

/*
================================================================================

TAB

================================================================================
*/

Tab::Tab() : button(NULL), page(NULL)
{
}

Tab::~Tab()
{
}

Tab *Tab::setLabel(const std::string &label)
{
	button->setLabel(label.c_str());
	return this;
}

Widget *Tab::add(Widget *widget)
{
	page->add(widget);
	return widget;
}

void Tab::remove(Widget *widget)
{
	page->remove(widget);
}

/*
================================================================================

TABBED WIDGET

================================================================================
*/

Tabbed::Tabbed()
{
	type = WZ_TYPE_TABBED;

	tabBar = new TabBar;
	tabBar->addEventHandler(WZ_EVENT_TAB_BAR_TAB_CHANGED, this, &Tabbed::onTabChanged);
	addChildWidget(tabBar);
}

void Tabbed::onRectChanged()
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

void Tabbed::draw(Rect clip)
{
	renderer->drawTabbed(this, clip);
}

Size Tabbed::measure()
{
	return renderer->measureTabbed(this);
}

void Tabbed::addTab(Tab *tab)
{
	addTab(&tab->button, &tab->page);
}

void Tabbed::addTab(TabButton **tab, TabPage **page)
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

void Tabbed::onTabChanged(Event *e)
{
	Tabbed *tabbed;

	WZ_ASSERT(e);
	tabbed = (Tabbed *)e->base.widget->parent;

	// Set the corresponding page to visible, hide all the others.
	for (size_t i = 0; i < tabbed->pages.size(); i++)
	{
		tabbed->pages[i].page->setVisible(tabbed->pages[i].tab == e->tabBar.tab);
	}
}

} // namespace wz
