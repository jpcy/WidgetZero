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
#include "wz_internal.h"

struct wzList
{
	struct wzWidget base;
	wzBorder itemsBorder;
	int itemHeight;
	int nItems;
	int firstItem;
	int selectedItem;
	int pressedItem;
	int hoveredItem;

	// The same as hoveredItem, except when pressedItem != -1.
	int mouseOverItem;

	struct wzScroller *scroller;

	wzEventCallback *item_selected_callbacks;
};

/*
================================================================================

SCROLLER

================================================================================
*/

static void wz_list_update_scroller_max_value(struct wzList *list)
{
	int max = 0;

	assert(list);

	// Avoid divide by zero.
	if (list->itemHeight != 0)
	{
		max = list->nItems * list->itemHeight - wz_list_get_items_rect(list).h;
	}

	wz_scroller_set_max_value(list->scroller, max);
}

// Hide/show scroller depending on if it's needed.
static bool wz_list_scroller_is_visible(const struct wzWidget *widget)
{
	struct wzList *list;
	int itemsHeight, max;

	assert(widget);
	list = (struct wzList *)widget->parent;

	// Avoid divide by zero.
	if (list->itemHeight == 0)
		return false;

	// Can't call wz_list_get_items_rect, because it checks scroller visibility (stack overflow), so just calculate the items rect height here.
	itemsHeight = wz_widget_get_height(widget->parent) - (list->itemsBorder.top + list->itemsBorder.bottom);
	max = list->nItems * list->itemHeight - itemsHeight;

	if (max <= 0)
	{
		return false;
	}
	
	return true;
}

static wzRect wz_list_scroller_get_rect(const struct wzWidget *widget)
{
	struct wzList *list;
	wzRect listRect, rect;

	assert(widget);
	list = (struct wzList *)widget->parent;
	listRect = wz_widget_get_rect(widget->parent);

	// Fit to the right of items rect. Width doesn't change.
	rect.w = widget->rect.w;
	rect.x = listRect.w - list->itemsBorder.right - rect.w;
	rect.y = list->itemsBorder.top;
	rect.h = listRect.h - (list->itemsBorder.top + list->itemsBorder.bottom);

	return rect;
}

static void wz_list_scroller_value_changed(wzEvent e)
{
	struct wzList *list = (struct wzList *)e.base.widget->parent;
	list->firstItem = e.scroller.value / list->itemHeight;
}

struct wzScroller *wz_list_get_scroller(struct wzList *list)
{
	assert(list);
	return list->scroller;
}

/*
================================================================================

LIST WIDGET

================================================================================
*/

static void wz_list_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzList *list;

	assert(widget);
	list = (struct wzList *)widget;
	widget->rect = rect;
	wz_list_update_scroller_max_value(list);
}

static void wz_list_autosize(struct wzWidget *widget)
{
	struct wzList *list;

	assert(widget);
	list = (struct wzList *)widget;
	wz_list_update_scroller_max_value(list);
}

static void wz_list_set_visible(struct wzWidget *widget, bool visible)
{
	assert(widget);
	widget->hidden = !visible;

	// Clear some additional state when hidden.
	if (!wz_widget_get_visible(widget))
	{
		struct wzList *list = (struct wzList *)widget;
		list->hoveredItem = -1;
	}
}

static void wz_list_update_mouse_over_item(struct wzList *list, int mouseX, int mouseY)
{
	wzRect itemsRect, rect;
	int i;

	list->mouseOverItem = -1;

	if (!list->base.hover)
		return;

	itemsRect = wz_list_get_absolute_items_rect(list);
	rect.x = itemsRect.x;
	rect.y = itemsRect.y - (wz_scroller_get_value(list->scroller) % list->itemHeight);
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
		list->hoveredItem = -1;
		wz_desktop_push_lock_input_widget(widget->desktop, widget);
	}
}

