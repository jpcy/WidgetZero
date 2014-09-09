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
	const wzRendererStyle *style = &widget->renderer->style;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect(widget);
	
	// Background.
	wz_renderer_draw_filled_rect(vg, rect, style->backgroundColor);

	// Border.
	if (!groupBox->label[0])
	{
		wz_renderer_draw_rect(vg, rect, style->borderColor);
	}
	else
	{
		int textWidth, textHeight;
		wz_widget_measure_text(widget, groupBox->label, 0, &textWidth, &textHeight);

		// Left, right, bottom, top left, top right.
		wz_renderer_draw_line(vg, rect.x, rect.y + textHeight / 2, rect.x, rect.y + rect.h, style->borderColor);
		wz_renderer_draw_line(vg, rect.x + rect.w, rect.y + textHeight / 2, rect.x + rect.w, rect.y + rect.h, style->borderColor);
		wz_renderer_draw_line(vg, rect.x, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h, style->borderColor);
		wz_renderer_draw_line(vg, rect.x, rect.y + textHeight / 2, rect.x + style->groupBoxTextLeftMargin - style->groupBoxTextBorderSpacing, rect.y + textHeight / 2, style->borderColor);
		wz_renderer_draw_line(vg, rect.x + style->groupBoxTextLeftMargin + textWidth + style->groupBoxTextBorderSpacing * 2, rect.y + textHeight / 2, rect.x + rect.w, rect.y + textHeight / 2, style->borderColor);

		// Label.
		wz_renderer_print(widget->renderer, rect.x + style->groupBoxTextLeftMargin, rect.y, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, widget->fontFace, widget->fontSize, style->textColor, groupBox->label, 0);
	}

	nvgRestore(vg);
}

static void wz_group_box_refresh_margin(struct wzGroupBox *groupBox)
{
	wzBorder margin;
	const wzRendererStyle *style = &groupBox->base.renderer->style;
	margin.top = margin.bottom = margin.left = margin.right = style->groupBoxMargin;

	if (groupBox->label[0])
	{
		margin.top = wz_widget_get_line_height((struct wzWidget *)groupBox) + style->groupBoxMargin;
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
	memset(groupBox, 0, sizeof(struct wzGroupBox));
	groupBox->base.type = WZ_TYPE_GROUP_BOX;
	groupBox->base.vtable.draw = wz_group_box_draw;
	groupBox->base.vtable.renderer_changed = wz_group_box_renderer_changed;
	groupBox->base.vtable.destroy = wz_group_box_destroy;
	groupBox->label = wz_string_empty();

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
