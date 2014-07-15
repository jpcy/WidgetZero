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

	wzEventCallback *value_changed_callbacks;
};

/*
================================================================================

SCROLLER NUB WIDGET

================================================================================
*/

static void wz_nub_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScrollerNub *nub;

	WZ_ASSERT(widget);
	nub = (struct wzScrollerNub *)widget;

	if (mouseButton == 1 && nub->base.hover)
	{
		nub->isPressed = true;
		wz_main_window_push_lock_input_widget(widget->mainWindow, widget);
	}
}

static void wz_nub_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScrollerNub *nub;

	WZ_ASSERT(widget);
	nub = (struct wzScrollerNub *)widget;

	if (mouseButton == 1)
	{
		nub->isPressed = false;
		wz_main_window_pop_lock_input_widget(widget->mainWindow, widget);
	}
}

static void wz_nub_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzScrollerNub *nub;
	wzRect rect;

	WZ_ASSERT(widget);
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

static void wz_scroller_nub_update_rect(struct wzScrollerNub *nub)
{
	wzSize scrollerSize;
	wzRect rect;
	wzSize decrementButtonSize, incrementButtonSize;

	// Space left when button size and nub size are subtracted.
	int scrollSpace;

	WZ_ASSERT(nub);

	// Buttons not set yet.
	if (!nub->scroller->decrementButton || !nub->scroller->incrementButton)
		return;

	scrollerSize = wz_widget_get_size(nub->base.parent);
	decrementButtonSize = wz_widget_get_size((struct wzWidget *)nub->scroller->decrementButton);
	incrementButtonSize = wz_widget_get_size((struct wzWidget *)nub->scroller->incrementButton);

	if (nub->scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		scrollSpace = scrollerSize.h - decrementButtonSize.h - incrementButtonSize.h - nub->scroller->nubSize;
		rect.x = 0;

		if (nub->scroller->maxValue == 0)
		{
			// Max value not set, just display at the top.
			rect.y = 0;
		}
		else
		{
			rect.y = (int)(decrementButtonSize.h + scrollSpace * (nub->scroller->value / (float)nub->scroller->maxValue));
		}

		rect.w = scrollerSize.w;
		rect.h = nub->scroller->nubSize;
	}
	else
	{
		scrollSpace = scrollerSize.w - decrementButtonSize.w - incrementButtonSize.w - nub->scroller->nubSize;

		if (nub->scroller->maxValue == 0)
		{
			// Max value not set, just display at the left.
			rect.x = 0;
		}
		else
		{
			rect.x = (int)(decrementButtonSize.w + scrollSpace * (nub->scroller->value / (float)nub->scroller->maxValue));
		}

		rect.y = 0;
		rect.w = nub->scroller->nubSize;
		rect.h = scrollerSize.h;
	}

	wz_widget_set_rect_internal((struct wzWidget *)nub, rect);
}

static struct wzScrollerNub *wz_scroller_nub_create(struct wzScroller *scroller)
{
	struct wzScrollerNub *nub;

	WZ_ASSERT(scroller);
	nub = (struct wzScrollerNub *)malloc(sizeof(struct wzScrollerNub));
	memset(nub, 0, sizeof(struct wzScrollerNub));
	//nub->base.mainWindow = scroller->base.mainWindow;
	nub->scroller = scroller;
	nub->base.vtable.mouse_button_down = wz_nub_mouse_button_down;
	nub->base.vtable.mouse_button_up = wz_nub_mouse_button_up;
	nub->base.vtable.mouse_move = wz_nub_mouse_move;
	return nub;
}

/*
================================================================================

DECREMENT AND INCREMENT BUTTONS

================================================================================
*/