static void wz_list_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzList *list;
	bool selectedItemAssignedTo = false;

	assert(widget);
	list = (struct wzList *)widget;

	if (mouseButton == 1)
	{
		if (list->pressedItem != -1)
		{
			list->selectedItem = list->pressedItem;
			selectedItemAssignedTo = true;
			list->pressedItem = -1;
		}

		// Refresh hovered item.
		wz_list_update_mouse_over_item(list, mouseX, mouseY);
		list->hoveredItem = list->mouseOverItem;

		wz_desktop_pop_lock_input_widget(widget->desktop, widget);
	}

	if (selectedItemAssignedTo)
	{
		wzEvent e;
		e.list.type = WZ_EVENT_LIST_ITEM_SELECTED;
		e.list.list = list;
		e.list.selectedItem = list->selectedItem;
		wz_invoke_event(e, list->item_selected_callbacks);
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

static void wz_list_mouse_wheel_move(struct wzWidget *widget, int x, int y)
{
	struct wzList *list;

	assert(widget);
	list = (struct wzList *)widget;

	if (wz_widget_get_visible((struct wzWidget *)list->scroller))
	{
		int value, stepValue;

		value = wz_scroller_get_value(list->scroller);
		stepValue = wz_scroller_get_step_value(list->scroller);
		wz_scroller_set_value(list->scroller, value - y * stepValue);
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

static void wz_list_destroy(struct wzWidget *widget)
{
	struct wzList *list;

	assert(widget);
	list = (struct wzList *)widget;
	wz_arr_free(list->item_selected_callbacks);
}

struct wzList *wz_list_create(struct wzDesktop *desktop)
{
	struct wzList *list;

	assert(desktop);
	list = (struct wzList *)malloc(sizeof(struct wzList));
	memset(list, 0, sizeof(struct wzList));
	list->base.type = WZ_TYPE_LIST;
	list->base.desktop = desktop;
	list->base.vtable.destroy = wz_list_destroy;
	list->base.vtable.set_rect = wz_list_set_rect;
	list->base.vtable.autosize = wz_list_autosize;
	list->base.vtable.set_visible = wz_list_set_visible;
	list->base.vtable.mouse_button_down = wz_list_mouse_button_down;
	list->base.vtable.mouse_button_up = wz_list_mouse_button_up;
	list->base.vtable.mouse_move = wz_list_mouse_move;
	list->base.vtable.mouse_wheel_move = wz_list_mouse_wheel_move;
	list->base.vtable.mouse_hover_off = wz_list_mouse_hover_off;
	list->selectedItem = -1;
	list->pressedItem = -1;
	list->hoveredItem = -1;
	list->mouseOverItem = -1;

	list->scroller = wz_scroller_create(desktop, WZ_SCROLLER_VERTICAL);
	((struct wzWidget *)list->scroller)->vtable.get_rect = wz_list_scroller_get_rect;
	((struct wzWidget *)list->scroller)->vtable.is_visible = wz_list_scroller_is_visible;
	wz_widget_add_child_widget((struct wzWidget *)list, (struct wzWidget *)list->scroller);
	wz_list_update_scroller_max_value(list);
	wz_scroller_add_callback_value_changed(list->scroller, wz_list_scroller_value_changed);

	return list;
}

void wz_list_set_items_border(struct wzList *list, wzBorder itemsBorder)
{
	assert(list);
	list->itemsBorder = itemsBorder;
	wz_list_update_scroller_max_value(list);
}

void wz_list_set_items_border_args(struct wzList *list, int top, int right, int bottom, int left)
{
	wzBorder border;
	border.top = top;
	border.right = right;
	border.bottom = bottom;
	border.left = left;
	wz_list_set_items_border(list, border);
}

wzBorder wz_list_get_items_border(const struct wzList *list)
{
	assert(list);
	return list->itemsBorder;
}

wzRect wz_list_get_items_rect(const struct wzList *list)
{
	wzRect listRect, rect;

	assert(list);
	listRect = wz_widget_get_rect((struct wzWidget *)list);
	rect.x = list->itemsBorder.left;
	rect.y = list->itemsBorder.top;
	rect.w = listRect.w - (list->itemsBorder.left + list->itemsBorder.right);
	rect.h = listRect.h - (list->itemsBorder.top + list->itemsBorder.bottom);

	// Subtract the scroller width.
	if (wz_widget_get_visible((struct wzWidget *)list->scroller))
	{
		rect.w -= wz_widget_get_size((struct wzWidget *)list->scroller).w;
	}

	return rect;
}

wzRect wz_list_get_absolute_items_rect(const struct wzList *list)
{
	wzRect rect;
	wzPosition offset;
	
	assert(list);
	rect = wz_list_get_items_rect(list);
	offset = wz_widget_get_absolute_position((const struct wzWidget *)list);
	rect.x += offset.x;
	rect.y += offset.y;
	
	return rect;
}

void wz_list_set_item_height(struct wzList *list, int itemHeight)
{
	assert(list);
	list->itemHeight = itemHeight;
	wz_scroller_set_step_value(list->scroller, itemHeight);
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
	wzEvent e;

	assert(list);
	list->selectedItem = selectedItem;
	
	e.list.type = WZ_EVENT_LIST_ITEM_SELECTED;
	e.list.list = list;
	e.list.selectedItem = list->selectedItem;
	wz_invoke_event(e, list->item_selected_callbacks);
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

void wz_list_add_callback_item_selected(struct wzList *list, wzEventCallback callback)
{
	assert(list);
	wz_arr_push(list->item_selected_callbacks, callback);
}
