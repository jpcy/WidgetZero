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
	max = maxHeight - list->getItemsRect().h;
	list->scroller->setMaxValue(max);

	// Fit to the right of items rect. Width doesn't change.
	listRect = list->getRect();

	rect.w = (list->scroller)->rect.w;
	rect.x = listRect.w - list->itemsBorder.right - rect.w;
	rect.y = list->itemsBorder.top;
	rect.h = listRect.h - (list->itemsBorder.top + list->itemsBorder.bottom);
	list->scroller->setRectInternal(rect);

	// Now that the height has been calculated, update the nub scale.
	list->scroller->setNubScale(1.0f - ((maxHeight - rect.h) / (float)maxHeight));

	// Hide/show scroller depending on if it's needed.
	if (max <= 0)
	{
		list->scroller->setVisible(false);
	}
	else
	{
		list->scroller->setVisible(true);
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
	list->scroller->setStepValue(itemHeight);
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
	NVGRenderer *r = (NVGRenderer *)widget->renderer;
	struct NVGcontext *vg = r->getContext();
	const Rect rect = widget->getAbsoluteRect();
	const Rect itemsRect = list->getAbsoluteItemsRect();

	nvgSave(vg);
	r->clipToRect(clip);
	
	// Background.
	nvgBeginPath(vg);
	nvgRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, WZ_SKIN_LIST_BG_COLOR1, WZ_SKIN_LIST_BG_COLOR2));
	nvgFill(vg);

	// Border.
	r->drawRect(rect, WZ_SKIN_LIST_BORDER_COLOR);

	// Items.
	if (!r->clipToRectIntersection(clip, itemsRect))
		return;

	y = itemsRect.y - (list->scroller->getValue() % list->itemHeight);

	for (i = list->getFirstItem(); i < list->nItems; i++)
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
			r->drawFilledRect(itemRect, WZ_SKIN_LIST_SET_COLOR);
		}
		else if (i == list->pressedItem || i == list->hoveredItem)
		{
			r->drawFilledRect(itemRect, WZ_SKIN_LIST_HOVER_COLOR);
		}

		if (list->draw_item)
		{
			list->draw_item(widget->renderer, itemRect, list, widget->fontFace, widget->fontSize, i, itemData);
		}
		else
		{
			r->print(itemsRect.x + WZ_SKIN_LIST_ITEM_LEFT_PADDING, y + list->itemHeight / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_LIST_TEXT_COLOR, (const char *)itemData, 0);
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
	wz_list_set_item_height_internal(list, list->getLineHeight() + 2);
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
	if (!widget->getVisible())
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

	itemsRect = list->getAbsoluteItemsRect();
	rect.x = itemsRect.x;
	rect.y = itemsRect.y - (list->scroller->getValue() % list->itemHeight);
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
		widget->mainWindow->pushLockInputWidget(widget);
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

		widget->mainWindow->popLockInputWidget(widget);
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

	if (list->scroller->getVisible())
	{
		int value, stepValue;

		value = list->scroller->getValue();
		stepValue = list->scroller->getStepValue();
		list->scroller->setValue(value - y * stepValue);

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

ListImpl::ListImpl(uint8_t *itemData, int itemStride, int nItems)
{
	type = WZ_TYPE_LIST;
	draw_item = NULL;
	itemHeight = 0;
	isItemHeightUserSet = false;
	firstItem = 0;
	selectedItem = pressedItem = hoveredItem = mouseOverItem = -1;
	scroller = NULL;
	itemsBorder.top = itemsBorder.right = itemsBorder.bottom = itemsBorder.left = 2;

	vtable.draw = wz_list_draw;
	vtable.renderer_changed = wz_list_renderer_changed;
	vtable.set_rect = wz_list_set_rect;
	vtable.set_visible = wz_list_set_visible;
	vtable.font_changed = wz_list_font_changed;
	vtable.mouse_button_down = wz_list_mouse_button_down;
	vtable.mouse_button_up = wz_list_mouse_button_up;
	vtable.mouse_move = wz_list_mouse_move;
	vtable.mouse_wheel_move = wz_list_mouse_wheel_move;
	vtable.mouse_hover_off = wz_list_mouse_hover_off;
	this->itemData = itemData;
	this->itemStride = itemStride;
	this->nItems = nItems;

	scroller = new ScrollerImpl(WZ_SCROLLER_VERTICAL, 0, 1, 0);
	addChildWidget(scroller);
	wz_list_update_scroller(this);
	scroller->addCallbackValueChanged(wz_list_scroller_value_changed);
}

Border ListImpl::getItemsBorder() const
{
	return itemsBorder;
}

Rect ListImpl::getItemsRect() const
{
	Rect rect;
	rect.x = itemsBorder.left;
	rect.y = itemsBorder.top;
	const Rect listRect = getRect();
	rect.w = listRect.w - (itemsBorder.left + itemsBorder.right);
	rect.h = listRect.h - (itemsBorder.top + itemsBorder.bottom);

	// Subtract the scroller width.
	if (scroller->getVisible())
	{
		rect.w -= scroller->getWidth();
	}

	return rect;
}

Rect ListImpl::getAbsoluteItemsRect() const
{
	Rect rect = getItemsRect();
	const Position offset = getAbsolutePosition();
	rect.x += offset.x;
	rect.y += offset.y;
	return rect;
}
void ListImpl::setDrawItemCallback(DrawListItemCallback callback)
{
	this->draw_item = callback;
}

DrawListItemCallback ListImpl::getDrawItemCallback() const
{
	return draw_item;
}

void ListImpl::setItemData(uint8_t *itemData)
{
	this->itemData = itemData;
}

uint8_t *ListImpl::getItemData() const
{
	return itemData;
}

void ListImpl::setItemStride(int itemStride)
{
	this->itemStride = itemStride;
}

int ListImpl::getItemStride() const
{
	return itemStride;
}

void ListImpl::setItemHeight(int itemHeight)
{
	isItemHeightUserSet = true;
	wz_list_set_item_height_internal(this, itemHeight);
}

int ListImpl::getItemHeight() const
{
	return itemHeight;
}

void ListImpl::setNumItems(int nItems)
{
	this->nItems = WZ_MAX(0, nItems);
	wz_list_update_scroller(this);
}

int ListImpl::getNumItems() const
{
	return nItems;
}

int ListImpl::getFirstItem() const
{
	return firstItem;
}

void ListImpl::setSelectedItem(int selectedItem)
{
	this->selectedItem = selectedItem;
	
	Event e;
	e.list.type = WZ_EVENT_LIST_ITEM_SELECTED;
	e.list.list = this;
	e.list.selectedItem = selectedItem;
	wz_invoke_event(&e, item_selected_callbacks);
}

int ListImpl::getSelectedItem() const
{
	return selectedItem;
}

int ListImpl::getPressedItem() const
{
	return pressedItem;
}

int ListImpl::getHoveredItem() const
{
	return hoveredItem;
}

int ListImpl::getScrollValue() const
{
	return scroller->getValue();
}

struct ScrollerImpl *ListImpl::getScroller()
{
	return scroller;
}

const struct ScrollerImpl *ListImpl::getScroller() const
{
	return scroller;
}

void ListImpl::addCallbackItemSelected(EventCallback callback)
{
	item_selected_callbacks.push_back(callback);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

List::List()
{
	impl = new ListImpl(NULL, 0, 0);
}

List::~List()
{
}

List *List::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	((ListImpl *)impl)->setItemData(itemData);
	((ListImpl *)impl)->setItemStride(itemStride);
	((ListImpl *)impl)->setNumItems(nItems);
	return this;
}

List *List::setSelectedItem(int index)
{
	((ListImpl *)impl)->setSelectedItem(index);
	return this;
}

List *List::setItemHeight(int height)
{
	((ListImpl *)impl)->setItemHeight(height);
	return this;
}

List *List::setDrawItemCallback(DrawListItemCallback callback)
{
	((ListImpl *)impl)->setDrawItemCallback(callback);
	return this;
}

} // namespace wz
