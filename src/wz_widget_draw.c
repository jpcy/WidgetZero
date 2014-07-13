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
#include "wz_desktop.h"
#include "wz_widget.h"
#include "wz_window.h"

typedef bool (*wzWidgetPredicate)(const struct wzWidget *);

/*
================================================================================

DRAW PRIORITY

================================================================================
*/

static void wz_widget_calculate_unique_draw_priorities_recursive(int *drawPriorities, int *nDrawPriorities, struct wzWidget *widget, wzWidgetPredicate add_draw_priority_predicate, wzWidgetPredicate recurse_predicate)
{
	int i;

	WZ_ASSERT(widget);

	if (!wz_widget_get_visible(widget))
		return;

	// Don't include the widget if it's outside its parent window.
	if (!wz_widget_overlaps_parent_window(widget))
		return;

	// Add the draw priority if it doesn't exist.
	if (add_draw_priority_predicate(widget))
	{
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
	}

	if (!recurse_predicate(widget))
		return;

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_calculate_unique_draw_priorities_recursive(drawPriorities, nDrawPriorities, widget->children[i], add_draw_priority_predicate, recurse_predicate);
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
	WZ_ASSERT(draw_priority_compare);
	WZ_ASSERT(widget);
	WZ_ASSERT(clip);

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

static void wz_widget_draw_by_less_than_or_equals_priority_recursive(int priority, struct wzWidget *widget, wzRect clip, wzWidgetPredicate recurse_predicate)
{
	int i;

	WZ_ASSERT(widget);

	if (!wz_widget_draw_internal(priority, wz_draw_priority_less_than_or_equals, widget, &clip))
		return;

	if (!recurse_predicate(widget))
		return;

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		wz_widget_draw_by_less_than_or_equals_priority_recursive(priority, widget->children[i], clip, recurse_predicate);
	}
}

static void wz_widget_draw_by_priority_recursive(int priority, struct wzWidget *widget, wzRect clip, wzWidgetPredicate recurse_predicate)
{
	int i;

	WZ_ASSERT(widget);

	if (!wz_widget_draw_internal(priority, wz_draw_priority_equals, widget, &clip))
		return;

	if (!recurse_predicate(widget))
		return;

	for (i = 0; i < wz_arr_len(widget->children); i++)
	{
		// If the priority is a match, draw children with <= priority.
		if (widget->drawPriority == priority)
		{
			wz_widget_draw_by_less_than_or_equals_priority_recursive(priority, widget->children[i], clip, recurse_predicate);
		}
		else
		{
			wz_widget_draw_by_priority_recursive(priority, widget->children[i], clip, recurse_predicate);
		}
	}
}

static void wz_widget_draw(struct wzWidget *widget, wzWidgetPredicate add_draw_priority_predicate, wzWidgetPredicate recurse_predicate)
{
	int drawPriorities[WZ_DRAW_PRIORITY_MAX];
	int nDrawPriorities;
	wzRect clip;
	int i;

	WZ_ASSERT(widget);

	// Calculate unique draw priorities.
	nDrawPriorities = 0;
	wz_widget_calculate_unique_draw_priorities_recursive(drawPriorities, &nDrawPriorities, widget, add_draw_priority_predicate, recurse_predicate);

	// Sort draw priorities in ascending order.
	qsort(drawPriorities, nDrawPriorities, sizeof(int), wz_compare_draw_priorities);

	// Do one draw pass per draw priority.
	clip = wz_widget_get_rect(widget);

	for (i = 0; i < nDrawPriorities; i++)
	{
		wz_widget_draw_by_priority_recursive(drawPriorities[i], widget, clip, recurse_predicate);
	}
}

static bool wz_widget_draw_priority_less_than_window(const struct wzWidget *widget)
{
	return widget->drawPriority < WZ_DRAW_PRIORITY_WINDOW;
}

static bool wz_widget_is_not_window(const struct wzWidget *widget)
{
	return widget->type != WZ_TYPE_WINDOW;
}

static bool wz_widget_draw_priority_greater_than_window(const struct wzWidget *widget)
{
	return widget->drawPriority > WZ_DRAW_PRIORITY_WINDOW;
}

static bool wz_widget_true(const struct wzWidget *widget)
{
	widget = widget;
	return true;
}

static int wz_compare_window_draw_priorities(const void *a, const void *b)
{
	const struct wzWindow *window1, *window2;
	bool window1Docked, window2Docked;

	window1 = *((const struct wzWindow **)a);
	window2 = *((const struct wzWindow **)b);
	window1Docked = wz_desktop_get_window_dock_position(((const struct wzWidget *)window1)->desktop, window1) != WZ_DOCK_POSITION_NONE;
	window2Docked = wz_desktop_get_window_dock_position(((const struct wzWidget *)window2)->desktop, window2) != WZ_DOCK_POSITION_NONE;

	if (window1Docked && !window2Docked)
	{
		return -1;
	}
	else if (window2Docked && !window1Docked)
	{
		return 1;
	}

	return wz_window_get_draw_priority(window1) - wz_window_get_draw_priority(window2);
}

void wz_widget_draw_desktop(struct wzDesktop *desktop)
{
	struct wzWindow *windows[WZ_MAX_WINDOWS];
	int nWindows;
	int i;

	WZ_ASSERT(desktop);

	// Draw the desktop and ancestors with draw priority < window. Don't recurse into windows.
	wz_widget_draw((struct wzWidget *)desktop, wz_widget_draw_priority_less_than_window, wz_widget_is_not_window);

	// Get a list of windows (excluding top).
	nWindows = 0;

	for (i = 0; i < wz_arr_len(((struct wzWidget *)desktop)->children); i++)
	{
		struct wzWidget *widget = ((struct wzWidget *)desktop)->children[i];
	
		if (widget->type == WZ_TYPE_WINDOW)
		{
			windows[nWindows] = (struct wzWindow *)widget;
			nWindows++;
		}
	}

	// Sort them in ascending order by draw priority.
	qsort(windows, nWindows, sizeof(struct wzWindow *), wz_compare_window_draw_priorities);

	// For each window, draw the window and all ancestors with draw priority < window.
	for (i = 0; i < nWindows; i++)
	{
		struct wzWidget *widget = (struct wzWidget *)windows[i];

		if (wz_widget_get_visible(widget) && widget->vtable.draw)
		{
			widget->vtable.draw(widget, ((struct wzWidget *)desktop)->rect);
		}

		wz_widget_draw(widget, wz_widget_draw_priority_less_than_window, wz_widget_true);
	}

	// Draw all widgets with draw priority > window.
	wz_widget_draw((struct wzWidget *)desktop, wz_widget_draw_priority_greater_than_window, wz_widget_true);
}

