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

static void wz_combo_update_list_rect(struct ComboImpl *combo)
{
	Rect rect, absRect, listRect;
	Border listItemsBorder;
	int listItemHeight, listNumItems, over;

	// Don't do anything if the mainWindow is NULL (widget hasn't been added yet).
	if (!combo->mainWindow)
		return;

	rect = combo->getRect();
	absRect = combo->getAbsoluteRect();

	// Set list rect.
	listRect.x = 0;
	listRect.y = rect.h;
	listRect.w = rect.w;

	// Make the height large enough to avoid scrolling.
	listItemsBorder = combo->list->getItemsBorder();
	listItemHeight = combo->list->getItemHeight();
	listNumItems = combo->list->getNumItems();
	listRect.h = listItemsBorder.top + listItemHeight * listNumItems + listItemsBorder.bottom;

	// Clip the height to the mainWindow.
	// Need to use absolute widget rect y coord to take into account parent window position.
	over = absRect.y + rect.h + listRect.h - combo->mainWindow->getHeight();
	
	if (over > 0)
	{
		listRect.h -= over;
	}

	combo->list->setRectInternal(listRect);
}

static void wz_combo_set_rect(struct WidgetImpl *widget, Rect rect)
{
	widget->rect = rect;
	wz_combo_update_list_rect((struct ComboImpl *)widget);
}

static void wz_combo_font_changed(struct WidgetImpl *widget, const char *fontFace, float fontSize)
{
	struct ComboImpl *combo = (struct ComboImpl *)widget;
	WZ_ASSERT(widget);
	combo->list->setFont(fontFace, fontSize);
}

static void wz_combo_mouse_button_down(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ComboImpl *combo;
	Rect listRect;

	WZ_ASSERT(widget);
	combo = (struct ComboImpl *)widget;

	if (mouseButton == 1)
	{
		listRect = combo->list->getAbsoluteRect();

		// Open dropdown.
		if (!combo->isOpen_)
		{
			// Lock input.
			widget->mainWindow->pushLockInputWidget(widget);

			// Show dropdown list and set it to draw last.
			combo->list->setVisible(true);
			wz_combo_update_list_rect(combo);

			combo->isOpen_ = true;
		}
		// Close dropdown.
		// Don't do it if the mouse cursor is over the dropdown list.
		else if (!WZ_POINT_IN_RECT(mouseX, mouseY, listRect))
		{
			// Unlock input.
			widget->mainWindow->popLockInputWidget(widget);

			// Hide dropdown list.
			combo->list->setVisible(false);

			combo->isOpen_ = false;
		}
	}
}

static Rect wz_combo_get_children_clip_rect(struct WidgetImpl *widget)
{
	// Don't clip children.
	Rect zero;
	zero.x = zero.y = zero.w = zero.h = 0;
	return zero;
}

static void wz_combo_list_item_selected(Event *e)
{
	struct ComboImpl *combo;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	combo = (struct ComboImpl *)e->base.widget->parent;

	// Unlock input.
	combo->mainWindow->popLockInputWidget(combo);

	// Hide dropdown list.
	combo->list->setVisible(false);

	combo->isOpen_ = false;
}

ComboImpl::ComboImpl(uint8_t *itemData, int itemStride, int nItems)
{
	type = WZ_TYPE_COMBO;
	isOpen_ = false;

	vtable.set_rect = wz_combo_set_rect;
	vtable.font_changed = wz_combo_font_changed;
	vtable.mouse_button_down = wz_combo_mouse_button_down;
	vtable.get_children_clip_rect = wz_combo_get_children_clip_rect;

	list = new ListImpl(itemData, itemStride, nItems);
	addChildWidget(list);
	list->setVisible(false);
	list->setClipInputToParent(false);
	list->addCallbackItemSelected(wz_combo_list_item_selected);
}

void ComboImpl::draw(Rect clip)
{
	renderer->drawCombo(this, clip);
}

Size ComboImpl::measure()
{
	return renderer->measureCombo(this);
}

struct ListImpl *ComboImpl::getList()
{
	return list;
}

const struct ListImpl *ComboImpl::getList() const
{
	return list;
}

bool ComboImpl::isOpen() const
{
	return isOpen_;
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

Combo::Combo()
{
	impl.reset(new ComboImpl(NULL, 0, 0));
}

Combo::~Combo()
{
}

Combo *Combo::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	ListImpl *list = ((ComboImpl *)impl.get())->getList();
	list->setItemData(itemData);
	list->setItemStride(itemStride);
	list->setNumItems(nItems);
	return this;
}

} // namespace wz
