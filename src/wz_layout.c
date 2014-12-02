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
#include "wz_arr.h"
#include "wz_widget.h"

struct wzStackLayout
{
	struct wzWidget base;
	wzStackLayoutDirection direction;

	// Spacing between child widgets. Applied to the top/left of children.
	int spacing;
};

/*
================================================================================

VERTICAL STACK LAYOUT

================================================================================
*/

static void wz_vertical_stack_layout_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzStackLayout *stackLayout;
	int nChildren, availableHeight;
	int nStretchingWidgets; // The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int i, y;

	WZ_ASSERT(widget);
	stackLayout = (struct wzStackLayout *)widget;
	widget->rect = rect;
	nChildren = wz_arr_len(widget->children);
	availableHeight = rect.h;
	nStretchingWidgets = 0;

	for (i = 0; i < nChildren; i++)
	{
		// Subtract all child widget top and bottom margins from the available height.
		availableHeight -= widget->children[i]->margin.top + widget->children[i]->margin.bottom;

		// Subtract layout spacing too, except for the first child (because spacing is applied to the top).
		if (i != 0)
		{
			availableHeight -= stackLayout->spacing;
		}

		if ((widget->children[i]->stretch & WZ_STRETCH_HEIGHT) != 0)
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

		if (i != 0)
		{
			y += stackLayout->spacing;
		}

		y += child->margin.top;
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
				childRect.x = child->margin.left + (int)((widget->rect.w - child->margin.right) / 2.0f - childRect.w / 2.0f);
			}
			else if ((child->align & WZ_ALIGN_RIGHT) != 0)
			{
				childRect.x = widget->rect.w - child->margin.right - childRect.w;
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

		wz_widget_set_rect_internal(child, childRect);
		y += childRect.h + child->margin.bottom;
	}
}

/*
================================================================================

HORIZONTAL STACK LAYOUT

================================================================================
*/

static void wz_horizontal_stack_layout_set_rect(struct wzWidget *widget, wzRect rect)
{
	struct wzStackLayout *stackLayout;
	int nChildren, availableWidth;
	int nStretchingWidgets; // The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int i, x;

	WZ_ASSERT(widget);
	stackLayout = (struct wzStackLayout *)widget;
	widget->rect = rect;
	nChildren = wz_arr_len(widget->children);
	availableWidth = rect.w;
	nStretchingWidgets = 0;

	for (i = 0; i < nChildren; i++)
	{
		// Subtract all child widget left and right margins from the available width.
		availableWidth -= widget->children[i]->margin.left + widget->children[i]->margin.right;

		// Subtract layout spacing too, except for the first child (because spacing is applied to the left).
		if (i != 0)
		{
			availableWidth -= stackLayout->spacing;
		}

		if ((widget->children[i]->stretch & WZ_STRETCH_WIDTH) != 0)
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

		if (i != 0)
		{
			x += stackLayout->spacing;
		}

		x += child->margin.left;
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
				childRect.y = child->margin.top + (int)((widget->rect.h - child->margin.bottom) / 2.0f - childRect.h / 2.0f);
			}
			else if ((child->align & WZ_ALIGN_BOTTOM) != 0)
			{
				childRect.y = widget->rect.h - child->margin.bottom - childRect.h;
			}
		}

		wz_widget_set_rect_internal(child, childRect);
		x += childRect.w + child->margin.right;
	}
}

/*
================================================================================

STACK LAYOUT

================================================================================
*/

static void wz_stack_layout_set_rect(struct wzWidget *widget, wzRect rect)
{
	WZ_ASSERT(widget);

	if (((struct wzStackLayout *)widget)->direction == WZ_STACK_LAYOUT_VERTICAL)
	{
		wz_vertical_stack_layout_set_rect(widget, rect);
	}
	else
	{
		wz_horizontal_stack_layout_set_rect(widget, rect);
	}
}

struct wzStackLayout *wz_stack_layout_create(wzStackLayoutDirection direction, int spacing)
{
	struct wzStackLayout *stackLayout;

	stackLayout = (struct wzStackLayout *)malloc(sizeof(struct wzStackLayout));
	memset(stackLayout, 0, sizeof(struct wzStackLayout));
	stackLayout->base.type = WZ_TYPE_STACK_LAYOUT;
	stackLayout->base.vtable.set_rect = wz_stack_layout_set_rect;
	stackLayout->direction = direction;
	stackLayout->spacing = spacing;
	return stackLayout;
}

void wz_stack_layout_set_direction(struct wzStackLayout *stackLayout, wzStackLayoutDirection direction)
{
	WZ_ASSERT(stackLayout);
	stackLayout->direction = direction;
	wz_widget_refresh_rect((struct wzWidget *)stackLayout);
}

void wz_stack_layout_set_spacing(struct wzStackLayout *stackLayout, int spacing)
{
	WZ_ASSERT(stackLayout);
	stackLayout->spacing = spacing;
	wz_widget_refresh_rect((struct wzWidget *)stackLayout);
}

int wz_stack_layout_get_spacing(const struct wzStackLayout *stackLayout)
{
	WZ_ASSERT(stackLayout);
	return stackLayout->spacing;
}

void wz_stack_layout_add(struct wzStackLayout *stackLayout, struct wzWidget *widget)
{
	WZ_ASSERT(stackLayout);
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	wz_widget_add_child_widget((struct wzWidget *)stackLayout, widget);
}

void wz_stack_layout_remove(struct wzStackLayout *stackLayout, struct wzWidget *widget)
{
	WZ_ASSERT(stackLayout);
	WZ_ASSERT(widget);
	wz_widget_remove_child_widget((struct wzWidget *)stackLayout, widget);
}

/*
================================================================================

STRETCHING (OUTSIDE LAYOUT)

================================================================================
*/

// Applies stretching to the provided rect.
wzRect wz_widget_calculate_stretched_rect(const struct wzWidget *widget, wzRect rect)
{
	WZ_ASSERT(widget);

	// Don't stretch if the widget is a child of a layout. The layout will handle stretching logic in that case.
	if (widget->parent && !wz_widget_is_layout(widget->parent) && (widget->stretch & WZ_STRETCH) != 0)
	{
		wzSize parentSize = wz_widget_get_size(widget->parent);

		if ((widget->stretch & WZ_STRETCH_WIDTH) != 0)
		{
			rect.x = widget->margin.left;
			rect.w = parentSize.w - (widget->margin.left + widget->margin.right);
		}

		if ((widget->stretch & WZ_STRETCH_HEIGHT) != 0)
		{
			rect.y = widget->margin.top;
			rect.h = parentSize.h - (widget->margin.top + widget->margin.bottom);
		}
	}

	return rect;
}
