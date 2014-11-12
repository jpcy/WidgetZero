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
#include "wz_renderer.h"
#include "wz_string.h"
#include "wz_widget.h"

struct wzMenuBarButton
{
	struct wzWidget base;
	wzBorder padding;
	wzString label;
	bool isPressed;
	bool isSet;
	wzEventCallback *pressed_callbacks;
	struct wzMenuBar *menuBar;
};

struct wzMenuBar
{
	struct wzWidget base;
	struct wzStackLayout *layout;
};

/*
================================================================================

MENU BAR BUTTON

================================================================================
*/

static wzSize wz_menu_bar_button_measure(struct wzWidget *widget)
{
	wzSize size;
	struct wzMenuBarButton *button = (struct wzMenuBarButton *)widget;

	wz_widget_measure_text(widget, button->label, 0, &size.w, &size.h);
	size.w += 12;
	return size;
}

static void wz_menu_bar_button_draw(struct wzWidget *widget, wzRect clip)
{
	struct wzMenuBarButton *button = (struct wzMenuBarButton *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	const wzMenuBarStyle *style = &button->menuBar->base.style.menuBar; // Use parent menubar style.
	const wzRect rect = wz_widget_get_absolute_rect(widget);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);

	if (button->isPressed)
	{
		wz_renderer_draw_filled_rect(vg, rect, style->setColor);
	}

	if (widget->hover)
	{
		wz_renderer_draw_rect(vg, rect, style->borderHoverColor);
	}

	wz_renderer_print(widget->renderer, rect.x + rect.w / 2, rect.y + rect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, style->textColor, button->label, 0);

	nvgRestore(vg);
}

static void wz_menu_bar_button_destroy(struct wzWidget *widget)
{
	struct wzMenuBarButton *button;

	WZ_ASSERT(widget);
	button = (struct wzMenuBarButton *)widget;
	wz_string_free(button->label);
	wz_arr_free(button->pressed_callbacks);
}

static void wz_menu_bar_button_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzMenuBarButton *button = (struct wzMenuBarButton *)widget;
	WZ_ASSERT(button);

	if (mouseButton == 1)
	{
		button->isPressed = true;

		// Lock input to the menu bar, not this button.
		wz_main_window_push_lock_input_widget(widget->mainWindow, (struct wzWidget *)button->menuBar);
	}
}

static void wz_menu_bar_button_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzMenuBarButton *button = (struct wzMenuBarButton *)widget;
	WZ_ASSERT(button);

	if (mouseButton == 1 && button->isPressed)
	{
		button->isPressed = false;
		wz_main_window_pop_lock_input_widget(widget->mainWindow, widget->parent->parent);
	}
}

static void wz_menu_bar_button_mouse_hover_on(struct wzWidget *widget)
{
	int i;
	struct wzMenuBarButton *button = (struct wzMenuBarButton *)widget;
	WZ_ASSERT(button);

	// See if any of the other buttons in the menubar are pressed.
	// If one is pressed, unpress it and press this one instead.
	for (i = 0; i < wz_arr_len(button->menuBar->base.children[0]->children); i++)
	{
		struct wzMenuBarButton *otherButton = (struct wzMenuBarButton *)button->menuBar->base.children[0]->children[i];

		if (otherButton == button || !otherButton->isPressed)
			continue;

		otherButton->isPressed = false;
		button->isPressed = true;
		return;
	}
}

static struct wzMenuBarButton *wz_menu_bar_button_create(struct wzMenuBar *menuBar)
{
	struct wzMenuBarButton *button = (struct wzMenuBarButton *)malloc(sizeof(struct wzMenuBarButton));
	memset(button, 0, sizeof(struct wzMenuBarButton));
	button->base.type = WZ_TYPE_BUTTON;
	button->base.vtable.measure = wz_menu_bar_button_measure;
	button->base.vtable.draw = wz_menu_bar_button_draw;
	button->base.vtable.destroy = wz_menu_bar_button_destroy;
	button->base.vtable.mouse_button_down = wz_menu_bar_button_mouse_button_down;
	button->base.vtable.mouse_button_up = wz_menu_bar_button_mouse_button_up;
	button->base.vtable.mouse_hover_on = wz_menu_bar_button_mouse_hover_on;
	button->label = wz_string_empty();
	button->menuBar = menuBar;
	return button;
}

void wz_menu_bar_button_set_label(struct wzMenuBarButton *button, const char *label)
{
	WZ_ASSERT(button);
	button->label = wz_string_copy(button->label, label);
	wz_widget_resize_to_measured(&button->base);
}

const char *wz_menu_bar_button_get_label(const struct wzMenuBarButton *button)
{
	WZ_ASSERT(button);
	return button->label;
}

bool wz_menu_bar_button_is_pressed(const struct wzMenuBarButton *button)
{
	WZ_ASSERT(button);
	return button->isPressed;
}

/*
================================================================================

MENU BAR

================================================================================
*/

static void wz_menu_bar_draw(struct wzWidget *widget, wzRect clip)
{
	struct NVGcontext *vg = widget->renderer->vg;
	const wzRect rect = wz_widget_get_absolute_rect(widget);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	wz_renderer_draw_filled_rect(vg, rect, widget->style.menuBar.bgColor);
	nvgRestore(vg);
}

static void wz_menu_bar_renderer_changed(struct wzWidget *widget)
{
	WZ_ASSERT(widget);
	wz_widget_set_height(widget, wz_widget_get_line_height(widget) + widget->style.menuBar.padding);
}

struct wzMenuBar *wz_menu_bar_create()
{
	struct wzMenuBar *menuBar = (struct wzMenuBar *)malloc(sizeof(struct wzMenuBar));
	wzMenuBarStyle *style = &menuBar->base.style.menuBar;

	memset(menuBar, 0, sizeof(struct wzMenuBar));
	menuBar->base.type = WZ_TYPE_MENU_BAR;
	menuBar->base.vtable.draw = wz_menu_bar_draw;
	menuBar->base.vtable.renderer_changed = wz_menu_bar_renderer_changed;

	menuBar->layout = wz_stack_layout_create(WZ_STACK_LAYOUT_HORIZONTAL, 0);
	wz_widget_set_stretch((struct wzWidget *)menuBar->layout, WZ_STRETCH);
	wz_widget_add_child_widget((struct wzWidget *)menuBar, (struct wzWidget *)menuBar->layout);

	style->textColor = WZ_STYLE_TEXT_COLOR;
	style->setColor = nvgRGB(40, 140, 190);
	style->borderHoverColor = WZ_STYLE_HOVER_COLOR;
	style->bgColor = nvgRGB(80, 80, 80);
	style->padding = 6;

	return menuBar;
}

struct wzMenuBarButton *wz_menu_bar_create_button(struct wzMenuBar *menuBar)
{
	struct wzMenuBarButton *button;

	WZ_ASSERT(menuBar);
	button = wz_menu_bar_button_create(menuBar);
	wz_widget_set_stretch((struct wzWidget *)button, WZ_STRETCH_HEIGHT);
	wz_stack_layout_add(menuBar->layout, (struct wzWidget *)button);
	return button;
}
