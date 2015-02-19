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
#include "wz_renderer_nanovg.h"

namespace wz {

/*
================================================================================

TAB BUTTON

================================================================================
*/

TabButton::TabButton(const std::string &label) : Button(label)
{
	setClickBehavior(ButtonClickBehavior::Down);
	setSetBehavior(ButtonSetBehavior::Sticky);
}

void TabButton::draw(Rect clip)
{
	renderer_->drawTabButton(this, clip);
}

/*
================================================================================

TAB BAR

================================================================================
*/

#if 0
// Ensure tab height stay the same as the tab bar.
static void wz_tab_set_rect(Widget *widget, Rect rect)
{
	WZ_ASSERT(widget);
	rect.h = ((TabBar *)widget->parent)->rect.h;
	widget->rect = rect;
}
#endif

TabBar::TabBar()
{
	type_ = WidgetType::TabBar;
	selectedTab_ = NULL;
	scrollValue_ = 0;

	// Set to draw last so the scroll buttons always overlap the tabs.
	decrementButton_ = new Button("<");
	decrementButton_->addEventHandler(EventType::ButtonClicked, this, &TabBar::onDecrementButtonClicked);
	addChildWidget(decrementButton_);
	decrementButton_->setWidthInternal(WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH);
	decrementButton_->setVisible(false);
	decrementButton_->setDrawLast(true);
	decrementButton_->setOverlap(true);

	incrementButton_ = new Button(">");
	incrementButton_->addEventHandler(EventType::ButtonClicked, this, &TabBar::onIncrementButtonClicked);
	addChildWidget(incrementButton_);
	incrementButton_->setWidthInternal(WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH);
	incrementButton_->setVisible(false);
	incrementButton_->setDrawLast(true);
	incrementButton_->setOverlap(true);
}

TabButton *TabBar::createTab()
{
	TabButton *tab = new TabButton();
	tab->addEventHandler(EventType::ButtonPressed, this, &TabBar::onTabButtonPressed);
	addChildWidget(tab);
	tabs_.push_back(tab);

	// Position to the right of the last tab.
	Rect rect;
	rect.x = rect.y = 0;
	rect.w = 50; // Default width.
	rect.h = rect_.h;

	for (size_t i = 0; i < tabs_.size(); i++)
	{
		rect.x += tabs_[i]->getWidth();
	}

	tab->setRectInternal(rect);

	// Select the first tab added.
	if (!selectedTab_)
	{
		tab->set(true);
		selectedTab_ = tab;
		invokeTabChanged();
	}

	updateScrollButtons();

	// Invoke the tab added event.
	Event e;
	e.tabBar.type = EventType::TabBarTabAdded;
	e.tabBar.tabBar = this;
	e.tabBar.tab = tab;
	invokeEvent(e);

	return tab;
}

void TabBar::destroyTab(TabButton *tab)
{
	WZ_ASSERT(tab);
	int deleteIndex = -1;

	for (size_t i = 0; i < tabs_.size(); i++)
	{
		if (tabs_[i] == tab)
		{
			deleteIndex = (int)i;
			break;
		}
	}

	if (deleteIndex == -1)
		return;

	// Invoke the tab removed event.
	Event e;
	e.tabBar.type = EventType::TabBarTabRemoved;
	e.tabBar.tabBar = this;
	e.tabBar.tab = tabs_[deleteIndex];
	invokeEvent(e);

	// Delete the tab.
	tabs_.erase(tabs_.begin() + deleteIndex);
	destroyChildWidget(tab);
}

void TabBar::clearTabs()
{
	for (size_t i = 0; i < tabs_.size(); i++)
	{
		// Invoke the tab removed event.
		Event e;
		e.tabBar.type = EventType::TabBarTabRemoved;
		e.tabBar.tabBar = this;
		e.tabBar.tab = tabs_[i];
		invokeEvent(e);

		// Destroy the tab.
		destroyChildWidget(tabs_[i]);
	}

	tabs_.clear();
	selectedTab_ = NULL;
	scrollValue_ = 0;
	decrementButton_->setVisible(false);
	incrementButton_->setVisible(false);
}

Button *TabBar::getDecrementButton()
{
	return decrementButton_;
}

Button *TabBar::getIncrementButton()
{
	return incrementButton_;
}

TabButton *TabBar::getSelectedTab()
{
	return selectedTab_;
}

void TabBar::selectTab(TabButton *tab)
{
	WZ_ASSERT(tab);

	if (selectedTab_ == tab)
		return; // Already selected.

	tab->set(true);
	selectedTab_ = tab;

	// Unset all the other tab bar buttons.
	for (size_t i = 0; i < tabs_.size(); i++)
	{
		if (tabs_[i] != selectedTab_)
		{
			tabs_[i]->set(false);
		}
	}
	
	invokeTabChanged();
}

void TabBar::addCallbackTabChanged(EventCallback callback)
{
	tabChangedCallbacks_.push_back(callback);
}

int TabBar::getScrollValue() const
{
	return scrollValue_;
}

void TabBar::onTabButtonPressed(Event e)
{
	selectTab((TabButton *)e.base.widget);
}

void TabBar::onRectChanged()
{
	// Set button heights to match.
	for (size_t i = 0; i < children_.size(); i++)
	{
		children_[i]->setHeightInternal(rect_.h);
	}

	updateTabs();
	updateScrollButtons();
}

void TabBar::draw(Rect clip)
{
	renderer_->drawTabBar(this, clip);
}

Size TabBar::measure()
{
	return renderer_->measureTabBar(this);
}

void TabBar::setScrollValue(int value)
{
	const int oldValue = scrollValue_;
	scrollValue_ = WZ_CLAMPED(0, value, WZ_MAX(0, (int)tabs_.size() - 1));

	if (oldValue != scrollValue_)
	{
		// Value has changed.
		updateTabs();
	}
}

void TabBar::invokeTabChanged()
{
	Event e;
	e.tabBar.type = EventType::TabBarTabChanged;
	e.tabBar.tabBar = this;
	e.tabBar.tab = selectedTab_;
	invokeEvent(e, tabChangedCallbacks_);
}

void TabBar::onDecrementButtonClicked(Event e)
{
	setScrollValue(scrollValue_ - 1);
}

void TabBar::onIncrementButtonClicked(Event e)
{
	setScrollValue(scrollValue_ + 1);
}

void TabBar::updateScrollButtons()
{
	// Total tab widths.
	int totalTabWidth = 0;

	for (size_t i = 0; i < tabs_.size(); i++)
	{
		Size size = tabs_[i]->getSize();
		totalTabWidth += size.w;
	}

	// Show/hide the scroll buttons and set their rects.
	bool wereScrollButtonsVisible = decrementButton_->getVisible();
	bool showScrollButtons = totalTabWidth > rect_.w;

	Rect buttonRect;
	buttonRect.w = decrementButton_->getWidth();
	buttonRect.x = rect_.w - buttonRect.w * 2;
	buttonRect.y = 0;
	buttonRect.h = rect_.h;
	decrementButton_->setRectInternal(buttonRect);
	decrementButton_->setVisible(showScrollButtons);

	buttonRect.w = incrementButton_->getWidth();
	buttonRect.x = rect_.w - buttonRect.w;
	buttonRect.y = 0;
	buttonRect.h = rect_.h;
	incrementButton_->setRectInternal(buttonRect);
	incrementButton_->setVisible(showScrollButtons);

	if (wereScrollButtonsVisible && showScrollButtons)
	{
		scrollValue_ = 0;
	}
}

void TabBar::updateTabs()
{
	// Start at the left edge of the tab bar.
	int x = 0;

	for (size_t i = 0; i < tabs_.size(); i++)
	{
		if ((int)i < scrollValue_)
		{
			// Scrolled out of view, hide it.
			tabs_[i]->setVisible(false);
			continue;
		}
		else
		{
			// Reposition and show.
			Rect tabRect;
			tabRect.x = x;
			tabRect.y = 0;
			tabRect.w = tabs_[i]->getWidth();
			tabRect.h = rect_.h;
			tabs_[i]->setRectInternal(tabRect);
			tabs_[i]->setVisible(true);
			x += tabRect.w;
		}
	}
}

} // namespace wz
