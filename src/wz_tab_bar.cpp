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

TAB BUTTON

================================================================================
*/

static void wz_tab_button_draw(struct WidgetImpl *widget, Rect clip)
{
	Rect labelRect;
	struct ButtonImpl *button = (struct ButtonImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const Rect rect = wz_widget_get_absolute_rect(widget);
	const Border padding = button->getPadding();

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);

	// Label.
	labelRect.x = rect.x + padding.left;
	labelRect.y = rect.y + padding.top;
	labelRect.w = rect.w - (padding.left + padding.right);
	labelRect.h = rect.h - (padding.top + padding.bottom);
	wz_renderer_print(widget->renderer, labelRect.x + labelRect.w / 2, labelRect.y + labelRect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, widget->hover ? WZ_SKIN_TAB_BUTTON_TEXT_HOVER_COLOR : WZ_SKIN_TAB_BUTTON_TEXT_COLOR, button->getLabel(), 0);

	nvgRestore(vg);
}

static struct ButtonImpl *wz_tab_button_create()
{
	struct ButtonImpl *button = new ButtonImpl;
	struct WidgetImpl *widget = button;
	widget->vtable.draw = wz_tab_button_draw;
	return button;
}

/*
================================================================================

TAB BAR

================================================================================
*/

// Ensure tab height stay the same as the tab bar.
static void wz_tab_set_rect(struct WidgetImpl *widget, Rect rect)
{
	WZ_ASSERT(widget);
	rect.h = ((struct TabBarImpl *)widget->parent)->rect.h;
	widget->rect = rect;
}

// Show the scroll buttons if they're required, hides them if they're not.
static void wz_tab_bar_update_scroll_buttons(struct TabBarImpl *tabBar)
{
	int totalTabWidth;
	bool wereScrollButtonsVisible, showScrollButtons;
	Rect rect;

	WZ_ASSERT(tabBar);

	// Total tab widths.
	totalTabWidth = 0;

	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		Size size = wz_widget_get_size(tabBar->tabs[i]);
		totalTabWidth += size.w;
	}

	// Show/hide the scroll buttons and set their rects.
	wereScrollButtonsVisible = wz_widget_get_visible(tabBar->decrementButton);
	showScrollButtons = totalTabWidth > tabBar->rect.w;

	rect.w = wz_widget_get_size(tabBar->decrementButton).w;
	rect.x = tabBar->rect.w - rect.w * 2;
	rect.y = 0;
	rect.h = tabBar->rect.h;
	wz_widget_set_rect_internal(tabBar->decrementButton, rect);
	wz_widget_set_visible(tabBar->decrementButton, showScrollButtons);

	rect.w = wz_widget_get_size(tabBar->incrementButton).w;
	rect.x = tabBar->rect.w - rect.w;
	rect.y = 0;
	rect.h = tabBar->rect.h;
	wz_widget_set_rect_internal(tabBar->incrementButton, rect);
	wz_widget_set_visible(tabBar->incrementButton, showScrollButtons);

	if (wereScrollButtonsVisible && showScrollButtons)
	{
		tabBar->scrollValue = 0;
	}
}

static void wz_tab_bar_update_tabs(struct TabBarImpl *tabBar)
{
	int x;

	WZ_ASSERT(tabBar);

	// Start at the left edge of the tab bar.
	x = 0;

	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		struct WidgetImpl *widget = tabBar->tabs[i];

		if ((int)i < tabBar->scrollValue)
		{
			// Scrolled out of view, hide it.
			wz_widget_set_visible(widget, false);
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
			wz_widget_set_rect_internal(widget, rect);
			wz_widget_set_visible(widget, true);
			x += rect.w;
		}
	}
}

// Sets the scroll value, and repositions and shows/hides the tabs accordingly.
static void wz_tab_bar_set_scroll_value(struct TabBarImpl *tabBar, int value)
{
	int oldValue = tabBar->scrollValue;
	tabBar->scrollValue = WZ_CLAMPED(0, value, WZ_MAX(0, (int)tabBar->tabs.size() - 1));

	if (oldValue != tabBar->scrollValue)
	{
		// Value has changed.
		wz_tab_bar_update_tabs(tabBar);
	}
}

static void wz_tab_bar_invoke_tab_changed(struct TabBarImpl *tabBar)
{
	Event e;

	WZ_ASSERT(tabBar);
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_CHANGED;
	e.tabBar.tabBar = tabBar;
	e.tabBar.tab = tabBar->selectedTab;
	wz_invoke_event(&e, tabBar->tab_changed_callbacks);
}

// Sets the tab bar (the button's parent) selected tab.
static void wz_tab_bar_button_pressed(Event *e)
{
	WZ_ASSERT(e);
	((struct TabBarImpl *)e->base.widget->parent)->selectTab((struct ButtonImpl *)e->base.widget);
}

