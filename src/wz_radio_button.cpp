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

struct RadioButtonImpl : public ButtonImpl
{
};

static void wz_radio_button_clicked(Event *e)
{
	struct WidgetImpl *parent = e->base.widget->parent;

	WZ_ASSERT(parent);

	// Unset all the other radio button siblings.
	for (size_t i = 0; i < parent->children.size(); i++)
	{
		if (parent->children[i]->type == WZ_TYPE_RADIO_BUTTON && parent->children[i] != e->base.widget)
		{
			wz_button_set((struct ButtonImpl *)parent->children[i], false);
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

	wz_button_set((struct ButtonImpl *)widget, true);
}

static Size wz_radio_button_measure(struct WidgetImpl *widget)
{
	Size size;
	struct RadioButtonImpl *radioButton = (struct RadioButtonImpl *)widget;
	wz_widget_measure_text(widget, radioButton->label.c_str(), 0, &size.w, &size.h);
	size.w += WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS * 2 + WZ_SKIN_RADIO_BUTTON_SPACING;
	size.h = WZ_MAX(size.h, WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS);
	return size;
}

static void wz_radio_button_draw(struct WidgetImpl *widget, Rect clip)
{
	Rect rect;
	struct RadioButtonImpl *radioButton = (struct RadioButtonImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;

	nvgSave(vg);
	rect = wz_widget_get_absolute_rect(widget);

	if (!wz_renderer_clip_to_rect_intersection(vg, clip, rect))
		return;

	// Inner circle.
	if (radioButton->isSet)
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
	wz_renderer_print(widget->renderer, rect.x + WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS * 2 + WZ_SKIN_RADIO_BUTTON_SPACING, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_RADIO_BUTTON_TEXT_COLOR, radioButton->label.c_str(), 0);

	nvgRestore(vg);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

RadioButton::RadioButton()
{
	impl = wz_radio_button_create(NULL);
}

RadioButton::RadioButton(const std::string &label)
{
	impl = wz_radio_button_create(label.c_str());
}

RadioButton::~RadioButton()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

const char *RadioButton::getLabel() const
{
	return wz_radio_button_get_label((const RadioButtonImpl *)impl);
}

RadioButton *RadioButton::setLabel(const std::string &label)
{
	wz_radio_button_set_label((RadioButtonImpl *)impl, label.c_str());
	return this;
}

struct RadioButtonImpl *wz_radio_button_create(const char *label)
{
	struct RadioButtonImpl *radioButton = (struct RadioButtonImpl *)new ButtonImpl(label);
	struct WidgetImpl *widget = (struct WidgetImpl *)radioButton;

	widget->type = WZ_TYPE_RADIO_BUTTON;
	widget->vtable.added = wz_radio_button_added;
	widget->vtable.measure = wz_radio_button_measure;
	widget->vtable.draw = wz_radio_button_draw;
	wz_button_set_set_behavior(radioButton, WZ_BUTTON_SET_BEHAVIOR_STICKY);
	wz_button_add_callback_clicked(radioButton, wz_radio_button_clicked);

	return radioButton;
}

void wz_radio_button_set_label(struct RadioButtonImpl *radioButton, const char *label)
{
	WZ_ASSERT(radioButton);
	wz_button_set_label(radioButton, label);
}

const char *wz_radio_button_get_label(const struct RadioButtonImpl *radioButton)
{
	WZ_ASSERT(radioButton);
	return radioButton->label.c_str();
}

bool wz_radio_button_is_set(const struct RadioButtonImpl *radioButton)
{
	WZ_ASSERT(radioButton);
	return radioButton->isSet;
}

void wz_radio_button_set(struct RadioButtonImpl *radioButton, bool value)
{
	WZ_ASSERT(radioButton);
	wz_button_set(radioButton, value);
}

void wz_radio_button_add_callback_clicked(struct RadioButtonImpl *radioButton, EventCallback callback)
{
	WZ_ASSERT(radioButton);
	wz_button_add_callback_clicked(radioButton, callback);
}

} // namespace wz