static void wz_scroller_decrement_button_update_rect(struct wzScroller *scroller)
{
	wzSize scrollerSize;
	wzRect rect;

	if (!scroller->decrementButton)
		return;

	scrollerSize = wz_widget_get_size((struct wzWidget *)scroller);
	rect.x = rect.y = 0;

	if (scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		rect.w = scrollerSize.w;
		rect.h = wz_widget_get_height((struct wzWidget *)scroller->decrementButton);
	}
	else
	{
		rect.w = wz_widget_get_width((struct wzWidget *)scroller->decrementButton);
		rect.h = scrollerSize.h;
	}

	wz_widget_set_rect_internal((struct wzWidget *)scroller->decrementButton, rect);
}

static void wz_scroller_increment_button_update_rect(struct wzScroller *scroller)
{
	wzSize scrollerSize;
	wzRect rect;

	if (!scroller->incrementButton)
		return;

	scrollerSize = wz_widget_get_size((struct wzWidget *)scroller);
	rect.x = rect.y = 0;

	if (scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		rect.y = scrollerSize.h - wz_widget_get_height((struct wzWidget *)scroller->incrementButton);
		rect.w = scrollerSize.w;
		rect.h = wz_widget_get_height((struct wzWidget *)scroller->incrementButton);
	}
	else
	{
		rect.x = scrollerSize.w - wz_widget_get_width((struct wzWidget *)scroller->incrementButton);
		rect.w = wz_widget_get_width((struct wzWidget *)scroller->incrementButton);
		rect.h = scrollerSize.h;
	}

	wz_widget_set_rect_internal((struct wzWidget *)scroller->incrementButton, rect);
}

/*
================================================================================

SCROLLER WIDGET

================================================================================
*/

static void wz_scroller_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScroller *scroller;

	WZ_ASSERT(widget);
	scroller = (struct wzScroller *)widget;
}

static void wz_scroller_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzScroller *scroller;

	WZ_ASSERT(widget);
	scroller = (struct wzScroller *)widget;
}

static void wz_scroller_mouse_wheel_move(struct wzWidget *widget, int x, int y)
{
	struct wzScroller *scroller;

	WZ_ASSERT(widget);
	scroller = (struct wzScroller *)widget;
	wz_scroller_set_value(scroller, scroller->value - y * scroller->stepValue);
}

static void wz_scroller_decrement_button_clicked(wzEvent *e)
{
	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	wz_scroller_decrement_value((struct wzScroller *)e->base.widget->parent);
}

static void wz_scroller_increment_button_clicked(wzEvent *e)
{
	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	wz_scroller_increment_value((struct wzScroller *)e->base.widget->parent);
}

static void wz_scroller_destroy(struct wzWidget *widget)
{
	struct wzScroller *scroller;

	WZ_ASSERT(widget);
	scroller = (struct wzScroller *)widget;
	wz_arr_free(scroller->value_changed_callbacks);
}

static void wz_scroller_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzScroller *scroller;

	WZ_ASSERT(widget);
	scroller = (struct wzScroller *)widget;
	widget->rect = rect;
	wz_scroller_nub_update_rect(scroller->nub);
	wz_scroller_decrement_button_update_rect(scroller);
	wz_scroller_increment_button_update_rect(scroller);
}

struct wzScroller *wz_scroller_create(struct wzButton *decrementButton, struct wzButton *incrementButton)
{
	struct wzScroller *scroller;

	WZ_ASSERT(decrementButton);
	WZ_ASSERT(incrementButton);

	scroller = (struct wzScroller *)malloc(sizeof(struct wzScroller));
	memset(scroller, 0, sizeof(struct wzScroller));
	scroller->base.type = WZ_TYPE_SCROLLER;
	scroller->base.vtable.destroy = wz_scroller_destroy;
	scroller->base.vtable.mouse_button_down = wz_scroller_mouse_button_down;
	scroller->base.vtable.mouse_button_up = wz_scroller_mouse_button_up;
	scroller->base.vtable.mouse_wheel_move = wz_scroller_mouse_wheel_move;
	scroller->base.vtable.set_rect = wz_scroller_set_rect;
	scroller->stepValue = 1;

