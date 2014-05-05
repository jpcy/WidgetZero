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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "stb_arr.h"
#include "wz_internal.h"

struct wzList
{
	struct wzWidget base;
	wzRect itemsRect;
	int itemHeight;
	int nItems;
	int firstItem;
	int selectedItem;
	int pressedItem;
	int hoveredItem;

	// The same as hoveredItem, except when pressedItem != -1.
	int mouseOverItem;

	struct wzScroller *scroller;
};

static void wz_list_update_scroller_max_value(struct wzList *list)
{
	assert(list);
	
	if (list->scroller)
	{
		int max = list->nItems - (int)(list->itemsRect.h / (float)list->itemHeight);
		wz_scroller_set_max_value(list->scroller, max);
	}
}

static void wz_list_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzList *list;

	assert(widget);
	widget->rect = rect;

	// Clip itemsRect to rect. itemsRect should always be equal/smaller.
	list = (struct wzList *)widget;
	list->itemsRect = wzClippedRect(rect, list->itemsRect);
	wz_list_update_scroller_max_value(list);
}

static void wz_list_update_mouse_over_item(struct wzList *list, int mouseX, int mouseY)
{
	wzRect itemsRect, rect;
	int i;

	list->mouseOverItem = -1;

	// Call wz_list_get_items_rect instead of using list->itemsRect so the scroller size is excluded.
	itemsRect = wz_list_get_items_rect(list);
	rect.x = itemsRect.x;
	rect.y = itemsRect.y;
	rect.w = itemsRect.w;
	rect.h = list->itemHeight;

	for (i = list->firstItem; i < list->nItems; i++)
	{
		// Outside widget?
		if (rect.y > itemsRect.y + itemsRect.h)
			break;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, rect))
		{
			list->mouseOverItem = i;
			break;
		}

		rect.y += list->itemHeight;
	}
}

static void wz_list_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzList *list;

	assert(widget);
	list = (struct wzList *)widget;

	if (mouseButton == 1 && list->hoveredItem != -1)
	{
		list->pressedItem = list->hoveredItem;
		list->selectedItem = -1;
		list->hoveredItem = -1;
		wz_desktop_lock_input(widget->desktop, widget);
	}
}

static void wz_list_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzList *list;

	assert(widget);
	list = (struct wzList *)widget;

	if (mouseButton == 1)
	{
		if (list->pressedItem != -1)
		{
			list->selectedItem = list->pressedItem;
			list->pressedItem = -1;
		}

		// Refresh hovered item.
		wz_list_update_mouse_over_item(list, mouseX, mouseY);
		list->hoveredItem = list->mouseOverItem;

		wz_desktop_unlock_input(widget->desktop);
	}
}

static void wz_list_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzList *list;

	assert(widget);
	list = (struct wzList *)widget;
	wz_list_update_mouse_over_item(list, mouseX, mouseY);
	
	if (list->pressedItem != -1)
	{
		if (list->mouseOverItem != -1)
			list->pressedItem = list->mouseOverItem;
	}
	else
	{
		list->hoveredItem = list->mouseOverItem;
	}
}

static void wz_list_mouse_hover_off(struct wzWidget *widget)
{
	struct wzList *list;

	assert(widget);
	list = (struct wzList *)widget;
	list->hoveredItem = -1;
	list->mouseOverItem = -1;
}

static void wz_list_scroller_value_changed(struct wzScroller *scroller, int value)
{
	struct wzList *list;

	assert(scroller);
	list = (struct wzList *)((struct wzWidget *)scroller)->parent;
	list->firstItem = value;
}

static void wz_list_update_scroller_size(struct wzList *list)
{
	wzRect rect;

	assert(list);

	if (!list->scroller)
		return;

	// Fit into items rect. Width doesn't change.
	rect = wz_widget_get_rect((struct wzWidget *)list->scroller);
	rect.x = list->itemsRect.x + list->itemsRect.w - rect.w;
	rect.y = list->itemsRect.y;
	rect.h = list->itemsRect.h;
	wz_widget_set_rect((struct wzWidget *)list->scroller, rect);
}

struct wzList *wz_list_create(struct wzContext *context)
{
	struct wzList *list;

	assert(context);
	list = (struct wzList *)malloc(sizeof(struct wzList));
	memset(list, 0, sizeof(struct wzList));
	list->base.type = WZ_TYPE_LIST;
	list->base.context = context;
	list->base.vtable.set_rect = wz_list_set_rect;
	list->base.vtable.mouse_button_down = wz_list_mouse_button_down;
	list->base.vtable.mouse_button_up = wz_list_mouse_button_up;
	list->base.vtable.mouse_move = wz_list_mouse_move;
	list->base.vtable.mouse_hover_off = wz_list_mouse_hover_off;
	list->selectedItem = -1;
	list->pressedItem = -1;
	list->hoveredItem = -1;
	list->mouseOverItem = -1;

	list->scroller = wz_scroller_create(context, WZ_SCROLLER_VERTICAL);
	wz_widget_add_child_widget((struct wzWidget *)list, (struct wzWidget *)list->scroller);
	wz_list_update_scroller_size(list);
	wz_list_update_scroller_max_value(list);
	wz_scroller_add_callback_value_changed(list->scroller, wz_list_scroller_value_changed);

	return list;
}

struct wzScroller *wz_list_get_scroller(struct wzList *list)
{
	assert(list);
	return list->scroller;
}

void wz_list_set_items_rect(struct wzList *list, wzRect itemsRect)
{
	assert(list);
	list->itemsRect = itemsRect;
	wz_list_update_scroller_size(list);
	wz_list_update_scroller_max_value(list);
}

wzRect wz_list_get_items_rect(const struct wzList *list)
{
	wzRect rect;

	assert(list);
	rect = list->itemsRect;

	// Subtract the scroller width.
	if (list->scroller)
	{
		rect.w -= wz_widget_get_size((struct wzWidget *)list->scroller).w;
	}

	return rect;
}

void wz_list_set_item_height(struct wzList *list, int itemHeight)
{
	assert(list);
	list->itemHeight = itemHeight;
	wz_list_update_scroller_max_value(list);
}

int wz_list_get_item_height(const struct wzList *list)
{
	assert(list);
	return list->itemHeight;
}

void wz_list_set_num_items(struct wzList *list, int nItems)
{
	assert(list);
	list->nItems = WZ_MAX(0, nItems);
	wz_list_update_scroller_max_value(list);
}

int wz_list_get_num_items(const struct wzList *list)
{
	assert(list);
	return list->nItems;
}

int wz_list_get_first_item(const struct wzList *list)
{
	assert(list);
	return list->firstItem;
}

void wz_list_set_selected_item(struct wzList *list, int selectedItem)
{
	assert(list);
	list->selectedItem = selectedItem;
}

int wz_list_get_selected_item(const struct wzList *list)
{
	assert(list);
	return list->selectedItem;
}

int wz_list_get_pressed_item(const struct wzList *list)
{
	assert(list);
	return list->pressedItem;
}

int wz_list_get_hovered_item(const struct wzList *list)
{
	assert(list);
	return list->hoveredItem;
}
