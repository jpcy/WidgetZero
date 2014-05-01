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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "stb_arr.h"
#include "wz_internal.h"

struct wzButton
{
	struct wzWidget base;
	bool toggleBehavior;
	bool isPressed;
	bool isSet;
	wzButtonPressedCallback *pressed_callbacks;
};

static void wz_button_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzButton *button;

	assert(widget);
	button = (struct wzButton *)widget;

	if (mouseButton == 1)
	{
		button->isPressed = true;
	}
}

static void wz_button_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzButton *button;
	int i;

	assert(widget);

	if (!widget->hover)
		return;

	button = (struct wzButton *)widget;

	if (mouseButton == 1)
	{
		button->isPressed = false;

		if (button->toggleBehavior)
		{
			button->isSet = !button->isSet;
		}

		for (i = 0; i < stb_arr_len(button->pressed_callbacks); i++)
		{
			button->pressed_callbacks[i](button);
		}
	}
}

static void wz_button_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzButton *button;

	assert(widget);
	button = (struct wzButton *)widget;

	if (!button->base.hover)
	{
		button->isPressed = false;
	}
}

static void wz_button_destroy(struct wzWidget *widget)
{
	struct wzButton *button;

	assert(widget);
	button = (struct wzButton *)widget;
	stb_arr_free(button->pressed_callbacks);
}

struct wzButton *wz_button_create(struct wzWindow *window)
{
	struct wzButton *button;

	assert(window);
	button = (struct wzButton *)malloc(sizeof(struct wzButton));
	memset(button, 0, sizeof(struct wzButton));
	button->base.type = WZ_TYPE_BUTTON;
	button->base.window = window;
	button->base.vtable.destroy = wz_button_destroy;
	button->base.vtable.mouse_button_down = wz_button_mouse_button_down;
	button->base.vtable.mouse_button_up = wz_button_mouse_button_up;
	button->base.vtable.mouse_move = wz_button_mouse_move;
	return button;
}

void wz_button_set_toggle_behavior(struct wzButton *button, bool enabled)
{
	assert(button);
	button->toggleBehavior = enabled;
}

bool wz_button_is_pressed(const struct wzButton *button)
{
	assert(button);
	return button->isPressed;
}

bool wz_button_is_set(const struct wzButton *button)
{
	assert(button);
	return button->isSet;
}

void wz_button_add_callback_pressed(struct wzButton *button, wzButtonPressedCallback callback)
{
	assert(button);
	stb_arr_push(button->pressed_callbacks, callback);
}
