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

struct wzScroller
{
	struct wzWidget base;
	wzScrollerType scrollerType;
	int value, stepValue, maxValue;
	struct wzButton *decrementButton;
	struct wzButton *incrementButton;

	// Height for vertical, width for horizontal.
	int nubSize;

	wzRect nubRect;
	bool nubHover;
	bool isNubPressed;

	wzScrollerValueChangedCallback *value_changed_callbacks;
};

static void wz_scroller_update_nub_rect(struct wzScroller *scroller)
{
	wzSize decrementButtonSize, incrementButtonSize;

	// Space left when button size and nub size are subtracted.
	int scrollSpace;

	decrementButtonSize = wz_widget_get_size((struct wzWidget *)scroller->decrementButton);
	incrementButtonSize = wz_widget_get_size((struct wzWidget *)scroller->incrementButton);

	if (scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		scrollSpace = scroller->base.rect.h - decrementButtonSize.h - incrementButtonSize.h - scroller->nubSize;
		scroller->nubRect.x = scroller->base.rect.x;

		if (scroller->maxValue == 0)
		{
			// Max value not set, just display at the top.
			scroller->nubRect.y = scroller->base.rect.y;
		}
		else
		{
			scroller->nubRect.y = (int)(scroller->base.rect.y + decrementButtonSize.h + scrollSpace * (scroller->value / (float)scroller->maxValue));
		}

		scroller->nubRect.w = scroller->base.rect.w;
		scroller->nubRect.h = scroller->nubSize;
	}
	else
	{
		scrollSpace = scroller->base.rect.w - decrementButtonSize.w - incrementButtonSize.w - scroller->nubSize;

		if (scroller->maxValue == 0)
		{
			// Max value not set, just display at the left.
			scroller->nubRect.x = scroller->base.rect.x;
		}
		else
		{
			scroller->nubRect.x = (int)(scroller->base.rect.x + decrementButtonSize.w + scrollSpace * (scroller->value / (float)scroller->maxValue));
		}

		scroller->nubRect.y = scroller->base.rect.y;
		scroller->nubRect.w = scroller->nubSize;
		scroller->nubRect.h = scroller->base.rect.h;
	}
}

static void wz_scroller_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzScroller *scroller;
	wzRect buttonRect;
	
	assert(widget);
	scroller = (struct wzScroller *)widget;
	widget->rect = rect;
	
	// Set button rects.
	buttonRect.x = rect.x;
	buttonRect.y = rect.y;

	if (scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		buttonRect.w = rect.w;
		buttonRect.h = ((struct wzWidget *)scroller->decrementButton)->rect.h;
	}
	else
	{
		buttonRect.w = ((struct wzWidget *)scroller->decrementButton)->rect.w;
		buttonRect.h = rect.h;
	}

	wz_widget_set_rect((struct wzWidget *)scroller->decrementButton, buttonRect);

	if (scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		buttonRect.y = rect.y + rect.h - buttonRect.h;
		buttonRect.h = ((struct wzWidget *)scroller->incrementButton)->rect.h;
	}
	else
	{
		buttonRect.x = rect.x + rect.w - buttonRect.w;
		buttonRect.w = ((struct wzWidget *)scroller->incrementButton)->rect.w;
	}

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
		wz_desktop_lock_input(widget->desktop, widget);
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
		wz_desktop_unlock_input(widget->desktop);
	}
}

static void wz_scroller_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzScroller *scroller;

	assert(widget);
	scroller = (struct wzScroller *)widget;
	scroller->nubHover = WZ_POINT_IN_RECT(mouseX, mouseY, scroller->nubRect);

	// Handle nub dragging.
	if (scroller->isNubPressed)
	{
		wzSize decrementButtonSize, incrementButtonSize;

		// Space left when button size and nub size are subtracted.
		int scrollSpace;

		// Min & max possible positions for nub center. Y for vertical, X for horizontal.
		int minPos, maxPos;

		float scrollPercent;

		decrementButtonSize = wz_widget_get_size((struct wzWidget *)scroller->decrementButton);
		incrementButtonSize = wz_widget_get_size((struct wzWidget *)scroller->incrementButton);

		if (scroller->scrollerType == WZ_SCROLLER_VERTICAL)
		{
			scrollSpace = scroller->base.rect.h - decrementButtonSize.h - incrementButtonSize.h - scroller->nubSize;
			minPos = (int)(scroller->base.rect.y + decrementButtonSize.h + scroller->nubSize / 2.0f);
			maxPos = (int)(scroller->base.rect.y + scroller->base.rect.h - decrementButtonSize.h - scroller->nubSize / 2.0f);
			scrollPercent = WZ_CLAMPED(0, (mouseY - minPos) / (float)(maxPos - minPos), 1.0f);
		}
		else
		{
			scrollSpace = scroller->base.rect.w - decrementButtonSize.w - incrementButtonSize.w - scroller->nubSize;
			minPos = (int)(scroller->base.rect.x + decrementButtonSize.w + scroller->nubSize / 2.0f);
			maxPos = (int)(scroller->base.rect.x + scroller->base.rect.w - decrementButtonSize.w - scroller->nubSize / 2.0f);
			scrollPercent = WZ_CLAMPED(0, (mouseX - minPos) / (float)(maxPos - minPos), 1.0f);
		}

		wz_scroller_set_value(scroller, (int)(scroller->maxValue * scrollPercent));
	}
}

