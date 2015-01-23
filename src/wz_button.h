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
#pragma once

#include <string>
#include <vector>
#include <wz.h>

namespace wz {

typedef enum
{
	// Click the button on mouse up (default).
	WZ_BUTTON_CLICK_BEHAVIOR_UP,

	// Click the button on mouse down
	WZ_BUTTON_CLICK_BEHAVIOR_DOWN
}
ButtonClickBehavior;

typedef enum
{
	// Button is never set.
	WZ_BUTTON_SET_BEHAVIOR_DEFAULT,

	// Click to toggle whether the button is set.
	WZ_BUTTON_SET_BEHAVIOR_TOGGLE,

	// Click to set the button. Clicking again does nothing.
	WZ_BUTTON_SET_BEHAVIOR_STICKY
}
ButtonSetBehavior;

struct ButtonImpl : public WidgetImpl
{
	ButtonImpl();

	ButtonClickBehavior clickBehavior;
	ButtonSetBehavior setBehavior;
	Border padding;
	std::string label;
	std::string icon;
	bool isPressed;
	bool isSet;
	bool *boundValue;
	std::vector<EventCallback> pressed_callbacks;
	std::vector<EventCallback> clicked_callbacks;
};

void wz_button_set_click_behavior(struct ButtonImpl *button, ButtonClickBehavior clickBehavior);
void wz_button_set_set_behavior(struct ButtonImpl *button, ButtonSetBehavior clickBehavior);

} // namespace wz
