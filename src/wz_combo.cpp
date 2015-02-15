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

Combo::Combo(uint8_t *itemData, int itemStride, int nItems)
{
	type = WZ_TYPE_COMBO;
	isOpen_ = false;

	list = new List(itemData, itemStride, nItems);
	addChildWidget(list);
	list->setVisible(false);
	list->setClipInputToParent(false);
	list->addEventHandler(WZ_EVENT_LIST_ITEM_SELECTED, this, &Combo::onListItemSelected);
}

void Combo::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	list->setItemData(itemData);
	list->setItemStride(itemStride);
	list->setNumItems(nItems);
}

void Combo::onFontChanged(const char *fontFace, float fontSize)
{
	list->setFont(fontFace, fontSize);
}

void Combo::onRectChanged()
{
	updateListRect();
}

void Combo::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		const Rect listRect = list->getAbsoluteRect();

		// Open dropdown.
		if (!isOpen_)
		{
			// Lock input.
			mainWindow->pushLockInputWidget(this);

			// Show dropdown list and set it to draw last.
			list->setVisible(true);
			updateListRect();

			isOpen_ = true;
		}
		// Close dropdown.
		// Don't do it if the mouse cursor is over the dropdown list.
		else if (!WZ_POINT_IN_RECT(mouseX, mouseY, listRect))
		{
			// Unlock input.
			mainWindow->popLockInputWidget(this);

			// Hide dropdown list.
			list->setVisible(false);

			isOpen_ = false;
		}
	}
}

Rect Combo::getChildrenClipRect() const
{
	// Don't clip children.
	return Rect();
}

void Combo::draw(Rect clip)
{
	renderer->drawCombo(this, clip);
}

Size Combo::measure()
{
	return renderer->measureCombo(this);
}

List *Combo::getList()
{
	return list;
}

const List *Combo::getList() const
{
	return list;
}

bool Combo::isOpen() const
{
	return isOpen_;
}

void Combo::onListItemSelected(Event e)
{
	// Unlock input.
	mainWindow->popLockInputWidget(this);

	// Hide dropdown list.
	list->setVisible(false);

	isOpen_ = false;
}

void Combo::updateListRect()
{
	// Don't do anything if the mainWindow is NULL (widget hasn't been added yet).
	if (!mainWindow)
		return;

	// Make the height large enough to avoid scrolling.
	Border listItemsBorder = list->getItemsBorder();
	Rect listRect(0, rect.h, rect.w, listItemsBorder.top + list->getItemHeight() * list->getNumItems() + listItemsBorder.bottom);

	// Clip the height to the mainWindow.
	// Need to use absolute widget rect y coord to take into account parent window position.
	const Rect absRect = getAbsoluteRect();
	int over = absRect.y + rect.h + listRect.h - mainWindow->getHeight();
	
	if (over > 0)
	{
		listRect.h -= over;
	}

	list->setRectInternal(listRect);
}

} // namespace wz
