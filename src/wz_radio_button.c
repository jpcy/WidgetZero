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
#include "wz_renderer.h"
#include "wz_widget.h"
#include "wz_button.h"

static void wz_radio_button_clicked(wzEvent *e)
{
	int i;
	struct wzWidget *parent = e->base.widget->parent;

	WZ_ASSERT(parent);

	// Unset all the other radio button siblings.
	for (i = 0; i < wz_arr_len(parent->children); i++)
	{
		if (parent->children[i]->type == WZ_TYPE_RADIO_BUTTON && parent->children[i] != e->base.widget)
		{
			wz_button_set((struct wzButton *)parent->children[i], false);
		}
	}
}

static void wz_radio_button_added(struct wzWidget *parent, struct wzWidget *widget)
{
	int i;

	// If this is the only radio button child, set it.
	// This means that the first radio button will be selected.
	for (i = 0; i < wz_arr_len(parent->children); i++)
	{
		if (parent->children[i]->type == WZ_TYPE_RADIO_BUTTON && parent->children[i] != widget)
			return;
	}

	wz_button_set((struct wzButton *)widget, true);
}

static wzSize wz_radio_button_measure(struct wzWidget *widget)
{
	wzSize size;
	struct wzButton *button = (struct wzButton *)widget;
	const wzRadioButtonStyle *style = &widget->style.radioButton;
	wz_widget_measure_text(widget, wz_button_get_label(button), 0, &size.w, &size.h);
	size.w += style->outerRadius * 2 + style->spacing;
	size.h = WZ_MAX(size.h, style->outerRadius);
	return size;
}

static void wz_radio_button_draw(struct wzWidget *widget, wzRect clip)
{
	wzRect rect;
	struct wzButton *button = (struct wzButton *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzRadioButtonStyle *style = &widget->style.radioButton;

	nvgSave(vg);
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)button);

	if (!wz_renderer_clip_to_rect_intersection(vg, clip, rect))
		return;

	// Inner circle.
	if (wz_button_is_set(button))
	{
		nvgBeginPath(vg);
		nvgCircle(vg, (float)(rect.x + style->outerRadius), rect.y + rect.h / 2.0f, (float)style->innerRadius);
		nvgFillColor(vg, style->setColor);
		nvgFill(vg);
	}

	// Outer circle.
	nvgBeginPath(vg);
	nvgCircle(vg, (float)(rect.x + style->outerRadius), rect.y + rect.h / 2.0f, (float)style->outerRadius - 0.5f);
	nvgStrokeColor(vg, widget->hover ? style->borderHoverColor : style->borderColor);
	nvgStroke(vg);

	// Label.
	wz_renderer_print(widget->renderer, rect.x + style->outerRadius * 2 + style->spacing, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, style->textColor, wz_button_get_label(button), 0);

	nvgRestore(vg);
}

struct wzButton *wz_radio_button_create(const char *label)
{
	struct wzButton *button = wz_button_create(label, NULL);
	struct wzWidget *widget = (struct wzWidget *)button;
	wzRadioButtonStyle *style = &widget->style.radioButton;

	widget->type = WZ_TYPE_RADIO_BUTTON;
	widget->vtable.added = wz_radio_button_added;
	widget->vtable.measure = wz_radio_button_measure;
	widget->vtable.draw = wz_radio_button_draw;
	wz_button_set_set_behavior(button, WZ_BUTTON_SET_BEHAVIOR_STICKY);
	wz_button_add_callback_clicked(button, wz_radio_button_clicked);

	style->textColor = WZ_STYLE_TEXT_COLOR;
	style->setColor = nvgRGBf(0.1529f, 0.5569f, 0.7412f);
	style->borderColor = WZ_STYLE_LIGHT_BORDER_COLOR;
	style->borderHoverColor = WZ_STYLE_HOVER_COLOR;
	style->outerRadius = 8;
	style->innerRadius = 4;
	style->spacing = 8;

	return button;
}