static void wz_scroller_clear_input_state(struct wzWidget *widget)
{
	struct wzScroller *scroller;

	assert(widget);
	scroller = (struct wzScroller *)widget;
	scroller->nubHover = false;
	scroller->isNubPressed = false;
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

static void wz_scroller_destroy(struct wzWidget *widget)
{
	struct wzScroller *scroller;

	assert(widget);
	scroller = (struct wzScroller *)widget;
	stb_arr_free(scroller->value_changed_callbacks);
}

struct wzScroller *wz_scroller_create(struct wzContext *context, wzScrollerType scrollerType)
{
	struct wzScroller *scroller;

	assert(context);
	scroller = (struct wzScroller *)malloc(sizeof(struct wzScroller));
	memset(scroller, 0, sizeof(struct wzScroller));
	scroller->base.type = WZ_TYPE_SCROLLER;
	scroller->base.context = context;
	scroller->base.vtable.destroy = wz_scroller_destroy;
	scroller->base.vtable.set_rect = wz_scroller_set_rect;
	scroller->base.vtable.mouse_button_down = wz_scroller_mouse_button_down;
	scroller->base.vtable.mouse_button_up = wz_scroller_mouse_button_up;
	scroller->base.vtable.mouse_move = wz_scroller_mouse_move;
	scroller->base.vtable.clear_input_state = wz_scroller_clear_input_state;
	scroller->scrollerType = scrollerType;
	scroller->stepValue = 1;

	scroller->decrementButton = wz_button_create(context);
	wz_button_add_callback_pressed(scroller->decrementButton, wz_scroller_decrement_button_pressed);
	wz_widget_add_child_widget((struct wzWidget *)scroller, (struct wzWidget *)scroller->decrementButton);
	
	scroller->incrementButton = wz_button_create(context);
	wz_button_add_callback_pressed(scroller->incrementButton, wz_scroller_increment_button_pressed);
	wz_widget_add_child_widget((struct wzWidget *)scroller, (struct wzWidget *)scroller->incrementButton);

	return scroller;
}

int wz_scroller_get_value(const struct wzScroller *scroller)
{
	assert(scroller);
	return scroller->value;
}

// This is the only place wzScroller value should be set.
void wz_scroller_set_value(struct wzScroller *scroller, int value)
{
	int i;
	int oldValue;

	assert(scroller);
	oldValue = scroller->value;
	scroller->value = WZ_CLAMPED(0, value, scroller->maxValue);

	// Don't fire callbacks or update the nub rect if the value hasn't changed.
	if (oldValue == scroller->value)
		return;

	for (i = 0; i < stb_arr_len(scroller->value_changed_callbacks); i++)
	{
		scroller->value_changed_callbacks[i](scroller, scroller->value);
	}

	wz_scroller_update_nub_rect(scroller);
}

void wz_scroller_decrement_value(struct wzScroller *scroller)
{
	assert(scroller);
	wz_scroller_set_value(scroller, scroller->value - scroller->stepValue);
}

void wz_scroller_increment_value(struct wzScroller *scroller)
{
	assert(scroller);
	wz_scroller_set_value(scroller, scroller->value + scroller->stepValue);
}

void wz_scroller_set_step_value(struct wzScroller *scroller, int stepValue)
{
	assert(scroller);
	scroller->stepValue = WZ_MAX(1, stepValue);
	wz_scroller_update_nub_rect(scroller);
}

void wz_scroller_set_max_value(struct wzScroller *scroller, int maxValue)
{
	assert(scroller);
	scroller->maxValue = WZ_MAX(0, maxValue);

	// Keep value in 0 to maxValue range.
	// wz_scroller_set_value does the sanity check, just pass in the current value.
	wz_scroller_set_value(scroller, scroller->value);

	// wz_scroller_set_value calls this, but only if the value has changed. We need to call it because of the maxValue change.
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

void wz_scroller_add_callback_value_changed(struct wzScroller *scroller, wzScrollerValueChangedCallback callback)
{
	assert(scroller);
	stb_arr_push(scroller->value_changed_callbacks, callback);
}
