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

RadioButton::RadioButton(const std::string &label) : Button(label)
{
	type_ = WidgetType::RadioButton;
	setSetBehavior(ButtonSetBehavior::Sticky);
	addEventHandler(EventType::ButtonClicked, this, &RadioButton::onClicked);
}

void RadioButton::onParented(Widget *parent)
{
	// If this is the only radio button child, set it.
	// This means that the first radio button will be selected.
	for (size_t i = 0; i < parent->getChildren().size(); i++)
	{
		if (parent->getChildren()[i]->getType() == WidgetType::RadioButton && parent->getChildren()[i] != this)
			return;
	}

	set(true);
}

void RadioButton::draw(Rect clip)
{
	renderer_->drawRadioButton(this, clip);
}

Size RadioButton::measure()
{
	return renderer_->measureRadioButton(this);
}

void RadioButton::onClicked(Event)
{
	// Unset all the other radio button siblings.
	for (size_t i = 0; i < parent_->getChildren().size(); i++)
	{
		Widget *sibling = parent_->getChildren()[i];

		if (sibling->getType() == WidgetType::RadioButton && sibling != this)
		{
			((RadioButton *)sibling)->set(false);
		}
	}
}

} // namespace wz
