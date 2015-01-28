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
#include <string>
#include <stdlib.h>
#include <string.h>
#include "wz_widget.h"
#include "wz_renderer.h"
#include "wz_skin.h"

namespace wz {

struct GroupBoxImpl : public WidgetImpl
{
	GroupBoxImpl()
	{
		type = WZ_TYPE_GROUP_BOX;
		content = NULL;
	}

	struct WidgetImpl *content;
	std::string label;
};

static void wz_group_box_draw(struct WidgetImpl *widget, Rect clip)
{
	Rect rect;
	struct GroupBoxImpl *groupBox = (struct GroupBoxImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect(widget);
	
	if (groupBox->label.empty())
	{
		nvgBeginPath(vg);
		nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, WZ_SKIN_GROUP_BOX_CORNER_RADIUS);
		nvgStrokeColor(vg, WZ_SKIN_GROUP_BOX_BORDER_COLOR);
		nvgStroke(vg);
	}
	else
	{
		int textWidth, textHeight;
		Rect borderRect;

		wz_widget_measure_text(widget, groupBox->label.c_str(), 0, &textWidth, &textHeight);
		borderRect = rect;
		borderRect.y += textHeight / 2;
		borderRect.h -= textHeight / 2;

		// Border top, left of text.
		wz_renderer_draw_line(vg, (int)(borderRect.x + WZ_SKIN_GROUP_BOX_CORNER_RADIUS), borderRect.y, borderRect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN - WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING, borderRect.y, WZ_SKIN_GROUP_BOX_BORDER_COLOR);

		// Border top, right of text.
		wz_renderer_draw_line(vg, borderRect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN + textWidth + WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING * 2, borderRect.y, (int)(borderRect.x + borderRect.w - WZ_SKIN_GROUP_BOX_CORNER_RADIUS), borderRect.y, WZ_SKIN_GROUP_BOX_BORDER_COLOR);

		// The rest of the border.
		wz_renderer_create_rect_path(vg, borderRect, WZ_SKIN_GROUP_BOX_CORNER_RADIUS, WZ_SIDE_LEFT | WZ_SIDE_RIGHT | WZ_SIDE_BOTTOM, WZ_CORNER_ALL);
		nvgStrokeColor(vg, WZ_SKIN_GROUP_BOX_BORDER_COLOR);
		nvgStroke(vg);

		// Label.
		wz_renderer_print(widget->renderer, rect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN, rect.y, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, widget->fontFace, widget->fontSize, WZ_SKIN_GROUP_BOX_TEXT_COLOR, groupBox->label.c_str(), 0);
	}

	nvgRestore(vg);
}

static void wz_group_box_refresh_margin(struct GroupBoxImpl *groupBox)
{
	Border margin;
	margin.top = margin.bottom = margin.left = margin.right = WZ_SKIN_GROUP_BOX_MARGIN;

	if (!groupBox->label.empty())
	{
		margin.top = wz_widget_get_line_height((struct WidgetImpl *)groupBox) + WZ_SKIN_GROUP_BOX_MARGIN;
	}

	wz_widget_set_margin(groupBox->content, margin);
}

static void wz_group_box_renderer_changed(struct WidgetImpl *widget)
{
	wz_group_box_refresh_margin((struct GroupBoxImpl *)widget);
}

GroupBox::GroupBox()
{
	impl = wz_group_box_create(NULL);
	wz_widget_set_size_args(impl, 200, 200);
}

GroupBox::GroupBox(const std::string &label)
{
	impl = wz_group_box_create(label.c_str());
	wz_widget_set_size_args(impl, 200, 200);
}

GroupBox::~GroupBox()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

const char *GroupBox::getLabel() const
{
	return wz_group_box_get_label((GroupBoxImpl *)impl);
}

GroupBox *GroupBox::setLabel(const std::string &label)
{
	wz_group_box_set_label((GroupBoxImpl *)impl, label.c_str());
	return this;
}

Widget *GroupBox::add(Widget *widget)
{
	wz_group_box_add((GroupBoxImpl *)impl, widget->impl);
	return widget;
}

void GroupBox::remove(Widget *widget)
{
	wz_group_box_remove((GroupBoxImpl *)impl, widget->impl);
}

struct GroupBoxImpl *wz_group_box_create(const char *label)
{
	struct GroupBoxImpl *groupBox = new struct GroupBoxImpl;
	groupBox->vtable.draw = wz_group_box_draw;
	groupBox->vtable.renderer_changed = wz_group_box_renderer_changed;
	groupBox->label = label ? std::string(label) : std::string();

	// Create content widget.
	groupBox->content = new struct WidgetImpl;
	groupBox->content->stretch = WZ_STRETCH;
	wz_widget_add_child_widget((struct WidgetImpl *)groupBox, groupBox->content);

	return groupBox;
}

void wz_group_box_set_label(struct GroupBoxImpl *groupBox, const char *label)
{
	WZ_ASSERT(groupBox);
	groupBox->label = std::string(label);

	// Update the margin.
	if (groupBox->renderer)
	{
		wz_group_box_refresh_margin(groupBox);
	}
}

const char *wz_group_box_get_label(const struct GroupBoxImpl *groupBox)
{
	WZ_ASSERT(groupBox);
	return groupBox->label.c_str();
}

void wz_group_box_add(struct GroupBoxImpl *groupBox, struct WidgetImpl *widget)
{
	WZ_ASSERT(groupBox);
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	wz_widget_add_child_widget(groupBox->content, widget);
}

void wz_group_box_remove(struct GroupBoxImpl *groupBox, struct WidgetImpl *widget)
{
	WZ_ASSERT(groupBox);
	WZ_ASSERT(widget);
	wz_widget_remove_child_widget(groupBox->content, widget);
}

} // namespace wz