static Size wz_tab_bar_measure(struct WidgetImpl *widget)
{
	Size size;
	struct TabBarImpl *tabBar = (struct TabBarImpl *)widget;
	size.h = wz_widget_get_line_height(widget) + 8; // Padding.
	return size;
}

static void wz_tab_bar_set_rect(struct WidgetImpl *widget, Rect rect)
{
	WZ_ASSERT(widget);
	widget->rect = rect;

	// Set button heights to match.
	for (size_t i = 0; i < widget->children.size(); i++)
	{
		wz_widget_set_height_internal(widget->children[i], rect.h);
	}

	wz_tab_bar_update_tabs((struct TabBarImpl *)widget);
	wz_tab_bar_update_scroll_buttons((struct TabBarImpl *)widget);
}

static Rect wz_tab_bar_get_children_clip_rect(struct WidgetImpl *widget)
{
	return wz_widget_get_absolute_rect(widget);
}

static void wz_tab_bar_decrement_button_clicked(Event *e)
{
	struct TabBarImpl *tabBar;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	tabBar = (struct TabBarImpl *)e->base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue - 1);
}

static void wz_tab_bar_increment_button_clicked(Event *e)
{
	struct TabBarImpl *tabBar;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	tabBar = (struct TabBarImpl *)e->base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue + 1);
}

TabBarImpl::TabBarImpl()
{
	type = WZ_TYPE_TAB_BAR;
	selectedTab = NULL;
	scrollValue = 0;
	vtable.measure = wz_tab_bar_measure;
	vtable.set_rect = wz_tab_bar_set_rect;
	vtable.get_children_clip_rect = wz_tab_bar_get_children_clip_rect;

	// Set to draw last so the scroll buttons always overlap the tabs.
	decrementButton = new ButtonImpl("<");
	decrementButton->addCallbackClicked(wz_tab_bar_decrement_button_clicked);
	wz_widget_add_child_widget(this, decrementButton);
	wz_widget_set_width_internal(decrementButton, WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH);
	wz_widget_set_visible(decrementButton, false);
	wz_widget_set_draw_last(decrementButton, true);
	wz_widget_set_overlap(decrementButton, true);

	incrementButton = new ButtonImpl(">");
	incrementButton->addCallbackClicked(wz_tab_bar_increment_button_clicked);
	wz_widget_add_child_widget(this, incrementButton);
	wz_widget_set_width_internal(incrementButton, WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH);
	wz_widget_set_visible(incrementButton, false);
	wz_widget_set_draw_last(incrementButton, true);
	wz_widget_set_overlap(incrementButton, true);
}

struct ButtonImpl *TabBarImpl::createTab()
{
	struct ButtonImpl *tab = wz_tab_button_create();

	// Position to the right of the last tab.
	Rect rect;
	rect.x = rect.y = 0;
	rect.w = 50; // Default width.
	rect.h = this->rect.h;

	for (size_t i = 0; i < tabs.size(); i++)
	{
		Size size = wz_widget_get_size(tabs[i]);
		rect.x += size.w;
	}

	(tab)->vtable.set_rect = wz_tab_set_rect;
	tab->addCallbackPressed(wz_tab_bar_button_pressed);
	tab->setClickBehavior(WZ_BUTTON_CLICK_BEHAVIOR_DOWN);
	tab->setSetBehavior(WZ_BUTTON_SET_BEHAVIOR_STICKY);
	wz_widget_add_child_widget(this, tab);
	tabs.push_back(tab);
	wz_widget_set_rect_internal(tab, rect);

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
	wz_invoke_event(&e);

	return tab;
}

void TabBarImpl::destroyTab(struct ButtonImpl *tab)
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
	wz_invoke_event(&e);

	// Delete the tab.
	tabs.erase(tabs.begin() + deleteIndex);
	wz_widget_destroy_child_widget(this, tab);
}

void TabBarImpl::clearTabs()
{
	for (size_t i = 0; i < tabs.size(); i++)
	{
		// Invoke the tab removed event.
		Event e;
		e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_REMOVED;
		e.tabBar.tabBar = this;
		e.tabBar.tab = tabs[i];
		wz_invoke_event(&e);

		// Destroy the tab.
		wz_widget_destroy_child_widget(this, tabs[i]);
	}

	tabs.clear();
	selectedTab = NULL;
	scrollValue = 0;
	wz_widget_set_visible(decrementButton, false);
	wz_widget_set_visible(incrementButton, false);
}

struct ButtonImpl *TabBarImpl::getDecrementButton()
{
	return decrementButton;
}

struct ButtonImpl *TabBarImpl::getIncrementButton()
{
	return incrementButton;
}

struct ButtonImpl *TabBarImpl::getSelectedTab()
{
	return selectedTab;
}

void TabBarImpl::selectTab(struct ButtonImpl *tab)
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

void TabBarImpl::addCallbackTabChanged(EventCallback callback)
{
	tab_changed_callbacks.push_back(callback);
}

int TabBarImpl::getScrollValue() const
{
	return scrollValue;
}

} // namespace wz
