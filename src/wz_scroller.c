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
#include "wz_internal.h"

struct wzScroller
{
	struct wzWidget base;
	int value, stepValue, maxValue;
	struct wzButton *decrementButton;
	struct wzButton *incrementButton;
	int nubSize;
	wzRect nubRect;
	bool nubHover;
	bool isNubPressed;
};

static void wz_scroller_update_nub_rect(struct wzScroller *scroller)
{
	int decrementButtonHeight;
	int incrementButtonHeight;
	int scrollSpace; // Space left when button size and nub size are subtracted.

	decrementButtonHeight = wz_widget_get_size((struct wzWidget *)scroller->decrementButton).h;
	incrementButtonHeight = wz_widget_get_size((struct wzWidget *)scroller->incrementButton).h;
	scrollSpace = scroller->base.rect.h - decrementButtonHeight - incrementButtonHeight - scroller->nubSize;
	scroller->nubRect.x = scroller->base.rect.x;
	scroller->nubRect.y = (int)(scroller->base.rect.y + decrementButtonHeight + scrollSpace * (scroller->value / (float)scroller->maxValue));
	scroller->nubRect.w = scroller->base.rect.w;
	scroller->nubRect.h = scroller->nubSize;
}

static void wz_scroller_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzScroller *scroller;
	wzRect buttonRect;
	
	assert(widget);
	scroller = (struct wzScroller *)widget;
	memcpy(&widget->rect, &rect, sizeof(wzRect));
	
	// Set button rects.
	buttonRect.x = rect.x;
	buttonRect.y = rect.y;
	buttonRect.w = rect.w;
	buttonRect.h = ((struct wzWidget *)scroller->decrementButton)->rect.h;
	wz_widget_set_rect((struct wzWidget *)scroller->decrementButton, buttonRect);

	buttonRect.y = rect.y + rect.h - buttonRect.h;
	buttonRect.h = ((struct wzWidget *)scroller->incrementButton)->rect.h;
	wz_widget_set_rect((struct wzWidget *)scroller->incrementButton, buttonRect);

	wz_scroller_update_nub_rect(scroller);
}

static void wz_scroller_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScroller *scroller;

	assert(widget);
	scroller = (struct wzScroller *)widget;

	if (mouseButton == 1 && scroller->nubHover)
	{
		scroller->isNubPressed = true;
	}
}

static void wz_scroller_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScroller *scroller;

	assert(widget);
	scroller = (struct wzScroller *)widget;

	if (mouseButton == 1)
	{
		scroller->isNubPressed = false;
	}
}

static void wz_scroller_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzScroller *scroller;

	assert(widget);
	scroller = (struct wzScroller *)widget;
	scroller->nubHover = WZ_POINT_IN_RECT(mouseX, mouseY, scroller->nubRect);

	// Handle nub dragging.
	if (scroller->isNubPressed && mouseDeltaY != 0)
	{
		int decrementButtonHeight;
		int incrementButtonHeight;
		int scrollSpace; // Space left when button size and nub size are subtracted.
		int minY, maxY; // Min & max possible y coordinates for nub center.
		float scrollPercent;

		decrementButtonHeight = wz_widget_get_size((struct wzWidget *)scroller->decrementButton).h;
		incrementButtonHeight = wz_widget_get_size((struct wzWidget *)scroller->incrementButton).h;
		scrollSpace = scroller->base.rect.h - decrementButtonHeight - incrementButtonHeight - scroller->nubSize;
		minY = (int)(scroller->base.rect.y + decrementButtonHeight + scroller->nubSize / 2.0f);
		maxY = (int)(scroller->base.rect.y + scroller->base.rect.h - decrementButtonHeight - scroller->nubSize / 2.0f);
		scrollPercent = (mouseY - minY) / (float)(maxY - minY);

		if (scrollPercent < 0)
			scrollPercent = 0;
		else if (scrollPercent > 1.0f)
			scrollPercent = 1.0f;

		scroller->value = (int)(scroller->maxValue * scrollPercent);
		wz_scroller_update_nub_rect(scroller);
	}
}

