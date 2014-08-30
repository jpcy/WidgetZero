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
#include "wz_widget.h"
#include "wz_string.h"

struct wzLabel
{
	struct wzWidget base;
	wzString text;
	bool multiline;
	wzColor textColor;
	bool isTextColorUserSet;
};

static wzSize wz_label_measure(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return widget->renderer->measure_label(widget->renderer, (struct wzLabel *)widget);
}

static void wz_label_draw(struct wzWidget *widget, wzRect clip)
{
	WZ_ASSERT(widget);
	widget->renderer->draw_label(widget->renderer, clip, (struct wzLabel *)widget);
}

static void wz_label_renderer_changed(struct wzWidget *widget)
{
	struct wzLabel *label = (struct wzLabel *)widget;
	WZ_ASSERT(label);

	if (!label->isTextColorUserSet)
	{
		label->textColor = widget->renderer->get_default_text_color(widget->renderer);
	}
}

static void wz_label_destroy(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	wz_string_free(((struct wzLabel *)widget)->text);
}

struct wzLabel *wz_label_create()
{
	struct wzLabel *label = (struct wzLabel *)malloc(sizeof(struct wzLabel));
	memset(label, 0, sizeof(struct wzLabel));
	label->base.type = WZ_TYPE_LABEL;
	label->base.vtable.measure = wz_label_measure;
	label->base.vtable.draw = wz_label_draw;
	label->base.vtable.renderer_changed = wz_label_renderer_changed;
	label->base.vtable.destroy = wz_label_destroy;
	label->text = wz_string_empty();
	return label;
}

void wz_label_set_multiline(struct wzLabel *label, bool multiline)
{
	WZ_ASSERT(label);
	label->multiline = multiline;
}

bool wz_label_get_multiline(const struct wzLabel *label)
{
	WZ_ASSERT(label);
	return label->multiline;
}

void wz_label_set_text(struct wzLabel *label, const char *text)
{
	WZ_ASSERT(label);
	label->text = wz_string_copy(label->text, text);
	wz_widget_resize_to_measured(&label->base);
}

const char *wz_label_get_text(const struct wzLabel *label)
{
	WZ_ASSERT(label);
	return label->text;
}

void wz_label_set_text_color(struct wzLabel *label, wzColor color)
{
	WZ_ASSERT(label);
	label->textColor = color;
	label->isTextColorUserSet = true;
}

wzColor wz_label_get_text_color(const struct wzLabel *label)
{
	return label->textColor;
}