	scroller->decrementButton = decrementButton;
	wz_button_add_callback_clicked(scroller->decrementButton, wz_scroller_decrement_button_clicked);
	wz_widget_add_child_widget_internal((struct wzWidget *)scroller, (struct wzWidget *)scroller->decrementButton);
	wz_scroller_decrement_button_update_rect(scroller);

	scroller->incrementButton = incrementButton;
	wz_button_add_callback_clicked(scroller->incrementButton, wz_scroller_increment_button_clicked);
	wz_widget_add_child_widget_internal((struct wzWidget *)scroller, (struct wzWidget *)scroller->incrementButton);
	wz_scroller_increment_button_update_rect(scroller);

	scroller->nub = wz_scroller_nub_create(scroller);
	wz_widget_add_child_widget_internal((struct wzWidget *)scroller, (struct wzWidget *)scroller->nub);

	return scroller;
}

void wz_scroller_set_type(struct wzScroller *scroller, wzScrollerType scrollerType)
{
	WZ_ASSERT(scroller);
	scroller->scrollerType = scrollerType;
}

wzScrollerType wz_scroller_get_type(struct wzScroller *scroller)
{
	WZ_ASSERT(scroller);
	return scroller->scrollerType;
}

int wz_scroller_get_value(const struct wzScroller *scroller)
{
	WZ_ASSERT(scroller);
	return scroller->value;
}

// This is the only place wzScroller value should be set.
void wz_scroller_set_value(struct wzScroller *scroller, int value)
{
	int oldValue;
	wzEvent e;

	WZ_ASSERT(scroller);
	oldValue = scroller->value;
	scroller->value = WZ_CLAMPED(0, value, scroller->maxValue);

	// Don't fire callbacks or update the nub rect if the value hasn't changed.
	if (oldValue == scroller->value)
		return;

	e.scroller.type = WZ_EVENT_SCROLLER_VALUE_CHANGED;
	e.scroller.scroller = scroller;
	e.scroller.oldValue = oldValue;
	e.scroller.value = scroller->value;
	wz_invoke_event(&e, scroller->value_changed_callbacks);

	wz_scroller_nub_update_rect(scroller->nub);
}

void wz_scroller_decrement_value(struct wzScroller *scroller)
{
	WZ_ASSERT(scroller);
	wz_scroller_set_value(scroller, scroller->value - scroller->stepValue);
}

void wz_scroller_increment_value(struct wzScroller *scroller)
{
	WZ_ASSERT(scroller);
	wz_scroller_set_value(scroller, scroller->value + scroller->stepValue);
}

void wz_scroller_set_step_value(struct wzScroller *scroller, int stepValue)
{
	WZ_ASSERT(scroller);
	scroller->stepValue = WZ_MAX(1, stepValue);
}

int wz_scroller_get_step_value(struct wzScroller *scroller)
{
	WZ_ASSERT(scroller);
	return scroller->stepValue;
}

void wz_scroller_set_max_value(struct wzScroller *scroller, int maxValue)
{
	WZ_ASSERT(scroller);
	scroller->maxValue = WZ_MAX(0, maxValue);

	// Keep value in 0 to maxValue range.
	// wz_scroller_set_value does the sanity check, just pass in the current value.
	wz_scroller_set_value(scroller, scroller->value);
}

int wz_scroller_get_nub_size(struct wzScroller *scroller)
{
	WZ_ASSERT(scroller);
	return scroller->nubSize;
}

void wz_scroller_set_nub_size(struct wzScroller *scroller, int size)
{
	WZ_ASSERT(scroller);
	scroller->nubSize = size;
	wz_scroller_nub_update_rect(scroller->nub);
}

wzRect wz_scroller_get_nub_rect(struct wzScroller *scroller)
{
	WZ_ASSERT(scroller);
	return wz_widget_get_absolute_rect((const struct wzWidget *)scroller->nub);
}

void wz_scroller_add_callback_value_changed(struct wzScroller *scroller, wzEventCallback callback)
{
	WZ_ASSERT(scroller);
	wz_arr_push(scroller->value_changed_callbacks, callback);
}
