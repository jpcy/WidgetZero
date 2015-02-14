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

// Show the scroll buttons if they're required, hides them if they're not.
static void wz_tab_bar_update_scroll_buttons(TabBar *tabBar)
{
	int totalTabWidth;
	bool wereScrollButtonsVisible, showScrollButtons;
	Rect rect;

	WZ_ASSERT(tabBar);

	// Total tab widths.
	totalTabWidth = 0;

	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		Size size = tabBar->tabs[i]->getSize();
		totalTabWidth += size.w;
	}

	// Show/hide the scroll buttons and set their rects.
	wereScrollButtonsVisible = tabBar->decrementButton->getVisible();
	showScrollButtons = totalTabWidth > tabBar->rect.w;

	rect.w = tabBar->decrementButton->getWidth();
	rect.x = tabBar->rect.w - rect.w * 2;
	rect.y = 0;
	rect.h = tabBar->rect.h;
	tabBar->decrementButton->setRectInternal(rect);
	tabBar->decrementButton->setVisible(showScrollButtons);

	rect.w = tabBar->incrementButton->getWidth();
	rect.x = tabBar->rect.w - rect.w;
	rect.y = 0;
	rect.h = tabBar->rect.h;
	tabBar->incrementButton->setRectInternal(rect);
	tabBar->incrementButton->setVisible(showScrollButtons);

	if (wereScrollButtonsVisible && showScrollButtons)
	{
		tabBar->scrollValue = 0;
	}
}

static void wz_tab_bar_update_tabs(TabBar *tabBar)
{
	int x;

	WZ_ASSERT(tabBar);

	// Start at the left edge of the tab bar.
	x = 0;

	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		Widget *widget = tabBar->tabs[i];

		if ((int)i < tabBar->scrollValue)
		{
			// Scrolled out of view, hide it.
			widget->setVisible(false);
			continue;
		}
		else
		{
			// Reposition and show.
			Rect rect;
			rect.x = x;
			rect.y = 0;
			rect.w = widget->rect.w;
			rect.h = tabBar->rect.h;
			widget->setRectInternal(rect);
			widget->setVisible(true);
			x += rect.w;
		}
	}
}

// Sets the scroll value, and repositions and shows/hides the tabs accordingly.
static void wz_tab_bar_set_scroll_value(TabBar *tabBar, int value)
{
	int oldValue = tabBar->scrollValue;
	tabBar->scrollValue = WZ_CLAMPED(0, value, WZ_MAX(0, (int)tabBar->tabs.size() - 1));

	if (oldValue != tabBar->scrollValue)
	{
		// Value has changed.
		wz_tab_bar_update_tabs(tabBar);
	}
}

static void wz_tab_bar_invoke_tab_changed(TabBar *tabBar)
{
	Event e;

	WZ_ASSERT(tabBar);
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_CHANGED;
	e.tabBar.tabBar = tabBar;
	e.tabBar.tab = tabBar->selectedTab;
	tabBar->invokeEvent(&e, tabBar->tab_changed_callbacks);
}

// Sets the tab bar (the button's parent) selected tab.
static void wz_tab_bar_button_pressed(Event *e)
{
	WZ_ASSERT(e);
	((TabBar *)e->base.widget->parent)->selectTab((TabButton *)e->base.widget);
}

static void wz_tab_bar_decrement_button_clicked(Event *e)
{
	TabBar *tabBar;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	tabBar = (TabBar *)e->base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue - 1);
}

static void wz_tab_bar_increment_button_clicked(Event *e)
{
	TabBar *tabBar;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	tabBar = (TabBar *)e->base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue + 1);
}

TabBar::TabBar()
{
	type = WZ_TYPE_TAB_BAR;
	selectedTab = NULL;
	scrollValue = 0;

	// Set to draw last so the scroll buttons always overlap the tabs.
	decrementButton = new Button("<");
	decrementButton->addCallbackClicked(wz_tab_bar_decrement_button_clicked);
	addChildWidget(decrementButton);
	decrementButton->setWidthInternal(WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH);
	decrementButton->setVisible(false);
	decrementButton->setDrawLast(true);
	decrementButton->setOverlap(true);

	incrementButton = new Button(">");
	incrementButton->addCallbackClicked(wz_tab_bar_increment_button_clicked);
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

	wz_tab_bar_update_tabs(this);
	wz_tab_bar_update_scroll_buttons(this);
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
	tab->addCallbackPressed(wz_tab_bar_button_pressed);
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
		wz_tab_bar_invoke_tab_changed(this);
	}

	wz_tab_bar_update_scroll_buttons(this);

	// Invoke the tab added event.
	Event e;
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_ADDED;
	e.tabBar.tabBar = this;
	e.tabBar.tab = tab;
	invokeEvent(&e);

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
	invokeEvent(&e);

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
		invokeEvent(&e);

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
	
	wz_tab_bar_invoke_tab_changed(this);
}

void TabBar::addCallbackTabChanged(EventCallback callback)
{
	tab_changed_callbacks.push_back(callback);
}

int TabBar::getScrollValue() const
{
	return scrollValue;
}

} // namespace wz
