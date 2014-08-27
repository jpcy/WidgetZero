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

typedef enum
{
	WZ_BUTTON_STYLE_DEFAULT,
	WZ_BUTTON_STYLE_CHECK,
	WZ_BUTTON_STYLE_RADIO,
	WZ_BUTTON_STYLE_TAB,
}
wzButtonStyle;

struct wzButton
{
	struct wzWidget base;
	wzButtonStyle style;
	wzButtonClickBehavior clickBehavior;
	wzButtonSetBehavior setBehavior;
	wzBorder padding;
	char label[128];
	char icon[128];
	bool isPressed;
	bool isSet;
	bool *boundValue;
	wzEventCallback *pressed_callbacks;
	wzEventCallback *clicked_callbacks;
};

static wzSize wz_button_measure(struct wzWidget *widget)
{
	struct wzButton *button = (struct wzButton *)widget;
	WZ_ASSERT(button);

	if (button->style == WZ_BUTTON_STYLE_CHECK)
	{
		return widget->renderer->measure_checkbox(widget->renderer, button);
	}
	else if (button->style == WZ_BUTTON_STYLE_RADIO)
	{
		return widget->renderer->measure_radio_button(widget->renderer, button);
	}

	return widget->renderer->measure_button(widget->renderer, button);
}

static void wz_button_draw(struct wzWidget *widget, wzRect clip)
{
	struct wzButton *button = (struct wzButton *)widget;
	WZ_ASSERT(button);

	if (button->style == WZ_BUTTON_STYLE_CHECK)
	{
		widget->renderer->draw_checkbox(widget->renderer, clip, button);
	}
	else if (button->style == WZ_BUTTON_STYLE_RADIO)
	{
		widget->renderer->draw_radio_button(widget->renderer, clip, button);
	}
	else if (button->style == WZ_BUTTON_STYLE_TAB)
	{
		widget->renderer->draw_tab_button(widget->renderer, clip, button);
	}
	else
	{
		widget->renderer->draw_button(widget->renderer, clip, button);
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

static struct wzButton *wz_button_create_internal(struct wzRenderer *renderer, wzButtonStyle style)
{
	struct wzButton *button = (struct wzButton *)malloc(sizeof(struct wzButton));
	memset(button, 0, sizeof(struct wzButton));
	button->base.type = WZ_TYPE_BUTTON;
	button->base.renderer = renderer;
	button->base.vtable.measure = wz_button_measure;
	button->base.vtable.draw = wz_button_draw;
	button->base.vtable.destroy = wz_button_destroy;
	button->base.vtable.mouse_button_down = wz_button_mouse_button_down;
	button->base.vtable.mouse_button_up = wz_button_mouse_button_up;
	button->style = style;
	button->padding = renderer->get_button_padding(renderer, button);
	return button;
}

struct wzButton *wz_button_create(struct wzRenderer *renderer)
{
	return wz_button_create_internal(renderer, WZ_BUTTON_STYLE_DEFAULT);
}

struct wzButton *wz_check_box_create(struct wzRenderer *renderer)
{
	struct wzButton *button = wz_button_create_internal(renderer, WZ_BUTTON_STYLE_CHECK);
	wz_button_set_set_behavior(button, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
	return button;
}

struct wzButton *wz_radio_button_create(struct wzRenderer *renderer)
{
	return wz_button_create_internal(renderer, WZ_BUTTON_STYLE_RADIO);
}

struct wzButton *wz_tab_button_create(struct wzRenderer *renderer)
{
	return wz_button_create_internal(renderer, WZ_BUTTON_STYLE_TAB);
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

void wz_button_set_label(struct wzButton *button, const char *label)
{
	WZ_ASSERT(button);
	strcpy(button->label, label);
	wz_widget_resize_to_measured(&button->base);
}

const char *wz_button_get_label(const struct wzButton *button)
{
	WZ_ASSERT(button);
	return button->label;
}

void wz_button_set_icon(struct wzButton *button, const char *icon)
{
	WZ_ASSERT(button);
	strcpy(button->icon, icon);
	wz_widget_resize_to_measured(&button->base);
}

const char *wz_button_get_icon(const struct wzButton *button)
{
	WZ_ASSERT(button);
	return button->icon;
}

void wz_button_set_padding(struct wzButton *button, wzBorder padding)
{
	WZ_ASSERT(button);
	button->padding = padding;
	wz_widget_resize_to_measured(&button->base);
}

wzBorder wz_button_get_padding(const struct wzButton *button)
{
	WZ_ASSERT(button);
	return button->padding;
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
