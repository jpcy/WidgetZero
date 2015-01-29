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
#include <vector>
#include <stdlib.h>
#include <string.h>
#include "wz_button.h"
#include "wz_main_window.h"
#include "wz_renderer.h"
#include "wz_widget.h"
#include "wz_skin.h"

#define WZ_MINIMUM_NUB_SIZE 8
#define WZ_DEFAULT_NUB_SIZE 16

namespace wz { 

struct ScrollerNub : public WidgetImpl
{
	ScrollerNub();

	struct ScrollerImpl *scroller;
	bool isPressed;

	// The position of the nub when the it was pressed.
	Position pressPosition;

	// The position of the mouse when the nub was pressed.
	Position pressMousePosition;
};

struct ScrollerImpl : public WidgetImpl
{
	ScrollerImpl();

	ScrollerType scrollerType;
	int value, stepValue, maxValue;
	float nubScale;
	struct ScrollerNub *nub;
	std::vector<EventCallback> value_changed_callbacks;
};

/*
================================================================================

NUB CONTAINER

================================================================================
*/

static void wz_scroller_nub_container_mouse_button_down(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ScrollerImpl *scroller;
	Rect nubRect;

	WZ_ASSERT(widget);

	if (mouseButton != 1)
		return;

	scroller = (struct ScrollerImpl *)widget->parent->parent;
	nubRect = wz_widget_get_absolute_rect((const struct WidgetImpl *)scroller->nub);

	if ((scroller->scrollerType == WZ_SCROLLER_VERTICAL && mouseY < nubRect.y) || (scroller->scrollerType == WZ_SCROLLER_HORIZONTAL && mouseX < nubRect.x))
	{
		wz_scroller_set_value(scroller, scroller->value - scroller->stepValue * 3);
	}
	else if ((scroller->scrollerType == WZ_SCROLLER_VERTICAL && mouseY > nubRect.y + nubRect.h) || (scroller->scrollerType == WZ_SCROLLER_HORIZONTAL && mouseX > nubRect.x + nubRect.w))
	{
		wz_scroller_set_value(scroller, scroller->value + scroller->stepValue * 3);
	}
}

/*
================================================================================

SCROLLER NUB WIDGET

================================================================================
*/

static void wz_nub_mouse_button_down(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ScrollerNub *nub;
	Rect rect;

	WZ_ASSERT(widget);
	nub = (struct ScrollerNub *)widget;
	rect = wz_widget_get_absolute_rect((struct WidgetImpl *)nub);

	if (mouseButton == 1 && nub->hover)
	{
		nub->isPressed = true;
		nub->pressPosition.x = rect.x;
		nub->pressPosition.y = rect.y;
		nub->pressMousePosition.x = mouseX;
		nub->pressMousePosition.y = mouseY;
		wz_main_window_push_lock_input_widget(widget->mainWindow, widget);
	}
}

static void wz_nub_mouse_button_up(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ScrollerNub *nub;

	WZ_ASSERT(widget);
	nub = (struct ScrollerNub *)widget;

	if (mouseButton == 1)
	{
		nub->isPressed = false;
		wz_main_window_pop_lock_input_widget(widget->mainWindow, widget);
	}
}

static void wz_nub_mouse_move(struct WidgetImpl *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct ScrollerNub *nub;
	Size nubSize;
	Rect containerRect;

	WZ_ASSERT(widget);
	nub = (struct ScrollerNub *)widget;
	nubSize = wz_widget_get_size((struct WidgetImpl *)nub);
	containerRect = wz_widget_get_absolute_rect(nub->parent);

	// Handle dragging.
	if (nub->isPressed)
	{
		int minPos, maxPos, newPos;

		if (nub->scroller->scrollerType == WZ_SCROLLER_VERTICAL)
		{
			minPos = containerRect.y;
			maxPos = containerRect.y + containerRect.h - nubSize.h;
			newPos = nub->pressPosition.y + (mouseY - nub->pressMousePosition.y);
		}
		else
		{
			minPos = containerRect.x;
			maxPos = containerRect.x + containerRect.w - nubSize.w;
			newPos = nub->pressPosition.x + (mouseX - nub->pressMousePosition.x);
		}

		wz_scroller_set_value(nub->scroller, (int)(nub->scroller->maxValue * WZ_CLAMPED(0, (newPos - minPos) / (float)(maxPos - minPos), 1.0f)));
	}
}

static void wz_scroller_nub_update_rect(struct ScrollerNub *nub)
{
	Size containerSize;
	Rect rect;

	WZ_ASSERT(nub);
	containerSize = wz_widget_get_size(nub->parent);

	if (nub->scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		int availableSpace = containerSize.h;

		if (nub->scroller->maxValue == 0)
		{
			// Max value not set, fill available space.
			rect.y = 0;
			rect.h = availableSpace;
		}
		else
		{
			if (nub->scroller->nubScale > 0)
			{
				rect.h = WZ_CLAMPED(WZ_MINIMUM_NUB_SIZE, (int)(availableSpace * nub->scroller->nubScale), availableSpace);
			}
			else
			{
				rect.h = WZ_DEFAULT_NUB_SIZE;
			}

			availableSpace -= rect.h;
			rect.y = (int)(availableSpace * (nub->scroller->value / (float)nub->scroller->maxValue));
		}

		rect.x = 0;
		rect.w = containerSize.w;
	}
	else
	{
		int availableSpace = containerSize.w;

		if (nub->scroller->maxValue == 0)
		{
			// Max value not set, just display at the left.
			rect.x = 0;
			rect.w = availableSpace;
		}
		else
		{
			if (nub->scroller->nubScale > 0)
			{
				rect.w = WZ_CLAMPED(WZ_MINIMUM_NUB_SIZE, (int)(availableSpace * nub->scroller->nubScale), availableSpace);
			}
			else
			{
				rect.w = WZ_DEFAULT_NUB_SIZE;
			}

			availableSpace -= rect.w;
			rect.x = (int)(availableSpace * (nub->scroller->value / (float)nub->scroller->maxValue));
		}

		rect.y = 0;
		rect.h = containerSize.h;
	}

	wz_widget_set_rect_internal((struct WidgetImpl *)nub, rect);
}

ScrollerNub::ScrollerNub()
{
	isPressed = false;
}

static struct ScrollerNub *wz_scroller_nub_create(struct ScrollerImpl *scroller)
{
	struct ScrollerNub *nub;

