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
#include "wz_pch.h"
#pragma hdrstop

namespace wz {

/*
================================================================================

MEASURING

================================================================================
*/

static Size wz_button_measure(struct WidgetImpl *widget)
{
	Size size;
	struct ButtonImpl *button = (struct ButtonImpl *)widget;

	wz_widget_measure_text(widget, button->label.c_str(), 0, &size.w, &size.h);

	if (!button->icon.empty())
	{
		int handle, w, h;

		handle = wz_renderer_create_image(widget->renderer, button->icon.c_str(), &w, &h);

		if (handle)
		{
			size.w += w + WZ_SKIN_BUTTON_ICON_SPACING;
			size.h = WZ_MAX(size.h, h);
		}
	}

	size.w += button->padding.left + button->padding.right;
	size.h += button->padding.top + button->padding.bottom;
	return size;
}

/*
================================================================================

DRAWING

================================================================================
*/

static void wz_button_draw(struct WidgetImpl *widget, Rect clip)
{
	NVGcolor bgColor1, bgColor2;
	Rect paddedRect;
	Size iconSize;
	int iconHandle, labelWidth, iconX, labelX;
	struct ButtonImpl *button = (struct ButtonImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const Rect rect = wz_widget_get_absolute_rect(widget);

	nvgSave(vg);

	if (!wz_renderer_clip_to_rect_intersection(vg, clip, rect))
		return;

	// Background color.
	if (button->isPressed && widget->hover)
	{
		bgColor1 = WZ_SKIN_BUTTON_BG_PRESSED_COLOR1;
		bgColor2 = WZ_SKIN_BUTTON_BG_PRESSED_COLOR2;
	}
	else if (button->isSet)
	{
		bgColor1 = WZ_SKIN_BUTTON_BG_SET_COLOR1;
		bgColor2 = WZ_SKIN_BUTTON_BG_SET_COLOR2;
	}
	else
	{
		bgColor1 = WZ_SKIN_BUTTON_BG_COLOR1;
		bgColor2 = WZ_SKIN_BUTTON_BG_COLOR2;
	}

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, WZ_SKIN_BUTTON_CORNER_RADIUS);

	// Background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, bgColor1, bgColor2));
	nvgFill(vg);

	// Border.
	nvgStrokeColor(vg, widget->hover ? WZ_SKIN_BUTTON_BORDER_HOVER_COLOR : WZ_SKIN_BUTTON_BORDER_COLOR);
	nvgStroke(vg);

	// Calculate padded rect.
	paddedRect.x = rect.x + button->padding.left;
	paddedRect.y = rect.y + button->padding.top;
	paddedRect.w = rect.w - (button->padding.left + button->padding.right);
	paddedRect.h = rect.h - (button->padding.top + button->padding.bottom);

	// Calculate icon and label sizes.
	iconSize.w = iconSize.h = 0;

	if (!button->icon.empty())
	{
		iconHandle = wz_renderer_create_image(widget->renderer, button->icon.c_str(), &iconSize.w, &iconSize.h);
	}

	wz_widget_measure_text(widget, button->label.c_str(), 0, &labelWidth, NULL);

	// Position the icon and label centered.
	if (!button->icon.empty() && iconHandle && !button->label.empty())
	{
		iconX = paddedRect.x + (int)(paddedRect.w / 2.0f - (iconSize.w + WZ_SKIN_BUTTON_ICON_SPACING + labelWidth) / 2.0f);
		labelX = iconX + iconSize.w + WZ_SKIN_BUTTON_ICON_SPACING;
	}
	else if (!button->icon.empty() && iconHandle)
	{
		iconX = paddedRect.x + (int)(paddedRect.w / 2.0f - iconSize.w / 2.0f);
	}
	else if (!button->label.empty())
	{
		labelX = paddedRect.x + (int)(paddedRect.w / 2.0f - labelWidth / 2.0f);
	}

	// Draw the icon.
	if (!button->icon.empty() && iconHandle)
	{
		Rect iconRect;
		iconRect.x = iconX;
		iconRect.y = paddedRect.y + (int)(paddedRect.h / 2.0f - iconSize.h / 2.0f);
		iconRect.w = iconSize.w;
		iconRect.h = iconSize.h;
		wz_renderer_draw_image(vg, iconRect, iconHandle);
	}

	// Draw the label.
	if (!button->label.empty())
	{
		wz_renderer_print(widget->renderer, labelX, paddedRect.y + paddedRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_BUTTON_TEXT_COLOR, button->label.c_str(), 0);
	}

	nvgRestore(vg);
}

/*
================================================================================

VTABLE FUNCTIONS

================================================================================
*/

static void wz_button_click(struct ButtonImpl *button)
{
	Event e;

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

static void wz_button_mouse_button_down(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ButtonImpl *button;

	WZ_ASSERT(widget);
	button = (struct ButtonImpl *)widget;

	if (mouseButton == 1)
	{
		Event e;

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

static void wz_button_mouse_button_up(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ButtonImpl *button;

	WZ_ASSERT(widget);
	button = (struct ButtonImpl *)widget;

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

/*
================================================================================

PRIVATE INTERFACE

================================================================================
*/

ButtonImpl::ButtonImpl(const std::string &label, const std::string &icon)
{
	type = WZ_TYPE_BUTTON;
	vtable.measure = wz_button_measure;
	vtable.draw = wz_button_draw;
	vtable.mouse_button_down = wz_button_mouse_button_down;
	vtable.mouse_button_up = wz_button_mouse_button_up;
	clickBehavior = WZ_BUTTON_CLICK_BEHAVIOR_UP;
	setBehavior = WZ_BUTTON_SET_BEHAVIOR_DEFAULT;
	isPressed = isSet = false;
	boundValue = NULL;
	padding.left = padding.right = 8;
	padding.top = padding.bottom = 4;
	this->label = label;
	this->icon = icon;
}

void wz_button_set_click_behavior(struct ButtonImpl *button, ButtonClickBehavior clickBehavior)
{
	WZ_ASSERT(button);
	button->clickBehavior = clickBehavior;
}

void wz_button_set_set_behavior(struct ButtonImpl *button, ButtonSetBehavior setBehavior)
{
	WZ_ASSERT(button);
	button->setBehavior = setBehavior;
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

Button::Button()
{
	impl = new ButtonImpl;
}

Button::Button(const std::string &label, const std::string &icon)
{
	impl = new ButtonImpl(label, icon);
}

Button::~Button()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

Border Button::getPadding() const
{
	return wz_button_get_padding((const ButtonImpl *)impl);
}

Button *Button::setPadding(Border padding)
{
	wz_button_set_padding((ButtonImpl *)impl, padding);
	return this;
}

Button *Button::setPadding(int top, int right, int bottom, int left)
{
	Border padding;
	padding.top = top;
	padding.right = right;
	padding.bottom = bottom;
	padding.left = left;
	wz_button_set_padding((ButtonImpl *)impl, padding);
	return this;
}

const char *Button::getIcon() const
{
	return wz_button_get_icon((const ButtonImpl *)impl);
}

Button *Button::setIcon(const std::string &icon)
{
	wz_button_set_icon((ButtonImpl *)impl, icon.c_str());
	return this;
}

const char *Button::getLabel() const
{
	return wz_button_get_label((const ButtonImpl *)impl);
}

Button *Button::setLabel(const std::string &label)
{
	wz_button_set_label((ButtonImpl *)impl, label.c_str());
	return this;
}

void wz_button_set_label(struct ButtonImpl *button, const char *label)
{
	WZ_ASSERT(button);
	button->label = label;
	wz_widget_resize_to_measured(button);
}

const char *wz_button_get_label(const struct ButtonImpl *button)
{
	WZ_ASSERT(button);
	return button->label.c_str();
}

void wz_button_set_icon(struct ButtonImpl *button, const char *icon)
{
	WZ_ASSERT(button);
	button->icon = std::string(icon);
	wz_widget_resize_to_measured(button);
}

const char *wz_button_get_icon(const struct ButtonImpl *button)
{
	WZ_ASSERT(button);
	return button->icon.c_str();
}

void wz_button_set_padding(struct ButtonImpl *button, Border padding)
{
	WZ_ASSERT(button);
	button->padding = padding;
	wz_widget_resize_to_measured(button);
}

void wz_button_set_padding_args(struct ButtonImpl *button, int top, int right, int bottom, int left)
{
	WZ_ASSERT(button);
	button->padding.top = top;
	button->padding.right = right;
	button->padding.bottom = bottom;
	button->padding.left = left;
	wz_widget_resize_to_measured(button);
}

Border wz_button_get_padding(const struct ButtonImpl *button)
{
	WZ_ASSERT(button);
	return button->padding;
}

bool wz_button_is_pressed(const struct ButtonImpl *button)
{
	WZ_ASSERT(button);
	return button->isPressed;
}

bool wz_button_is_set(const struct ButtonImpl *button)
{
	WZ_ASSERT(button);
	return button->isSet;
}

void wz_button_set(struct ButtonImpl *button, bool value)
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
		Event e;
		e.button.type = WZ_EVENT_BUTTON_CLICKED;
		e.button.button = button;
		e.button.isSet = button->isSet;
		wz_invoke_event(&e, button->clicked_callbacks);
	}
}

void wz_button_bind_value(struct ButtonImpl *button, bool *value)
{
	WZ_ASSERT(button);
	button->boundValue = value;

	if (value)
	{
		wz_button_set(button, *value);
	}
}

void wz_button_add_callback_pressed(struct ButtonImpl *button, EventCallback callback)
{
	WZ_ASSERT(button);
	button->pressed_callbacks.push_back(callback);
}

void wz_button_add_callback_clicked(struct ButtonImpl *button, EventCallback callback)
{
	WZ_ASSERT(button);
	button->clicked_callbacks.push_back(callback);
}

/*
================================================================================

TOGGLE BUTTON PUBLIC INTERFACE

================================================================================
*/

ToggleButton::ToggleButton()
{
	impl = new ButtonImpl;
	wz_button_set_set_behavior((ButtonImpl *)impl, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
}

ToggleButton::ToggleButton(const std::string &label, const std::string &icon)
{
	impl = new ButtonImpl(label.c_str(), icon.c_str());
	wz_button_set_set_behavior((ButtonImpl *)impl, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
}

ToggleButton::~ToggleButton()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

Border ToggleButton::getPadding() const
{
	return wz_button_get_padding((const ButtonImpl *)impl);
}

ToggleButton *ToggleButton::setPadding(Border padding)
{
	wz_button_set_padding((ButtonImpl *)impl, padding);
	return this;
}

ToggleButton *ToggleButton::setPadding(int top, int right, int bottom, int left)
{
	Border padding;
	padding.top = top;
	padding.right = right;
	padding.bottom = bottom;
	padding.left = left;
	wz_button_set_padding((ButtonImpl *)impl, padding);
	return this;
}

const char *ToggleButton::getIcon() const
{
	return wz_button_get_icon((const ButtonImpl *)impl);
}

ToggleButton *ToggleButton::setIcon(const std::string &icon)
{
	wz_button_set_icon((ButtonImpl *)impl, icon.c_str());
	return this;
}

const char *ToggleButton::getLabel() const
{
	return wz_button_get_label((const ButtonImpl *)impl);
}

ToggleButton *ToggleButton::setLabel(const std::string &label)
{
	wz_button_set_label((ButtonImpl *)impl, label.c_str());
	return this;
}

} // namespace wz
