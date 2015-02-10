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
#include "wz_internal.h"
#pragma hdrstop

namespace wz {

static void wz_radio_button_clicked(Event *e)
{
	struct WidgetImpl *parent = e->base.widget->parent;

	WZ_ASSERT(parent);

	// Unset all the other radio button siblings.
	for (size_t i = 0; i < parent->children.size(); i++)
	{
		if (parent->children[i]->type == WZ_TYPE_RADIO_BUTTON && parent->children[i] != e->base.widget)
		{
			((struct ButtonImpl *)parent->children[i])->set(false);
		}
	}
}

static void wz_radio_button_added(struct WidgetImpl *parent, struct WidgetImpl *widget)
{
	// If this is the only radio button child, set it.
	// This means that the first radio button will be selected.
	for (size_t i = 0; i < parent->children.size(); i++)
	{
		if (parent->children[i]->type == WZ_TYPE_RADIO_BUTTON && parent->children[i] != widget)
			return;
	}

	((struct ButtonImpl *)widget)->set(true);
}

static Size wz_radio_button_measure(struct WidgetImpl *widget)
{
	Size size;
	struct RadioButtonImpl *radioButton = (struct RadioButtonImpl *)widget;
	widget->measureText(radioButton->label.c_str(), 0, &size.w, &size.h);
	size.w += WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS * 2 + WZ_SKIN_RADIO_BUTTON_SPACING;
	size.h = WZ_MAX(size.h, WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS);
	return size;
}

static void wz_radio_button_draw(struct WidgetImpl *widget, Rect clip)
{
	Rect rect;
	struct RadioButtonImpl *radioButton = (struct RadioButtonImpl *)widget;
	NVGRenderer *r = (NVGRenderer *)widget->renderer;
	struct NVGcontext *vg = r->getContext();

	nvgSave(vg);
	rect = widget->getAbsoluteRect();

	if (!r->clipToRectIntersection(clip, rect))
		return;

	// Inner circle.
	if (radioButton->isSet())
	{
		nvgBeginPath(vg);
		nvgCircle(vg, (float)(rect.x + WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS), rect.y + rect.h / 2.0f, (float)WZ_SKIN_RADIO_BUTTON_INNER_RADIUS);
		nvgFillColor(vg, WZ_SKIN_RADIO_BUTTON_SET_COLOR);
		nvgFill(vg);
	}

	// Outer circle.
	nvgBeginPath(vg);
	nvgCircle(vg, (float)(rect.x + WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS), rect.y + rect.h / 2.0f, (float)WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS - 0.5f);
	nvgStrokeColor(vg, widget->hover ? WZ_SKIN_RADIO_BUTTON_BORDER_HOVER_COLOR : WZ_SKIN_RADIO_BUTTON_BORDER_COLOR);
	nvgStroke(vg);

	// Label.
	r->print(rect.x + WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS * 2 + WZ_SKIN_RADIO_BUTTON_SPACING, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_RADIO_BUTTON_TEXT_COLOR, radioButton->label.c_str(), 0);

	nvgRestore(vg);
}

RadioButtonImpl::RadioButtonImpl(const std::string &label) : ButtonImpl(label)
{
	type = WZ_TYPE_RADIO_BUTTON;
	vtable.added = wz_radio_button_added;
	vtable.measure = wz_radio_button_measure;
	vtable.draw = wz_radio_button_draw;
	setSetBehavior(WZ_BUTTON_SET_BEHAVIOR_STICKY);
	addCallbackClicked(wz_radio_button_clicked);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

RadioButton::RadioButton()
{
	impl.reset(new RadioButtonImpl);
}

RadioButton::RadioButton(const std::string &label)
{
	impl.reset(new RadioButtonImpl(label));
}

RadioButton::~RadioButton()
{
}

const char *RadioButton::getLabel() const
{
	return getImpl()->getLabel();
}

RadioButton *RadioButton::setLabel(const std::string &label)
{
	getImpl()->setLabel(label.c_str());
	return this;
}

ButtonImpl *RadioButton::getImpl()
{
	return (ButtonImpl *)impl.get();
}

const ButtonImpl *RadioButton::getImpl() const
{
	return (const ButtonImpl *)impl.get();
}

} // namespace wz
