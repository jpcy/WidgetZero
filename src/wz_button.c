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
#include <stdlib.h>
#include <string.h>
#include "wz_main_window.h"
#include "wz_widget.h"

struct wzButton
{
	struct wzWidget base;
	wzButtonClickBehavior clickBehavior;
	wzButtonSetBehavior setBehavior;
	bool isPressed;
	bool isSet;
	bool *boundValue;
	wzEventCallback *pressed_callbacks;
	wzEventCallback *clicked_callbacks;
};

static void wz_button_click(struct wzButton *button)
{
	wzEvent e;

	if (button->setBehavior == WZ_BUTTON_SET_BEHAVIOR_TOGGLE)
	{
		button->isSet = !button->isSet;
	}
	else if (button->setBehavior == WZ_BUTTON_SET_BEHAVIOR_STICKY)
	{
		// Don't invoke the clicked event if already set.
		if (button->isSet)
			return;

		button->isSet = true;
	}

	// isSet assigned to, update the bound value.
	if (button->boundValue)
	{
		*button->boundValue = button->isSet;
	}

	e.button.type = WZ_EVENT_BUTTON_CLICKED;
	e.button.button = button;
	e.button.isSet = button->isSet;
	wz_invoke_event(&e, button->clicked_callbacks);
}

static void wz_button_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzButton *button;

	WZ_ASSERT(widget);
	button = (struct wzButton *)widget;

	if (mouseButton == 1)
	{
		wzEvent e;

		button->isPressed = true;
		wz_main_window_push_lock_input_widget(widget->mainWindow, widget);

		e.button.type = WZ_EVENT_BUTTON_PRESSED;
		e.button.button = button;
		e.button.isSet = button->isSet;
		wz_invoke_event(&e, button->pressed_callbacks);

		if (button->clickBehavior == WZ_BUTTON_CLICK_BEHAVIOR_DOWN)
		{
			wz_button_click(button);
		}
	}
}

static void wz_button_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzButton *button;

	WZ_ASSERT(widget);
	button = (struct wzButton *)widget;

	if (mouseButton == 1 && button->isPressed)
	{
		button->isPressed = false;
		wz_main_window_pop_lock_input_widget(widget->mainWindow, widget);

		if (widget->hover && button->clickBehavior == WZ_BUTTON_CLICK_BEHAVIOR_UP)
		{
			wz_button_click(button);
		}
	}
}

static void wz_button_destroy(struct wzWidget *widget)
{
	struct wzButton *button;

	WZ_ASSERT(widget);
	button = (struct wzButton *)widget;
	wz_arr_free(button->pressed_callbacks);
	wz_arr_free(button->clicked_callbacks);
}

struct wzButton *wz_button_create()
{
	struct wzButton *button;

	button = (struct wzButton *)malloc(sizeof(struct wzButton));
	memset(button, 0, sizeof(struct wzButton));
	button->base.type = WZ_TYPE_BUTTON;
	button->base.vtable.destroy = wz_button_destroy;
	button->base.vtable.mouse_button_down = wz_button_mouse_button_down;
	button->base.vtable.mouse_button_up = wz_button_mouse_button_up;
	return button;
}

void wz_button_set_click_behavior(struct wzButton *button, wzButtonClickBehavior clickBehavior)
{
	WZ_ASSERT(button);
	button->clickBehavior = clickBehavior;
}

void wz_button_set_set_behavior(struct wzButton *button, wzButtonSetBehavior setBehavior)
{
	WZ_ASSERT(button);
	button->setBehavior = setBehavior;
}

bool wz_button_is_pressed(const struct wzButton *button)
{
	WZ_ASSERT(button);
	return button->isPressed;
}

bool wz_button_is_set(const struct wzButton *button)
{
	WZ_ASSERT(button);
	return button->isSet;
}

void wz_button_set(struct wzButton *button, bool value)
{
	WZ_ASSERT(button);

	// No such thing as setting a button if using the default behavior.
	if (button->setBehavior == WZ_BUTTON_SET_BEHAVIOR_DEFAULT)
		return;

	if (value && button->isSet)
	{
		// Already set, don't invoke click event.
		return;
	}

	button->isSet = value;

	// isSet assigned to, update the bound value.
	if (button->boundValue)
	{
		*button->boundValue = button->isSet;
	}

	if (button->isSet)
	{
		wzEvent e;
		e.button.type = WZ_EVENT_BUTTON_CLICKED;
		e.button.button = button;
		e.button.isSet = button->isSet;
		wz_invoke_event(&e, button->clicked_callbacks);
	}
}

void wz_button_bind_value(struct wzButton *button, bool *value)
{
	assert(button);
	button->boundValue = value;

	if (value)
	{
		wz_button_set(button, *value);
	}
}

void wz_button_add_callback_pressed(struct wzButton *button, wzEventCallback callback)
{
	WZ_ASSERT(button);
	wz_arr_push(button->pressed_callbacks, callback);
}

void wz_button_add_callback_clicked(struct wzButton *button, wzEventCallback callback)
{
	WZ_ASSERT(button);
	wz_arr_push(button->clicked_callbacks, callback);
}
