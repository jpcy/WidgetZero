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
#include "wz.h"
#pragma hdrstop

namespace wz {

static void wz_list_scroller_value_changed(Event *e)
{
	List *list;
	
	WZ_ASSERT(e);
	list = (List *)e->base.widget->parent;
	list->firstItem = e->scroller.value / list->itemHeight;
}

List::List(uint8_t *itemData, int itemStride, int nItems)
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

	scroller = new Scroller(WZ_SCROLLER_VERTICAL, 0, 1, 0);
	addChildWidget(scroller);
	updateScroller();
	scroller->addCallbackValueChanged(wz_list_scroller_value_changed);
}

void List::onRendererChanged()
{
	refreshItemHeight();
}

void List::onFontChanged(const char *fontFace, float fontSize)
{
	// Doesn't matter if we can't call this yet (NULL renderer), since onRendererChanged will call it too.
	if (renderer)
	{
		refreshItemHeight();
	}
}

void List::onVisibilityChanged()
{
	// Clear some additional state when hidden.
	if (!getVisible())
	{
		hoveredItem = -1;
	}
}

void List::onRectChanged()
{
	updateScroller();
}

void List::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1 && hoveredItem != -1)
	{
		pressedItem = hoveredItem;
		hoveredItem = -1;
		mainWindow->pushLockInputWidget(this);
	}
}

void List::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
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
		invokeEvent(&e, item_selected_callbacks);
	}
}

void List::onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
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

void List::onMouseWheelMove(int x, int y)
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

void List::onMouseHoverOff()
{
	hoveredItem = -1;
	mouseOverItem = -1;
}

void List::draw(Rect clip)
{
	renderer->drawList(this, clip);
}

Size List::measure()
{
	return renderer->measureList(this);
}

Border List::getItemsBorder() const
{
	return itemsBorder;
}

Rect List::getItemsRect() const
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

Rect List::getAbsoluteItemsRect() const
{
	Rect rect = getItemsRect();
	const Position offset = getAbsolutePosition();
	rect.x += offset.x;
	rect.y += offset.y;
	return rect;
}
void List::setDrawItemCallback(DrawListItemCallback callback)
{
	this->draw_item = callback;
}

DrawListItemCallback List::getDrawItemCallback() const
{
	return draw_item;
}

void List::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	setItemData(itemData);
	setItemStride(itemStride);
	setNumItems(nItems);
}

void List::setItemData(uint8_t *itemData)
{
	this->itemData = itemData;
}

uint8_t *List::getItemData() const
{
	return itemData;
}

void List::setItemStride(int itemStride)
{
	this->itemStride = itemStride;
}

int List::getItemStride() const
{
	return itemStride;
}

void List::setItemHeight(int itemHeight)
{
	isItemHeightUserSet = true;
	setItemHeightInternal(itemHeight);
}

int List::getItemHeight() const
{
	return itemHeight;
}

void List::setNumItems(int nItems)
{
	this->nItems = WZ_MAX(0, nItems);
	updateScroller();
}

int List::getNumItems() const
{
	return nItems;
}

int List::getFirstItem() const
{
	return firstItem;
}

void List::setSelectedItem(int selectedItem)
{
	this->selectedItem = selectedItem;
	
	Event e;
	e.list.type = WZ_EVENT_LIST_ITEM_SELECTED;
	e.list.list = this;
	e.list.selectedItem = selectedItem;
	invokeEvent(&e, item_selected_callbacks);
}

int List::getSelectedItem() const
{
	return selectedItem;
}

int List::getPressedItem() const
{
	return pressedItem;
}

int List::getHoveredItem() const
{
	return hoveredItem;
}

int List::getScrollValue() const
{
	return scroller->getValue();
}

Scroller *List::getScroller()
{
	return scroller;
}

const Scroller *List::getScroller() const
{
	return scroller;
}

void List::addCallbackItemSelected(EventCallback callback)
{
	item_selected_callbacks.push_back(callback);
}

void List::setItemHeightInternal(int itemHeight)
{
	this->itemHeight = itemHeight;
	scroller->setStepValue(itemHeight);
	updateScroller();
}

void List::refreshItemHeight()
{
	// Don't stomp on user set value.
	if (isItemHeightUserSet)
		return;

	// Add a little padding.
	setItemHeightInternal(getLineHeight() + 2);
}

void List::updateMouseOverItem(int mouseX, int mouseY)
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

void List::updateScroller()
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

} // namespace wz
