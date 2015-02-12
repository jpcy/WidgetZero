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

ButtonImpl::ButtonImpl(const std::string &label, const std::string &icon)
{
	type = WZ_TYPE_BUTTON;
	clickBehavior = WZ_BUTTON_CLICK_BEHAVIOR_UP;
	setBehavior = WZ_BUTTON_SET_BEHAVIOR_DEFAULT;
	isPressed_ = isSet_ = false;
	boundValue = NULL;
	padding.left = padding.right = 8;
	padding.top = padding.bottom = 4;
	this->label = label;
	this->icon = icon;
}

void ButtonImpl::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		isPressed_ = true;
		mainWindow->pushLockInputWidget(this);

		Event e;
		e.button.type = WZ_EVENT_BUTTON_PRESSED;
		e.button.button = this;
		e.button.isSet = isSet_;
		wz_invoke_event(&e, pressed_callbacks);

		if (clickBehavior == WZ_BUTTON_CLICK_BEHAVIOR_DOWN)
		{
			click();
		}
	}
}

void ButtonImpl::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1 && isPressed_)
	{
		isPressed_ = false;
		mainWindow->popLockInputWidget(this);

		if (hover && clickBehavior == WZ_BUTTON_CLICK_BEHAVIOR_UP)
		{
			click();
		}
	}
}

void ButtonImpl::draw(Rect clip)
{
	renderer->drawButton(this, clip);
}

Size ButtonImpl::measure()
{
	return renderer->measureButton(this);
}

void ButtonImpl::setLabel(const char *label)
{
	this->label = label;
	resizeToMeasured();
}

const char *ButtonImpl::getLabel() const
{
	return label.c_str();
}

void ButtonImpl::setIcon(const char *icon)
{
	this->icon = icon;
	resizeToMeasured();
}

const char *ButtonImpl::getIcon() const
{
	return icon.c_str();
}

void ButtonImpl::setPadding(Border padding)
{
	this->padding = padding;
	resizeToMeasured();
}

void ButtonImpl::setPadding(int top, int right, int bottom, int left)
{
	padding.top = top;
	padding.right = right;
	padding.bottom = bottom;
	padding.left = left;
	resizeToMeasured();
}

Border ButtonImpl::getPadding() const
{
	return padding;
}

bool ButtonImpl::isPressed() const
{
	return isPressed_;
}

bool ButtonImpl::isSet() const
{
	return isSet_;
}

void ButtonImpl::set(bool value)
{
	// No such thing as setting a button if using the default behavior.
	if (setBehavior == WZ_BUTTON_SET_BEHAVIOR_DEFAULT)
		return;

	if (value && isSet_)
	{
		// Already set, don't invoke click event.
		return;
	}

	isSet_ = value;

	// isSet assigned to, update the bound value.
	if (boundValue)
	{
		*boundValue = isSet_;
	}

	if (isSet_)
	{
		Event e;
		e.button.type = WZ_EVENT_BUTTON_CLICKED;
		e.button.button = this;
		e.button.isSet = isSet_;
		wz_invoke_event(&e, clicked_callbacks);
	}
}

void ButtonImpl::bindValue(bool *value)
{
	boundValue = value;

	if (value)
	{
		set(*value);
	}
}

void ButtonImpl::addCallbackPressed(EventCallback callback)
{
	pressed_callbacks.push_back(callback);
}

void ButtonImpl::addCallbackClicked(EventCallback callback)
{
	clicked_callbacks.push_back(callback);
}

void ButtonImpl::setClickBehavior(ButtonClickBehavior clickBehavior)
{
	this->clickBehavior = clickBehavior;
}

void ButtonImpl::setSetBehavior(ButtonSetBehavior setBehavior)
{
	this->setBehavior = setBehavior;
}

void ButtonImpl::click()
{
	if (setBehavior == WZ_BUTTON_SET_BEHAVIOR_TOGGLE)
	{
		isSet_ = !isSet_;
	}
	else if (setBehavior == WZ_BUTTON_SET_BEHAVIOR_STICKY)
	{
		// Don't invoke the clicked event if already set.
		if (isSet_)
			return;

		isSet_ = true;
	}

	// isSet assigned to, update the bound value.
	if (boundValue)
	{
		*boundValue = isSet_;
	}

	Event e;
	e.button.type = WZ_EVENT_BUTTON_CLICKED;
	e.button.button = this;
	e.button.isSet = isSet_;
	wz_invoke_event(&e, clicked_callbacks);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

Button::Button()
{
	impl.reset(new ButtonImpl);
}

Button::Button(const std::string &label, const std::string &icon)
{
	impl.reset(new ButtonImpl(label, icon));
}

Button::~Button()
{
}

Border Button::getPadding() const
{
	return getImpl()->getPadding();
}

Button *Button::setPadding(Border padding)
{
	getImpl()->setPadding(padding);
	return this;
}

Button *Button::setPadding(int top, int right, int bottom, int left)
{
	getImpl()->setPadding(top, right, bottom, left);
	return this;
}

const char *Button::getIcon() const
{
	return getImpl()->getIcon();
}

Button *Button::setIcon(const std::string &icon)
{
	getImpl()->setIcon(icon.c_str());
	return this;
}

const char *Button::getLabel() const
{
	return getImpl()->getLabel();
}

Button *Button::setLabel(const std::string &label)
{
	getImpl()->setLabel(label.c_str());
	return this;
}

ButtonImpl *Button::getImpl()
{
	return (ButtonImpl *)impl.get();
}

const ButtonImpl *Button::getImpl() const
{
	return (const ButtonImpl *)impl.get();
}

/*
================================================================================

TOGGLE BUTTON PUBLIC INTERFACE

================================================================================
*/

ToggleButton::ToggleButton()
{
	impl.reset(new ButtonImpl);
	getImpl()->setSetBehavior(WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
}

ToggleButton::ToggleButton(const std::string &label, const std::string &icon)
{
	impl.reset(new ButtonImpl(label.c_str(), icon.c_str()));
	getImpl()->setSetBehavior(WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
}

ToggleButton::~ToggleButton()
{
}

Border ToggleButton::getPadding() const
{
	return getImpl()->getPadding();
}

ToggleButton *ToggleButton::setPadding(Border padding)
{
	getImpl()->setPadding(padding);
	return this;
}

ToggleButton *ToggleButton::setPadding(int top, int right, int bottom, int left)
{
	getImpl()->setPadding(top, right, bottom, left);
	return this;
}

const char *ToggleButton::getIcon() const
{
	return getImpl()->getIcon();
}

ToggleButton *ToggleButton::setIcon(const std::string &icon)
{
	getImpl()->setIcon(icon.c_str());
	return this;
}

const char *ToggleButton::getLabel() const
{
	return getImpl()->getLabel();
}

ToggleButton *ToggleButton::setLabel(const std::string &label)
{
	getImpl()->setLabel(label.c_str());
	return this;
}

ButtonImpl *ToggleButton::getImpl()
{
	return (ButtonImpl *)impl.get();
}

const ButtonImpl *ToggleButton::getImpl() const
{
	return (const ButtonImpl *)impl.get();
}

} // namespace wz
