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

LabelImpl::LabelImpl(const std::string &text) : text_(text)
{
	type = WZ_TYPE_LABEL;
	multiline_ = false;
	textColor_ = WZ_SKIN_LABEL_TEXT_COLOR;
	isTextColorUserSet_ = false;
}

void LabelImpl::draw(Rect clip)
{
	renderer->drawLabel(this, clip);
}

Size LabelImpl::measure()
{
	return renderer->measureLabel(this);
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
	impl.reset(new LabelImpl);
}

Label::Label(const std::string &text)
{
	impl.reset(new LabelImpl(text));
}

Label::~Label()
{
}

Label *Label::setText(const char *format, ...)
{
	static char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	
	((LabelImpl *)impl.get())->setText(buffer);
	return this;
}

Label *Label::setTextColor(float r, float g, float b, float a)
{
	NVGcolor color;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	((LabelImpl *)impl.get())->setTextColor(color);
	return this;
}

Label *Label::setMultiline(bool multiline)
{
	((LabelImpl *)impl.get())->setMultiline(multiline);
	return this;
}

} // namespace wz
