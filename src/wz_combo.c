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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "wz_desktop.h"
#include "wz_widget.h"

struct wzCombo
{
	struct wzWidget base;
	bool isOpen;
	struct wzList *list;
};

static void wz_combo_update_list_rect(struct wzCombo *combo)
{
	wzRect rect, absRect, listRect;
	wzBorder listItemsBorder;
	int listItemHeight, listNumItems, over;

	// Don't do anything if the desktop is NULL (widget hasn't been added yet).
	if (!combo->base.desktop)
		return;

	if (!combo->list)
		return;

	rect = wz_widget_get_rect((struct wzWidget *)combo);
	absRect = wz_widget_get_absolute_rect((struct wzWidget *)combo);

	// Set list rect.
	listRect.x = 0;
	listRect.y = rect.h;
	listRect.w = rect.w;

	// Make the height large enough to avoid scrolling.
	listItemsBorder = wz_list_get_items_border(combo->list);
	listItemHeight = wz_list_get_item_height(combo->list);
	listNumItems = wz_list_get_num_items(combo->list);
	listRect.h = listItemsBorder.top + listItemHeight * listNumItems + listItemsBorder.bottom;

	// Clip the height to the desktop.
	// Need to use absolute widget rect y coord to take into account parent window position.
	over = absRect.y + rect.h + listRect.h - wz_widget_get_size((struct wzWidget *)combo->base.desktop).h;
	
	if (over > 0)
	{
		listRect.h -= over;
	}

	wz_widget_set_rect((struct wzWidget *)combo->list, listRect);
}

static void wz_combo_set_rect(struct wzWidget *widget, wzRect rect)
{
	widget->rect = rect;
	wz_combo_update_list_rect((struct wzCombo *)widget);
}

static void wz_combo_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzCombo *combo;
	wzRect listRect;

	assert(widget);
	combo = (struct wzCombo *)widget;

	if (mouseButton == 1)
	{
		listRect = wz_widget_get_absolute_rect((struct wzWidget *)combo->list);

		// Open dropdown.
		if (!combo->isOpen)
		{
			// Lock input.
			wz_desktop_push_lock_input_widget(widget->desktop, widget);

			// Show dropdown list and set it to draw last.
			wz_widget_set_visible((struct wzWidget *)combo->list, true);
			wz_combo_update_list_rect(combo);

			combo->isOpen = true;
		}
		// Close dropdown.
		// Don't do it if the mouse cursor is over the dropdown list.
		else if (!WZ_POINT_IN_RECT(mouseX, mouseY, listRect))
		{
			// Unlock input.
			wz_desktop_pop_lock_input_widget(widget->desktop, widget);

			// Hide dropdown list.
			wz_widget_set_visible((struct wzWidget *)combo->list, false);

			combo->isOpen = false;
		}
	}
}

static wzRect wz_combo_get_children_clip_rect(struct wzWidget *widget)
{
	// Don't clip children.
	wzRect zero;
	zero.x = zero.y = zero.w = zero.h = 0;
	return zero;
}

static void wz_combo_list_item_selected(wzEvent e)
{
	struct wzCombo *combo;

	assert(e.base.widget);
	combo = (struct wzCombo *)e.base.widget->parent;

	// Unlock input.
	wz_desktop_pop_lock_input_widget(combo->base.desktop, (struct wzWidget *)combo);

	// Hide dropdown list.
	wz_widget_set_visible((struct wzWidget *)combo->list, false);

	combo->isOpen = false;
}

struct wzCombo *wz_combo_create()
{
	struct wzCombo *combo;

	combo = (struct wzCombo *)malloc(sizeof(struct wzCombo));
	memset(combo, 0, sizeof(struct wzCombo));
	combo->base.type = WZ_TYPE_COMBO;
	combo->base.vtable.set_rect = wz_combo_set_rect;
	combo->base.vtable.mouse_button_down = wz_combo_mouse_button_down;
	combo->base.vtable.get_children_clip_rect = wz_combo_get_children_clip_rect;
	return combo;
}

void wz_combo_set_list(struct wzCombo *combo, struct wzList *list)
{
	assert(combo);
	assert(list);

	if (combo->list)
		return;

	combo->list = list;
	wz_widget_set_draw_priority((struct wzWidget *)combo->list, WZ_DRAW_PRIORITY_COMBO_DROPDOWN);
	wz_widget_add_child_widget_internal((struct wzWidget *)combo, (struct wzWidget *)combo->list);
	wz_widget_set_visible((struct wzWidget *)combo->list, false);
	wz_widget_set_clip_input_to_parent((struct wzWidget *)combo->list, false);
	wz_list_add_callback_item_selected(combo->list, wz_combo_list_item_selected);
}

struct wzList *wz_combo_get_list(struct wzCombo *combo)
{
	assert(combo);
	return combo->list;
}

bool wz_combo_is_open(struct wzCombo *combo)
{
	assert(combo);
	return combo->isOpen;
}
