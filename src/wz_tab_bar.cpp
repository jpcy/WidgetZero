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
	setClickBehavior(WZ_BUTTON_CLICK_BEHAVIOR_DOWN);
	setSetBehavior(WZ_BUTTON_SET_BEHAVIOR_STICKY);
}

void TabButton::draw(Rect clip)
{
	renderer->drawTabButton(this, clip);
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
	type = WZ_TYPE_TAB_BAR;
	selectedTab = NULL;
	scrollValue = 0;

	// Set to draw last so the scroll buttons always overlap the tabs.
	decrementButton = new Button("<");
	decrementButton->addEventHandler(WZ_EVENT_BUTTON_CLICKED, this, &TabBar::onDecrementButtonClicked);
	addChildWidget(decrementButton);
	decrementButton->setWidthInternal(WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH);
	decrementButton->setVisible(false);
	decrementButton->setDrawLast(true);
	decrementButton->setOverlap(true);

	incrementButton = new Button(">");
	incrementButton->addEventHandler(WZ_EVENT_BUTTON_CLICKED, this, &TabBar::onIncrementButtonClicked);
	addChildWidget(incrementButton);
	incrementButton->setWidthInternal(WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH);
	incrementButton->setVisible(false);
	incrementButton->setDrawLast(true);
	incrementButton->setOverlap(true);
}

void TabBar::onRectChanged()
{
	// Set button heights to match.
	for (size_t i = 0; i < children.size(); i++)
	{
		children[i]->setHeightInternal(rect.h);
	}

	updateTabs();
	updateScrollButtons();
}

void TabBar::draw(Rect clip)
{
	renderer->drawTabBar(this, clip);
}

Size TabBar::measure()
{
	return renderer->measureTabBar(this);
}

TabButton *TabBar::createTab()
{
	TabButton *tab = new TabButton();
	tab->addEventHandler(WZ_EVENT_BUTTON_PRESSED, this, &TabBar::onTabButtonPressed);
	addChildWidget(tab);
	tabs.push_back(tab);

	// Position to the right of the last tab.
	Rect rect;
	rect.x = rect.y = 0;
	rect.w = 50; // Default width.
	rect.h = this->rect.h;

	for (size_t i = 0; i < tabs.size(); i++)
	{
		rect.x += tabs[i]->getWidth();
	}

	tab->setRectInternal(rect);

	// Select the first tab added.
	if (!selectedTab)
	{
		tab->set(true);
		selectedTab = tab;
		invokeTabChanged();
	}

	updateScrollButtons();

	// Invoke the tab added event.
	Event e;
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_ADDED;
	e.tabBar.tabBar = this;
	e.tabBar.tab = tab;
	invokeEvent(e);

	return tab;
}

void TabBar::destroyTab(TabButton *tab)
{
	WZ_ASSERT(tab);
	int deleteIndex = -1;

	for (size_t i = 0; i < tabs.size(); i++)
	{
		if (tabs[i] == tab)
		{
			deleteIndex = (int)i;
			break;
		}
	}

	if (deleteIndex == -1)
		return;

	// Invoke the tab removed event.
	Event e;
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_REMOVED;
	e.tabBar.tabBar = this;
	e.tabBar.tab = tabs[deleteIndex];
	invokeEvent(e);

	// Delete the tab.
	tabs.erase(tabs.begin() + deleteIndex);
	destroyChildWidget(tab);
}

void TabBar::clearTabs()
{
	for (size_t i = 0; i < tabs.size(); i++)
	{
		// Invoke the tab removed event.
		Event e;
		e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_REMOVED;
		e.tabBar.tabBar = this;
		e.tabBar.tab = tabs[i];
		invokeEvent(e);

		// Destroy the tab.
		destroyChildWidget(tabs[i]);
	}

	tabs.clear();
	selectedTab = NULL;
	scrollValue = 0;
	decrementButton->setVisible(false);
	incrementButton->setVisible(false);
}

Button *TabBar::getDecrementButton()
{
	return decrementButton;
}

Button *TabBar::getIncrementButton()
{
	return incrementButton;
}

TabButton *TabBar::getSelectedTab()
{
	return selectedTab;
}

void TabBar::selectTab(TabButton *tab)
{
	WZ_ASSERT(tab);

	if (selectedTab == tab)
		return; // Already selected.

	tab->set(true);
	selectedTab = tab;

	// Unset all the other tab bar buttons.
	for (size_t i = 0; i < tabs.size(); i++)
	{
		if (tabs[i] != selectedTab)
		{
			tabs[i]->set(false);
		}
	}
	
	invokeTabChanged();
}

void TabBar::addCallbackTabChanged(EventCallback callback)
{
	tab_changed_callbacks.push_back(callback);
}

int TabBar::getScrollValue() const
{
	return scrollValue;
}

void TabBar::onTabButtonPressed(Event e)
{
	selectTab((TabButton *)e.base.widget);
}

void TabBar::setScrollValue(int value)
{
	const int oldValue = scrollValue;
	scrollValue = WZ_CLAMPED(0, value, WZ_MAX(0, (int)tabs.size() - 1));

	if (oldValue != scrollValue)
	{
		// Value has changed.
		updateTabs();
	}
}

void TabBar::invokeTabChanged()
{
	Event e;
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_CHANGED;
	e.tabBar.tabBar = this;
	e.tabBar.tab = selectedTab;
	invokeEvent(e, tab_changed_callbacks);
}

void TabBar::onDecrementButtonClicked(Event e)
{
	setScrollValue(scrollValue - 1);
}

void TabBar::onIncrementButtonClicked(Event e)
{
	setScrollValue(scrollValue + 1);
}

void TabBar::updateScrollButtons()
{
	// Total tab widths.
	int totalTabWidth = 0;

	for (size_t i = 0; i < tabs.size(); i++)
	{
		Size size = tabs[i]->getSize();
		totalTabWidth += size.w;
	}

	// Show/hide the scroll buttons and set their rects.
	bool wereScrollButtonsVisible = decrementButton->getVisible();
	bool showScrollButtons = totalTabWidth > rect.w;

	Rect buttonRect;
	buttonRect.w = decrementButton->getWidth();
	buttonRect.x = rect.w - buttonRect.w * 2;
	buttonRect.y = 0;
	buttonRect.h = rect.h;
	decrementButton->setRectInternal(buttonRect);
	decrementButton->setVisible(showScrollButtons);

	buttonRect.w = incrementButton->getWidth();
	buttonRect.x = rect.w - buttonRect.w;
	buttonRect.y = 0;
	buttonRect.h = rect.h;
	incrementButton->setRectInternal(buttonRect);
	incrementButton->setVisible(showScrollButtons);

	if (wereScrollButtonsVisible && showScrollButtons)
	{
		scrollValue = 0;
	}
}

void TabBar::updateTabs()
{
	// Start at the left edge of the tab bar.
	int x = 0;

	for (size_t i = 0; i < tabs.size(); i++)
	{
		if ((int)i < scrollValue)
		{
			// Scrolled out of view, hide it.
			tabs[i]->setVisible(false);
			continue;
		}
		else
		{
			// Reposition and show.
			Rect tabRect;
			tabRect.x = x;
			tabRect.y = 0;
			tabRect.w = tabs[i]->rect.w;
			tabRect.h = rect.h;
			tabs[i]->setRectInternal(tabRect);
			tabs[i]->setVisible(true);
			x += tabRect.w;
		}
	}
}

} // namespace wz
