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

StackLayout::StackLayout(StackLayoutDirection direction, int spacing)
{
	type = WZ_TYPE_STACK_LAYOUT;
	direction_ = direction;
	spacing_ = spacing;
}

void StackLayout::setDirection(StackLayoutDirection direction)
{
	direction_ = direction;
	refreshRect();
}

void StackLayout::setSpacing(int spacing)
{
	spacing_ = spacing;
	refreshRect();
}

int StackLayout::getSpacing() const
{
	return spacing_;
}

void StackLayout::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	addChildWidget(widget);
}

void StackLayout::remove(Widget *widget)
{
	WZ_ASSERT(widget);
	removeChildWidget(widget);
}

void StackLayout::onRectChanged()
{
	if (direction_ == WZ_STACK_LAYOUT_VERTICAL)
	{
		layoutVertical();
	}
	else
	{
		layoutHorizontal();
	}
}

void StackLayout::layoutVertical()
{
	int availableHeight = rect.h;

	// The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int nStretchingWidgets = 0;

	for (size_t i = 0; i < children.size(); i++)
	{
		// Subtract all child widget top and bottom margins from the available height.
		availableHeight -= children[i]->margin.top + children[i]->margin.bottom;

		// Subtract layout spacing too, except for the first child (because spacing is applied to the top).
		if (i != 0)
		{
			availableHeight -= spacing_;
		}

		if ((children[i]->stretch & WZ_STRETCH_HEIGHT) != 0)
		{
			// Count the number of widgets that stretch in the same direction as the the layout, so available space can be divided evenly between them.
			nStretchingWidgets++;
		}
		else
		{
			// Subtract the heights of child widgets that aren't being stretched in the same direction as the the layout.
			availableHeight -= children[i]->rect.h;
		}
	}

	// Layout the children.
	int y = 0;

	for (size_t i = 0; i < children.size(); i++)
	{
		Widget *child = children[i];

		if (i != 0)
		{
			y += spacing_;
		}

		y += child->margin.top;

		Rect childRect;
		childRect.x = child->margin.left;
		childRect.y = y;

		if ((child->stretch & WZ_STRETCH_WIDTH) != 0)
		{
			// Fit the width of the layout.
			childRect.w = rect.w - (child->margin.left + child->margin.right);
		}
		else
		{
			// Don't change the width.
			childRect.w = child->rect.w;

			// Handle horizontal alignment.
			if ((child->align & WZ_ALIGN_CENTER) != 0)
			{
				childRect.x = child->margin.left + (int)((rect.w - child->margin.right) / 2.0f - childRect.w / 2.0f);
			}
			else if ((child->align & WZ_ALIGN_RIGHT) != 0)
			{
				childRect.x = rect.w - child->margin.right - childRect.w;
			}
		}

		if ((child->stretch & WZ_STRETCH_HEIGHT) != 0)
		{
			// The available height is evenly divided between children that stretch vertically.
			childRect.h = (int)(availableHeight / (float)nStretchingWidgets);
		}
		else
		{
			// Don't change the height.
			childRect.h = child->rect.h;
		}

		child->setRectInternal(childRect);
		y += childRect.h + child->margin.bottom;
	}
}

void StackLayout::layoutHorizontal()
{
	int availableWidth = rect.w;

	// The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int nStretchingWidgets = 0;

	for (size_t i = 0; i < children.size(); i++)
	{
		// Subtract all child widget left and right margins from the available width.
		availableWidth -= children[i]->margin.left + children[i]->margin.right;

		// Subtract layout spacing too, except for the first child (because spacing is applied to the left).
		if (i != 0)
		{
			availableWidth -= spacing_;
		}

		if ((children[i]->stretch & WZ_STRETCH_WIDTH) != 0)
		{
			// Count the number of widgets that stretch in the same direction as the the layout, so available space can be divided evenly between them.
			nStretchingWidgets++;
		}
		else
		{
			// Subtract the widths of child widgets that aren't being stretched in the same direction as the the layout.
			availableWidth -= children[i]->rect.w;
		}
	}

	// Layout the children.
	int x = 0;

	for (size_t i = 0; i < children.size(); i++)
	{
		Widget *child = children[i];

		if (i != 0)
		{
			x += spacing_;
		}

		x += child->margin.left;

		Rect childRect;
		childRect.x = x;
		childRect.y = child->margin.top;

		if ((child->stretch & WZ_STRETCH_WIDTH) != 0)
		{
			// The available width is evenly divided between children that stretch horizontally.
			childRect.w = (int)(availableWidth / (float)nStretchingWidgets);
		}
		else
		{
			// Don't change the width.
			childRect.w = child->rect.w;
		}

		if ((child->stretch & WZ_STRETCH_HEIGHT) != 0)
		{
			// Fit the height of the layout.
			childRect.h = rect.h - (child->margin.top + child->margin.bottom);
		}
		else
		{
			// Don't change the height.
			childRect.h = child->rect.h;

			// Handle vertical alignment.
			if ((child->align & WZ_ALIGN_MIDDLE) != 0)
			{
				childRect.y = child->margin.top + (int)((rect.h - child->margin.bottom) / 2.0f - childRect.h / 2.0f);
			}
			else if ((child->align & WZ_ALIGN_BOTTOM) != 0)
			{
				childRect.y = rect.h - child->margin.bottom - childRect.h;
			}
		}

		child->setRectInternal(childRect);
		x += childRect.w + child->margin.right;
	}
}

} // namespace wz
