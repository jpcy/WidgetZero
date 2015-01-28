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
#include <stdlib.h>
#include <string.h>
#include "wz_list.h"
#include "wz_main_window.h"
#include "wz_renderer.h"
#include "wz_widget.h"
#include "wz_skin.h"

namespace wz {

struct ComboImpl : public WidgetImpl
{
	ComboImpl();

	bool isOpen;
	struct ListImpl *list;
};

static Size wz_combo_measure(struct WidgetImpl *widget)
{
	Size size;
	int i;
	struct WidgetImpl *scroller;
	struct ComboImpl *combo = (struct ComboImpl *)widget;
	uint8_t *itemData = wz_list_get_item_data(combo->list);
	int itemStride = wz_list_get_item_stride(combo->list);
	int nItems = wz_list_get_num_items(combo->list);

	// Use the widest item text.
	size.w = 0;

	for (i = 0; i < nItems; i++)
	{
		int w;
		wz_widget_measure_text(widget, *((const char **)&itemData[i * itemStride]), 0, &w, NULL);
		size.w = WZ_MAX(size.w, w);
	}

	// Use line height.
	size.h = wz_widget_get_line_height(widget);

	// Add scroller width or button width, whichever is largest.
	scroller = (struct WidgetImpl *)wz_list_get_scroller(combo->list);
	size.w += WZ_MAX(scroller->vtable.measure(scroller).w, WZ_SKIN_COMBO_BUTTON_WIDTH);

	// Padding.
	size.w += WZ_SKIN_COMBO_PADDING_X;
	size.h += WZ_SKIN_COMBO_PADDING_Y;
	return size;
}

static void wz_combo_draw(struct WidgetImpl *widget, Rect clip)
{
	int buttonX;
	const struct ComboImpl *combo = (struct ComboImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const Rect rect = wz_widget_get_absolute_rect(widget);
	const uint8_t *itemData = wz_list_get_item_data(combo->list);
	const int itemStride = wz_list_get_item_stride(combo->list);
	const int nItems = wz_list_get_num_items(combo->list);
	const int selectedItemIndex = wz_list_get_selected_item(combo->list);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);

	nvgBeginPath(vg);

	// Don't round the bottom corners if the combo is open.
	if (combo->isOpen)
	{
		wz_renderer_create_rect_path(vg, rect, WZ_SKIN_COMBO_CORNER_RADIUS, WZ_SIDE_ALL, WZ_CORNER_TL | WZ_CORNER_TR);
	}
	else
	{
		nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, WZ_SKIN_COMBO_CORNER_RADIUS);
	}

	// Background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, WZ_SKIN_COMBO_BG_COLOR1, WZ_SKIN_COMBO_BG_COLOR2));
	nvgFill(vg);

	// Border.
	nvgStrokeColor(vg, widget->hover ? WZ_SKIN_COMBO_BORDER_HOVER_COLOR : WZ_SKIN_COMBO_BORDER_COLOR);
	nvgStroke(vg);

	// Internal border.
	buttonX = rect.x + rect.w - WZ_SKIN_COMBO_BUTTON_WIDTH;
	wz_renderer_draw_line(vg, buttonX, rect.y + 1, buttonX, rect.y + rect.h - 1, widget->hover ? WZ_SKIN_COMBO_BORDER_HOVER_COLOR : WZ_SKIN_COMBO_BORDER_COLOR);

	// Icon.
	{
		const float buttonCenterX = buttonX + WZ_SKIN_COMBO_BUTTON_WIDTH * 0.5f;
		const float buttonCenterY = rect.y + rect.h * 0.5f;

		nvgBeginPath(vg);
		nvgMoveTo(vg, buttonCenterX, buttonCenterY + WZ_SKIN_COMBO_ICON_HEIGHT * 0.5f); // bottom
		nvgLineTo(vg, buttonCenterX + WZ_SKIN_COMBO_ICON_WIDTH * 0.5f, buttonCenterY - WZ_SKIN_COMBO_ICON_HEIGHT * 0.5f); // right
		nvgLineTo(vg, buttonCenterX - WZ_SKIN_COMBO_ICON_WIDTH * 0.5f, buttonCenterY - WZ_SKIN_COMBO_ICON_HEIGHT * 0.5f); // left
		nvgFillColor(vg, WZ_SKIN_COMBO_ICON_COLOR);
		nvgFill(vg);
	}

	// Selected item.
	if (selectedItemIndex >= 0)
	{
		wz_renderer_print(widget->renderer, rect.x + WZ_SKIN_COMBO_PADDING_X / 2, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_COMBO_TEXT_COLOR, *((const char **)&itemData[selectedItemIndex * itemStride]), 0);
	}

