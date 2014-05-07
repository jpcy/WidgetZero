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
#include <stdlib.h>
#include <string.h>
#include "stb_arr.h"
#include "wz_internal.h"

struct wzCombo
{
	struct wzWidget base;
	bool isOpen;
	struct wzList *list;
};

static void wz_combo_update_list_rect(struct wzCombo *combo)
{
	wzRect rect, listRect;

	rect = wz_widget_get_absolute_rect((struct wzWidget *)combo);

	// Set list rect.
	listRect.x = rect.x;
	listRect.y = rect.y + rect.h;
	listRect.w = rect.w;
	listRect.h = 200; // FIXME
	wz_widget_set_rect((struct wzWidget *)combo->list, listRect);
}

static void wz_combo_set_rect(struct wzWidget *widget, wzRect rect)
{
	widget->rect = rect;
	wz_combo_update_list_rect((struct wzCombo *)widget);
}

static void wz_combo_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzCombo *combo;

	assert(widget);
	combo = (struct wzCombo *)widget;

	if (mouseButton == 1)
	{
		// Open dropdown.
		if (!combo->isOpen)
		{
			// Lock input.
			wz_desktop_push_lock_input_widget(widget->desktop, widget);

			// Show dropdown list.
			wz_widget_set_visible((struct wzWidget *)combo->list, true);
			wz_combo_update_list_rect(combo);

			combo->isOpen = true;
		}
		// Close dropdown.
		// Don't do it if hovering over the dropdown list.
		else if (!((struct wzWidget *)combo->list)->hover)
		{
			// Unlock input.
			wz_desktop_pop_lock_input_widget(widget->desktop, widget);

			// Hide dropdown list.
			wz_widget_set_visible((struct wzWidget *)combo->list, false);

			combo->isOpen = false;
		}
	}
}

struct wzCombo *wz_combo_create(struct wzContext *context)
{
	struct wzCombo *combo;

	assert(context);
	combo = (struct wzCombo *)malloc(sizeof(struct wzCombo));
	memset(combo, 0, sizeof(struct wzCombo));
	combo->base.type = WZ_TYPE_COMBO;
	combo->base.context = context;
	combo->base.vtable.set_rect = wz_combo_set_rect;
	combo->base.vtable.mouse_button_down = wz_combo_mouse_button_down;

	combo->list = wz_list_create(context);
	wz_widget_add_child_widget((struct wzWidget *)combo, (struct wzWidget *)combo->list);
	wz_widget_set_visible((struct wzWidget *)combo->list, false);

	return combo;
}

struct wzList *wz_combo_get_list(struct wzCombo *combo)
{
	assert(combo);
	return combo->list;
}

bool wz_combo_is_open(struct wzCombo *combo)
{
	assert(combo);
	return combo->isOpen;
}