	WZ_ASSERT(scroller);
	nub = new struct ScrollerNub;
	nub->scroller = scroller;
	nub->vtable.mouse_button_down = wz_nub_mouse_button_down;
	nub->vtable.mouse_button_up = wz_nub_mouse_button_up;
	nub->vtable.mouse_move = wz_nub_mouse_move;
	return nub;
}

/*
================================================================================

DECREMENT AND INCREMENT BUTTONS

================================================================================
*/

static void wz_scroller_button_draw(struct WidgetImpl *widget, Rect clip, bool decrement)
{
	NVGcolor bgColor1, bgColor2;
	int sides, roundedCorners;
	struct ButtonImpl *button = (struct ButtonImpl *)widget;
	struct ScrollerImpl *scroller = (struct ScrollerImpl *)widget->parent;
	struct NVGcontext *vg = widget->renderer->vg;
	const Rect r = wz_widget_get_absolute_rect(widget);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	
	// Background color.
	if (wz_button_is_pressed(button) && widget->hover)
	{
		bgColor1 = WZ_SKIN_SCROLLER_BG_PRESSED_COLOR1;
		bgColor2 = WZ_SKIN_SCROLLER_BG_PRESSED_COLOR2;
	}
	else
	{
		bgColor1 = WZ_SKIN_SCROLLER_BG_COLOR1;
		bgColor2 = WZ_SKIN_SCROLLER_BG_COLOR2;
	}

	nvgBeginPath(vg);

	if (wz_scroller_get_type(scroller) == WZ_SCROLLER_VERTICAL)
	{
		if (decrement)
		{
			sides = WZ_SIDE_LEFT | WZ_SIDE_TOP | WZ_SIDE_RIGHT;
			roundedCorners = WZ_CORNER_TL | WZ_CORNER_TR;
		}
		else
		{
			sides = WZ_SIDE_LEFT | WZ_SIDE_BOTTOM | WZ_SIDE_RIGHT;
			roundedCorners = WZ_CORNER_BL | WZ_CORNER_BR;
		}
	}
	else
	{
		if (decrement)
		{
			sides = WZ_SIDE_TOP | WZ_SIDE_LEFT | WZ_SIDE_BOTTOM;
			roundedCorners = WZ_CORNER_TL | WZ_CORNER_BL;
		}
		else
		{
			sides = WZ_SIDE_TOP | WZ_SIDE_RIGHT | WZ_SIDE_BOTTOM;
			roundedCorners = WZ_CORNER_TR | WZ_CORNER_BR;
		}
	}

	// Background.
	wz_renderer_create_rect_path(vg, r, WZ_SKIN_SCROLLER_CORNER_RADIUS, WZ_SIDE_ALL, roundedCorners);
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)r.x, (float)r.y, (float)r.x, (float)r.y + r.h, bgColor1, bgColor2));
	nvgFill(vg);

	// Border.
	wz_renderer_create_rect_path(vg, r, WZ_SKIN_SCROLLER_CORNER_RADIUS, sides, roundedCorners);
	nvgStrokeColor(vg, widget->hover ? WZ_SKIN_SCROLLER_BORDER_HOVER_COLOR : WZ_SKIN_SCROLLER_BORDER_COLOR);
	nvgStroke(vg);

	// Icon.
	nvgBeginPath(vg);

	if (wz_scroller_get_type(scroller) == WZ_SCROLLER_VERTICAL)
	{
		if (decrement)
		{
			nvgMoveTo(vg, r.x + r.w * 0.5f, r.y + r.h * 0.25f); // top
			nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.75f); // left
			nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.75f); // right
		}
		else
		{
			nvgMoveTo(vg, r.x + r.w * 0.5f, r.y + r.h * 0.75f); // bottom
			nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.25f); // right
			nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.25f); // left
		}
	}
	else
	{
		if (decrement)
		{
			nvgMoveTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.5f); // left
			nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.75f); // bottom
			nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.25f); // top
		}
		else
		{
			nvgMoveTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.5f); // right
			nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.25f); // top
			nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.75f); // bottom
		}
	}

	nvgFillColor(vg, widget->hover ? WZ_SKIN_SCROLLER_ICON_HOVER_COLOR : WZ_SKIN_SCROLLER_ICON_COLOR);
	nvgFill(vg);
	nvgRestore(vg);
}

