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

RadioButtonImpl::RadioButtonImpl(const std::string &label) : ButtonImpl(label)
{
	type = WZ_TYPE_RADIO_BUTTON;
	setSetBehavior(WZ_BUTTON_SET_BEHAVIOR_STICKY);
	addCallbackClicked(wz_radio_button_clicked);
}

void RadioButtonImpl::onParented(struct WidgetImpl *parent)
{
	// If this is the only radio button child, set it.
	// This means that the first radio button will be selected.
	for (size_t i = 0; i < parent->children.size(); i++)
	{
		if (parent->children[i]->type == WZ_TYPE_RADIO_BUTTON && parent->children[i] != this)
			return;
	}

	set(true);
}

void RadioButtonImpl::draw(Rect clip)
{
	renderer->drawRadioButton(this, clip);
}

Size RadioButtonImpl::measure()
{
	return renderer->measureRadioButton(this);
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

RadioButtonImpl *RadioButton::getImpl()
{
	return (RadioButtonImpl *)impl.get();
}

const RadioButtonImpl *RadioButton::getImpl() const
{
	return (const RadioButtonImpl *)impl.get();
}

} // namespace wz
