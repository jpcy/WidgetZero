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
#include <vector>
#include <stdlib.h>
#include <string.h>
#include "wz_main_window.h"
#include "wz_renderer.h"
#include "wz_widget.h"
#include "wz_button.h"
#include "wz_skin.h"

namespace wz {

struct wzTabBar : public wzWidget
{
	wzTabBar();

	struct wzButton *selectedTab;
	std::vector<struct wzButton *> tabs;

	int scrollValue;
	struct wzButton *decrementButton;
	struct wzButton *incrementButton;

	std::vector<wzEventCallback> tab_changed_callbacks;
};

/*
================================================================================

TAB BUTTON

================================================================================
*/

static void wz_tab_button_draw(struct wzWidget *widget, wzRect clip)
{
	wzRect labelRect;
	struct wzButton *button = (struct wzButton *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzRect rect = wz_widget_get_absolute_rect(widget);
	const wzBorder padding = wz_button_get_padding(button);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);

	// Label.
	labelRect.x = rect.x + padding.left;
	labelRect.y = rect.y + padding.top;
	labelRect.w = rect.w - (padding.left + padding.right);
	labelRect.h = rect.h - (padding.top + padding.bottom);
	wz_renderer_print(widget->renderer, labelRect.x + labelRect.w / 2, labelRect.y + labelRect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, widget->hover ? WZ_SKIN_TAB_BUTTON_TEXT_HOVER_COLOR : WZ_SKIN_TAB_BUTTON_TEXT_COLOR, wz_button_get_label(button), 0);

	nvgRestore(vg);
}

static struct wzButton *wz_tab_button_create()
{
	struct wzButton *button = wz_button_create(NULL, NULL);
	struct wzWidget *widget = (struct wzWidget *)button;
	widget->vtable.draw = wz_tab_button_draw;
	return button;
}

/*
================================================================================

TAB BAR

================================================================================
*/

// Ensure tab height stay the same as the tab bar.
static void wz_tab_set_rect(struct wzWidget *widget, wzRect rect)
{
	WZ_ASSERT(widget);
	rect.h = ((struct wzTabBar *)widget->parent)->rect.h;
	widget->rect = rect;
}

// Show the scroll buttons if they're required, hides them if they're not.
static void wz_tab_bar_update_scroll_buttons(struct wzTabBar *tabBar)
{
	int totalTabWidth;
	bool wereScrollButtonsVisible, showScrollButtons;
	wzRect rect;

	WZ_ASSERT(tabBar);

	// Total tab widths.
	totalTabWidth = 0;

	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		wzSize size = wz_widget_get_size((struct wzWidget *)tabBar->tabs[i]);
		totalTabWidth += size.w;
	}

	// Show/hide the scroll buttons and set their rects.
	wereScrollButtonsVisible = wz_widget_get_visible((struct wzWidget *)tabBar->decrementButton);
	showScrollButtons = totalTabWidth > tabBar->rect.w;

	rect.w = wz_widget_get_size((struct wzWidget *)tabBar->decrementButton).w;
	rect.x = tabBar->rect.w - rect.w * 2;
	rect.y = 0;
	rect.h = tabBar->rect.h;
	wz_widget_set_rect_internal((struct wzWidget *)tabBar->decrementButton, rect);
	wz_widget_set_visible((struct wzWidget *)tabBar->decrementButton, showScrollButtons);

	rect.w = wz_widget_get_size((struct wzWidget *)tabBar->incrementButton).w;
	rect.x = tabBar->rect.w - rect.w;
	rect.y = 0;
	rect.h = tabBar->rect.h;
	wz_widget_set_rect_internal((struct wzWidget *)tabBar->incrementButton, rect);
	wz_widget_set_visible((struct wzWidget *)tabBar->incrementButton, showScrollButtons);

	if (wereScrollButtonsVisible && showScrollButtons)
	{
		tabBar->scrollValue = 0;
	}
}

