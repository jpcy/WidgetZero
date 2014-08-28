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

struct wzGroupBox
{
	struct wzWidget base;
	struct wzWidget *content;
	char label[128];
};

static struct wzWidget *wz_group_box_get_content_widget(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	return ((struct wzGroupBox *)widget)->content;
}

static void wz_group_box_draw(struct wzWidget *widget, wzRect clip)
{
	WZ_ASSERT(widget);
	widget->renderer->draw_group_box(widget->renderer, clip, (struct wzGroupBox *)widget);
}

static void wz_group_box_renderer_changed(struct wzWidget *widget)
{
	struct wzGroupBox *groupBox = (struct wzGroupBox *)widget;
	WZ_ASSERT(groupBox);
	wz_widget_set_margin(groupBox->content, widget->renderer->measure_group_box_margin(widget->renderer, groupBox));
}

struct wzGroupBox *wz_group_box_create()
{
	struct wzGroupBox *groupBox = (struct wzGroupBox *)malloc(sizeof(struct wzGroupBox));
	memset(groupBox, 0, sizeof(struct wzGroupBox));
	groupBox->base.type = WZ_TYPE_GROUP_BOX;
	groupBox->base.vtable.get_content_widget = wz_group_box_get_content_widget;
	groupBox->base.vtable.draw = wz_group_box_draw;
	groupBox->base.vtable.renderer_changed = wz_group_box_renderer_changed;

	// Create content widget.
	groupBox->content = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(groupBox->content, 0, sizeof(struct wzWidget));
	groupBox->content->stretch = WZ_STRETCH;
	wz_widget_add_child_widget_internal((struct wzWidget *)groupBox, groupBox->content);

	return groupBox;
}

void wz_group_box_set_label(struct wzGroupBox *groupBox, const char *label)
{
	WZ_ASSERT(groupBox);
	strcpy(groupBox->label, label);

	// Update the margin.
	if (groupBox->base.renderer)
	{
		wz_widget_set_margin(groupBox->content, groupBox->base.renderer->measure_group_box_margin(groupBox->base.renderer, groupBox));
	}
}

const char *wz_group_box_get_label(const struct wzGroupBox *groupBox)
{
	WZ_ASSERT(groupBox);
	return groupBox->label;
}
