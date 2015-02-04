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

LabelImpl::LabelImpl(const std::string &text) : text_(text)
{
	type = WZ_TYPE_LABEL;
	multiline_ = false;
	textColor_ = WZ_SKIN_LABEL_TEXT_COLOR;
	isTextColorUserSet_ = false;
}

void LabelImpl::draw(Rect clip)
{
	NVGRenderer *r = (NVGRenderer *)renderer;
	struct NVGcontext *vg = r->getContext();
	const Rect rect = getAbsoluteRect();

	nvgSave(vg);
	r->clipToRect(clip);

	if (multiline_)
	{
		r->printBox(rect, fontFace, fontSize, textColor_, text_.c_str(), 0);
	}
	else
	{
		r->print(rect.x, (int)(rect.y + rect.h * 0.5f), NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, textColor_, text_.c_str(), 0);
	}

	nvgRestore(vg);
}

Size LabelImpl::measure()
{
	Size size;
	NVGRenderer *r = (NVGRenderer *)renderer;
	struct NVGcontext *vg = r->getContext();

	if (multiline_)
	{
		nvgFontSize(vg, fontSize == 0 ? r->getDefaultFontSize() : fontSize);
		r->setFontFace(fontFace);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgTextLineHeight(vg, 1.2f);
		float bounds[4];
		nvgTextBoxBounds(vg, 0, 0, (float)rect.w, text_.c_str(), NULL, bounds);
		size.w = (int)bounds[2];
		size.h = (int)bounds[3];
	}
	else
	{
		wz_widget_measure_text(this, text_.c_str(), 0, &size.w, &size.h);
	}

	return size;
}

void LabelImpl::setMultiline(bool multiline)
{
	multiline_ = multiline;
}

bool LabelImpl::getMultiline() const
{
	return multiline_;
}

void LabelImpl::setText(const char *text)
{
	text_ = text;
	resizeToMeasured();
}

const char *LabelImpl::getText() const
{
	return text_.c_str();
}

void LabelImpl::setTextColor(NVGcolor color)
{
	textColor_ = color;
	isTextColorUserSet_ = true;
}

NVGcolor LabelImpl::getTextColor() const
{
	return textColor_;
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

Label::Label()
{
	impl = new LabelImpl;
}

Label::Label(const std::string &text)
{
	impl = new LabelImpl(text);
}

Label::~Label()
{
	if (!impl->getMainWindow())
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
	
	((LabelImpl *)impl)->setText(buffer);
	return this;
}

Label *Label::setTextColor(float r, float g, float b, float a)
{
	NVGcolor color;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	((LabelImpl *)impl)->setTextColor(color);
	return this;
}

Label *Label::setMultiline(bool multiline)
{
	((LabelImpl *)impl)->setMultiline(multiline);
	return this;
}

} // namespace wz
