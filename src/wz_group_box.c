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
#include "wz_renderer.h"
#include "wz_string.h"

struct wzGroupBox
{
	struct wzWidget base;
	struct wzWidget *content;
	wzString label;
};

static void wz_group_box_draw(struct wzWidget *widget, wzRect clip)
{
	wzRect rect;
	struct wzGroupBox *groupBox = (struct wzGroupBox *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzGroupBoxStyle *style = &widget->style.groupBox;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect(widget);
	
	if (!groupBox->label[0])
	{
		nvgBeginPath(vg);
		nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, 5.0f);
		nvgStrokeColor(vg, style->borderColor);
		nvgStroke(vg);
	}
	else
	{
		const float r = 5;
		const float NVG_KAPPA90 = 0.5522847493f;
		wzRect borderRect;
		float x, y, w, h, rx, ry;
		int textWidth, textHeight;

		wz_widget_measure_text(widget, groupBox->label, 0, &textWidth, &textHeight);
		borderRect = rect;
		borderRect.y += textHeight / 2;
		borderRect.h -= textHeight / 2;
		x = borderRect.x + 0.5f;
		y = borderRect.y + 0.5f;
		w = borderRect.w - 1.0f;
		h = borderRect.h - 1.0f;
		rx = WZ_MIN(r, WZ_ABS(w) * 0.5f) * WZ_SIGN(w);
		ry = WZ_MIN(r, WZ_ABS(h) * 0.5f) * WZ_SIGN(h);

		nvgBeginPath(vg);
		nvgMoveTo(vg, x + style->textLeftMargin - style->textBorderSpacing, y);
		nvgLineTo(vg, x+rx, y); // top straight (left of text)
		nvgBezierTo(vg, x+rx*(1-NVG_KAPPA90), y, x, y+ry*(1-NVG_KAPPA90), x, y+ry); // top left arc
		nvgLineTo(vg, x, y + h - ry); // left straight
		nvgBezierTo(vg, x, y+h-ry*(1-NVG_KAPPA90), x+rx*(1-NVG_KAPPA90), y+h, x+rx, y+h); // bottom left arc
		nvgLineTo(vg, x+w-rx, y+h); // bottom straight
		nvgBezierTo(vg, x+w-rx*(1-NVG_KAPPA90), y+h, x+w, y+h-ry*(1-NVG_KAPPA90), x+w, y+h-ry); // bottom right arc
		nvgLineTo(vg, x+w, y+ry); // right straight
		nvgBezierTo(vg, x+w, y+ry*(1-NVG_KAPPA90), x+w-rx*(1-NVG_KAPPA90), y, x+w-rx, y); // top right arc
		nvgLineTo(vg, x + style->textLeftMargin + textWidth + style->textBorderSpacing, y); // top straight (right of text)
		nvgStrokeColor(vg, style->borderColor);
		nvgStroke(vg);

		// Label.
		wz_renderer_print(widget->renderer, rect.x + style->textLeftMargin, rect.y, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, widget->fontFace, widget->fontSize, style->textColor, groupBox->label, 0);
	}

	nvgRestore(vg);
}

static void wz_group_box_refresh_margin(struct wzGroupBox *groupBox)
{
	wzBorder margin;
	const wzGroupBoxStyle *style = &groupBox->base.style.groupBox;
	margin.top = margin.bottom = margin.left = margin.right = style->margin;

	if (groupBox->label[0])
	{
		margin.top = wz_widget_get_line_height((struct wzWidget *)groupBox) + style->margin;
	}

	wz_widget_set_margin(groupBox->content, margin);
}

static void wz_group_box_renderer_changed(struct wzWidget *widget)
{
	wz_group_box_refresh_margin((struct wzGroupBox *)widget);
}

static void wz_group_box_destroy(struct wzWidget *widget)
{
	struct wzGroupBox *groupBox = (struct wzGroupBox *)widget;
	WZ_ASSERT(groupBox);
	wz_string_free(groupBox->label);
}

struct wzGroupBox *wz_group_box_create()
{
	struct wzGroupBox *groupBox = (struct wzGroupBox *)malloc(sizeof(struct wzGroupBox));
	wzGroupBoxStyle *style = &groupBox->base.style.groupBox;

	memset(groupBox, 0, sizeof(struct wzGroupBox));
	groupBox->base.type = WZ_TYPE_GROUP_BOX;
	groupBox->base.vtable.draw = wz_group_box_draw;
	groupBox->base.vtable.renderer_changed = wz_group_box_renderer_changed;
	groupBox->base.vtable.destroy = wz_group_box_destroy;
	groupBox->label = wz_string_empty();

	style->textColor = nvgRGBf(1, 1, 1);
	style->borderColor = WZ_STYLE_DARK_BORDER_COLOR;
	style->margin = 8;
	style->textLeftMargin = 20;
	style->textBorderSpacing = 5;

	// Create content widget.
	groupBox->content = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(groupBox->content, 0, sizeof(struct wzWidget));
	groupBox->content->stretch = WZ_STRETCH;
	wz_widget_add_child_widget((struct wzWidget *)groupBox, groupBox->content);

	return groupBox;
}

void wz_group_box_set_label(struct wzGroupBox *groupBox, const char *label)
{
	WZ_ASSERT(groupBox);
	groupBox->label = wz_string_copy(groupBox->label, label);

	// Update the margin.
	if (groupBox->base.renderer)
	{
		wz_group_box_refresh_margin(groupBox);
	}
}

const char *wz_group_box_get_label(const struct wzGroupBox *groupBox)
{
	WZ_ASSERT(groupBox);
	return groupBox->label;
}

void wz_group_box_add(struct wzGroupBox *groupBox, struct wzWidget *widget)
{
	WZ_ASSERT(groupBox);
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	wz_widget_add_child_widget(groupBox->content, widget);
}

void wz_group_box_remove(struct wzGroupBox *groupBox, struct wzWidget *widget)
{
	WZ_ASSERT(groupBox);
	WZ_ASSERT(widget);
	wz_widget_remove_child_widget(groupBox->content, widget);
}
