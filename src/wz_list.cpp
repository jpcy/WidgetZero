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

List::List(uint8_t *itemData, int itemStride, int nItems)
{
	type_ = WidgetType::List;
	drawItem_ = NULL;
	itemHeight_ = 0;
	isItemHeightUserSet_ = false;
	firstItem_ = 0;
	selectedItem_ = pressedItem_ = hoveredItem_ = mouseOverItem_ = -1;
	scroller_ = NULL;
	itemsBorder_.top = itemsBorder_.right = itemsBorder_.bottom = itemsBorder_.left = 2;

	itemData_ = itemData;
	itemStride_ = itemStride;
	nItems_ = nItems;

	scroller_ = new Scroller(ScrollerDirection::Vertical, 0, 1, 0);
	addChildWidget(scroller_);
	scroller_->addEventHandler(EventType::ScrollerValueChanged, this, &List::onScrollerValueChanged);
}

Border List::getItemsBorder() const
{
	return itemsBorder_;
}

Rect List::getItemsRect() const
{
	Rect rect;
	rect.x = itemsBorder_.left;
	rect.y = itemsBorder_.top;
	const Rect listRect = getRect();
	rect.w = listRect.w - (itemsBorder_.left + itemsBorder_.right);
	rect.h = listRect.h - (itemsBorder_.top + itemsBorder_.bottom);

	// Subtract the scroller width.
	if (scroller_->isVisible())
	{
		rect.w -= scroller_->getMeasuredSize().w;
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
	drawItem_ = callback;
}

DrawListItemCallback List::getDrawItemCallback() const
{
	return drawItem_;
}

void List::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	setItemData(itemData);
	setItemStride(itemStride);
	setNumItems(nItems);
}

void List::setItemData(uint8_t *itemData)
{
	itemData_ = itemData;
}

uint8_t *List::getItemData() const
{
	return itemData_;
}

void List::setItemStride(int itemStride)
{
	itemStride_ = itemStride;
}

int List::getItemStride() const
{
	return itemStride_;
}

void List::setItemHeight(int itemHeight)
{
	isItemHeightUserSet_ = true;
	setItemHeightInternal(itemHeight);
}

int List::getItemHeight() const
{
	return itemHeight_;
}

void List::setNumItems(int nItems)
{
	nItems_ = WZ_MAX(0, nItems);
	updateScroller();
}

int List::getNumItems() const
{
	return nItems_;
}

int List::getFirstItem() const
{
	return firstItem_;
}

void List::setSelectedItem(int selectedItem)
{
	selectedItem_ = selectedItem;
	
	Event e;
	e.list.type = EventType::ListItemSelected;
	e.list.list = this;
	e.list.selectedItem = selectedItem;
	invokeEvent(e, itemSelectedCallbacks_);
}

int List::getSelectedItem() const
{
	return selectedItem_;
}

int List::getPressedItem() const
{
	return pressedItem_;
}

int List::getHoveredItem() const
{
	return hoveredItem_;
}

int List::getScrollValue() const
{
	return scroller_->getValue();
}

Scroller *List::getScroller()
{
	return scroller_;
}

const Scroller *List::getScroller() const
{
	return scroller_;
}

void List::addCallbackItemSelected(EventCallback callback)
{
	itemSelectedCallbacks_.push_back(callback);
}

void List::onRendererChanged()
{
	refreshItemHeight();
}

void List::onFontChanged(const char * /*fontFace*/, float /*fontSize*/)
{
	// Doesn't matter if we can't call this yet (NULL renderer), since onRendererChanged will call it too.
	if (renderer_)
	{
		refreshItemHeight();
	}
}

void List::onVisibilityChanged()
{
	// Clear some additional state when hidden.
	if (!isVisible())
	{
		hoveredItem_ = -1;
	}
}

void List::onRectChanged()
{
	updateScroller();
}

void List::onMouseButtonDown(int mouseButton, int /*mouseX*/, int /*mouseY*/)
{
	if (mouseButton == 1 && hoveredItem_ != -1)
	{
		pressedItem_ = hoveredItem_;
		hoveredItem_ = -1;
		mainWindow_->pushLockInputWidget(this);
	}
}

