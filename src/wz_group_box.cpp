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
#include "wz_pch.h"
#pragma hdrstop

namespace wz {

static void wz_group_box_draw(struct WidgetImpl *widget, Rect clip)
{
	Rect rect;
	struct GroupBoxImpl *groupBox = (struct GroupBoxImpl *)widget;
	NVGRenderer *r = (NVGRenderer *)widget->renderer;
	struct NVGcontext *vg = r->getContext();

	nvgSave(vg);
	r->clipToRect(clip);
	rect = widget->getAbsoluteRect();
	
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
		r->drawLine((int)(borderRect.x + WZ_SKIN_GROUP_BOX_CORNER_RADIUS), borderRect.y, borderRect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN - WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING, borderRect.y, WZ_SKIN_GROUP_BOX_BORDER_COLOR);

		// Border top, right of text.
		r->drawLine(borderRect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN + textWidth + WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING * 2, borderRect.y, (int)(borderRect.x + borderRect.w - WZ_SKIN_GROUP_BOX_CORNER_RADIUS), borderRect.y, WZ_SKIN_GROUP_BOX_BORDER_COLOR);

		// The rest of the border.
		r->createRectPath(borderRect, WZ_SKIN_GROUP_BOX_CORNER_RADIUS, WZ_SIDE_LEFT | WZ_SIDE_RIGHT | WZ_SIDE_BOTTOM, WZ_CORNER_ALL);
		nvgStrokeColor(vg, WZ_SKIN_GROUP_BOX_BORDER_COLOR);
		nvgStroke(vg);

		// Label.
		r->print(rect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN, rect.y, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, widget->fontFace, widget->fontSize, WZ_SKIN_GROUP_BOX_TEXT_COLOR, groupBox->label.c_str(), 0);
	}

	nvgRestore(vg);
}

static void wz_group_box_refresh_margin(struct GroupBoxImpl *groupBox)
{
	Border margin;
	margin.top = margin.bottom = margin.left = margin.right = WZ_SKIN_GROUP_BOX_MARGIN;

	if (!groupBox->label.empty())
	{
		margin.top = wz_widget_get_line_height(groupBox) + WZ_SKIN_GROUP_BOX_MARGIN;
	}

	groupBox->content->setMargin(margin);
}

static void wz_group_box_renderer_changed(struct WidgetImpl *widget)
{
	wz_group_box_refresh_margin((struct GroupBoxImpl *)widget);
}

GroupBoxImpl::GroupBoxImpl(const std::string &label) : label(label)
{
	type = WZ_TYPE_GROUP_BOX;
	vtable.draw = wz_group_box_draw;
	vtable.renderer_changed = wz_group_box_renderer_changed;

	// Create content widget.
	content = new WidgetImpl;
	content->stretch = WZ_STRETCH;
	wz_widget_add_child_widget(this, content);
}

void GroupBoxImpl::setLabel(const char *label)
{
	this->label = label;

	// Update the margin.
	if (renderer)
	{
		wz_group_box_refresh_margin(this);
	}
}

const char *GroupBoxImpl::getLabel() const
{
	return label.c_str();
}

void GroupBoxImpl::add(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	wz_widget_add_child_widget(content, widget);
}

void GroupBoxImpl::remove(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	wz_widget_remove_child_widget(content, widget);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

GroupBox::GroupBox()
{
	impl = new GroupBoxImpl;
	impl->setSize(200, 200);
}

GroupBox::GroupBox(const std::string &label)
{
	impl = new GroupBoxImpl(label);
	impl->setSize(200, 200);
}

GroupBox::~GroupBox()
{
	if (!impl->getMainWindow())
	{
		wz_widget_destroy(impl);
	}
}

const char *GroupBox::getLabel() const
{
	return ((GroupBoxImpl *)impl)->getLabel();
}

GroupBox *GroupBox::setLabel(const std::string &label)
{
	((GroupBoxImpl *)impl)->setLabel(label.c_str());
	return this;
}

Widget *GroupBox::add(Widget *widget)
{
	((GroupBoxImpl *)impl)->add(widget->impl);
	return widget;
}

void GroupBox::remove(Widget *widget)
{
	((GroupBoxImpl *)impl)->remove(widget->impl);
}

} // namespace wz
