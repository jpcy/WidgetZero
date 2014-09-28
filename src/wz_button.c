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
#include "wz_renderer.h"
#include "wz_string.h"

struct wzButton
{
	struct wzWidget base;
	wzButtonClickBehavior clickBehavior;
	wzButtonSetBehavior setBehavior;
	wzBorder padding;
	wzString label;
	wzString icon;
	bool isPressed;
	bool isSet;
	bool *boundValue;
	wzEventCallback *pressed_callbacks;
	wzEventCallback *clicked_callbacks;
};

/*
================================================================================

MEASURING

================================================================================
*/

static wzSize wz_button_measure(struct wzWidget *widget)
{
	wzSize size;
	struct wzButton *button = (struct wzButton *)widget;

	wz_widget_measure_text(widget, button->label, 0, &size.w, &size.h);

	if (button->icon[0])
	{
		int handle, w, h;

		handle = wz_renderer_create_image(widget->renderer, button->icon, &w, &h);

		if (handle)
		{
			size.w += w + widget->style.button.iconSpacing;
			size.h = WZ_MAX(size.h, h);
		}
	}

	size.w += button->padding.left + button->padding.right;
	size.h += button->padding.top + button->padding.bottom;
	return size;
}

static wzSize wz_check_box_measure(struct wzWidget *widget)
{
	wzSize size;
	struct wzButton *button = (struct wzButton *)widget;
	wz_widget_measure_text(widget, button->label, 0, &size.w, &size.h);
	size.w += widget->style.checkBox.boxSize + widget->style.checkBox.boxRightMargin;
	return size;
}

/*
================================================================================

DRAWING

================================================================================
*/

static void wz_button_draw(struct wzWidget *widget, wzRect clip)
{
	NVGcolor bgColor1, bgColor2;
	wzRect paddedRect;
	wzSize iconSize;
	int iconHandle, labelWidth, iconX, labelX;
	struct wzButton *button = (struct wzButton *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzButtonStyle *style = &widget->style.button;
	const wzRect rect = wz_widget_get_absolute_rect(widget);

	nvgSave(vg);

	if (!wz_renderer_clip_to_rect_intersection(vg, clip, rect))
		return;

	// Background color.
	if (button->isPressed && widget->hover)
	{
		bgColor1 = style->bgPressedColor1;
		bgColor2 = style->bgPressedColor2;
	}
	else if (button->isSet)
	{
		bgColor1 = style->bgSetColor1;
		bgColor2 = style->bgSetColor2;
	}
	else
	{
		bgColor1 = style->bgColor1;
		bgColor2 = style->bgColor2;
	}

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, style->cornerRadius);

	// Background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, bgColor1, bgColor2));
	nvgFill(vg);

	// Border.
	nvgStrokeColor(vg, widget->hover ? style->borderHoverColor : style->borderColor);
	nvgStroke(vg);

	// Calculate padded rect.
	paddedRect.x = rect.x + button->padding.left;
	paddedRect.y = rect.y + button->padding.top;
	paddedRect.w = rect.w - (button->padding.left + button->padding.right);
	paddedRect.h = rect.h - (button->padding.top + button->padding.bottom);

	// Calculate icon and label sizes.
	iconSize.w = iconSize.h = 0;

	if (button->icon[0])
	{
		iconHandle = wz_renderer_create_image(widget->renderer, button->icon, &iconSize.w, &iconSize.h);
	}

	wz_widget_measure_text(widget, button->label, 0, &labelWidth, NULL);

	// Position the icon and label centered.
	if (button->icon[0] && iconHandle && button->label[0])
	{
		iconX = paddedRect.x + (int)(paddedRect.w / 2.0f - (iconSize.w + style->iconSpacing + labelWidth) / 2.0f);
		labelX = iconX + iconSize.w + style->iconSpacing;
	}
	else if (button->icon[0] && iconHandle)
	{
		iconX = paddedRect.x + (int)(paddedRect.w / 2.0f - iconSize.w / 2.0f);
	}
	else if (button->label[0])
	{
		labelX = paddedRect.x + (int)(paddedRect.w / 2.0f - labelWidth / 2.0f);
	}

	// Draw the icon.
	if (button->icon[0] && iconHandle)
	{
		wzRect iconRect;
		iconRect.x = iconX;
		iconRect.y = paddedRect.y + (int)(paddedRect.h / 2.0f - iconSize.h / 2.0f);
		iconRect.w = iconSize.w;
		iconRect.h = iconSize.h;
		wz_renderer_draw_image(vg, iconRect, iconHandle);
	}

	// Draw the label.
	if (button->label[0])
	{
		wz_renderer_print(widget->renderer, labelX, paddedRect.y + paddedRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, style->textColor, button->label, 0);
	}

	nvgRestore(vg);
}