void List::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	bool selectedItemAssignedTo = false;

	if (mouseButton == 1)
	{
		if (pressedItem_ != -1)
		{
			selectedItem_ = pressedItem_;
			selectedItemAssignedTo = true;
			pressedItem_ = -1;
		}

		// Refresh hovered item.
		updateMouseOverItem(mouseX, mouseY);
		hoveredItem_ = mouseOverItem_;

		mainWindow_->popLockInputWidget(this);
	}

	if (selectedItemAssignedTo)
	{
		Event e;
		e.list.type = EventType::ListItemSelected;
		e.list.list = this;
		e.list.selectedItem = selectedItem_;
		invokeEvent(e, itemSelectedCallbacks_);
	}
}

void List::onMouseMove(int mouseX, int mouseY, int /*mouseDeltaX*/, int /*mouseDeltaY*/)
{
	lastMousePosition_.x = mouseX;
	lastMousePosition_.y = mouseY;
	updateMouseOverItem(mouseX, mouseY);

	if (pressedItem_ != -1)
	{
		if (mouseOverItem_ != -1)
			pressedItem_ = mouseOverItem_;
	}
	else
	{
		hoveredItem_ = mouseOverItem_;
	}
}

void List::onMouseWheelMove(int /*x*/, int y)
{
	if (scroller_->isVisible())
	{
		scroller_->setValue(scroller_->getValue() - y * scroller_->getStepValue());

		// Refresh hovered item.
		updateMouseOverItem(lastMousePosition_.x, lastMousePosition_.y);
		hoveredItem_ = mouseOverItem_;
	}
}

void List::onMouseHoverOff()
{
	hoveredItem_ = -1;
	mouseOverItem_ = -1;
}

void List::draw(Rect clip)
{
	renderer_->drawList(this, clip);
}

Size List::measure()
{
	return renderer_->measureList(this);
}

void List::onScrollerValueChanged(Event e)
{
	firstItem_ = e.scroller.value / itemHeight_;
}

void List::setItemHeightInternal(int itemHeight)
{
	itemHeight_ = itemHeight;
	scroller_->setStepValue(itemHeight);
	updateScroller();
}

void List::refreshItemHeight()
{
	// Don't stomp on user set value.
	if (isItemHeightUserSet_)
		return;

	// Add a little padding.
	setItemHeightInternal(getLineHeight() + 2);
}

void List::updateMouseOverItem(int mouseX, int mouseY)
{
	mouseOverItem_ = -1;

	if (!hover_)
		return;

	const Rect itemsRect = getAbsoluteItemsRect();

	Rect currentItemRect;
	currentItemRect.x = itemsRect.x;
	currentItemRect.y = itemsRect.y - (scroller_->getValue() % itemHeight_);
	currentItemRect.w = itemsRect.w;
	currentItemRect.h = itemHeight_;

	for (int i = firstItem_; i < nItems_; i++)
	{
		// Outside widget?
		if (currentItemRect.y > itemsRect.y + itemsRect.h)
			break;

		if (WZ_POINT_IN_RECT(mouseX, mouseY, currentItemRect))
		{
			mouseOverItem_ = i;
			break;
		}

		currentItemRect.y += itemHeight_;
	}
}

void List::updateScroller()
{
	// Size not set yet.
	if (rect_.w == 0 && rect_.h == 0)
		return;

	// Update max value.
	const int maxHeight = nItems_ * itemHeight_;
	const int max = maxHeight - getItemsRect().h;
	scroller_->setMaxValue(max);

	// Fit to the right of items rect. Width doesn't change.
	Rect scrollerRect;
	scrollerRect.w = scroller_->getMeasuredSize().w;
	scrollerRect.x = rect_.w - itemsBorder_.right - scrollerRect.w;
	scrollerRect.y = itemsBorder_.top;
	scrollerRect.h = rect_.h - (itemsBorder_.top + itemsBorder_.bottom);
	scroller_->setRect(scrollerRect);

	// Now that the height has been calculated, update the nub scale.
	scroller_->setNubScale(1.0f - ((maxHeight - rect_.h) / (float)maxHeight));

	// Hide/show scroller depending on if it's needed.
	scroller_->setVisible(max > 0);
}

} // namespace wz
