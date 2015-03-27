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

CheckBox::CheckBox(const std::string &label) : Button(label)
{
	type_ = WidgetType::CheckBox;
	setSetBehavior(ButtonSetBehavior::Toggle);
}

bool CheckBox::isChecked() const
{
	return isSet();
}

void CheckBox::check(bool value)
{
	set(value);
}

void CheckBox::addCallbackChecked(EventCallback callback)
{
	addCallbackClicked(callback);
}

void CheckBox::draw(Rect clip)
{
	renderer_->drawCheckBox(this, clip);
}

Size CheckBox::measure()
{
	return renderer_->measureCheckBox(this);
}

} // namespace wz
