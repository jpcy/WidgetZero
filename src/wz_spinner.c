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
#include "wz_renderer.h"
#include "wz_text_edit.h"

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

	if (!spinner->base.renderer)
		return; // Not fully initialized yet.

	rect = wz_widget_get_rect((const struct wzWidget *)spinner);

	// Update text edit rect.
	textEditRect.x = textEditRect.y = 0;
	textEditRect.w = rect.w;
	textEditRect.h = rect.h;
	wz_widget_set_rect_internal((struct wzWidget *)spinner->textEdit, textEditRect);

	// Update decrement button rect.
	buttonRect.y = 0;
	buttonRect.w = spinner->base.renderer->get_spinner_button_width(spinner->base.renderer);
	buttonRect.h = rect.h / 2;
	buttonRect.x = rect.w - buttonRect.w;
	wz_widget_set_rect_internal((struct wzWidget *)spinner->decrementButton, buttonRect);

	// Update increment button rect.
	buttonRect.y = buttonRect.h;
	wz_widget_set_rect_internal((struct wzWidget *)spinner->incrementButton, buttonRect);
}

static void wz_spinner_decrement_button_draw(struct wzWidget *widget, wzRect clip)
{
	WZ_ASSERT(widget);
	widget->renderer->draw_spinner_button(widget->renderer, clip, (struct wzSpinner *)widget->parent, (struct wzButton *)widget, true);
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

static void wz_spinner_increment_button_draw(struct wzWidget *widget, wzRect clip)
{
	WZ_ASSERT(widget);
	widget->renderer->draw_spinner_button(widget->renderer, clip, (struct wzSpinner *)widget->parent, (struct wzButton *)widget, false);
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

static wzSize wz_spinner_measure(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->renderer->measure_spinner(widget->renderer, (struct wzSpinner *)widget, ((struct wzSpinner *)widget)->textEdit);
}

static void wz_spinner_draw(struct wzWidget *widget, wzRect clip)
{
	WZ_ASSERT(widget);
	widget->renderer->draw_spinner(widget->renderer, clip, (struct wzSpinner *)widget);
}

static void wz_spinner_renderer_changed(struct wzWidget *widget)
{
	struct wzSpinner *spinner;
	wzBorder textEditBorder;

	WZ_ASSERT(widget);
	spinner = (struct wzSpinner *)widget;

	// Shrink the text edit border to exclude the increment and decrement buttons.
	textEditBorder = wz_text_edit_get_border(spinner->textEdit);
	textEditBorder.right += widget->renderer->get_spinner_button_width(widget->renderer);
	wz_text_edit_set_border(spinner->textEdit, textEditBorder);

	wz_spinner_update_child_rects(spinner);
}

static void wz_spinner_set_rect(struct wzWidget *widget, wzRect rect)
{
	WZ_ASSERT(widget);
	widget->rect = rect;
	wz_spinner_update_child_rects((struct wzSpinner *)widget);
}

// Set the text edit font to match.
static void wz_spinner_font_changed(struct wzWidget *widget, const char *fontFace, float fontSize)
{
	struct wzSpinner *spinner = (struct wzSpinner *)widget;
	WZ_ASSERT(spinner);
	wz_widget_set_font((struct wzWidget *)spinner->textEdit, fontFace, fontSize);
}

struct wzSpinner *wz_spinner_create()
{
	struct wzSpinner *spinner = (struct wzSpinner *)malloc(sizeof(struct wzSpinner));
	memset(spinner, 0, sizeof(struct wzSpinner));
	spinner->base.type = WZ_TYPE_SPINNER;
	spinner->base.vtable.measure = wz_spinner_measure;
	spinner->base.vtable.draw = wz_spinner_draw;
	spinner->base.vtable.renderer_changed = wz_spinner_renderer_changed;
	spinner->base.vtable.set_rect = wz_spinner_set_rect;
	spinner->base.vtable.font_changed = wz_spinner_font_changed;

	spinner->textEdit = wz_text_edit_create(false, 256);
	wz_text_edit_set_validate_text_callback(spinner->textEdit, wz_spinner_validate_text);
	wz_widget_add_child_widget((struct wzWidget *)spinner, (struct wzWidget *)spinner->textEdit);

	spinner->decrementButton = wz_button_create();
	wz_widget_set_draw_callback((struct wzWidget *)spinner->decrementButton, wz_spinner_decrement_button_draw);
	wz_widget_set_draw_priority((struct wzWidget *)spinner->decrementButton, WZ_DRAW_PRIORITY_SPINNER_BUTTON);
	wz_button_add_callback_clicked(spinner->decrementButton, wz_spinner_decrement_button_clicked);
	wz_widget_add_child_widget((struct wzWidget *)spinner, (struct wzWidget *)spinner->decrementButton);

	spinner->incrementButton = wz_button_create();
	wz_widget_set_draw_callback((struct wzWidget *)spinner->incrementButton, wz_spinner_increment_button_draw);
	wz_widget_set_draw_priority((struct wzWidget *)spinner->incrementButton, WZ_DRAW_PRIORITY_SPINNER_BUTTON);
	wz_button_add_callback_clicked(spinner->incrementButton, wz_spinner_increment_button_clicked);
	wz_widget_add_child_widget((struct wzWidget *)spinner, (struct wzWidget *)spinner->incrementButton);

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
