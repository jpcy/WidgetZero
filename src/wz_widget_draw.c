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
#include <assert.h>
#include "wz_internal.h"

/*
================================================================================

DRAW PRIORITY

================================================================================
*/

static void wz_widget_calculate_unique_draw_priorities_recursive(int *drawPriorities, int *nDrawPriorities, struct wzWidget *widget)
{
	int i;

	assert(widget);

	if (!wz_widget_get_visible(widget))
		return;

	// Don't include the widget if it's outside its parent window.
	if (!wz_widget_overlaps_parent_window(widget))
		return;

	// Add the draw priority if it doesn't exist.
	for (i = 0; i < *nDrawPriorities; i++)
	{
		if (drawPriorities[i] == widget->drawPriority)
			break;
	}

	if (i == *nDrawPriorities)
	{
		drawPriorities[*nDrawPriorities] = widget->drawPriority;
		(*nDrawPriorities)++;
	}

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_calculate_unique_draw_priorities_recursive(drawPriorities, nDrawPriorities, widget->children[i]);
	}
}

static int wz_compare_draw_priorities(const void *a, const void *b)
{
	return *(const int *)a - *(const int *)b;
}

static bool wz_draw_priority_less_than_or_equals(int widgetDrawPriority, int drawPriority)
{
	return widgetDrawPriority <= drawPriority;
}

static bool wz_draw_priority_equals(int widgetDrawPriority, int drawPriority)
{
	return widgetDrawPriority == drawPriority;
}

/*
================================================================================

DRAWING

================================================================================
*/

static bool wz_widget_draw_internal(int priority, bool (*draw_priority_compare)(int, int), struct wzWidget *widget, wzRect *clip)
{
	assert(draw_priority_compare);
	assert(widget);
	assert(clip);

	if (!wz_widget_get_visible(widget))
		return false;

	// Don't render the widget if it's outside its parent window.
	if (!wz_widget_overlaps_parent_window(widget))
		return false;

	if (draw_priority_compare(widget->drawPriority, priority) && widget->vtable.draw)
	{
		widget->vtable.draw(widget, *clip);
	}

	// Update clip rect.
	if (widget->vtable.get_children_clip_rect)
	{
		if (!wz_intersect_rects(*clip, widget->vtable.get_children_clip_rect(widget), clip))
		{
			// Reset to desktop clip rect.
			*clip = wz_widget_get_rect((struct wzWidget *)widget->desktop);
		}
	}

	return true;
}

static void wz_widget_draw_by_less_than_or_equals_priority_recursive(int priority, struct wzWidget *widget, wzRect clip)
{
	int i;

	assert(widget);

	if (!wz_widget_draw_internal(priority, wz_draw_priority_less_than_or_equals, widget, &clip))
		return;

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_draw_by_less_than_or_equals_priority_recursive(priority, widget->children[i], clip);
	}
}

static void wz_widget_draw_by_priority_recursive(int priority, struct wzWidget *widget, wzRect clip)
{
	int i;

	assert(widget);

	if (!wz_widget_draw_internal(priority, wz_draw_priority_equals, widget, &clip))
		return;

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		// If the priority is a match, draw children with <= priority.
		if (widget->drawPriority == priority)
		{
			wz_widget_draw_by_less_than_or_equals_priority_recursive(priority, widget->children[i], clip);
		}
		else
		{
			wz_widget_draw_by_priority_recursive(priority, widget->children[i], clip);
		}
	}
}

void wz_widget_draw_desktop(struct wzDesktop *desktop)
{
	int drawPriorities[WZ_DRAW_PRIORITY_MAX];
	int nDrawPriorities;
	wzRect clip;
	int i;

	assert(desktop);

	// Calculate unique draw priorities.
	nDrawPriorities = 0;
	wz_widget_calculate_unique_draw_priorities_recursive(drawPriorities, &nDrawPriorities, (struct wzWidget *)desktop);

	// Sort draw priorities in ascending order.
	qsort(drawPriorities, nDrawPriorities, sizeof(int), wz_compare_draw_priorities);

	// Do one draw pass per draw priority.
	clip = wz_widget_get_rect((struct wzWidget *)desktop);

	for (i = 0; i < nDrawPriorities; i++)
	{
		wz_widget_draw_by_priority_recursive(drawPriorities[i], (struct wzWidget *)desktop, clip);
	}
}
