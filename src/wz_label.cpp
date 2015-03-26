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
#include "wz.h"
#pragma hdrstop

namespace wz {

Label::Label(const std::string &text) : text_(text), textColor_(1, 1, 1)
{
	type_ = WidgetType::Label;
	multiline_ = false;
	isTextColorUserSet_ = false;
}

void Label::setMultiline(bool multiline)
{
	multiline_ = multiline;
}

bool Label::getMultiline() const
{
	return multiline_;
}

void Label::setText(const char *text)
{
	text_ = text;
	setMeasureDirty();
}

void Label::setTextf(const char *format, ...)
{
	static char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	setText(buffer);
}

const char *Label::getText() const
{
	return text_.c_str();
}

void Label::setTextColor(Color color)
{
	textColor_ = color;
	isTextColorUserSet_ = true;
}

void Label::setTextColor(float r, float g, float b)
{
	textColor_ = Color(r, g, b);
	isTextColorUserSet_ = true;
}

Color Label::getTextColor() const
{
	return textColor_;
}

void Label::onRendererChanged()
{
	if (!isTextColorUserSet_)
	{
		textColor_ = renderer_->getLabelTextColor(this);
	}
}

void Label::draw(Rect clip)
{
	renderer_->drawLabel(this, clip);
}

Size Label::measure()
{
	return renderer_->measureLabel(this);
}

} // namespace wz
