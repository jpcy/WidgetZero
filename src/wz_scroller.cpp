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
#include "wz.h"
#pragma hdrstop
#include "wz_renderer_nanovg.h"

#define WZ_MINIMUM_NUB_SIZE 8
#define WZ_DEFAULT_NUB_SIZE 16

namespace wz { 

class ScrollerDecrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer->drawScrollerDecrementButton(this, clip);
	}
};

class ScrollerIncrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer->drawScrollerIncrementButton(this, clip);
	}
};

/*
================================================================================

NUB CONTAINER

================================================================================
*/

class ScrollerNubContainer : public Widget
{
public:
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
	{
		if (mouseButton != 1)
			return;

		Scroller *scroller = (Scroller *)parent->parent;
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

ScrollerNub::ScrollerNub(Scroller *scroller)
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

SCROLLER WIDGET

================================================================================
*/

Scroller::Scroller(ScrollerType scrollerType, int value, int stepValue, int maxValue)
{
	type = WZ_TYPE_SCROLLER;
	nubScale = 0;
	this->scrollerType = scrollerType;
	this->stepValue = WZ_MAX(1, stepValue);
	this->maxValue = WZ_MAX(0, maxValue);
	this->value = WZ_CLAMPED(0, value, maxValue);

	StackLayout *layout = new StackLayout(scrollerType == WZ_SCROLLER_VERTICAL ? WZ_STACK_LAYOUT_VERTICAL : WZ_STACK_LAYOUT_HORIZONTAL, 0);
	layout->setStretch(WZ_STRETCH);
	addChildWidget(layout);

	ScrollerDecrementButton *decrementButton = new ScrollerDecrementButton();
	decrementButton->setSize(WZ_SKIN_SCROLLER_BUTTON_SIZE, WZ_SKIN_SCROLLER_BUTTON_SIZE);
	decrementButton->addEventHandler(WZ_EVENT_BUTTON_CLICKED, this, &Scroller::onDecrementButtonClicked);
	layout->add(decrementButton);

	ScrollerNubContainer *nubContainer = new ScrollerNubContainer();
	nubContainer->setStretch(WZ_STRETCH);
	layout->add(nubContainer);

	nub = new ScrollerNub(this);
	nubContainer->addChildWidget(nub);

	ScrollerIncrementButton *incrementButton = new ScrollerIncrementButton();
	incrementButton->setSize(WZ_SKIN_SCROLLER_BUTTON_SIZE, WZ_SKIN_SCROLLER_BUTTON_SIZE);
	incrementButton->addEventHandler(WZ_EVENT_BUTTON_CLICKED, this, &Scroller::onIncrementButtonClicked);
	layout->add(incrementButton);

	nub->updateRect();
}

void Scroller::onRectChanged()
{
	nub->updateRect();
}

void Scroller::onMouseWheelMove(int /*x*/, int y)
{
	setValue(value - y * stepValue);
}

void Scroller::draw(Rect clip)
{
	renderer->drawScroller(this, clip);
}

Size Scroller::measure()
{
	return renderer->measureScroller(this);
}

ScrollerType Scroller::getType() const
{
	return scrollerType;
}

int Scroller::getValue() const
{
	return value;
}

// This is the only place Scroller value should be set.
void Scroller::setValue(int value)
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
	invokeEvent(&e, value_changed_callbacks);

	nub->updateRect();
}

void Scroller::decrementValue()
{
	setValue(value - stepValue);
}

void Scroller::incrementValue()
{
	setValue(value + stepValue);
}

void Scroller::setStepValue(int stepValue)
{
	this->stepValue = WZ_MAX(1, stepValue);
}

int Scroller::getStepValue()
{
	return stepValue;
}

void Scroller::setMaxValue(int maxValue)
{
	this->maxValue = WZ_MAX(0, maxValue);

	// Keep value in 0 to maxValue range.
	// setValue does the sanity check, just pass in the current value.
	setValue(value);
}

void Scroller::setNubScale(float nubScale)
{
	this->nubScale = nubScale;
	nub->updateRect();
}

void Scroller::getNubState(Rect *containerRect, Rect *rect, bool *hover, bool *pressed) const
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

void Scroller::addCallbackValueChanged(EventCallback callback)
{
	value_changed_callbacks.push_back(callback);
}

void Scroller::onDecrementButtonClicked(Event *e)
{
	decrementValue();
}

void Scroller::onIncrementButtonClicked(Event *e)
{
	incrementValue();
}

} // namespace wz
