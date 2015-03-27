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

#define WZ_MINIMUM_NUB_SIZE 8
#define WZ_DEFAULT_NUB_SIZE 16

namespace wz { 

class ScrollerDecrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer_->drawScrollerDecrementButton(this, clip);
	}
};

class ScrollerIncrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer_->drawScrollerIncrementButton(this, clip);
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
	void onRectChanged()
	{
		((ScrollerNub *)children_[0])->updateRect();
	}

	void onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
	{
		if (mouseButton != 1)
			return;

		Scroller *scroller = (Scroller *)parent_->getParent();
		Rect nubRect = scroller->getNub()->getAbsoluteRect();

		if ((scroller->getDirection() == ScrollerDirection::Vertical && mouseY < nubRect.y) || (scroller->getDirection() == ScrollerDirection::Horizontal && mouseX < nubRect.x))
		{
			scroller->setValue(scroller->getValue() - scroller->getStepValue() * 3);
		}
		else if ((scroller->getDirection() == ScrollerDirection::Vertical && mouseY > nubRect.y + nubRect.h) || (scroller->getDirection() == ScrollerDirection::Horizontal && mouseX > nubRect.x + nubRect.w))
		{
			scroller->setValue(scroller->getValue() + scroller->getStepValue() * 3);
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
	scroller_ = scroller;
	isPressed_ = false;
}

bool ScrollerNub::isPressed() const
{
	return isPressed_;
}

void ScrollerNub::updateRect()
{
	const Size containerSize = parent_->getSize();

	// Container size isn't set yet.
	if (containerSize == Size(0, 0))
		return;

	Rect rect;

	if (scroller_->getDirection() == ScrollerDirection::Vertical)
	{
		int availableSpace = containerSize.h;

		if (scroller_->getMaxValue() == 0)
		{
			// Max value not set, fill available space.
			rect.y = 0;
			rect.h = availableSpace;
		}
		else
		{
			if (scroller_->getNubScale() > 0)
			{
				rect.h = WZ_CLAMPED(WZ_MINIMUM_NUB_SIZE, (int)(availableSpace * scroller_->getNubScale()), availableSpace);
			}
			else
			{
				rect.h = WZ_DEFAULT_NUB_SIZE;
			}

			availableSpace -= rect.h;
			rect.y = (int)(availableSpace * (scroller_->getValue() / (float)scroller_->getMaxValue()));
		}

		rect.x = 0;
		rect.w = containerSize.w;
	}
	else
	{
		int availableSpace = containerSize.w;

		if (scroller_->getMaxValue() == 0)
		{
			// Max value not set, just display at the left.
			rect.x = 0;
			rect.w = availableSpace;
		}
		else
		{
			if (scroller_->getNubScale() > 0)
			{
				rect.w = WZ_CLAMPED(WZ_MINIMUM_NUB_SIZE, (int)(availableSpace * scroller_->getNubScale()), availableSpace);
			}
			else
			{
				rect.w = WZ_DEFAULT_NUB_SIZE;
			}

			availableSpace -= rect.w;
			rect.x = (int)(availableSpace * (scroller_->getValue() / (float)scroller_->getMaxValue()));
		}

		rect.y = 0;
		rect.h = containerSize.h;
	}

	setRect(rect);
}

void ScrollerNub::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1 && hover_)
	{
		const Rect rect = getAbsoluteRect();
		isPressed_ = true;
		pressPosition_.x = rect.x;
		pressPosition_.y = rect.y;
		pressMousePosition_.x = mouseX;
		pressMousePosition_.y = mouseY;
		mainWindow_->pushLockInputWidget(this);
	}
}

void ScrollerNub::onMouseButtonUp(int mouseButton, int /*mouseX*/, int /*mouseY*/)
{
	if (mouseButton == 1)
	{
		isPressed_ = false;
		mainWindow_->popLockInputWidget(this);
	}
}

void ScrollerNub::onMouseMove(int mouseX, int mouseY, int /*mouseDeltaX*/, int /*mouseDeltaY*/)
{
	// Handle dragging.
	if (isPressed_)
	{
		const Rect containerRect = parent_->getAbsoluteRect();
		int minPos, maxPos, newPos;

		if (scroller_->getDirection() == ScrollerDirection::Vertical)
		{
			minPos = containerRect.y;
			maxPos = containerRect.y + containerRect.h - rect_.h;
			newPos = pressPosition_.y + (mouseY - pressMousePosition_.y);
		}
		else
		{
			minPos = containerRect.x;
			maxPos = containerRect.x + containerRect.w - rect_.w;
			newPos = pressPosition_.x + (mouseX - pressMousePosition_.x);
		}

		scroller_->setValue((int)(scroller_->getMaxValue() * WZ_CLAMPED(0, (newPos - minPos) / (float)(maxPos - minPos), 1.0f)));
	}
}

