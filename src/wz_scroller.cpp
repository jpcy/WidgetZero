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
#include "wz_internal.h"
#pragma hdrstop

#define WZ_MINIMUM_NUB_SIZE 8
#define WZ_DEFAULT_NUB_SIZE 16

namespace wz { 

/*
================================================================================

NUB CONTAINER

================================================================================
*/

struct ScrollerNubContainer : public WidgetImpl
{
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
	{
		if (mouseButton != 1)
			return;

		struct ScrollerImpl *scroller = (struct ScrollerImpl *)parent->parent;
		Rect nubRect = scroller->nub->getAbsoluteRect();

		if ((scroller->scrollerType == WZ_SCROLLER_VERTICAL && mouseY < nubRect.y) || (scroller->scrollerType == WZ_SCROLLER_HORIZONTAL && mouseX < nubRect.x))
		{
			scroller->setValue(scroller->value - scroller->stepValue * 3);
		}
		else if ((scroller->scrollerType == WZ_SCROLLER_VERTICAL && mouseY > nubRect.y + nubRect.h) || (scroller->scrollerType == WZ_SCROLLER_HORIZONTAL && mouseX > nubRect.x + nubRect.w))
		{
			scroller->setValue(scroller->value + scroller->stepValue * 3);
		}
	}
};

/*
================================================================================

SCROLLER NUB WIDGET

================================================================================
*/

ScrollerNub::ScrollerNub(struct ScrollerImpl *scroller)
{
	this->scroller = scroller;
	isPressed = false;
}

void ScrollerNub::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1 && hover)
	{
		const Rect rect = getAbsoluteRect();
		isPressed = true;
		pressPosition.x = rect.x;
		pressPosition.y = rect.y;
		pressMousePosition.x = mouseX;
		pressMousePosition.y = mouseY;
		mainWindow->pushLockInputWidget(this);
	}
}

void ScrollerNub::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		isPressed = false;
		mainWindow->popLockInputWidget(this);
	}
}

void ScrollerNub::onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	// Handle dragging.
	if (isPressed)
	{
		const Rect containerRect = parent->getAbsoluteRect();
		int minPos, maxPos, newPos;

		if (scroller->scrollerType == WZ_SCROLLER_VERTICAL)
		{
			minPos = containerRect.y;
			maxPos = containerRect.y + containerRect.h - rect.h;
			newPos = pressPosition.y + (mouseY - pressMousePosition.y);
		}
		else
		{
			minPos = containerRect.x;
			maxPos = containerRect.x + containerRect.w - rect.w;
			newPos = pressPosition.x + (mouseX - pressMousePosition.x);
		}

		scroller->setValue((int)(scroller->maxValue * WZ_CLAMPED(0, (newPos - minPos) / (float)(maxPos - minPos), 1.0f)));
	}
}

