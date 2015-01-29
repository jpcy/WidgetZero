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
#include "wz_pch.h"
#pragma hdrstop

namespace wz {

/*
================================================================================

VERTICAL STACK LAYOUT

================================================================================
*/

static void wz_vertical_stack_layout_set_rect(struct WidgetImpl *widget, Rect rect)
{
	struct StackLayoutImpl *stackLayout;
	int availableHeight;
	int nStretchingWidgets; // The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int y;

	WZ_ASSERT(widget);
	stackLayout = (struct StackLayoutImpl *)widget;
	widget->rect = rect;
	availableHeight = rect.h;
	nStretchingWidgets = 0;

	for (size_t i = 0; i < widget->children.size(); i++)
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

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		struct WidgetImpl *child;
		Rect childRect;

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

static void wz_horizontal_stack_layout_set_rect(struct WidgetImpl *widget, Rect rect)
{
	struct StackLayoutImpl *stackLayout;
	int availableWidth;
	int nStretchingWidgets; // The number of widgets that stretch in the same direction as the the layout. Available space is divided evenly between them.
	int x;

	WZ_ASSERT(widget);
	stackLayout = (struct StackLayoutImpl *)widget;
	widget->rect = rect;
	availableWidth = rect.w;
	nStretchingWidgets = 0;

	for (size_t i = 0; i < widget->children.size(); i++)
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

	for (size_t i = 0; i < widget->children.size(); i++)
	{
		struct WidgetImpl *child;
		Rect childRect;

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

static void wz_stack_layout_set_rect(struct WidgetImpl *widget, Rect rect)
{
	WZ_ASSERT(widget);

	if (((struct StackLayoutImpl *)widget)->direction == WZ_STACK_LAYOUT_VERTICAL)
	{
		wz_vertical_stack_layout_set_rect(widget, rect);
	}
	else
	{
		wz_horizontal_stack_layout_set_rect(widget, rect);
	}
}

StackLayoutImpl::StackLayoutImpl()
{
	direction = WZ_STACK_LAYOUT_VERTICAL;
	spacing = 0;
}

StackLayout::StackLayout()
{
	impl = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 0);
}

StackLayout::StackLayout(StackLayoutDirection direction)
{
	impl = wz_stack_layout_create(direction, 0);
}

StackLayout::~StackLayout()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

StackLayout *StackLayout::setDirection(StackLayoutDirection direction)
{
	wz_stack_layout_set_direction((StackLayoutImpl *)impl, direction);
	return this;
}

StackLayout *StackLayout::setSpacing(int spacing)
{
	wz_stack_layout_set_spacing((StackLayoutImpl *)impl, spacing);
	return this;
}

int StackLayout::getSpacing() const
{
	return wz_stack_layout_get_spacing((StackLayoutImpl *)impl);
}

Widget *StackLayout::add(Widget *widget)
{
	wz_stack_layout_add((StackLayoutImpl *)impl, widget->impl);
	return widget;
}

void StackLayout::remove(Widget *widget)
{
	wz_stack_layout_remove((StackLayoutImpl *)impl, widget->impl);
}

struct StackLayoutImpl *wz_stack_layout_create(StackLayoutDirection direction, int spacing)
{
	struct StackLayoutImpl *stackLayout;

	stackLayout = new struct StackLayoutImpl;
	stackLayout->type = WZ_TYPE_STACK_LAYOUT;
	stackLayout->vtable.set_rect = wz_stack_layout_set_rect;
	stackLayout->direction = direction;
	stackLayout->spacing = spacing;
	return stackLayout;
}

void wz_stack_layout_set_direction(struct StackLayoutImpl *stackLayout, StackLayoutDirection direction)
{
	WZ_ASSERT(stackLayout);
	stackLayout->direction = direction;
	wz_widget_refresh_rect((struct WidgetImpl *)stackLayout);
}

void wz_stack_layout_set_spacing(struct StackLayoutImpl *stackLayout, int spacing)
{
	WZ_ASSERT(stackLayout);
	stackLayout->spacing = spacing;
	wz_widget_refresh_rect((struct WidgetImpl *)stackLayout);
}

int wz_stack_layout_get_spacing(const struct StackLayoutImpl *stackLayout)
{
	WZ_ASSERT(stackLayout);
	return stackLayout->spacing;
}

void wz_stack_layout_add(struct StackLayoutImpl *stackLayout, struct WidgetImpl *widget)
{
	WZ_ASSERT(stackLayout);
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	wz_widget_add_child_widget((struct WidgetImpl *)stackLayout, widget);
}

void wz_stack_layout_remove(struct StackLayoutImpl *stackLayout, struct WidgetImpl *widget)
{
	WZ_ASSERT(stackLayout);
	WZ_ASSERT(widget);
	wz_widget_remove_child_widget((struct WidgetImpl *)stackLayout, widget);
}

/*
================================================================================

ALIGNMENT AND STRETCHING (OUTSIDE LAYOUT)

================================================================================
*/

Rect wz_widget_calculate_aligned_stretched_rect(const struct WidgetImpl *widget, Rect rect)
{
	Rect parentRect;

	WZ_ASSERT(widget);

	// Can't align or stretch to parent rect if there is no parent.
	if (!widget->parent)
		return rect;

	// Don't align or stretch if the widget is a child of a layout. The layout will handle the logic in that case.
	if (widget->parent && wz_widget_is_layout(widget->parent))
		return rect;

	parentRect = wz_widget_get_rect(widget->parent);

	// Handle stretching.
	if ((widget->stretch & WZ_STRETCH_WIDTH) != 0)
	{
		const float scale = (widget->stretchWidthScale < 0.01f) ? 1 : widget->stretchWidthScale;

		rect.x = widget->margin.left;
		rect.w = (int)(parentRect.w * scale) - (widget->margin.left + widget->margin.right);
	}

	if ((widget->stretch & WZ_STRETCH_HEIGHT) != 0)
	{
		const float scale = (widget->stretchHeightScale < 0.01f) ? 1 : widget->stretchHeightScale;

		rect.y = widget->margin.top;
		rect.h = (int)(parentRect.h * scale) - (widget->margin.top + widget->margin.bottom);
	}

	// Handle horizontal alignment.
	if ((widget->align & WZ_ALIGN_LEFT) != 0)
	{
		rect.x = widget->margin.left;
	}
	else if ((widget->align & WZ_ALIGN_CENTER) != 0)
	{
		rect.x = widget->margin.left + (int)((parentRect.w - widget->margin.right) / 2.0f - rect.w / 2.0f);
	}
	else if ((widget->align & WZ_ALIGN_RIGHT) != 0)
	{
		rect.x = parentRect.w - widget->margin.right - rect.w;
	}

	// Handle vertical alignment.
	if ((widget->align & WZ_ALIGN_TOP) != 0)
	{
		rect.y = widget->margin.top;
	}
	else if ((widget->align & WZ_ALIGN_MIDDLE) != 0)
	{
		rect.y = widget->margin.top + (int)((parentRect.h - widget->margin.bottom) / 2.0f - rect.h / 2.0f);
	}
	else if ((widget->align & WZ_ALIGN_BOTTOM) != 0)
	{
		rect.y = parentRect.h - widget->margin.bottom - rect.h;
	}

	return rect;
}

} // namespace wz
