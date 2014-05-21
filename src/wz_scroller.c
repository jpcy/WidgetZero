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

struct wzScrollerNub
{
	struct wzWidget base;
	struct wzScroller *scroller;
	bool isPressed;
};

struct wzScroller
{
	struct wzWidget base;
	wzScrollerType scrollerType;
	int value, stepValue, maxValue;
	struct wzButton *decrementButton;
	struct wzButton *incrementButton;

	// Height for vertical, width for horizontal.
	int nubSize;

	struct wzScrollerNub *nub;

	wzScrollerValueChangedCallback *value_changed_callbacks;
};

/*
================================================================================

SCROLLER NUB WIDGET

================================================================================
*/

static void wz_nub_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScrollerNub *nub;

	assert(widget);
	nub = (struct wzScrollerNub *)widget;

	if (mouseButton == 1 && nub->base.hover)
	{
		nub->isPressed = true;
		wz_desktop_push_lock_input_widget(widget->desktop, widget);
	}
}

static void wz_nub_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScrollerNub *nub;

	assert(widget);
	nub = (struct wzScrollerNub *)widget;

	if (mouseButton == 1)
	{
		nub->isPressed = false;
		wz_desktop_pop_lock_input_widget(widget->desktop, widget);
	}
}

static void wz_nub_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzScrollerNub *nub;
	wzRect rect;

	assert(widget);
	nub = (struct wzScrollerNub *)widget;
	rect = wz_widget_get_absolute_rect((struct wzWidget *)nub->scroller);

	// Handle dragging.
	if (nub->isPressed)
	{
		wzSize decrementButtonSize, incrementButtonSize;

		// Space left when button size and nub size are subtracted.
		int scrollSpace;

		// Min & max possible positions for nub center. Y for vertical, X for horizontal.
		int minPos, maxPos;

		float scrollPercent;

		decrementButtonSize = wz_widget_get_size((struct wzWidget *)nub->scroller->decrementButton);
		incrementButtonSize = wz_widget_get_size((struct wzWidget *)nub->scroller->incrementButton);

		if (nub->scroller->scrollerType == WZ_SCROLLER_VERTICAL)
		{
			scrollSpace = rect.h - decrementButtonSize.h - incrementButtonSize.h - nub->scroller->nubSize;
			minPos = (int)(rect.y + decrementButtonSize.h + nub->scroller->nubSize / 2.0f);
			maxPos = (int)(rect.y + rect.h - decrementButtonSize.h - nub->scroller->nubSize / 2.0f);
			scrollPercent = WZ_CLAMPED(0, (mouseY - minPos) / (float)(maxPos - minPos), 1.0f);
		}
		else
		{
			scrollSpace = rect.w - decrementButtonSize.w - incrementButtonSize.w - nub->scroller->nubSize;
			minPos = (int)(rect.x + decrementButtonSize.w + nub->scroller->nubSize / 2.0f);
			maxPos = (int)(rect.x + rect.w - decrementButtonSize.w - nub->scroller->nubSize / 2.0f);
			scrollPercent = WZ_CLAMPED(0, (mouseX - minPos) / (float)(maxPos - minPos), 1.0f);
		}

		wz_scroller_set_value(nub->scroller, (int)(nub->scroller->maxValue * scrollPercent));
	}
}

static struct wzScrollerNub *wz_scroller_nub_create(struct wzContext *context, struct wzScroller *scroller)
{
	struct wzScrollerNub *nub;

	assert(context);
	nub = (struct wzScrollerNub *)malloc(sizeof(struct wzScrollerNub));
	memset(nub, 0, sizeof(struct wzScrollerNub));
	nub->base.context = context;
	nub->scroller = scroller;
	nub->base.vtable.mouse_button_down = wz_nub_mouse_button_down;
	nub->base.vtable.mouse_button_up = wz_nub_mouse_button_up;
	nub->base.vtable.mouse_move = wz_nub_mouse_move;
	return nub;
}

