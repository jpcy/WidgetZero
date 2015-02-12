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
#include "wz_internal.h"
#pragma hdrstop

namespace wz {

static void wz_list_scroller_value_changed(Event *e)
{
	struct ListImpl *list;
	
	WZ_ASSERT(e);
	list = (struct ListImpl *)e->base.widget->parent;
	list->firstItem = e->scroller.value / list->itemHeight;
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

	this->itemData = itemData;
	this->itemStride = itemStride;
	this->nItems = nItems;

	scroller = new ScrollerImpl(WZ_SCROLLER_VERTICAL, 0, 1, 0);
	addChildWidget(scroller);
	updateScroller();
	scroller->addCallbackValueChanged(wz_list_scroller_value_changed);
}

void ListImpl::onRendererChanged()
{
	refreshItemHeight();
}

void ListImpl::onFontChanged(const char *fontFace, float fontSize)
{
	// Doesn't matter if we can't call this yet (NULL renderer), since onRendererChanged will call it too.
	if (renderer)
	{
		refreshItemHeight();
	}
}

void ListImpl::onVisibilityChanged()
{
	// Clear some additional state when hidden.
	if (!getVisible())
	{
		hoveredItem = -1;
	}
}

void ListImpl::onRectChanged()
{
	updateScroller();
}

void ListImpl::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1 && hoveredItem != -1)
	{
		pressedItem = hoveredItem;
		hoveredItem = -1;
		mainWindow->pushLockInputWidget(this);
	}
}

void ListImpl::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	bool selectedItemAssignedTo = false;

	if (mouseButton == 1)
	{
		if (pressedItem != -1)
		{
			selectedItem = pressedItem;
			selectedItemAssignedTo = true;
			pressedItem = -1;
		}

		// Refresh hovered item.
		updateMouseOverItem(mouseX, mouseY);
		hoveredItem = mouseOverItem;

		mainWindow->popLockInputWidget(this);
	}

	if (selectedItemAssignedTo)
	{
		Event e;
		e.list.type = WZ_EVENT_LIST_ITEM_SELECTED;
		e.list.list = this;
		e.list.selectedItem = selectedItem;
		wz_invoke_event(&e, item_selected_callbacks);
	}
}

void ListImpl::onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	lastMousePosition.x = mouseX;
	lastMousePosition.y = mouseY;
	updateMouseOverItem(mouseX, mouseY);
	
	if (pressedItem != -1)
	{
		if (mouseOverItem != -1)
			pressedItem = mouseOverItem;
	}
	else
	{
		hoveredItem = mouseOverItem;
	}
}

void ListImpl::onMouseWheelMove(int x, int y)
{
	if (scroller->getVisible())
	{
		int value = scroller->getValue();
		int stepValue = scroller->getStepValue();
		scroller->setValue(value - y * stepValue);

		// Refresh hovered item.
		updateMouseOverItem(lastMousePosition.x, lastMousePosition.y);
		hoveredItem = mouseOverItem;
	}
}

void ListImpl::onMouseHoverOff()
{
	hoveredItem = -1;
	mouseOverItem = -1;
}

void ListImpl::draw(Rect clip)
{
	renderer->drawList(this, clip);
}

Size ListImpl::measure()
{
	return renderer->measureList(this);
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
	setItemHeightInternal(itemHeight);
}

int ListImpl::getItemHeight() const
{
	return itemHeight;
}

void ListImpl::setNumItems(int nItems)
{
	this->nItems = WZ_MAX(0, nItems);
	updateScroller();
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

void ListImpl::setItemHeightInternal(int itemHeight)
{
	this->itemHeight = itemHeight;
	scroller->setStepValue(itemHeight);
	updateScroller();
}

void ListImpl::refreshItemHeight()
{
	// Don't stomp on user set value.
	if (isItemHeightUserSet)
		return;

	// Add a little padding.
	setItemHeightInternal(getLineHeight() + 2);
}

void ListImpl::updateMouseOverItem(int mouseX, int mouseY)
{
	mouseOverItem = -1;

	if (!hover)
		return;

	const Rect itemsRect = getAbsoluteItemsRect();

	Rect currentItemRect;
	currentItemRect.x = itemsRect.x;
	currentItemRect.y = itemsRect.y - (scroller->getValue() % itemHeight);
	currentItemRect.w = itemsRect.w;
	currentItemRect.h = itemHeight;

	for (int i = firstItem; i < nItems; i++)
	{
		// Outside widget?
		if (currentItemRect.y > itemsRect.y + itemsRect.h)
			break;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, currentItemRect))
		{
			mouseOverItem = i;
			break;
		}

		currentItemRect.y += itemHeight;
	}
}

void ListImpl::updateScroller()
{
	// Update max value.
	const int maxHeight = nItems * itemHeight;
	const int max = maxHeight - getItemsRect().h;
	scroller->setMaxValue(max);

	// Fit to the right of items rect. Width doesn't change.
	Rect scrollerRect;
	scrollerRect.w = scroller->rect.w;
	scrollerRect.x = rect.w - itemsBorder.right - scrollerRect.w;
	scrollerRect.y = itemsBorder.top;
	scrollerRect.h = rect.h - (itemsBorder.top + itemsBorder.bottom);
	scroller->setRectInternal(scrollerRect);

	// Now that the height has been calculated, update the nub scale.
	scroller->setNubScale(1.0f - ((maxHeight - rect.h) / (float)maxHeight));

	// Hide/show scroller depending on if it's needed.
	scroller->setVisible(max > 0);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

List::List()
{
	impl.reset(new ListImpl(NULL, 0, 0));
}

List::~List()
{
}

List *List::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	getImpl()->setItemData(itemData);
	getImpl()->setItemStride(itemStride);
	getImpl()->setNumItems(nItems);
	return this;
}

List *List::setSelectedItem(int index)
{
	getImpl()->setSelectedItem(index);
	return this;
}

List *List::setItemHeight(int height)
{
	getImpl()->setItemHeight(height);
	return this;
}

List *List::setDrawItemCallback(DrawListItemCallback callback)
{
	getImpl()->setDrawItemCallback(callback);
	return this;
}

ListImpl *List::getImpl()
{
	return (ListImpl *)impl.get();
}

const ListImpl *List::getImpl() const
{
	return (const ListImpl *)impl.get();
}

} // namespace wz
