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
#include "wz_arr.h"
#include "wz_widget.h"

/*
================================================================================

HORIZONTAL STACK LAYOUT

================================================================================
*/

struct wzHorizontalStackLayout
{
	struct wzWidget base;
};

static void wz_horizontal_stack_layout_set_rect(struct wzWidget *widget, wzRect rect)
{
	int nChildren, availableWidth;
	int nStretchingWidgets; // The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int i, x;

	assert(widget);
	widget->rect = rect;
	nChildren = wz_arr_len(widget->children);
	availableWidth = rect.w;
	nStretchingWidgets = 0;

	for (i = 0; i < nChildren; i++)
	{
		// Subtract all child widget left and right margins from the available width.
		availableWidth -= widget->children[i]->margin.left + widget->children[i]->margin.right;

		if ((widget->children[i]->stretch & WZ_STRETCH_HORIZONTAL) != 0)
		{
			// Count the number of widgets that stretch in the same direction as the the layout, so available space can be divided evenly between them.
			nStretchingWidgets++;
		}
		else
		{
			// Subtract the widths of child widgets that aren't being stretched in the same direction as the the layout.
			availableWidth -= widget->children[i]->rect.w;
		}
	}

	// Layout the children.
	x = 0;

	for (i = 0; i < nChildren; i++)
	{
		struct wzWidget *child;
		wzRect childRect;

		child = widget->children[i];
		x += child->margin.left;
		childRect.x = x;
		childRect.y = child->margin.top;

		if ((child->stretch & WZ_STRETCH_HORIZONTAL) != 0)
		{
			// The available width is evenly divided between children that stretch horizontally.
			childRect.w = (int)(availableWidth / (float)nStretchingWidgets);
		}
		else
		{
			// Don't change the width.
			childRect.w = child->rect.w;
		}

		if ((child->stretch & WZ_STRETCH_VERTICAL) != 0)
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
				childRect.y = child->margin.top + (int)((widget->rect.h - child->margin.bottom) / 2.0f - childRect.h / 2.0f);
			}
			else if ((child->align & WZ_ALIGN_BOTTOM) != 0)
			{
				childRect.y = widget->rect.h - child->margin.bottom - childRect.h;
			}
		}

		wz_widget_set_rect(child, childRect);
		x += childRect.w + child->margin.right;
	}
}

struct wzHorizontalStackLayout *wz_horizontal_stack_layout_create(struct wzDesktop *desktop)
{
	struct wzHorizontalStackLayout *layout;

	assert(desktop);
	layout = (struct wzHorizontalStackLayout *)malloc(sizeof(struct wzHorizontalStackLayout));
	memset(layout, 0, sizeof(struct wzHorizontalStackLayout));
	layout->base.type = WZ_TYPE_HORIZONTAL_STACK_LAYOUT;
	layout->base.desktop = desktop;
	layout->base.vtable.set_rect = wz_horizontal_stack_layout_set_rect;
	return layout;
}

/*
================================================================================

VERTICAL STACK LAYOUT

================================================================================
*/

struct wzVerticalStackLayout
{
	struct wzWidget base;
};

static void wz_vertical_stack_layout_set_rect(struct wzWidget *widget, wzRect rect)
{
	int nChildren, availableHeight;
	int nStretchingWidgets; // The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int i, y;

	assert(widget);
	widget->rect = rect;
	nChildren = wz_arr_len(widget->children);
	availableHeight = rect.h;
	nStretchingWidgets = 0;

	for (i = 0; i < nChildren; i++)
	{
		// Subtract all child widget top and bottom margins from the available height.
		availableHeight -= widget->children[i]->margin.top + widget->children[i]->margin.bottom;

		if ((widget->children[i]->stretch & WZ_STRETCH_VERTICAL) != 0)
		{
			// Count the number of widgets that stretch in the same direction as the the layout, so available space can be divided evenly between them.
			nStretchingWidgets++;
		}
		else
		{
			// Subtract the heights of child widgets that aren't being stretched in the same direction as the the layout.
			availableHeight -= widget->children[i]->rect.h;
		}
	}

	// Layout the children.
	y = 0;

	for (i = 0; i < nChildren; i++)
	{
		struct wzWidget *child;
		wzRect childRect;

		child = widget->children[i];
		y += child->margin.top;
		childRect.x = child->margin.left;
		childRect.y = y;

		if ((child->stretch & WZ_STRETCH_HORIZONTAL) != 0)
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
				childRect.x = child->margin.left + (int)((widget->rect.w - child->margin.right) / 2.0f - childRect.w / 2.0f);
			}
			else if ((child->align & WZ_ALIGN_RIGHT) != 0)
			{
				childRect.x = widget->rect.w - child->margin.right - childRect.w;
			}
		}

		if ((child->stretch & WZ_STRETCH_VERTICAL) != 0)
		{
			// The available height is evenly divided between children that stretch vertically.
			childRect.h = (int)(availableHeight / (float)nStretchingWidgets);
		}
		else
		{
			// Don't change the height.
			childRect.h = child->rect.h;
		}

		wz_widget_set_rect(child, childRect);
		y += childRect.h + child->margin.bottom;
	}
}

struct wzVerticalStackLayout *wz_vertical_stack_layout_create(struct wzDesktop *desktop)
{
	struct wzVerticalStackLayout *layout;

	assert(desktop);
	layout = (struct wzVerticalStackLayout *)malloc(sizeof(struct wzVerticalStackLayout));
	memset(layout, 0, sizeof(struct wzVerticalStackLayout));
	layout->base.type = WZ_TYPE_VERTICAL_STACK_LAYOUT;
	layout->base.desktop = desktop;
	layout->base.vtable.set_rect = wz_vertical_stack_layout_set_rect;
	return layout;
}
