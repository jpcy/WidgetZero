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
#include "wz_main_window.h"
#include "wz_widget.h"
#include "wz_renderer.h"
#include "wz_string.h"
#include "wz_button.h"
#include "wz_skin.h"

struct wzCheckBox
{
	union
	{
		struct wzWidget base;
		struct wzButton button;
	};
};

static wzSize wz_check_box_measure(struct wzWidget *widget)
{
	wzSize size;
	struct wzCheckBox *checkBox = (struct wzCheckBox *)widget;
	wz_widget_measure_text(widget, wz_check_box_get_label(checkBox), 0, &size.w, &size.h);
	size.w += WZ_SKIN_CHECK_BOX_BOX_SIZE + WZ_SKIN_CHECK_BOX_BOX_RIGHT_MARGIN;
	return size;
}

static void wz_check_box_draw(struct wzWidget *widget, wzRect clip)
{
	wzRect boxRect;
	struct wzCheckBox *checkBox = (struct wzCheckBox *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzRect rect = wz_widget_get_absolute_rect(widget);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);

	// Calculate box rect.
	boxRect.x = rect.x;
	boxRect.y = (int)(rect.y + rect.h / 2.0f - WZ_SKIN_CHECK_BOX_BOX_SIZE / 2.0f);
	boxRect.w = boxRect.h = WZ_SKIN_CHECK_BOX_BOX_SIZE;

	// Box border.
	wz_renderer_draw_rect(vg, boxRect, widget->hover ? WZ_SKIN_CHECK_BOX_BORDER_HOVER_COLOR : WZ_SKIN_CHECK_BOX_BORDER_COLOR);

	// Box checkmark.
	if (wz_check_box_is_checked(checkBox))
	{
		const float left = (float)boxRect.x + WZ_SKIN_CHECK_BOX_BOX_INTERNAL_MARGIN;
		const float right = (float)boxRect.x + boxRect.w - WZ_SKIN_CHECK_BOX_BOX_INTERNAL_MARGIN;
		const float top = (float)boxRect.y + WZ_SKIN_CHECK_BOX_BOX_INTERNAL_MARGIN;
		const float bottom = (float)boxRect.y + boxRect.h - WZ_SKIN_CHECK_BOX_BOX_INTERNAL_MARGIN;

		nvgBeginPath(vg);
		nvgMoveTo(vg, left, top);
		nvgLineTo(vg, right, bottom);
		nvgStrokeColor(vg, WZ_SKIN_CHECK_BOX_CHECK_COLOR);
		nvgStrokeWidth(vg, WZ_SKIN_CHECK_BOX_CHECK_THICKNESS);
		nvgStroke(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, left, bottom);
		nvgLineTo(vg, right, top);
		nvgStroke(vg);
	}

	// Label.
	wz_renderer_print(widget->renderer, rect.x + WZ_SKIN_CHECK_BOX_BOX_SIZE + WZ_SKIN_CHECK_BOX_BOX_RIGHT_MARGIN, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_CHECK_BOX_TEXT_COLOR, wz_check_box_get_label(checkBox), 0);

	nvgRestore(vg);
}

struct wzCheckBox *wz_check_box_create(const char *label)
{
	struct wzCheckBox *checkBox = (struct wzCheckBox *)wz_button_create(label, NULL);
	wz_button_set_set_behavior(&checkBox->button, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
	checkBox->base.vtable.measure = wz_check_box_measure;
	checkBox->base.vtable.draw = wz_check_box_draw;
	return checkBox;
}

void wz_check_box_set_label(struct wzCheckBox *checkBox, const char *label)
{
	WZ_ASSERT(checkBox);
	wz_button_set_label(&checkBox->button, label);
}

const char *wz_check_box_get_label(const struct wzCheckBox *checkBox)
{
	WZ_ASSERT(checkBox);
	return wz_button_get_label(&checkBox->button);
}

bool wz_check_box_is_checked(const struct wzCheckBox *checkBox)
{
	WZ_ASSERT(checkBox);
	return wz_button_is_set(&checkBox->button);
}

void wz_check_box_check(struct wzCheckBox *checkBox, bool value)
{
	WZ_ASSERT(checkBox);
	wz_button_set(&checkBox->button, value);
}

void wz_check_box_bind_value(struct wzCheckBox *checkBox, bool *value)
{
	WZ_ASSERT(checkBox);
	wz_button_bind_value(&checkBox->button, value);
}

void wz_check_box_add_callback_checked(struct wzCheckBox *checkBox, wzEventCallback callback)
{
	WZ_ASSERT(checkBox);
	wz_button_add_callback_clicked(&checkBox->button, callback);
}
