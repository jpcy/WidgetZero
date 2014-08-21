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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wz_widget.h"

struct wzSpinner
{
	struct wzWidget base;
	struct wzTextEdit *textEdit;
	struct wzButton *decrementButton;
	struct wzButton *incrementButton;
};

static bool wz_spinner_validate_text(const char *text)
{
	size_t i;

	for (i = 0; i < strlen(text); i++)
	{
		if (!(isdigit(text[i]) || text[i] == '.'|| text[i] == '-'))
			return false;
	}

	return true;
}

static void wz_spinner_update_child_rects(struct wzSpinner *spinner)
{
	wzRect rect, textEditRect, buttonRect;

	WZ_ASSERT(spinner);
	rect = wz_widget_get_rect((const struct wzWidget *)spinner);

	// Update text edit rect.
	textEditRect.x = textEditRect.y = 0;
	textEditRect.w = rect.w;
	textEditRect.h = rect.h;
	wz_widget_set_rect_internal((struct wzWidget *)spinner->textEdit, textEditRect);

	// Update decrement button rect.
	buttonRect.y = 0;
	buttonRect.w = WZ_MAX(wz_widget_get_width((struct wzWidget *)spinner->decrementButton), wz_widget_get_width((struct wzWidget *)spinner->incrementButton));
	buttonRect.h = rect.h / 2;
	buttonRect.x = rect.w - buttonRect.w;
	wz_widget_set_rect_internal((struct wzWidget *)spinner->decrementButton, buttonRect);

	// Update increment button rect.
	buttonRect.y = buttonRect.h;
	wz_widget_set_rect_internal((struct wzWidget *)spinner->incrementButton, buttonRect);
}

static void wz_spinner_decrement_button_clicked(wzEvent *e)
{
	struct wzSpinner *spinner;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	spinner = (struct wzSpinner *)e->base.widget->parent;
	wz_spinner_set_value(spinner, wz_spinner_get_value(spinner) - 1);
}

static void wz_spinner_increment_button_clicked(wzEvent *e)
{
	struct wzSpinner *spinner;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	spinner = (struct wzSpinner *)e->base.widget->parent;
	wz_spinner_set_value(spinner, wz_spinner_get_value(spinner) + 1);
}

static void wz_spinner_set_rect(struct wzWidget *widget, wzRect rect)
{
	WZ_ASSERT(widget);
	widget->rect = rect;
	wz_spinner_update_child_rects((struct wzSpinner *)widget);
}

struct wzSpinner *wz_spinner_create(struct wzTextEdit *textEdit, struct wzButton *decrementButton, struct wzButton *incrementButton)
{
	struct wzSpinner *spinner;
	wzBorder textEditBorder;

	WZ_ASSERT(textEdit);
	WZ_ASSERT(!wz_text_edit_is_multiline(textEdit));
	WZ_ASSERT(decrementButton);
	WZ_ASSERT(incrementButton);

	spinner = (struct wzSpinner *)malloc(sizeof(struct wzSpinner));
	memset(spinner, 0, sizeof(struct wzSpinner));
	spinner->base.type = WZ_TYPE_SPINNER;
	spinner->base.vtable.set_rect = wz_spinner_set_rect;

	spinner->textEdit = textEdit;
	wz_text_edit_set_validate_text_callback(spinner->textEdit, wz_spinner_validate_text);
	wz_widget_add_child_widget_internal((struct wzWidget *)spinner, (struct wzWidget *)spinner->textEdit);

	spinner->decrementButton = decrementButton;
	wz_widget_set_draw_priority((struct wzWidget *)spinner->decrementButton, WZ_DRAW_PRIORITY_SPINNER_BUTTON);
	wz_button_add_callback_clicked(spinner->decrementButton, wz_spinner_decrement_button_clicked);
	wz_widget_add_child_widget_internal((struct wzWidget *)spinner, (struct wzWidget *)spinner->decrementButton);

	spinner->incrementButton = incrementButton;
	wz_widget_set_draw_priority((struct wzWidget *)spinner->incrementButton, WZ_DRAW_PRIORITY_SPINNER_BUTTON);
	wz_button_add_callback_clicked(spinner->incrementButton, wz_spinner_increment_button_clicked);
	wz_widget_add_child_widget_internal((struct wzWidget *)spinner, (struct wzWidget *)spinner->incrementButton);

	// Shrink the text edit border to exclude the increment and decrement buttons.
	textEditBorder = wz_text_edit_get_border(spinner->textEdit);
	textEditBorder.right += WZ_MAX(wz_widget_get_width((struct wzWidget *)spinner->decrementButton), wz_widget_get_width((struct wzWidget *)spinner->incrementButton));
	wz_text_edit_set_border(spinner->textEdit, textEditBorder);

	wz_spinner_update_child_rects(spinner);

	return spinner;
}

int wz_spinner_get_value(const struct wzSpinner *spinner)
{
	int value = 0;

	WZ_ASSERT(spinner);
	sscanf(wz_text_edit_get_text(spinner->textEdit), "%d", &value);
	return value;
}

void wz_spinner_set_value(struct wzSpinner *spinner, int value)
{
	char buffer[32];

	WZ_ASSERT(spinner);
	sprintf(buffer, "%d", value);
	wz_text_edit_set_text(spinner->textEdit, buffer);
}
