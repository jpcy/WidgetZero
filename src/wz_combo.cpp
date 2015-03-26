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
	type_ = WidgetType::Combo;
	isOpen_ = false;

	list_ = new List(itemData, itemStride, nItems);
	addChildWidget(list_);
	list_->setVisible(false);
	list_->setClipInputToParent(false);
	list_->addEventHandler(EventType::ListItemSelected, this, &Combo::onListItemSelected);
}

void Combo::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	list_->setItemData(itemData);
	list_->setItemStride(itemStride);
	list_->setNumItems(nItems);
}

List *Combo::getList()
{
	return list_;
}

const List *Combo::getList() const
{
	return list_;
}

bool Combo::isOpen() const
{
	return isOpen_;
}

void Combo::onFontChanged(const char *fontFace, float fontSize)
{
	list_->setFont(fontFace, fontSize);
}

void Combo::onRectChanged()
{
	updateListRect();
}

void Combo::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		const Rect listRect = list_->getAbsoluteRect();

		// Open dropdown.
		if (!isOpen_)
		{
			// Lock input.
			mainWindow_->pushLockInputWidget(this);

			// Show dropdown list and set it to draw last.
			list_->setVisible(true);
			updateListRect();

			isOpen_ = true;
		}
		// Close dropdown.
		// Don't do it if the mouse cursor is over the dropdown list.
		else if (!WZ_POINT_IN_RECT(mouseX, mouseY, listRect))
		{
			// Unlock input.
			mainWindow_->popLockInputWidget(this);

			// Hide dropdown list.
			list_->setVisible(false);

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
	renderer_->drawCombo(this, clip);
}

Size Combo::measure()
{
	return renderer_->measureCombo(this);
}

void Combo::onListItemSelected(Event)
{
	// Unlock input.
	mainWindow_->popLockInputWidget(this);

	// Hide dropdown list.
	list_->setVisible(false);

	isOpen_ = false;
}

void Combo::updateListRect()
{
	// Don't do anything if the mainWindow is NULL (widget hasn't been added yet).
	if (!mainWindow_)
		return;

	// Make the height large enough to avoid scrolling.
	Border listItemsBorder = list_->getItemsBorder();
	Rect listRect(0, rect_.h, rect_.w, listItemsBorder.top + list_->getItemHeight() * list_->getNumItems() + listItemsBorder.bottom);

	// Clip the height to the mainWindow.
	// Need to use absolute widget rect y coord to take into account parent window position.
	const Rect absRect = getAbsoluteRect();
	int over = absRect.y + rect_.h + listRect.h - mainWindow_->getHeight();
	
	if (over > 0)
	{
		listRect.h -= over;
	}

	list_->setRect(listRect);
}

} // namespace wz
