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

static Size wz_label_measure(struct WidgetImpl *widget)
{
	Size size;
	struct LabelImpl *label = (struct LabelImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;

	if (label->multiline)
	{
		float bounds[4];

		nvgFontSize(vg, widget->fontSize == 0 ? widget->renderer->defaultFontSize : widget->fontSize);
		wz_renderer_set_font_face(widget->renderer, widget->fontFace);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgTextLineHeight(vg, 1.2f);
		nvgTextBoxBounds(vg, 0, 0, (float)widget->rect.w, label->text.c_str(), NULL, bounds);
		size.w = (int)bounds[2];
		size.h = (int)bounds[3];
	}
	else
	{
		wz_widget_measure_text(widget, label->text.c_str(), 0, &size.w, &size.h);
	}

	return size;
}

static void wz_label_draw(struct WidgetImpl *widget, Rect clip)
{
	struct LabelImpl *label = (struct LabelImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const Rect rect = wz_widget_get_absolute_rect(widget);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);

	if (label->multiline)
	{
		wz_renderer_print_box(widget->renderer, rect, widget->fontFace, widget->fontSize, label->textColor, label->text.c_str(), 0);
	}
	else
	{
		wz_renderer_print(widget->renderer, rect.x, (int)(rect.y + rect.h * 0.5f), NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, label->textColor, label->text.c_str(), 0);
	}

	nvgRestore(vg);
}

LabelImpl::LabelImpl()
{
	type = WZ_TYPE_LABEL;
	multiline = false;
	textColor = WZ_SKIN_LABEL_TEXT_COLOR;
	isTextColorUserSet = false;
}

Label::Label()
{
	impl = wz_label_create(NULL);
}

Label::Label(const std::string &text)
{
	impl = wz_label_create(text.c_str());
}

Label::~Label()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

Label *Label::setText(const char *format, ...)
{
	static char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	
	wz_label_set_text((LabelImpl *)impl, buffer);
	return this;
}

Label *Label::setTextColor(float r, float g, float b, float a)
{
	NVGcolor color;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	wz_label_set_text_color((LabelImpl *)impl, color);
	return this;
}

Label *Label::setMultiline(bool multiline)
{
	wz_label_set_multiline((LabelImpl *)impl, multiline);
	return this;
}

struct LabelImpl *wz_label_create(const char *text)
{
	struct LabelImpl *label = new struct LabelImpl;
	label->vtable.measure = wz_label_measure;
	label->vtable.draw = wz_label_draw;
	label->text = text ? std::string(text) : std::string();
	return label;
}

void wz_label_set_multiline(struct LabelImpl *label, bool multiline)
{
	WZ_ASSERT(label);
	label->multiline = multiline;
}

bool wz_label_get_multiline(const struct LabelImpl *label)
{
	WZ_ASSERT(label);
	return label->multiline;
}

void wz_label_set_text(struct LabelImpl *label, const char *text)
{
	WZ_ASSERT(label);
	label->text = std::string(text);
	wz_widget_resize_to_measured(label);
}

const char *wz_label_get_text(const struct LabelImpl *label)
{
	WZ_ASSERT(label);
	return label->text.c_str();
}

void wz_label_set_text_color(struct LabelImpl *label, NVGcolor color)
{
	WZ_ASSERT(label);
	label->textColor = color;
	label->isTextColorUserSet = true;
}

NVGcolor wz_label_get_text_color(const struct LabelImpl *label)
{
	return label->textColor;
}

} // namespace wz