static void wz_tab_bar_update_tabs(struct wzTabBar *tabBar)
{
	int x;

	WZ_ASSERT(tabBar);

	// Start at the left edge of the tab bar.
	x = 0;

	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		struct wzWidget *widget = (struct wzWidget *)tabBar->tabs[i];

		if ((int)i < tabBar->scrollValue)
		{
			// Scrolled out of view, hide it.
			wz_widget_set_visible(widget, false);
			continue;
		}
		else
		{
			// Reposition and show.
			wzRect rect;
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
static void wz_tab_bar_set_scroll_value(struct wzTabBar *tabBar, int value)
{
	int oldValue = tabBar->scrollValue;
	tabBar->scrollValue = WZ_CLAMPED(0, value, WZ_MAX(0, (int)tabBar->tabs.size() - 1));

	if (oldValue != tabBar->scrollValue)
	{
		// Value has changed.
		wz_tab_bar_update_tabs(tabBar);
	}
}

static void wz_tab_bar_invoke_tab_changed(struct wzTabBar *tabBar)
{
	wzEvent e;

	WZ_ASSERT(tabBar);
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_CHANGED;
	e.tabBar.tabBar = tabBar;
	e.tabBar.tab = tabBar->selectedTab;
	wz_invoke_event(&e, tabBar->tab_changed_callbacks);
}

// Sets the tab bar (the button's parent) selected tab.
static void wz_tab_bar_button_pressed(wzEvent *e)
{
	WZ_ASSERT(e);
	wz_tab_bar_select_tab((struct wzTabBar *)e->base.widget->parent, (struct wzButton *)e->base.widget);
}

static wzSize wz_tab_bar_measure(struct wzWidget *widget)
{
	wzSize size;
	struct wzTabBar *tabBar = (struct wzTabBar *)widget;
	size.h = wz_widget_get_line_height(widget) + 8; // Padding.
	return size;
}

static void wz_tab_bar_set_rect(struct wzWidget *widget, wzRect rect)
{
	WZ_ASSERT(widget);
	widget->rect = rect;

	// Set button heights to match.
	for (size_t i = 0; i < widget->children.size(); i++)
	{
		wz_widget_set_height_internal(widget->children[i], rect.h);
	}

	wz_tab_bar_update_tabs((struct wzTabBar *)widget);
	wz_tab_bar_update_scroll_buttons((struct wzTabBar *)widget);
}

static wzRect wz_tab_bar_get_children_clip_rect(struct wzWidget *widget)
{
	return wz_widget_get_absolute_rect(widget);
}

static void wz_tab_bar_decrement_button_clicked(wzEvent *e)
{
	struct wzTabBar *tabBar;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	tabBar = (struct wzTabBar *)e->base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue - 1);
}

static void wz_tab_bar_increment_button_clicked(wzEvent *e)
{
	struct wzTabBar *tabBar;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	tabBar = (struct wzTabBar *)e->base.widget->parent;
	wz_tab_bar_set_scroll_value(tabBar, tabBar->scrollValue + 1);
}

wzTabBar::wzTabBar()
{
	type = WZ_TYPE_TAB_BAR;
	selectedTab = NULL;
	scrollValue = 0;
}

struct wzTabBar *wz_tab_bar_create()
{
	struct wzTabBar *tabBar = new struct wzTabBar;
	tabBar->vtable.measure = wz_tab_bar_measure;
	tabBar->vtable.set_rect = wz_tab_bar_set_rect;
	tabBar->vtable.get_children_clip_rect = wz_tab_bar_get_children_clip_rect;

	// Set to draw last so the scroll buttons always overlap the tabs.
	tabBar->decrementButton = wz_button_create("<", NULL);
	wz_button_add_callback_clicked(tabBar->decrementButton, wz_tab_bar_decrement_button_clicked);
	wz_widget_add_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)tabBar->decrementButton);
	wz_widget_set_width_internal((struct wzWidget *)tabBar->decrementButton, WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH);
	wz_widget_set_visible((struct wzWidget *)tabBar->decrementButton, false);
	wz_widget_set_draw_last((struct wzWidget *)tabBar->decrementButton, true);
	wz_widget_set_overlap((struct wzWidget *)tabBar->decrementButton, true);

	tabBar->incrementButton = wz_button_create(">", NULL);
	wz_button_add_callback_clicked(tabBar->incrementButton, wz_tab_bar_increment_button_clicked);
	wz_widget_add_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)tabBar->incrementButton);
	wz_widget_set_width_internal((struct wzWidget *)tabBar->incrementButton, WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH);
	wz_widget_set_visible((struct wzWidget *)tabBar->incrementButton, false);
	wz_widget_set_draw_last((struct wzWidget *)tabBar->incrementButton, true);
	wz_widget_set_overlap((struct wzWidget *)tabBar->incrementButton, true);

	return tabBar;
}

struct wzButton *wz_tab_bar_create_tab(struct wzTabBar *tabBar)
{
	struct wzButton *tab;
	wzRect rect;
	wzEvent e;

	WZ_ASSERT(tabBar);
	tab = wz_tab_button_create();