static void wz_scroller_decrement_button_draw(struct WidgetImpl *widget, Rect clip)
{
	WZ_ASSERT(widget);
	wz_scroller_button_draw(widget, clip, true);
}

static void wz_scroller_increment_button_draw(struct WidgetImpl *widget, Rect clip)
{
	WZ_ASSERT(widget);
	wz_scroller_button_draw(widget, clip, false);
}

/*
================================================================================

SCROLLER WIDGET

================================================================================
*/

static void wz_scroller_mouse_button_up(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct ScrollerImpl *scroller;

	WZ_ASSERT(widget);
	scroller = (struct ScrollerImpl *)widget;
}

static void wz_scroller_mouse_wheel_move(struct WidgetImpl *widget, int x, int y)
{
	struct ScrollerImpl *scroller;

	WZ_ASSERT(widget);
	scroller = (struct ScrollerImpl *)widget;
	wz_scroller_set_value(scroller, scroller->value - y * scroller->stepValue);
}

static void wz_scroller_decrement_button_clicked(Event *e)
{
	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	wz_scroller_decrement_value((struct ScrollerImpl *)e->base.widget->parent->parent);
}

static void wz_scroller_increment_button_clicked(Event *e)
{
	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	wz_scroller_increment_value((struct ScrollerImpl *)e->base.widget->parent->parent);
}

static Size wz_scroller_measure(struct WidgetImpl *widget)
{
	Size size;
	struct ScrollerImpl *scroller = (struct ScrollerImpl *)widget;

	if (scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		size.w = WZ_SKIN_SCROLLER_DEFAULT_SIZE;
		size.h = 0;
	}
	else
	{
		size.w = 0;
		size.h = WZ_SKIN_SCROLLER_DEFAULT_SIZE;
	}

	return size;
}