/*
================================================================================

SCROLLER WIDGET

================================================================================
*/

Scroller::Scroller(ScrollerDirection::Enum direction, int value, int stepValue, int maxValue)
{
	type_ = WidgetType::Scroller;
	nubScale_ = 0;
	direction_ = direction;
	stepValue_ = WZ_MAX(1, stepValue);
	maxValue_ = WZ_MAX(0, maxValue);
	value_ = WZ_CLAMPED(0, value, maxValue_);

	StackLayout *layout = new StackLayout(direction_ == ScrollerDirection::Vertical ? StackLayoutDirection::Vertical : StackLayoutDirection::Horizontal);
	layout->setStretch(Stretch::All);
	addChildWidget(layout);

	decrementButton_ = new ScrollerDecrementButton();
	//decrementButton_->setSize(16, 16);
	decrementButton_->addEventHandler(EventType::ButtonClicked, this, &Scroller::onDecrementButtonClicked);
	layout->add(decrementButton_);

	ScrollerNubContainer *nubContainer = new ScrollerNubContainer();
	nubContainer->setStretch(Stretch::All);
	layout->add(nubContainer);

	nub_ = new ScrollerNub(this);
	nubContainer->addChildWidget(nub_);

	incrementButton_ = new ScrollerIncrementButton();
	//incrementButton_->setSize(16, 16);
	incrementButton_->addEventHandler(EventType::ButtonClicked, this, &Scroller::onIncrementButtonClicked);
	layout->add(incrementButton_);
}

ScrollerDirection::Enum Scroller::getDirection() const
{
	return direction_;
}

int Scroller::getValue() const
{
	return value_;
}

// This is the only place Scroller value should be set.
void Scroller::setValue(int value)
{
	int oldValue = value_;
	value_ = WZ_CLAMPED(0, value, maxValue_);

	// Don't fire callbacks or update the nub rect if the value hasn't changed.
	if (oldValue == value_)
		return;

	Event e;
	e.scroller.type = EventType::ScrollerValueChanged;
	e.scroller.scroller = this;
	e.scroller.oldValue = oldValue;
	e.scroller.value = value_;
	invokeEvent(e, valueChangedCallbacks_);

	nub_->updateRect();
}

void Scroller::decrementValue()
{
	setValue(value_ - stepValue_);
}

void Scroller::incrementValue()
{
	setValue(value_ + stepValue_);
}

void Scroller::setStepValue(int stepValue)
{
	stepValue_ = WZ_MAX(1, stepValue);
}

int Scroller::getStepValue()
{
	return stepValue_;
}

void Scroller::setMaxValue(int maxValue)
{
	maxValue_ = WZ_MAX(0, maxValue);

	// Keep value in 0 to maxValue_ range.
	// setValue does the sanity check, just pass in the current value.
	setValue(value_);
}

int Scroller::getMaxValue() const
{
	return maxValue_;
}

void Scroller::setNubScale(float nubScale)
{
	nubScale_ = nubScale;
	nub_->updateRect();
}

float Scroller::getNubScale() const
{
	return nubScale_;
}

ScrollerNub *Scroller::getNub()
{
	return nub_;
}

const ScrollerNub *Scroller::getNub() const
{
	return nub_;
}

void Scroller::getNubState(Rect *containerRect, Rect *rect, bool *hover, bool *pressed) const
{
	if (containerRect)
		*containerRect = nub_->getParent()->getAbsoluteRect();

	if (rect)
		*rect = nub_->getAbsoluteRect();

	if (hover)
		*hover = nub_->getHover();

	if (pressed)
		*pressed = nub_->isPressed();
}

void Scroller::addCallbackValueChanged(EventCallback callback)
{
	valueChangedCallbacks_.push_back(callback);
}

void Scroller::onRectChanged()
{
	// Match the buttons to the scroller thickness, and keep square.
	const int thickness = direction_ == ScrollerDirection::Vertical ? rect_.w : rect_.h;
	decrementButton_->setSize(thickness, thickness);
	incrementButton_->setSize(thickness, thickness);
}

void Scroller::onMouseWheelMove(int /*x*/, int y)
{
	setValue(value_ - y * stepValue_);
}

void Scroller::draw(Rect clip)
{
	renderer_->drawScroller(this, clip);
}

Size Scroller::measure()
{
	return renderer_->measureScroller(this);
}

void Scroller::onDecrementButtonClicked(Event)
{
	decrementValue();
}

void Scroller::onIncrementButtonClicked(Event)
{
	incrementValue();
}

} // namespace wz