void ScrollerNub::updateRect()
{
	const Size containerSize = parent->getSize();
	Rect rect;

	if (scroller->scrollerType == WZ_SCROLLER_VERTICAL)
	{
		int availableSpace = containerSize.h;

		if (scroller->maxValue == 0)
		{
			// Max value not set, fill available space.
			rect.y = 0;
			rect.h = availableSpace;
		}
		else
		{
			if (scroller->nubScale > 0)
			{
				rect.h = WZ_CLAMPED(WZ_MINIMUM_NUB_SIZE, (int)(availableSpace * scroller->nubScale), availableSpace);
			}
			else
			{
				rect.h = WZ_DEFAULT_NUB_SIZE;
			}

			availableSpace -= rect.h;
			rect.y = (int)(availableSpace * (scroller->value / (float)scroller->maxValue));
		}

		rect.x = 0;
		rect.w = containerSize.w;
	}
	else
	{
		int availableSpace = containerSize.w;

		if (scroller->maxValue == 0)
		{
			// Max value not set, just display at the left.
			rect.x = 0;
			rect.w = availableSpace;
		}
		else
		{
			if (scroller->nubScale > 0)
			{
				rect.w = WZ_CLAMPED(WZ_MINIMUM_NUB_SIZE, (int)(availableSpace * scroller->nubScale), availableSpace);
			}
			else
			{
				rect.w = WZ_DEFAULT_NUB_SIZE;
			}

			availableSpace -= rect.w;
			rect.x = (int)(availableSpace * (scroller->value / (float)scroller->maxValue));
		}

		rect.y = 0;
		rect.h = containerSize.h;
	}

	setRectInternal(rect);
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
	NVGRenderer *r = (NVGRenderer *)widget->renderer;
	struct NVGcontext *vg = r->getContext();
	const Rect rect = widget->getAbsoluteRect();

	nvgSave(vg);
	r->clipToRect(clip);
	
	// Background color.
	if (button->isPressed() && widget->hover)
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

	if (scroller->getType() == WZ_SCROLLER_VERTICAL)
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
	r->createRectPath(rect, WZ_SKIN_SCROLLER_CORNER_RADIUS, WZ_SIDE_ALL, roundedCorners);
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, bgColor1, bgColor2));
	nvgFill(vg);

	// Border.
	r->createRectPath(rect, WZ_SKIN_SCROLLER_CORNER_RADIUS, sides, roundedCorners);
	nvgStrokeColor(vg, widget->hover ? WZ_SKIN_SCROLLER_BORDER_HOVER_COLOR : WZ_SKIN_SCROLLER_BORDER_COLOR);
	nvgStroke(vg);

	// Icon.
	nvgBeginPath(vg);

	if (scroller->getType() == WZ_SCROLLER_VERTICAL)
	{
		if (decrement)
		{
			nvgMoveTo(vg, rect.x + rect.w * 0.5f, rect.y + rect.h * 0.25f); // top
			nvgLineTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.75f); // left
			nvgLineTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.75f); // right
		}
		else
		{
			nvgMoveTo(vg, rect.x + rect.w * 0.5f, rect.y + rect.h * 0.75f); // bottom
			nvgLineTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.25f); // right
			nvgLineTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.25f); // left
		}
	}
	else
	{
		if (decrement)
		{
			nvgMoveTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.5f); // left
			nvgLineTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.75f); // bottom
			nvgLineTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.25f); // top
		}
		else
		{
			nvgMoveTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.5f); // right
			nvgLineTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.25f); // top
			nvgLineTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.75f); // bottom
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

static void wz_scroller_decrement_button_clicked(Event *e)
{
	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	((struct ScrollerImpl *)e->base.widget->parent->parent)->decrementValue();
}

static void wz_scroller_increment_button_clicked(Event *e)
{
	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	((struct ScrollerImpl *)e->base.widget->parent->parent)->incrementValue();
}

ScrollerImpl::ScrollerImpl(ScrollerType scrollerType, int value, int stepValue, int maxValue)
{
	type = WZ_TYPE_SCROLLER;
	nubScale = 0;
	this->scrollerType = scrollerType;
	this->stepValue = WZ_MAX(1, stepValue);
	this->maxValue = WZ_MAX(0, maxValue);
	this->value = WZ_CLAMPED(0, value, maxValue);

	struct StackLayoutImpl *layout = new StackLayoutImpl(scrollerType == WZ_SCROLLER_VERTICAL ? WZ_STACK_LAYOUT_VERTICAL : WZ_STACK_LAYOUT_HORIZONTAL, 0);
	layout->setStretch(WZ_STRETCH);
	addChildWidget(layout);

	struct ButtonImpl *decrementButton = new ButtonImpl();
	decrementButton->setSize(WZ_SKIN_SCROLLER_BUTTON_SIZE, WZ_SKIN_SCROLLER_BUTTON_SIZE);
	decrementButton->setDrawCallback(wz_scroller_decrement_button_draw);
	decrementButton->addCallbackClicked(wz_scroller_decrement_button_clicked);
	layout->add(decrementButton);

	struct ScrollerNubContainer *nubContainer = new ScrollerNubContainer();
	nubContainer->setStretch(WZ_STRETCH);
	layout->add(nubContainer);

	nub = new ScrollerNub(this);
	nubContainer->addChildWidget(nub);

	struct ButtonImpl *incrementButton = new ButtonImpl();
	incrementButton->setSize(WZ_SKIN_SCROLLER_BUTTON_SIZE, WZ_SKIN_SCROLLER_BUTTON_SIZE);
	incrementButton->setDrawCallback(wz_scroller_increment_button_draw);
	incrementButton->addCallbackClicked(wz_scroller_increment_button_clicked);
	layout->add(incrementButton);

	nub->updateRect();
}