static void wz_scroller_draw(struct WidgetImpl *widget, Rect clip)
{
	const struct ScrollerImpl *scroller = (struct ScrollerImpl *)widget;
	struct NVGcontext *vg = widget->renderer->vg;
	Rect nubContainerRect, nubRect;
	bool hover, pressed;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	wz_scroller_get_nub_state(scroller, &nubContainerRect, &nubRect, &hover, &pressed);

	// Nub container.
	wz_renderer_draw_filled_rect(vg, nubContainerRect, WZ_SKIN_SCROLLER_BG_COLOR1);

	// Nub.
	{
		const Rect r = nubRect;
		NVGcolor bgColor1, bgColor2;
		int i;

		// Background color.
		if (pressed)
		{
			bgColor1 = WZ_SKIN_SCROLLER_BG_PRESSED_COLOR1;
			bgColor2 = WZ_SKIN_SCROLLER_BG_PRESSED_COLOR2;
		}
		else
		{
			bgColor1 = WZ_SKIN_SCROLLER_BG_COLOR1;
			bgColor2 = WZ_SKIN_SCROLLER_BG_COLOR2;
		}

		nvgBeginPath(vg);
		nvgRect(vg, r.x + 0.5f, r.y + 0.5f, r.w - 1.0f, r.h - 1.0f);

		// Background.
		nvgFillPaint(vg, nvgLinearGradient(vg, (float)r.x, (float)r.y, (float)r.x, (float)r.y + r.h, bgColor1, bgColor2));
		nvgFill(vg);

		// Border.
		nvgStrokeColor(vg, hover || pressed ? WZ_SKIN_SCROLLER_BORDER_HOVER_COLOR : WZ_SKIN_SCROLLER_BORDER_COLOR);
		nvgStroke(vg);

		// Icon.
		for (i = 0; i < 3; i++)
		{
			nvgBeginPath(vg);

			if (scroller->scrollerType == WZ_STACK_LAYOUT_VERTICAL)
			{
				const float y = (float)((int)(r.y + r.h * 0.5f) + WZ_SKIN_SCROLLER_NUB_ICON_SPACING * (i - 1));
				nvgMoveTo(vg, (float)r.x + WZ_SKIN_SCROLLER_NUB_ICON_MARGIN, y);
				nvgLineTo(vg, (float)r.x + r.w - WZ_SKIN_SCROLLER_NUB_ICON_MARGIN, y);
			}
			else
			{
				const float x = (float)((int)(r.x + r.w * 0.5f) + WZ_SKIN_SCROLLER_NUB_ICON_SPACING * (i - 1));
				nvgMoveTo(vg, x, (float)r.y + WZ_SKIN_SCROLLER_NUB_ICON_MARGIN);
				nvgLineTo(vg, x, (float)r.y + r.h - WZ_SKIN_SCROLLER_NUB_ICON_MARGIN);
			}

			nvgStrokeColor(vg, hover || pressed ? WZ_SKIN_SCROLLER_ICON_HOVER_COLOR : WZ_SKIN_SCROLLER_ICON_COLOR);
			nvgStrokeWidth(vg, 2);
			nvgLineCap(vg, NVG_ROUND);
			nvgStroke(vg);
		}
	}

	nvgRestore(vg);
}

static void wz_scroller_set_rect(struct WidgetImpl *widget, Rect rect)
{
	struct ScrollerImpl *scroller;

	WZ_ASSERT(widget);
	scroller = (struct ScrollerImpl *)widget;
	widget->rect = rect;
	wz_scroller_nub_update_rect(scroller->nub);
}

ScrollerImpl::ScrollerImpl()
{
	type = WZ_TYPE_SCROLLER;
	nubScale = 0;
}

Scroller::Scroller(ScrollerType type)
{
	impl = wz_scroller_create(type, 0, 1, 0);
}

Scroller::~Scroller()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

Scroller *Scroller::setValue(int value)
{
	wz_scroller_set_value((ScrollerImpl *)impl, value);
	return this;
}

Scroller *Scroller::setStepValue(int stepValue)
{
	wz_scroller_set_step_value((ScrollerImpl *)impl, stepValue);
	return this;
}

Scroller *Scroller::setMaxValue(int maxValue)
{
	wz_scroller_set_max_value((ScrollerImpl *)impl, maxValue);
	return this;
}

int Scroller::getValue() const
{
	return wz_scroller_get_value((ScrollerImpl *)impl);
}

struct ScrollerImpl *wz_scroller_create(ScrollerType scrollerType, int value, int stepValue, int maxValue)
{
	struct StackLayoutImpl *layout;
	struct ButtonImpl *decrementButton, *incrementButton;
	struct WidgetImpl *nubContainer;
	
	struct ScrollerImpl *scroller = new struct ScrollerImpl;
	scroller->type = WZ_TYPE_SCROLLER;
	scroller->vtable.measure = wz_scroller_measure;
	scroller->vtable.draw = wz_scroller_draw;
	scroller->vtable.mouse_button_up = wz_scroller_mouse_button_up;
	scroller->vtable.mouse_wheel_move = wz_scroller_mouse_wheel_move;
	scroller->vtable.set_rect = wz_scroller_set_rect;
	scroller->scrollerType = scrollerType;
	scroller->stepValue = WZ_MAX(1, stepValue);
	scroller->maxValue = WZ_MAX(0, maxValue);
	scroller->value = WZ_CLAMPED(0, value, scroller->maxValue);

	layout = wz_stack_layout_create(scrollerType == WZ_SCROLLER_VERTICAL ? WZ_STACK_LAYOUT_VERTICAL : WZ_STACK_LAYOUT_HORIZONTAL, 0);
	wz_widget_set_stretch((struct WidgetImpl *)layout, WZ_STRETCH);
	wz_widget_add_child_widget((struct WidgetImpl *)scroller, (struct WidgetImpl *)layout);

