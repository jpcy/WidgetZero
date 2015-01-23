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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "wz_main_window.h"
#include "wz_renderer.h"
#include "wz_widget.h"
#include "wz_skin.h"

namespace wz {

struct ListImpl : public WidgetImpl
{
	ListImpl();

	Border itemsBorder;
	DrawListItemCallback draw_item;
	uint8_t *itemData;
	int itemStride;
	int itemHeight;
	bool isItemHeightUserSet;
	int nItems;
	int firstItem;
	int selectedItem;
	int pressedItem;
	int hoveredItem;

	// The same as hoveredItem, except when pressedItem != -1.
	int mouseOverItem;

	struct ScrollerImpl *scroller;

	std::vector<EventCallback> item_selected_callbacks;

	// Set when the mouse moves. Used to refresh the hovered item when scrolling via the mouse wheel.
	Position lastMousePosition;
};

/*
================================================================================

SCROLLER

================================================================================
*/

static void wz_list_update_scroller(struct ListImpl *list)
{
	Rect listRect, rect;
	int maxHeight, max;

	WZ_ASSERT(list);

	// Update max value.
	maxHeight = list->nItems * list->itemHeight;
	max = maxHeight - wz_list_get_items_rect(list).h;
	wz_scroller_set_max_value(list->scroller, max);

	// Fit to the right of items rect. Width doesn't change.
	listRect = wz_widget_get_rect((struct WidgetImpl *)list);

	rect.w = ((struct WidgetImpl *)list->scroller)->rect.w;
	rect.x = listRect.w - list->itemsBorder.right - rect.w;
	rect.y = list->itemsBorder.top;
	rect.h = listRect.h - (list->itemsBorder.top + list->itemsBorder.bottom);
	wz_widget_set_rect_internal((struct WidgetImpl *)list->scroller, rect);

	// Now that the height has been calculated, update the nub scale.
	wz_scroller_set_nub_scale(list->scroller, 1.0f - ((maxHeight - rect.h) / (float)maxHeight));

	// Hide/show scroller depending on if it's needed.
	if (max <= 0)
	{
		wz_widget_set_visible((struct WidgetImpl *)list->scroller, false);
	}
	else
	{
		wz_widget_set_visible((struct WidgetImpl *)list->scroller, true);
	}
}

static void wz_list_scroller_value_changed(Event *e)
{
	struct ListImpl *list;
	
	WZ_ASSERT(e);
	list = (struct ListImpl *)e->base.widget->parent;
	list->firstItem = e->scroller.value / list->itemHeight;
}

/*
================================================================================

PRIVATE UTILITY FUNCTIONS

================================================================================
*/

static void wz_list_set_item_height_internal(struct ListImpl *list, int itemHeight)
{
	WZ_ASSERT(list);
	list->itemHeight = itemHeight;
	wz_scroller_set_step_value(list->scroller, itemHeight);
	wz_list_update_scroller(list);
}

/*
================================================================================

LIST WIDGET

================================================================================
*/

static void wz_list_draw(struct WidgetImpl *widget, Rect clip)
{
	int y, i;
	struct ListImpl *list = (struct ListImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const Rect rect = wz_widget_get_absolute_rect(widget);
	const Rect itemsRect = wz_list_get_absolute_items_rect(list);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	
	// Background.
	nvgBeginPath(vg);
	nvgRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, WZ_SKIN_LIST_BG_COLOR1, WZ_SKIN_LIST_BG_COLOR2));
	nvgFill(vg);

	// Border.
	wz_renderer_draw_rect(vg, rect, WZ_SKIN_LIST_BORDER_COLOR);

	// Items.
	if (!wz_renderer_clip_to_rect_intersection(vg, clip, itemsRect))
		return;

	y = itemsRect.y - (wz_scroller_get_value(list->scroller) % list->itemHeight);

	for (i = wz_list_get_first_item(list); i < list->nItems; i++)
	{
		Rect itemRect;
		const uint8_t *itemData;

		// Outside widget?
		if (y > itemsRect.y + itemsRect.h)
			break;

		itemRect.x = itemsRect.x;
		itemRect.y = y;
		itemRect.w = itemsRect.w;
		itemRect.h = list->itemHeight;
		itemData = *((uint8_t **)&list->itemData[i * list->itemStride]);

		if (i == list->selectedItem)
		{
			wz_renderer_draw_filled_rect(vg, itemRect, WZ_SKIN_LIST_SET_COLOR);
		}
		else if (i == list->pressedItem || i == list->hoveredItem)
		{
			wz_renderer_draw_filled_rect(vg, itemRect, WZ_SKIN_LIST_HOVER_COLOR);
		}

		if (list->draw_item)
		{
			list->draw_item(widget->renderer, itemRect, list, widget->fontFace, widget->fontSize, i, itemData);
		}
		else
		{
			wz_renderer_print(widget->renderer, itemsRect.x + WZ_SKIN_LIST_ITEM_LEFT_PADDING, y + list->itemHeight / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_LIST_TEXT_COLOR, (const char *)itemData, 0);
		}

		y += list->itemHeight;
	}