	nvgRestore(vg);
}

static void wz_combo_update_list_rect(struct ComboImpl *combo)
{
	Rect rect, absRect, listRect;
	Border listItemsBorder;
	int listItemHeight, listNumItems, over;

	// Don't do anything if the mainWindow is NULL (widget hasn't been added yet).
	if (!combo->mainWindow)
		return;

	rect = wz_widget_get_rect((struct WidgetImpl *)combo);
	absRect = wz_widget_get_absolute_rect((struct WidgetImpl *)combo);

	// Set list rect.
	listRect.x = 0;
	listRect.y = rect.h;
	listRect.w = rect.w;

	// Make the height large enough to avoid scrolling.
	listItemsBorder = wz_list_get_items_border(combo->list);
	listItemHeight = wz_list_get_item_height(combo->list);
	listNumItems = wz_list_get_num_items(combo->list);
	listRect.h = listItemsBorder.top + listItemHeight * listNumItems + listItemsBorder.bottom;

	// Clip the height to the mainWindow.
	// Need to use absolute widget rect y coord to take into account parent window position.
	over = absRect.y + rect.h + listRect.h - wz_widget_get_size((struct WidgetImpl *)combo->mainWindow).h;
	
	if (over > 0)
	{
		listRect.h -= over;
	}

	wz_widget_set_rect_internal((struct WidgetImpl *)combo->list, listRect);
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
	wz_widget_set_font((struct WidgetImpl *)combo->list, fontFace, fontSize);
}

static void wz_combo_mouse_button_down(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ComboImpl *combo;
	Rect listRect;

	WZ_ASSERT(widget);
	combo = (struct ComboImpl *)widget;

	if (mouseButton == 1)
	{
		listRect = wz_widget_get_absolute_rect((struct WidgetImpl *)combo->list);

		// Open dropdown.
		if (!combo->isOpen)
		{
			// Lock input.
			wz_main_window_push_lock_input_widget(widget->mainWindow, widget);

			// Show dropdown list and set it to draw last.
			wz_widget_set_visible((struct WidgetImpl *)combo->list, true);
			wz_combo_update_list_rect(combo);

			combo->isOpen = true;
		}
		// Close dropdown.
		// Don't do it if the mouse cursor is over the dropdown list.
		else if (!WZ_POINT_IN_RECT(mouseX, mouseY, listRect))
		{
			// Unlock input.
			wz_main_window_pop_lock_input_widget(widget->mainWindow, widget);

			// Hide dropdown list.
			wz_widget_set_visible((struct WidgetImpl *)combo->list, false);

			combo->isOpen = false;
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
	wz_main_window_pop_lock_input_widget(combo->mainWindow, (struct WidgetImpl *)combo);

	// Hide dropdown list.
	wz_widget_set_visible((struct WidgetImpl *)combo->list, false);

	combo->isOpen = false;
}

ComboImpl::ComboImpl()
{
	type = WZ_TYPE_COMBO;
	isOpen = false;
}

Combo::Combo()
{
	impl = wz_combo_create(NULL, 0, 0);
}

Combo::~Combo()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

Combo *Combo::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	ListImpl *list = wz_combo_get_list((ComboImpl *)impl);
	wz_list_set_item_data(list, itemData);
	wz_list_set_item_stride(list, itemStride);
	wz_list_set_num_items(list, nItems);
	return this;
}

struct ComboImpl *wz_combo_create(uint8_t *itemData, int itemStride, int nItems)
{
	struct ComboImpl *combo = new ComboImpl;
	combo->vtable.measure = wz_combo_measure;
	combo->vtable.draw = wz_combo_draw;
	combo->vtable.set_rect = wz_combo_set_rect;
	combo->vtable.font_changed = wz_combo_font_changed;
	combo->vtable.mouse_button_down = wz_combo_mouse_button_down;
	combo->vtable.get_children_clip_rect = wz_combo_get_children_clip_rect;

	combo->list = wz_list_create(itemData, itemStride, nItems);
	wz_widget_add_child_widget((struct WidgetImpl *)combo, (struct WidgetImpl *)combo->list);
	wz_widget_set_visible((struct WidgetImpl *)combo->list, false);
	wz_widget_set_clip_input_to_parent((struct WidgetImpl *)combo->list, false);
	wz_list_add_callback_item_selected(combo->list, wz_combo_list_item_selected);

	return combo;
}

struct ListImpl *wz_combo_get_list(const struct ComboImpl *combo)
{
	WZ_ASSERT(combo);
	return combo->list;
}

bool wz_combo_is_open(struct ComboImpl *combo)
{
	WZ_ASSERT(combo);
	return combo->isOpen;
}

} // namespace wz