	decrementButton = new ButtonImpl();
	wz_widget_set_size_args((struct WidgetImpl *)decrementButton, WZ_SKIN_SCROLLER_BUTTON_SIZE, WZ_SKIN_SCROLLER_BUTTON_SIZE);
	wz_widget_set_draw_callback((struct WidgetImpl *)decrementButton, wz_scroller_decrement_button_draw);
	wz_button_add_callback_clicked(decrementButton, wz_scroller_decrement_button_clicked);
	wz_stack_layout_add(layout, (struct WidgetImpl *)decrementButton);

	nubContainer = new WidgetImpl();
	((struct WidgetImpl *)nubContainer)->vtable.mouse_button_down = wz_scroller_nub_container_mouse_button_down;
	wz_widget_set_stretch((struct WidgetImpl *)nubContainer, WZ_STRETCH);
	wz_stack_layout_add(layout, (struct WidgetImpl *)nubContainer);

	scroller->nub = wz_scroller_nub_create(scroller);
	wz_widget_add_child_widget((struct WidgetImpl *)nubContainer, (struct WidgetImpl *)scroller->nub);

	incrementButton = new ButtonImpl();
	wz_widget_set_size_args((struct WidgetImpl *)incrementButton, WZ_SKIN_SCROLLER_BUTTON_SIZE, WZ_SKIN_SCROLLER_BUTTON_SIZE);
	wz_widget_set_draw_callback((struct WidgetImpl *)incrementButton, wz_scroller_increment_button_draw);
	wz_button_add_callback_clicked(incrementButton, wz_scroller_increment_button_clicked);
	wz_stack_layout_add(layout, (struct WidgetImpl *)incrementButton);

	wz_scroller_nub_update_rect(scroller->nub);
	return scroller;
}

ScrollerType wz_scroller_get_type(const struct ScrollerImpl *scroller)
{
	WZ_ASSERT(scroller);
	return scroller->scrollerType;
}

int wz_scroller_get_value(const struct ScrollerImpl *scroller)
{
	WZ_ASSERT(scroller);
	return scroller->value;
}

// This is the only place ScrollerImpl value should be set.
void wz_scroller_set_value(struct ScrollerImpl *scroller, int value)
{
	int oldValue;
	Event e;

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

void wz_scroller_decrement_value(struct ScrollerImpl *scroller)
{
	WZ_ASSERT(scroller);
	wz_scroller_set_value(scroller, scroller->value - scroller->stepValue);
}

void wz_scroller_increment_value(struct ScrollerImpl *scroller)
{
	WZ_ASSERT(scroller);
	wz_scroller_set_value(scroller, scroller->value + scroller->stepValue);
}

void wz_scroller_set_step_value(struct ScrollerImpl *scroller, int stepValue)
{
	WZ_ASSERT(scroller);
	scroller->stepValue = WZ_MAX(1, stepValue);
}

int wz_scroller_get_step_value(struct ScrollerImpl *scroller)
{
	WZ_ASSERT(scroller);
	return scroller->stepValue;
}

void wz_scroller_set_max_value(struct ScrollerImpl *scroller, int maxValue)
{
	WZ_ASSERT(scroller);
	scroller->maxValue = WZ_MAX(0, maxValue);

	// Keep value in 0 to maxValue range.
	// wz_scroller_set_value does the sanity check, just pass in the current value.
	wz_scroller_set_value(scroller, scroller->value);
}

void wz_scroller_set_nub_scale(struct ScrollerImpl *scroller, float nubScale)
{
	WZ_ASSERT(scroller);
	scroller->nubScale = nubScale;
	wz_scroller_nub_update_rect(scroller->nub);
}

void wz_scroller_get_nub_state(const struct ScrollerImpl *scroller, Rect *containerRect, Rect *rect, bool *hover, bool *pressed)
{
	WZ_ASSERT(scroller);

	if (containerRect)
		*containerRect = wz_widget_get_absolute_rect(scroller->nub->parent);

	if (rect)
		*rect = wz_widget_get_absolute_rect((const struct WidgetImpl *)scroller->nub);

	if (hover)
		*hover = wz_widget_get_hover((const struct WidgetImpl *)scroller->nub);

	if (pressed)
		*pressed = scroller->nub->isPressed;
}

void wz_scroller_add_callback_value_changed(struct ScrollerImpl *scroller, EventCallback callback)
{
	WZ_ASSERT(scroller);
	scroller->value_changed_callbacks.push_back(callback);
}

} // namespace wz