	nvgRestore(vg);
}

static void wz_list_refresh_item_height(struct ListImpl *list)
{
	// Don't stomp on user set value.
	if (list->isItemHeightUserSet)
		return;

	// Add a little padding.
	wz_list_set_item_height_internal(list, wz_widget_get_line_height((struct WidgetImpl *)list) + 2);
}

static void wz_list_renderer_changed(struct WidgetImpl *widget)
{
	wz_list_refresh_item_height((struct ListImpl *)widget);
}

static void wz_list_set_rect(struct WidgetImpl *widget, Rect rect)
{
	struct ListImpl *list;

	WZ_ASSERT(widget);
	widget->rect = rect;
	list = (struct ListImpl *)widget;
	wz_list_update_scroller(list);
}

static void wz_list_set_visible(struct WidgetImpl *widget, bool visible)
{
	WZ_ASSERT(widget);
	widget->hidden = !visible;

	// Clear some additional state when hidden.
	if (!wz_widget_get_visible(widget))
	{
		struct ListImpl *list = (struct ListImpl *)widget;
		list->hoveredItem = -1;
	}
}

static void wz_list_font_changed(struct WidgetImpl *widget, const char *fontFace, float fontSize)
{
	WZ_ASSERT(widget);

	// Doesn't matter if we can't call this yet (NULL renderer), since wz_list_renderer_changed will call it too.
	if (widget->renderer)
	{
		wz_list_refresh_item_height((struct ListImpl *)widget);
	}
}

static void wz_list_update_mouse_over_item(struct ListImpl *list, int mouseX, int mouseY)
{
	Rect itemsRect, rect;
	int i;

	list->mouseOverItem = -1;

	if (!list->hover)
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

static void wz_list_mouse_button_down(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ListImpl *list;

	WZ_ASSERT(widget);
	list = (struct ListImpl *)widget;

	if (mouseButton == 1 && list->hoveredItem != -1)
	{
		list->pressedItem = list->hoveredItem;
		list->hoveredItem = -1;
		wz_main_window_push_lock_input_widget(widget->mainWindow, widget);
	}
}

static void wz_list_mouse_button_up(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ListImpl *list;
	bool selectedItemAssignedTo = false;

	WZ_ASSERT(widget);
	list = (struct ListImpl *)widget;

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

		wz_main_window_pop_lock_input_widget(widget->mainWindow, widget);
	}

	if (selectedItemAssignedTo)
	{
		Event e;
		e.list.type = WZ_EVENT_LIST_ITEM_SELECTED;
		e.list.list = list;
		e.list.selectedItem = list->selectedItem;
		wz_invoke_event(&e, list->item_selected_callbacks);
	}
}