static void wz_check_box_draw(struct wzWidget *widget, wzRect clip)
{
	wzRect boxRect;
	struct wzButton *button = (struct wzButton *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzCheckBoxStyle *style = &widget->style.checkBox;
	const wzRect rect = wz_widget_get_absolute_rect(widget);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);

	// Calculate box rect.
	boxRect.x = rect.x;
	boxRect.y = (int)(rect.y + rect.h / 2.0f - style->boxSize / 2.0f);
	boxRect.w = boxRect.h = style->boxSize;

	// Box border.
	wz_renderer_draw_rect(vg, boxRect, widget->hover ? style->borderHoverColor : style->borderColor);

	// Box checkmark.
	if (button->isSet)
	{
		const float left = (float)boxRect.x + style->boxInternalMargin;
		const float right = (float)boxRect.x + boxRect.w - style->boxInternalMargin;
		const float top = (float)boxRect.y + style->boxInternalMargin;
		const float bottom = (float)boxRect.y + boxRect.h - style->boxInternalMargin;

		nvgBeginPath(vg);
		nvgMoveTo(vg, left, top);
		nvgLineTo(vg, right, bottom);
		nvgStrokeColor(vg, style->checkColor);
		nvgStrokeWidth(vg, style->checkThickness);
		nvgStroke(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, left, bottom);
		nvgLineTo(vg, right, top);
		nvgStroke(vg);
	}

	// Label.
	wz_renderer_print(widget->renderer, rect.x + style->boxSize + style->boxRightMargin, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, style->textColor, button->label, 0);

	nvgRestore(vg);
}

static void wz_tab_button_draw(struct wzWidget *widget, wzRect clip)
{
	NVGcolor bgColor1, bgColor2;
	wzRect labelRect;
	struct wzButton *button = (struct wzButton *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzTabButtonStyle *style = &widget->style.tabButton;
	const wzRect rect = wz_widget_get_absolute_rect(widget);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);

	// Background color.
	if (button->isPressed && widget->hover)
	{
		bgColor1 = style->bgPressedColor1;
		bgColor2 = style->bgPressedColor2;
	}
	else if (button->isSet)
	{
		bgColor1 = style->bgSetColor1;
		bgColor2 = style->bgSetColor2;
	}
	else
	{
		bgColor1 = style->bgColor1;
		bgColor2 = style->bgColor2;
	}

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, style->cornerRadius);

	// Background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, bgColor1, bgColor2));
	nvgFill(vg);

	// Border.
	nvgStrokeColor(vg, widget->hover ? style->borderHoverColor : style->borderColor);
	nvgStroke(vg);

	// Label.
	labelRect.x = rect.x + button->padding.left;
	labelRect.y = rect.y + button->padding.top;
	labelRect.w = rect.w - (button->padding.left + button->padding.right);
	labelRect.h = rect.h - (button->padding.top + button->padding.bottom);
	wz_renderer_print(widget->renderer, labelRect.x + labelRect.w / 2, labelRect.y + labelRect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, style->textColor, button->label, 0);

	nvgRestore(vg);
}

