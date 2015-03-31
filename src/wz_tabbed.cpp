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
	type_ = WidgetType::TabPage;
}

void TabPage::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->getType() == WidgetType::MainWindow || widget->getType() == WidgetType::Window)
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

Tab::Tab(const std::string &label, const std::string &icon)
{
	button_ = new TabButton(label, icon);
	page_ = new TabPage();
}

Tab::~Tab()
{
}

const TabButton *Tab::getButton() const
{
	return button_;
}

const TabPage *Tab::getPage()  const
{
	return page_;
}

Widget *Tab::add(Widget *widget)
{
	page_->add(widget);
	return widget;
}

void Tab::remove(Widget *widget)
{
	page_->remove(widget);
}

/*
================================================================================

TABBED WIDGET

================================================================================
*/

Tabbed::Tabbed()
{
	type_ = WidgetType::Tabbed;

	layout_ = new StackLayout(StackLayoutDirection::Vertical);
	layout_->setStretch(Stretch::All);
	addChildWidget(layout_);

	tabBar_ = new TabBar;
	tabBar_->setStretch(Stretch::Width);
	tabBar_->addEventHandler(EventType::TabBarTabChanged, this, &Tabbed::onTabChanged);
	layout_->add(tabBar_);

	pageContainer_ = new Widget;
	pageContainer_->setStretch(Stretch::All);
	layout_->add(pageContainer_);
}

Tab *Tabbed::getSelectedTab()
{
	TabButton *selected = tabBar_->getSelectedTab();

	for (size_t i = 0; i < tabs_.size(); i++)
	{
		if (tabs_[i]->button_ == selected)
			return tabs_[i];
	}

	return NULL;
}

void Tabbed::add(Tab *tab)
{
	WZ_ASSERT(tab);

	// Add the tab button to the tab bar.
	tabBar_->addTab(tab->button_);

	// Add the page widget.
	tab->page_->setStretch(Stretch::All);
	tab->page_->setVisible(tabBar_->getSelectedTab() == tab->button_);
	pageContainer_->addChildWidget(tab->page_);

	// Store this tab.
	tabs_.push_back(tab);
}

void Tabbed::draw(Rect clip)
{
	renderer_->drawTabbed(this, clip);
}

Size Tabbed::measure()
{
	return renderer_->measureTabbed(this);
}

void Tabbed::onTabChanged(Event e)
{
	// Set the corresponding page to visible, hide all the others.
	for (size_t i = 0; i < tabs_.size(); i++)
	{
		tabs_[i]->page_->setVisible(tabs_[i]->button_ == e.tabBar.tab);
	}
}

} // namespace wz