static void wz_list_mouse_move(struct WidgetImpl *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct ListImpl *list;

	WZ_ASSERT(widget);
	list = (struct ListImpl *)widget;
	list->lastMousePosition.x = mouseX;
	list->lastMousePosition.y = mouseY;
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

static void wz_list_mouse_wheel_move(struct WidgetImpl *widget, int x, int y)
{
	struct ListImpl *list;

	WZ_ASSERT(widget);
	list = (struct ListImpl *)widget;

	if (wz_widget_get_visible((struct WidgetImpl *)list->scroller))
	{
		int value, stepValue;

		value = wz_scroller_get_value(list->scroller);
		stepValue = wz_scroller_get_step_value(list->scroller);
		wz_scroller_set_value(list->scroller, value - y * stepValue);

		// Refresh hovered item.
		wz_list_update_mouse_over_item(list, list->lastMousePosition.x, list->lastMousePosition.y);
		list->hoveredItem = list->mouseOverItem;
	}
}

static void wz_list_mouse_hover_off(struct WidgetImpl *widget)
{
	struct ListImpl *list;

	WZ_ASSERT(widget);
	list = (struct ListImpl *)widget;
	list->hoveredItem = -1;
	list->mouseOverItem = -1;
}

ListImpl::ListImpl()
{
	draw_item = NULL;
	itemData = NULL;
	itemStride = itemHeight = nItems = 0;
	isItemHeightUserSet = false;
	firstItem = 0;
	selectedItem = pressedItem = hoveredItem = mouseOverItem = -1;
	scroller = NULL;
	itemsBorder.top = itemsBorder.right = itemsBorder.bottom = itemsBorder.left = 2;
}

struct ListImpl *wz_list_create(uint8_t *itemData, int itemStride, int nItems)
{
	struct ListImpl *list = new struct ListImpl;
	list->type = WZ_TYPE_LIST;
	list->vtable.draw = wz_list_draw;
	list->vtable.renderer_changed = wz_list_renderer_changed;
	list->vtable.set_rect = wz_list_set_rect;
	list->vtable.set_visible = wz_list_set_visible;
	list->vtable.font_changed = wz_list_font_changed;
	list->vtable.mouse_button_down = wz_list_mouse_button_down;
	list->vtable.mouse_button_up = wz_list_mouse_button_up;
	list->vtable.mouse_move = wz_list_mouse_move;
	list->vtable.mouse_wheel_move = wz_list_mouse_wheel_move;
	list->vtable.mouse_hover_off = wz_list_mouse_hover_off;
	list->itemData = itemData;
	list->itemStride = itemStride;
	list->nItems = nItems;

	list->scroller = wz_scroller_create(WZ_SCROLLER_VERTICAL, 0, 1, 0);
	wz_widget_add_child_widget((struct WidgetImpl *)list, (struct WidgetImpl *)list->scroller);
	wz_list_update_scroller(list);
	wz_scroller_add_callback_value_changed(list->scroller, wz_list_scroller_value_changed);

	return list;
}

Border wz_list_get_items_border(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->itemsBorder;
}

Rect wz_list_get_items_rect(const struct ListImpl *list)
{
	Rect listRect, rect;

	WZ_ASSERT(list);
	listRect = wz_widget_get_rect((struct WidgetImpl *)list);
	rect.x = list->itemsBorder.left;
	rect.y = list->itemsBorder.top;
	rect.w = listRect.w - (list->itemsBorder.left + list->itemsBorder.right);
	rect.h = listRect.h - (list->itemsBorder.top + list->itemsBorder.bottom);

	// Subtract the scroller width.
	if (wz_widget_get_visible((struct WidgetImpl *)list->scroller))
	{
		rect.w -= wz_widget_get_size((struct WidgetImpl *)list->scroller).w;
	}

	return rect;
}

Rect wz_list_get_absolute_items_rect(const struct ListImpl *list)
{
	Rect rect;
	Position offset;
	
	WZ_ASSERT(list);
	rect = wz_list_get_items_rect(list);
	offset = wz_widget_get_absolute_position((const struct WidgetImpl *)list);
	rect.x += offset.x;
	rect.y += offset.y;
	
	return rect;
}

void wz_list_set_draw_item_callback(struct ListImpl *list, DrawListItemCallback callback)
{
	WZ_ASSERT(list);
	list->draw_item = callback;
}

DrawListItemCallback wz_list_get_draw_item_callback(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->draw_item;
}

void wz_list_set_item_data(struct ListImpl *list, uint8_t *itemData)
{
	WZ_ASSERT(list);
	list->itemData = itemData;
}

uint8_t *wz_list_get_item_data(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->itemData;
}

void wz_list_set_item_stride(struct ListImpl *list, int itemStride)
{
	WZ_ASSERT(list);
	list->itemStride = itemStride;
}

int wz_list_get_item_stride(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->itemStride;
}

void wz_list_set_item_height(struct ListImpl *list, int itemHeight)
{
	list->isItemHeightUserSet = true;
	wz_list_set_item_height_internal(list, itemHeight);
}

int wz_list_get_item_height(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->itemHeight;
}

void wz_list_set_num_items(struct ListImpl *list, int nItems)
{
	WZ_ASSERT(list);
	list->nItems = WZ_MAX(0, nItems);
	wz_list_update_scroller(list);
}

int wz_list_get_num_items(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->nItems;
}

int wz_list_get_first_item(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->firstItem;
}

void wz_list_set_selected_item(struct ListImpl *list, int selectedItem)
{
	Event e;

	WZ_ASSERT(list);
	list->selectedItem = selectedItem;
	
	e.list.type = WZ_EVENT_LIST_ITEM_SELECTED;
	e.list.list = list;
	e.list.selectedItem = list->selectedItem;
	wz_invoke_event(&e, list->item_selected_callbacks);
}

int wz_list_get_selected_item(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->selectedItem;
}

int wz_list_get_pressed_item(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->pressedItem;
}

int wz_list_get_hovered_item(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return list->hoveredItem;
}

int wz_list_get_scroll_value(const struct ListImpl *list)
{
	WZ_ASSERT(list);
	return wz_scroller_get_value(list->scroller);
}

void wz_list_add_callback_item_selected(struct ListImpl *list, EventCallback callback)
{
	WZ_ASSERT(list);
	list->item_selected_callbacks.push_back(callback);
}

struct ScrollerImpl *wz_list_get_scroller(struct ListImpl *list)
{
	return list->scroller;
}

} // namespace wz