	// Position to the right of the last tab.
	rect.x = rect.y = 0;
	rect.w = 50; // Default width.
	rect.h = tabBar->rect.h;

	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		wzSize size = wz_widget_get_size((struct wzWidget *)tabBar->tabs[i]);
		rect.x += size.w;
	}

	((struct wzWidget *)tab)->vtable.set_rect = wz_tab_set_rect;
	wz_button_add_callback_pressed(tab, wz_tab_bar_button_pressed);
	wz_button_set_click_behavior(tab, WZ_BUTTON_CLICK_BEHAVIOR_DOWN);
	wz_button_set_set_behavior(tab, WZ_BUTTON_SET_BEHAVIOR_STICKY);
	wz_widget_add_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)tab);
	tabBar->tabs.push_back(tab);
	wz_widget_set_rect_internal((struct wzWidget *)tab, rect);

	// Select the first tab added.
	if (!tabBar->selectedTab)
	{
		wz_button_set(tab, true);
		tabBar->selectedTab = tab;
		wz_tab_bar_invoke_tab_changed(tabBar);
	}

	wz_tab_bar_update_scroll_buttons(tabBar);

	// Invoke the tab added event.
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_ADDED;
	e.tabBar.tabBar = tabBar;
	e.tabBar.tab = tab;
	wz_invoke_event(&e);

	return tab;
}

void wz_tab_bar_destroy_tab(struct wzTabBar *tabBar, struct wzButton *tab)
{
	int deleteIndex;
	wzEvent e;

	WZ_ASSERT(tabBar);
	WZ_ASSERT(tab);
	deleteIndex = -1;

	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		if (tabBar->tabs[i] == tab)
		{
			deleteIndex = (int)i;
			break;
		}
	}

	if (deleteIndex == -1)
		return;

	// Invoke the tab removed event.
	e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_REMOVED;
	e.tabBar.tabBar = tabBar;
	e.tabBar.tab = tabBar->tabs[deleteIndex];
	wz_invoke_event(&e);

	// Delete the tab.
	tabBar->tabs.erase(tabBar->tabs.begin() + deleteIndex);
	wz_widget_destroy_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)tab);
}

void wz_tab_bar_clear_tabs(struct wzTabBar *tabBar)
{
	WZ_ASSERT(tabBar);

	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		wzEvent e;

		// Invoke the tab removed event.
		e.tabBar.type = WZ_EVENT_TAB_BAR_TAB_REMOVED;
		e.tabBar.tabBar = tabBar;
		e.tabBar.tab = tabBar->tabs[i];
		wz_invoke_event(&e);

		// Destroy the tab.
		wz_widget_destroy_child_widget((struct wzWidget *)tabBar, (struct wzWidget *)tabBar->tabs[i]);
	}

	tabBar->tabs.clear();
	tabBar->selectedTab = NULL;
	tabBar->scrollValue = 0;
	wz_widget_set_visible((struct wzWidget *)tabBar->decrementButton, false);
	wz_widget_set_visible((struct wzWidget *)tabBar->incrementButton, false);
}

struct wzButton *wz_tab_bar_get_decrement_button(struct wzTabBar *tabBar)
{
	WZ_ASSERT(tabBar);
	return tabBar->decrementButton;
}

struct wzButton *wz_tab_bar_get_increment_button(struct wzTabBar *tabBar)
{
	WZ_ASSERT(tabBar);
	return tabBar->incrementButton;
}

struct wzButton *wz_tab_bar_get_selected_tab(struct wzTabBar *tabBar)
{
	WZ_ASSERT(tabBar);
	return tabBar->selectedTab;
}

void wz_tab_bar_select_tab(struct wzTabBar *tabBar, struct wzButton *tab)
{
	WZ_ASSERT(tabBar);
	WZ_ASSERT(tab);

	if (tabBar->selectedTab == tab)
		return; // Already selected.

	wz_button_set(tab, true);
	tabBar->selectedTab = tab;

	// Unset all the other tab bar buttons.
	for (size_t i = 0; i < tabBar->tabs.size(); i++)
	{
		if (tabBar->tabs[i] != tabBar->selectedTab)
		{
			wz_button_set(tabBar->tabs[i], false);
		}
	}
	
	wz_tab_bar_invoke_tab_changed(tabBar);
}

void wz_tab_bar_add_callback_tab_changed(struct wzTabBar *tabBar, wzEventCallback callback)
{
	WZ_ASSERT(tabBar);
	tabBar->tab_changed_callbacks.push_back(callback);
}

int wz_tab_bar_get_scroll_value(const struct wzTabBar *tabBar)
{
	WZ_ASSERT(tabBar);
	return tabBar->scrollValue;
}

} // namespace wz