static void wz_nub_update_rect(struct wzScrollerNub *nub)
{
	wzRect rect, nubRect;
	wzSize decrementButtonSize, incrementButtonSize;

	// Space left when button size and nub size are subtracted.
	int scrollSpace;

	decrementButtonSize = wz_widget_get_size((struct wzWidget *)nub->scroller->decrementButton);
	incrementButtonSize = wz_widget_get_size((struct wzWidget *)nub->scroller->incrementButton);
	rect = wz_widget_get_rect((const struct wzWidget *)nub->scroller);

	if (nub->scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		scrollSpace = rect.h - decrementButtonSize.h - incrementButtonSize.h - nub->scroller->nubSize;
		nubRect.x = rect.x;

		if (nub->scroller->maxValue == 0)
		{
			// Max value not set, just display at the top.
			nubRect.y = rect.y;
		}
		else
		{
			nubRect.y = (int)(rect.y + decrementButtonSize.h + scrollSpace * (nub->scroller->value / (float)nub->scroller->maxValue));
		}

		nubRect.w = rect.w;
		nubRect.h = nub->scroller->nubSize;
	}
	else
	{
		scrollSpace = rect.w - decrementButtonSize.w - incrementButtonSize.w - nub->scroller->nubSize;

		if (nub->scroller->maxValue == 0)
		{
			// Max value not set, just display at the left.
			nubRect.x = rect.x;
		}
		else
		{
			nubRect.x = (int)(rect.x + decrementButtonSize.w + scrollSpace * (nub->scroller->value / (float)nub->scroller->maxValue));
		}

		nubRect.y = rect.y;
		nubRect.w = nub->scroller->nubSize;
		nubRect.h = rect.h;
	}

	wz_widget_set_rect((struct wzWidget *)nub, nubRect);
}

/*
================================================================================

SCROLLER WIDGET

================================================================================
*/

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
	wz_nub_update_rect(scroller->nub);
}

static void wz_scroller_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScroller *scroller;

	assert(widget);
	scroller = (struct wzScroller *)widget;
}

static void wz_scroller_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScroller *scroller;

	assert(widget);
	scroller = (struct wzScroller *)widget;
}

static void wz_scroller_mouse_wheel_move(struct wzWidget *widget, int x, int y)
{
	struct wzScroller *scroller;

	assert(widget);
	scroller = (struct wzScroller *)widget;
	wz_scroller_set_value(scroller, scroller->value - y * scroller->stepValue);
}

static void wz_scroller_parent_window_move(struct wzWidget *widget)
{
	assert(widget);
	wz_nub_update_rect(((struct wzScroller *)widget)->nub);
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
	wz_arr_free(scroller->value_changed_callbacks);
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
	scroller->base.vtable.mouse_wheel_move = wz_scroller_mouse_wheel_move;
	scroller->base.vtable.parent_window_move = wz_scroller_parent_window_move;
	scroller->scrollerType = scrollerType;
	scroller->stepValue = 1;

	scroller->decrementButton = wz_button_create(context);
	wz_button_add_callback_pressed(scroller->decrementButton, wz_scroller_decrement_button_pressed);
	wz_widget_add_child_widget((struct wzWidget *)scroller, (struct wzWidget *)scroller->decrementButton);
	
	scroller->incrementButton = wz_button_create(context);
	wz_button_add_callback_pressed(scroller->incrementButton, wz_scroller_increment_button_pressed);
	wz_widget_add_child_widget((struct wzWidget *)scroller, (struct wzWidget *)scroller->incrementButton);

	scroller->nub = wz_scroller_nub_create(context, scroller);
	wz_widget_add_child_widget((struct wzWidget *)scroller, (struct wzWidget *)scroller->nub);

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

	for (i = 0; i < wz_arr_len(scroller->value_changed_callbacks); i++)
	{
		scroller->value_changed_callbacks[i](scroller, scroller->value);
	}

	wz_nub_update_rect(scroller->nub);
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
	wz_nub_update_rect(scroller->nub);
}

int wz_scroller_get_step_value(struct wzScroller *scroller)
{
	assert(scroller);
	return scroller->stepValue;
}

void wz_scroller_set_max_value(struct wzScroller *scroller, int maxValue)
{
	assert(scroller);
	scroller->maxValue = WZ_MAX(0, maxValue);

	// Keep value in 0 to maxValue range.
	// wz_scroller_set_value does the sanity check, just pass in the current value.
	wz_scroller_set_value(scroller, scroller->value);

	// wz_scroller_set_value calls this, but only if the value has changed. We need to call it because of the maxValue change.
	wz_nub_update_rect(scroller->nub);
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
	wz_nub_update_rect(scroller->nub);
}

wzRect wz_scroller_get_nub_rect(struct wzScroller *scroller)
{
	assert(scroller);
	return wz_widget_get_absolute_rect((const struct wzWidget *)scroller->nub);
}

void wz_scroller_add_callback_value_changed(struct wzScroller *scroller, wzScrollerValueChangedCallback callback)
{
	assert(scroller);
	wz_arr_push(scroller->value_changed_callbacks, callback);
}
