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

namespace wz {

StackLayout::StackLayout(StackLayoutDirection::Enum direction, int spacing)
{
	type_ = WidgetType::StackLayout;
	direction_ = direction;
	spacing_ = spacing;
}

void StackLayout::setDirection(StackLayoutDirection::Enum direction)
{
	direction_ = direction;
	setRectDirty();
}

void StackLayout::setSpacing(int spacing)
{
	spacing_ = spacing;
	setRectDirty();
}

int StackLayout::getSpacing() const
{
	return spacing_;
}

void StackLayout::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->getType() == WidgetType::MainWindow || widget->getType() == WidgetType::Window)
		return;

	addChildWidget(widget);
}

void StackLayout::remove(Widget *widget)
{
	WZ_ASSERT(widget);
	removeChildWidget(widget);
}

void StackLayout::doLayout()
{
	if (direction_ == StackLayoutDirection::Vertical)
	{
		layoutVertical();
	}
	else
	{
		layoutHorizontal();
	}
}

Size StackLayout::measure()
{
	Size s;

	for (size_t i = 0; i < children_.size(); i++)
	{
		Widget *child = children_[i];

		if (!child->isVisible())
			continue;

		const Size childSize = child->getMeasuredSize();

		if (direction_ == StackLayoutDirection::Vertical)
		{
			s.w = WZ_MAX(s.w, childSize.w);
			s.h += spacing_ + childSize.h;
		}
		else
		{
			s.w += spacing_ + childSize.w;
			s.h = WZ_MAX(s.h, childSize.h);
		}
	}

	return s + Size(margin_.left + margin_.right, margin_.top + margin_.bottom);
}

void StackLayout::layoutVertical()
{
	int availableHeight = rect_.h;

	// The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int nStretchingWidgets = 0;

	for (size_t i = 0; i < children_.size(); i++)
	{
		if (!children_[i]->isVisible())
			continue;

		// Subtract all child widget top and bottom margins from the available height.
		availableHeight -= children_[i]->getMargin().top + children_[i]->getMargin().bottom;

		// Subtract layout spacing too, except for the first child (because spacing is applied to the top).
		if (i != 0)
		{
			availableHeight -= spacing_;
		}

		if ((children_[i]->getStretch() & Stretch::Height) != 0)
		{
			// Count the number of widgets that stretch in the same direction as the the layout, so available space can be divided evenly between them.
			nStretchingWidgets++;
		}
		else
		{
			// Subtract the user or measured heights of child widgets that aren't being stretched in the same direction as the the layout.
			availableHeight -= children_[i]->getUserOrMeasuredSize().h;
		}
	}

	// Layout the children.
	int y = 0;

	for (size_t i = 0; i < children_.size(); i++)
	{
		Widget *child = children_[i];

		if (!child->isVisible())
			continue;

		if (i != 0)
		{
			y += spacing_;
		}

		y += child->getMargin().top;

		Rect childRect;
		childRect.x = child->getMargin().left;
		childRect.y = y;

		if ((child->getStretch() & Stretch::Width) != 0)
		{
			// Fit the width of the layout.
			childRect.w = rect_.w - (child->getMargin().left + child->getMargin().right);
		}
		else
		{
			// Use the user or measured width.
			childRect.w = child->getUserOrMeasuredSize().w;

			// Handle horizontal alignment.
			if ((child->getAlign() & Align::Center) != 0)
			{
				childRect.x = child->getMargin().left + (int)((rect_.w - child->getMargin().right) / 2.0f - childRect.w / 2.0f);
			}
			else if ((child->getAlign() & Align::Right) != 0)
			{
				childRect.x = rect_.w - child->getMargin().right - childRect.w;
			}
		}

		if ((child->getStretch() & Stretch::Height) != 0)
		{
			// The available height is evenly divided between children that stretch vertically.
			childRect.h = (int)(availableHeight / (float)nStretchingWidgets);
		}
		else
		{
			// Use the user or measured height.
			childRect.h = child->getUserOrMeasuredSize().h;
		}

		child->setRectInternal(childRect);
		y += childRect.h + child->getMargin().bottom;
	}
}

void StackLayout::layoutHorizontal()
{
	int availableWidth = rect_.w;

	// The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int nStretchingWidgets = 0;

	for (size_t i = 0; i < children_.size(); i++)
	{
		if (!children_[i]->isVisible())
			continue;

		// Subtract all child widget left and right margins from the available width.
		availableWidth -= children_[i]->getMargin().left + children_[i]->getMargin().right;

		// Subtract layout spacing too, except for the first child (because spacing is applied to the left).
		if (i != 0)
		{
			availableWidth -= spacing_;
		}

		if ((children_[i]->getStretch() & Stretch::Width) != 0)
		{
			// Count the number of widgets that stretch in the same direction as the the layout, so available space can be divided evenly between them.
			nStretchingWidgets++;
		}
		else
		{
			// Subtract the user or measured widths of child widgets that aren't being stretched in the same direction as the the layout.
			availableWidth -= children_[i]->getUserOrMeasuredSize().w;
		}
	}

	// Layout the children.
	int x = 0;

	for (size_t i = 0; i < children_.size(); i++)
	{
		Widget *child = children_[i];

		if (!child->isVisible())
			continue;

		if (i != 0)
		{
			x += spacing_;
		}

		x += child->getMargin().left;

		Rect childRect;
		childRect.x = x;
		childRect.y = child->getMargin().top;

		if ((child->getStretch() & Stretch::Width) != 0)
		{
			// The available width is evenly divided between children that stretch horizontally.
			childRect.w = (int)(availableWidth / (float)nStretchingWidgets);
		}
		else
		{
			// Use the user or measured width.
			childRect.w = child->getUserOrMeasuredSize().w;
		}

		if ((child->getStretch() & Stretch::Height) != 0)
		{
			// Fit the height of the layout.
			childRect.h = rect_.h - (child->getMargin().top + child->getMargin().bottom);
		}
		else
		{
			// Use the user or measured height.
			childRect.h = child->getUserOrMeasuredSize().h;

			// Handle vertical alignment.
			if ((child->getAlign() & Align::Middle) != 0)
			{
				childRect.y = child->getMargin().top + (int)((rect_.h - child->getMargin().bottom) / 2.0f - childRect.h / 2.0f);
			}
			else if ((child->getAlign() & Align::Bottom) != 0)
			{
				childRect.y = rect_.h - child->getMargin().bottom - childRect.h;
			}
		}

		child->setRectInternal(childRect);
		x += childRect.w + child->getMargin().right;
	}
}

} // namespace wz
