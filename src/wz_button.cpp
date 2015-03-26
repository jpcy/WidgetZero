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

Button::Button(const std::string &label, const std::string &icon)
{
	type_ = WidgetType::Button;
	clickBehavior_ = ButtonClickBehavior::Up;
	setBehavior_ = ButtonSetBehavior::Default;
	isPressed_ = isSet_ = false;
	boundValue_ = NULL;
	padding_.left = padding_.right = 8;
	padding_.top = padding_.bottom = 4;
	label_ = label;
	icon_ = icon;
}

void Button::setLabel(const char *label)
{
	label_ = label;
	setMeasureDirty();
}

const char *Button::getLabel() const
{
	return label_.c_str();
}

void Button::setIcon(const char *icon)
{
	icon_ = icon;
	setMeasureDirty();
}

const char *Button::getIcon() const
{
	return icon_.c_str();
}

bool Button::isPressed() const
{
	return isPressed_;
}

bool Button::isSet() const
{
	return isSet_;
}

void Button::set(bool value)
{
	// No such thing as setting a button if using the default behavior.
	if (setBehavior_ == ButtonSetBehavior::Default)
		return;

	if (value && isSet_)
	{
		// Already set, don't invoke click event.
		return;
	}

	isSet_ = value;

	// isSet assigned to, update the bound value.
	if (boundValue_)
	{
		*boundValue_ = isSet_;
	}

	if (isSet_)
	{
		Event e;
		e.button.type = EventType::ButtonClicked;
		e.button.button = this;
		e.button.isSet = isSet_;
		invokeEvent(e, clickedCallbacks_);
	}
}

void Button::bindValue(bool *value)
{
	boundValue_ = value;

	if (value)
	{
		set(*value);
	}
}

void Button::addCallbackPressed(EventCallback callback)
{
	pressedCallbacks_.push_back(callback);
}

void Button::addCallbackClicked(EventCallback callback)
{
	clickedCallbacks_.push_back(callback);
}

void Button::onMouseButtonDown(int mouseButton, int /*mouseX*/, int /*mouseY*/)
{
	if (mouseButton == 1)
	{
		isPressed_ = true;
		mainWindow_->pushLockInputWidget(this);

		Event e;
		e.button.type = EventType::ButtonPressed;
		e.button.button = this;
		e.button.isSet = isSet_;
		invokeEvent(e, pressedCallbacks_);

		if (clickBehavior_ == ButtonClickBehavior::Down)
		{
			click();
		}
	}
}

void Button::onMouseButtonUp(int mouseButton, int /*mouseX*/, int /*mouseY*/)
{
	if (mouseButton == 1 && isPressed_)
	{
		isPressed_ = false;
		mainWindow_->popLockInputWidget(this);

		if (hover_ && clickBehavior_ == ButtonClickBehavior::Up)
		{
			click();
		}
	}
}

void Button::draw(Rect clip)
{
	renderer_->drawButton(this, clip);
}

Size Button::measure()
{
	return renderer_->measureButton(this);
}

void Button::setClickBehavior(ButtonClickBehavior::Enum clickBehavior)
{
	clickBehavior_ = clickBehavior;
}

void Button::setSetBehavior(ButtonSetBehavior::Enum setBehavior)
{
	setBehavior_ = setBehavior;
}

void Button::click()
{
	if (setBehavior_ == ButtonSetBehavior::Toggle)
	{
		isSet_ = !isSet_;
	}
	else if (setBehavior_ == ButtonSetBehavior::Sticky)
	{
		// Don't invoke the clicked event if already set.
		if (isSet_)
			return;

		isSet_ = true;
	}

	// isSet assigned to, update the bound value.
	if (boundValue_)
	{
		*boundValue_ = isSet_;
	}

	Event e;
	e.button.type = EventType::ButtonClicked;
	e.button.button = this;
	e.button.isSet = isSet_;
	invokeEvent(e, clickedCallbacks_);
}

/*
================================================================================

TOGGLE BUTTON

================================================================================
*/

ToggleButton::ToggleButton(const std::string &label, const std::string &icon) : Button(label, icon)
{
	setSetBehavior(ButtonSetBehavior::Toggle);
}

} // namespace wz
