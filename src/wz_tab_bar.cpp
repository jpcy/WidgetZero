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

class TabBarDecrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer_->drawTabBarDecrementButton(this, clip);
	}
};

class TabBarIncrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer_->drawTabBarIncrementButton(this, clip);
	}
};

/*
================================================================================

TAB BUTTON

================================================================================
*/

TabButton::TabButton(const std::string &label, const std::string &icon) : Button(label, icon)
{
	setClickBehavior(ButtonClickBehavior::Down);
	setSetBehavior(ButtonSetBehavior::Sticky);
	setStretch(Stretch::Height);
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

TabBar::TabBar()
{
	type_ = WidgetType::TabBar;
	selectedTab_ = NULL;
	scrollValue_ = 0;

	layout_ = new StackLayout(StackLayoutDirection::Horizontal);
	layout_->setStretch(Stretch::All);
	addChildWidget(layout_);

	// Set to draw last so the scroll buttons always overlap the tabs.
	decrementButton_ = new TabBarDecrementButton();
	decrementButton_->addEventHandler(EventType::ButtonClicked, this, &TabBar::onDecrementButtonClicked);
	addChildWidget(decrementButton_);
	decrementButton_->setVisible(false);
	decrementButton_->setDrawLast(true);
	decrementButton_->setOverlap(true);

	incrementButton_ = new TabBarIncrementButton();
	incrementButton_->addEventHandler(EventType::ButtonClicked, this, &TabBar::onIncrementButtonClicked);
	addChildWidget(incrementButton_);
	incrementButton_->setVisible(false);
	incrementButton_->setDrawLast(true);
	incrementButton_->setOverlap(true);
}

void TabBar::addTab(TabButton *tab)
{
	tab->addEventHandler(EventType::ButtonPressed, this, &TabBar::onTabButtonPressed);
	layout_->add(tab);
	tabs_.push_back(tab);

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
	layout_->remove(tab);
	delete tab;
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
		layout_->remove(tabs_[i]);
		delete tabs_[i];
	}

	tabs_.clear();
	selectedTab_ = NULL;
	scrollValue_ = 0;
	decrementButton_->setVisible(false);
	incrementButton_->setVisible(false);
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

void TabBar::onRendererChanged()
{
	decrementButton_->setWidth(renderer_->getTabBarScrollButtonWidth(this));
	incrementButton_->setWidth(renderer_->getTabBarScrollButtonWidth(this));
}

void TabBar::onRectChanged()
{
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
		updateTabVisibility();
		updateScrollButtons();
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

void TabBar::onDecrementButtonClicked(Event)
{
	setScrollValue(scrollValue_ - 1);
}

void TabBar::onIncrementButtonClicked(Event)
{
	setScrollValue(scrollValue_ + 1);
}

void TabBar::updateScrollButtons()
{
	// Size isn't set yet.
	if (rect_.w == 0 && rect_.h == 0)
		return;

	// Total tab widths.
	int totalTabWidth = 0;

	for (size_t i = 0; i < tabs_.size(); i++)
	{
		totalTabWidth += tabs_[i]->getUserOrMeasuredSize().w;
	}

	// Show/hide the scroll buttons and set their rects.
	bool showScrollButtons = totalTabWidth > rect_.w || scrollValue_ != 0;

	Rect buttonRect;
	buttonRect.w = decrementButton_->getUserOrMeasuredSize().w;
	buttonRect.x = rect_.w - buttonRect.w * 2;
	buttonRect.y = 0;
	buttonRect.h = rect_.h;
	decrementButton_->setRect(buttonRect);
	decrementButton_->setVisible(showScrollButtons);

	buttonRect.w = incrementButton_->getUserOrMeasuredSize().w;
	buttonRect.x = rect_.w - buttonRect.w;
	buttonRect.y = 0;
	buttonRect.h = rect_.h;
	incrementButton_->setRect(buttonRect);
	incrementButton_->setVisible(showScrollButtons);
}

void TabBar::updateTabVisibility()
{
	for (size_t i = 0; i < tabs_.size(); i++)
	{
		tabs_[i]->setVisible((int)i >= scrollValue_);
	}
}

} // namespace wz