/*
================================================================================

VTABLE FUNCTIONS

================================================================================
*/

static void wz_button_destroy(struct wzWidget *widget)
{
	struct wzButton *button;

	WZ_ASSERT(widget);
	button = (struct wzButton *)widget;
	wz_string_free(button->label);
	wz_string_free(button->icon);
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

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

struct wzButton *wz_button_create()
{
	struct wzButton *button = (struct wzButton *)malloc(sizeof(struct wzButton));
	wzButtonStyle *style = &button->base.style.button;

	memset(button, 0, sizeof(struct wzButton));
	button->base.type = WZ_TYPE_BUTTON;
	button->base.vtable.measure = wz_button_measure;
	button->base.vtable.draw = wz_button_draw;
	button->base.vtable.destroy = wz_button_destroy;
	button->base.vtable.mouse_button_down = wz_button_mouse_button_down;
	button->base.vtable.mouse_button_up = wz_button_mouse_button_up;
	button->padding.left = button->padding.right = 8;
	button->padding.top = button->padding.bottom = 4;
	button->label = wz_string_empty();
	button->icon = wz_string_empty();

	style->textColor = WZ_STYLE_TEXT_COLOR;
	style->borderColor = WZ_STYLE_DARK_BORDER_COLOR;
	style->borderHoverColor = WZ_STYLE_HOVER_COLOR;
	style->bgColor1 = nvgRGB(80, 80, 80);
	style->bgColor2 = nvgRGB(70, 70, 70);
	style->bgPressedColor1 = nvgRGB(60, 60, 60);
	style->bgPressedColor2 = nvgRGB(50, 50, 50);
	style->bgSetColor1 = nvgRGB(150, 150, 150);
	style->bgSetColor2 = nvgRGB(140, 140, 140);
	style->iconSpacing = 6;
	style->cornerRadius = WZ_STYLE_CORNER_RADIUS;

	return button;
}

struct wzButton *wz_check_box_create()
{
	struct wzButton *button = wz_button_create();
	wzCheckBoxStyle *style = &button->base.style.checkBox;

	wz_button_set_set_behavior(button, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
	button->base.vtable.measure = wz_check_box_measure;
	button->base.vtable.draw = wz_check_box_draw;

	style->textColor = WZ_STYLE_TEXT_COLOR;
	style->checkColor = WZ_STYLE_TEXT_COLOR;
	style->borderColor = WZ_STYLE_LIGHT_BORDER_COLOR;
	style->borderHoverColor = WZ_STYLE_HOVER_COLOR;
	style->boxSize = 16;
	style->boxRightMargin = 8;
	style->boxInternalMargin = 4;
	style->checkThickness = 2.5f;

	return button;
}

struct wzButton *wz_tab_button_create()
{
	struct wzButton *button = wz_button_create();
	wzTabButtonStyle *style = &button->base.style.tabButton;

	button->base.vtable.draw = wz_tab_button_draw;

	style->textColor = WZ_STYLE_TEXT_COLOR;
	style->borderColor = WZ_STYLE_DARK_BORDER_COLOR;
	style->borderHoverColor = WZ_STYLE_HOVER_COLOR;
	style->bgColor1 = nvgRGB(80, 80, 80);
	style->bgColor2 = nvgRGB(70, 70, 70);
	style->bgPressedColor1 = nvgRGB(60, 60, 60);
	style->bgPressedColor2 = nvgRGB(50, 50, 50);
	style->bgSetColor1 = nvgRGB(150, 150, 150);
	style->bgSetColor2 = nvgRGB(140, 140, 140);
	style->cornerRadius = WZ_STYLE_CORNER_RADIUS;

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

void wz_button_set_label(struct wzButton *button, const char *label)
{
	WZ_ASSERT(button);
	button->label = wz_string_copy(button->label, label);
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
	button->icon = wz_string_copy(button->icon, icon);
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
