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
	int selectedItem;
	int hoveredItem;
	struct wzScroller *scroller;
};

static void wz_list_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
}

static void wz_list_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
}

static void wz_list_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
}

struct wzList *wz_list_create(struct wzWindow *window)
{
	struct wzList *list;

	assert(window);
	list = (struct wzList *)malloc(sizeof(struct wzList));
	memset(list, 0, sizeof(struct wzList));
	list->base.type = WZ_TYPE_LIST;
	list->base.window = window;
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
}

void wz_list_set_items_rect(struct wzList *list, wzRect itemsRect)
{
	assert(list);
	memcpy(&list->itemsRect, &itemsRect, sizeof(wzRect));
	wz_list_update_scroller_size(list);
}

wzRect wz_list_get_items_rect(struct wzList *list)
{
	wzRect rect;

	assert(list);
	memcpy(&rect, &list->itemsRect, sizeof(wzRect));

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

int wz_list_get_item_height(struct wzList *list)
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

int wz_list_get_num_items(struct wzList *list)
{
	assert(list);
	return list->nItems;
}

void wz_list_set_selected_item(struct wzList *list, int selectedItem)
{
	assert(list);
	list->selectedItem = selectedItem;
}

int wz_list_get_selected_item(struct wzList *list)
{
	assert(list);
	return list->selectedItem;
}

int wz_list_get_hovered_item(struct wzList *list)
{
	assert(list);
	return list->hoveredItem;
}