void ScrollerImpl::onRectChanged()
{
	nub->updateRect();
}

void ScrollerImpl::onMouseWheelMove(int /*x*/, int y)
{
	setValue(value - y * stepValue);
}

void ScrollerImpl::draw(Rect clip)
{
	renderer->drawScroller(this, clip);
}

Size ScrollerImpl::measure()
{
	return renderer->measureScroller(this);
}

ScrollerType ScrollerImpl::getType() const
{
	return scrollerType;
}

int ScrollerImpl::getValue() const
{
	return value;
}

// This is the only place ScrollerImpl value should be set.
void ScrollerImpl::setValue(int value)
{
	int oldValue = this->value;
	this->value = WZ_CLAMPED(0, value, maxValue);

	// Don't fire callbacks or update the nub rect if the value hasn't changed.
	if (oldValue == this->value)
		return;

	Event e;
	e.scroller.type = WZ_EVENT_SCROLLER_VALUE_CHANGED;
	e.scroller.scroller = this;
	e.scroller.oldValue = oldValue;
	e.scroller.value = this->value;
	wz_invoke_event(&e, value_changed_callbacks);

	nub->updateRect();
}

void ScrollerImpl::decrementValue()
{
	setValue(value - stepValue);
}

void ScrollerImpl::incrementValue()
{
	setValue(value + stepValue);
}

void ScrollerImpl::setStepValue(int stepValue)
{
	this->stepValue = WZ_MAX(1, stepValue);
}

int ScrollerImpl::getStepValue()
{
	return stepValue;
}

void ScrollerImpl::setMaxValue(int maxValue)
{
	this->maxValue = WZ_MAX(0, maxValue);

	// Keep value in 0 to maxValue range.
	// setValue does the sanity check, just pass in the current value.
	setValue(value);
}

void ScrollerImpl::setNubScale(float nubScale)
{
	this->nubScale = nubScale;
	nub->updateRect();
}

void ScrollerImpl::getNubState(Rect *containerRect, Rect *rect, bool *hover, bool *pressed) const
{
	if (containerRect)
		*containerRect = nub->parent->getAbsoluteRect();

	if (rect)
		*rect = nub->getAbsoluteRect();

	if (hover)
		*hover = nub->getHover();

	if (pressed)
		*pressed = nub->isPressed;
}

void ScrollerImpl::addCallbackValueChanged(EventCallback callback)
{
	value_changed_callbacks.push_back(callback);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

Scroller::Scroller(ScrollerType type)
{
	impl.reset(new ScrollerImpl(type, 0, 1, 0));
}

Scroller::~Scroller()
{
}

Scroller *Scroller::setValue(int value)
{
	getImpl()->setValue(value);
	return this;
}

Scroller *Scroller::setStepValue(int stepValue)
{
	getImpl()->setStepValue(stepValue);
	return this;
}

Scroller *Scroller::setMaxValue(int maxValue)
{
	getImpl()->setMaxValue(maxValue);
	return this;
}

int Scroller::getValue() const
{
	return getImpl()->getValue();
}

ScrollerImpl *Scroller::getImpl()
{
	return (ScrollerImpl *)impl.get();
}

const ScrollerImpl *Scroller::getImpl() const
{
	return (const ScrollerImpl *)impl.get();
}

} // namespace wz