static void wz_scroller_decrement_button_pressed(struct wzButton *button)
{
	struct wzWidget *buttonWidget;
	
	assert(button);
	buttonWidget = (struct wzWidget *)button;
	assert(buttonWidget->parent);
	wz_scroller_decrement_value((struct wzScroller *)buttonWidget->parent);
}

static void wz_scroller_increment_button_pressed(struct wzButton *button)
{
	struct wzWidget *buttonWidget;
	
	assert(button);
	buttonWidget = (struct wzWidget *)button;
	assert(buttonWidget->parent);
	wz_scroller_increment_value((struct wzScroller *)buttonWidget->parent);
}

struct wzScroller *wz_scroller_create(struct wzWindow *window)
{
	struct wzScroller *scroller;

	assert(window);
	scroller = (struct wzScroller *)malloc(sizeof(struct wzScroller));
	memset(scroller, 0, sizeof(struct wzScroller));
	scroller->base.type = WZ_TYPE_SCROLLER;
	scroller->base.window = window;
	scroller->base.vtable.set_rect = wz_scroller_set_rect;
	scroller->base.vtable.mouse_button_down = wz_scroller_mouse_button_down;
	scroller->base.vtable.mouse_button_up = wz_scroller_mouse_button_up;
	scroller->base.vtable.mouse_move = wz_scroller_mouse_move;

	scroller->decrementButton = wz_button_create(window);
	wz_button_add_callback_pressed(scroller->decrementButton, wz_scroller_decrement_button_pressed);
	wz_widget_add_child_widget((struct wzWidget *)scroller, (struct wzWidget *)scroller->decrementButton);
	
	scroller->incrementButton = wz_button_create(window);
	wz_button_add_callback_pressed(scroller->incrementButton, wz_scroller_increment_button_pressed);
	wz_widget_add_child_widget((struct wzWidget *)scroller, (struct wzWidget *)scroller->incrementButton);

	return scroller;
}

int wz_scroller_get_value(const struct wzScroller *scroller)
{
	assert(scroller);
	return scroller->value;
}

void wz_scroller_set_value(struct wzScroller *scroller, int value)
{
	assert(scroller);
	scroller->value = value;
	wz_scroller_update_nub_rect(scroller);
}

void wz_scroller_decrement_value(struct wzScroller *scroller)
{
	assert(scroller);
	scroller->value -= scroller->stepValue;

	if (scroller->value < 0)
		scroller->value = 0;

	wz_scroller_update_nub_rect(scroller);
}

void wz_scroller_increment_value(struct wzScroller *scroller)
{
	assert(scroller);
	scroller->value += scroller->stepValue;

	if (scroller->value > scroller->maxValue)
		scroller->value = scroller->maxValue;

	wz_scroller_update_nub_rect(scroller);
}

void wz_scroller_set_step_value(struct wzScroller *scroller, int stepValue)
{
	assert(scroller);
	scroller->stepValue = stepValue;
	wz_scroller_update_nub_rect(scroller);
}

void wz_scroller_set_max_value(struct wzScroller *scroller, int maxValue)
{
	assert(scroller);
	scroller->maxValue = maxValue;
	wz_scroller_update_nub_rect(scroller);
}

struct wzButton *wz_scroller_get_decrement_button(struct wzScroller *scroller)
{
	assert(scroller);
	return scroller->decrementButton;
}

struct wzButton *wz_scroller_get_increment_button(struct wzScroller *scroller)
{
	assert(scroller);
	return scroller->incrementButton;
}

int wz_scroller_get_nub_size(struct wzScroller *scroller)
{
	assert(scroller);
	return scroller->nubSize;
}

void wz_scroller_set_nub_size(struct wzScroller *scroller, int size)
{
	assert(scroller);
	scroller->nubSize = size;
	wz_scroller_update_nub_rect(scroller);
}

wzRect wz_scroller_get_nub_rect(struct wzScroller *scroller)
{
	assert(scroller);
	return scroller->nubRect;
}
