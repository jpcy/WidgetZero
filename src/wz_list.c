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
	int hoveredItem;
	struct wzScroller *scroller;
};

static void wz_list_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzList *list;
	int delta;

	assert(widget);
	widget->rect = rect;

	// Clip itemsRect to rect. itemsRect should always be equal/smaller.
	list = (struct wzList *)widget;
	list->itemsRect.x = WZ_MAX(rect.x, list->itemsRect.x);
	list->itemsRect.y = WZ_MAX(rect.y, list->itemsRect.y);

	delta = (list->itemsRect.x + list->itemsRect.w) - (rect.x + rect.w);

	if (delta > 0)
		list->itemsRect.w -= delta;

	delta = (list->itemsRect.y + list->itemsRect.h) - (rect.y + rect.h);

	if (delta > 0)
		list->itemsRect.h -= delta;
}

static void wz_list_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
}

static void wz_list_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
}

static void wz_list_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzList *list;
	wzRect itemsRect, rect;
	int i;

	assert(widget);
	list = (struct wzList *)widget;
	
	// Check for item hover.
	list->hoveredItem = -1;

	// Call wz_list_get_items_rect instead of using list->itemsRect so the scroller size is excluded.
	itemsRect = wz_list_get_items_rect(list);
	rect.x = itemsRect.x;
	rect.y = itemsRect.y;
	rect.w = itemsRect.w;
	rect.h = list->itemHeight;

	for (i = 0; i < list->nItems; i++)
	{
		// Outside widget?
		if (rect.y > itemsRect.y + itemsRect.h)
			break;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, rect))
		{
			list->hoveredItem = list->firstItem + i;
			break;
		}

		rect.y += list->itemHeight;
	}
}

static void wz_list_scroller_value_changed(struct wzScroller *scroller, int value)
{
	struct wzList *list;

	assert(scroller);
	list = (struct wzList *)((struct wzWidget *)scroller)->parent;
	list->firstItem = value;
}

struct wzList *wz_list_create(struct wzWindow *window)
{
	struct wzList *list;

	assert(window);
	list = (struct wzList *)malloc(sizeof(struct wzList));
	memset(list, 0, sizeof(struct wzList));
	list->base.type = WZ_TYPE_LIST;
	list->base.window = window;
	list->base.vtable.set_rect = wz_list_set_rect;
	list->base.vtable.mouse_button_down = wz_list_mouse_button_down;
	list->base.vtable.mouse_button_up = wz_list_mouse_button_up;
	list->base.vtable.mouse_move = wz_list_mouse_move;
	list->selectedItem = -1;
	list->hoveredItem = -1;
	return list;
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

void wz_list_set_scroller(struct wzList *list, struct wzScroller *scroller)
{
	assert(list);
	assert(scroller);
	wz_widget_add_child_widget((struct wzWidget *)list, (struct wzWidget *)scroller);
	list->scroller = scroller;
	wz_list_update_scroller_size(list);
	wz_scroller_set_max_value(list->scroller, list->nItems);
	wz_scroller_add_callback_value_changed(list->scroller, wz_list_scroller_value_changed);
}

void wz_list_set_items_rect(struct wzList *list, wzRect itemsRect)
{
	assert(list);
	list->itemsRect = itemsRect;
	wz_list_update_scroller_size(list);
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

	if (list->scroller)
		wz_scroller_set_max_value(list->scroller, list->nItems);
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

int wz_list_get_hovered_item(const struct wzList *list)
{
	assert(list);
	return list->hoveredItem;
}
