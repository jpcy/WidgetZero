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
#include "stb_arr.h"
#include "wz_internal.h"

#define WZ_NUM_DOCK_ICONS 4
#define WZ_DOCK_ICON_NORTH 0
#define WZ_DOCK_ICON_SOUTH 1
#define WZ_DOCK_ICON_EAST 2
#define WZ_DOCK_ICON_WEST 3

struct wzDesktop
{
	struct wzWidget base;

	struct wzWidget **lockInputWidgetStack;

	// Lock input to this window, i.e. don't call mouse_move, mouse_button_down or mouse_button_up on any widget that isn't this window or it's descendants.
	struct wzWindow *lockInputWindow;

	// This window is currently being moved and may be docked.
	struct wzWindow *movingWindow;

	wzDesktopDrawDockIconCallback draw_dock_icon;
	wzDesktopDrawDockPreviewCallback draw_dock_preview;

	// Hidden from the consumer.
	struct wzLabel *dockIcons[WZ_NUM_DOCK_ICONS];
	struct wzLabel *dockPreview;

	wzDock windowDock;
};

static void wz_desktop_draw_dock_icon(struct wzWidget *widget)
{
	assert(widget);

	if (widget->desktop->draw_dock_icon)
	{
		int i;

		for (i = 0; i < WZ_NUM_DOCK_ICONS; i++)
		{
			widget->desktop->draw_dock_icon(wz_widget_get_rect((struct wzWidget *)widget->desktop->dockIcons[i]), wz_widget_get_metadata((struct wzWidget *)widget->desktop->dockIcons[i]));
		}
	}
}

static void wz_desktop_update_dock_icon_positions(struct wzDesktop *desktop)
{
	// Push icons out this percent/100 from desktop edges.
	const float percent = 0.04f;
	wzSize ds, dis;
	int centerW, centerH;

	assert(desktop);
	ds = wz_widget_get_size((struct wzWidget *)desktop);
	dis = wz_widget_get_size((struct wzWidget *)desktop->dockIcons[WZ_DOCK_ICON_NORTH]);
	centerW = (int)(ds.w / 2.0f - dis.w / 2.0f);
	centerH = (int)(ds.h / 2.0f - dis.h / 2.0f);

	wz_widget_set_position_args((struct wzWidget *)desktop->dockIcons[WZ_DOCK_ICON_NORTH], centerW, (int)(ds.h * percent));
	wz_widget_set_position_args((struct wzWidget *)desktop->dockIcons[WZ_DOCK_ICON_SOUTH], centerW, (int)(ds.h * (1.0f - percent) - dis.h));
	wz_widget_set_position_args((struct wzWidget *)desktop->dockIcons[WZ_DOCK_ICON_EAST], (int)(ds.w * (1.0f - percent) - dis.h), centerH);
	wz_widget_set_position_args((struct wzWidget *)desktop->dockIcons[WZ_DOCK_ICON_WEST], (int)(ds.w * percent), centerH);
}

static void wz_desktop_draw_dock_preview(struct wzWidget *widget)
{
	assert(widget);

	if (widget->desktop->draw_dock_preview)
	{
		widget->desktop->draw_dock_preview(wz_widget_get_rect((struct wzWidget *)widget->desktop->dockPreview), wz_widget_get_metadata((struct wzWidget *)widget->desktop->dockPreview));
	}
}

static void wz_widget_update_dock_preview_rect(struct wzDesktop *desktop, int index)
{
	wzRect rect;
	wzSize windowSize;

	// e.g. north dock max height is desktop height * maxPreviewSizeMultiplier.
	const float maxPreviewSizeMultiplier = 0.3f;

	assert(desktop);
	assert(desktop->movingWindow);

	// Use the window width for east/west or height for north/south, but don't go over half of the desktop width/height.
	windowSize = wz_widget_get_size((struct wzWidget *)desktop->movingWindow);

	if (index == WZ_DOCK_ICON_NORTH)
	{
		rect.x = 0;
		rect.y = 0;
		rect.w = desktop->base.rect.w;
		rect.h = WZ_MIN(windowSize.h, (int)(desktop->base.rect.h * maxPreviewSizeMultiplier));
	}
	else if (index == WZ_DOCK_ICON_SOUTH)
	{
		const int h = WZ_MIN(windowSize.h, (int)(desktop->base.rect.h * maxPreviewSizeMultiplier));
		rect.x = 0;
		rect.y = desktop->base.rect.h - h;
		rect.w = desktop->base.rect.w;
		rect.h = h;
	}
	else if (index == WZ_DOCK_ICON_EAST)
	{
		const int w = WZ_MIN(windowSize.w, (int)(desktop->base.rect.w * maxPreviewSizeMultiplier));
		rect.x = desktop->base.rect.w - w;
		rect.y = 0;
		rect.w = w;
		rect.h = desktop->base.rect.h;
	}
	else if (index == WZ_DOCK_ICON_WEST)
	{
		rect.x = 0;
		rect.y = 0;
		rect.w = WZ_MIN(windowSize.w, (int)(desktop->base.rect.w * maxPreviewSizeMultiplier));
		rect.h = desktop->base.rect.h;
	}

	wz_widget_set_rect((struct wzWidget *)desktop->dockPreview, rect);
}

static void wz_desktop_update_desktop_preview_visible(struct wzDesktop *desktop, int mouseX, int mouseY)
{
	int i;
	bool showDockPreview = false;

	assert(desktop);

	if (!desktop->movingWindow)
		return;

	for (i = 0; i < WZ_NUM_DOCK_ICONS; i++)
	{
		wzRect rect = wz_widget_get_rect((struct wzWidget *)desktop->dockIcons[i]);

		if (WZ_POINT_IN_RECT(mouseX, mouseY, rect))
		{
			desktop->windowDock = i + 1;
			wz_widget_update_dock_preview_rect(desktop, i);
			showDockPreview = true;
			break;
		}
	}

	wz_widget_set_visible((struct wzWidget *)desktop->dockPreview, showDockPreview);
}

struct wzDesktop *wz_desktop_create(struct wzContext *context)
{
	struct wzDesktop *desktop;
	int i;
	struct wzWidget *widget;

	assert(context);
	desktop = (struct wzDesktop *)malloc(sizeof(struct wzDesktop));
	memset(desktop, 0, sizeof(struct wzDesktop));
	desktop->base.type = WZ_TYPE_DESKTOP;
	desktop->base.context = context;

	// Create dock icon widgets.
	for (i = 0; i < WZ_NUM_DOCK_ICONS; i++)
	{
		desktop->dockIcons[i] = wz_label_create(context);
		widget = (struct wzWidget *)desktop->dockIcons[i];
		wz_widget_set_draw_priority(widget, WZ_DRAW_PRIORITY_DOCK_ICON);
		wz_widget_set_draw_function(widget, wz_desktop_draw_dock_icon);
		wz_widget_set_visible(widget, false);
		wz_widget_add_child_widget((struct wzWidget *)desktop, widget);
	}

	wz_desktop_set_dock_icon_size_args(desktop, 48, 48);
	wz_desktop_update_dock_icon_positions(desktop);

	// Create dock preview widget.
	desktop->dockPreview = wz_label_create(context);
	widget = (struct wzWidget *)desktop->dockPreview;
	wz_widget_set_draw_priority(widget, WZ_DRAW_PRIORITY_DOCK_PREVIEW);
	wz_widget_set_draw_function(widget, wz_desktop_draw_dock_preview);
	wz_widget_set_visible(widget, false);
	wz_widget_add_child_widget((struct wzWidget *)desktop, widget);

	return desktop;
}

void wz_desktop_set_draw_dock_icon_callback(struct wzDesktop *desktop, wzDesktopDrawDockIconCallback callback, void *metadata)
{
	int i;

	assert(desktop);
	desktop->draw_dock_icon = callback;

	for (i = 0; i < WZ_NUM_DOCK_ICONS; i++)
	{
		wz_widget_set_metadata((struct wzWidget *)desktop->dockIcons[i], metadata);
	}
}

void wz_desktop_set_draw_dock_preview_callback(struct wzDesktop *desktop, wzDesktopDrawDockPreviewCallback callback, void *metadata)
{
	assert(desktop);
	desktop->draw_dock_preview = callback;
	wz_widget_set_metadata((struct wzWidget *)desktop->dockPreview, metadata);
}

void wz_desktop_set_dock_icon_size(struct wzDesktop *desktop, wzSize size)
{
	wz_desktop_set_dock_icon_size_args(desktop, size.w, size.h);
}

void wz_desktop_set_dock_icon_size_args(struct wzDesktop *desktop, int w, int h)
{
	int i;

	assert(desktop);

	for (i = 0; i < WZ_NUM_DOCK_ICONS; i++)
	{
		wz_widget_set_size_args((struct wzWidget *)desktop->dockIcons[i], w, h);
	}

	wz_desktop_update_dock_icon_positions(desktop);
}

void wz_desktop_set_size(struct wzDesktop *desktop, wzSize size)
{
	assert(desktop);
	desktop->base.rect.w = size.w;
	desktop->base.rect.h = size.h;
	wz_desktop_update_dock_icon_positions(desktop);
}

void wz_desktop_set_size_args(struct wzDesktop *desktop, int width, int height)
{
	assert(desktop);
	desktop->base.rect.w = width;
	desktop->base.rect.h = height;
	wz_desktop_update_dock_icon_positions(desktop);
}

wzSize wz_desktop_get_size(const struct wzDesktop *desktop)
{
	wzSize size;

	assert(desktop);
	size.w = desktop->base.rect.w;
	size.h = desktop->base.rect.h;
	return size;
}

// Returns the window that the mouse cursor is hovering over. NULL if there isn't one.
static struct wzWindow *wz_desktop_get_hover_window(struct wzDesktop *desktop, int mouseX, int mouseY)
{
	struct wzWidget *widget;
	struct wzWindow *result;
	int drawPriority;

	assert(desktop);
	widget = desktop->base.firstChild;
	result = NULL;
	drawPriority = WZ_DRAW_PRIORITY_WINDOW_START;

	while (widget)
	{
		if (widget->type == WZ_TYPE_WINDOW && WZ_POINT_IN_RECT(mouseX, mouseY, widget->rect) && widget->drawPriority >= drawPriority)
		{
			drawPriority = widget->drawPriority;
			result = (struct wzWindow *)widget;
		}

		widget = widget->next == desktop->base.firstChild ? NULL : widget->next;
	}

	return result;
}

static int wz_compare_widget_draw_priorities(const void *a, const void *b)
{
	return ((struct wzWidget *)a)->drawPriority - ((struct wzWidget *)b)->drawPriority;
}

// top can be NULL
static void wz_desktop_update_window_draw_priorities(struct wzDesktop *desktop, struct wzWindow *top)
{
	struct wzWidget *widget;
	struct wzWidget *windows[WZ_DRAW_PRIORITY_WINDOW_END];
	int nWindows;
	int i;

	assert(desktop);

	// Get a list of windows (excluding top).
	widget = desktop->base.firstChild;
	nWindows = 0;

	while (widget)
	{
		if (widget->type == WZ_TYPE_WINDOW && widget != (struct wzWidget *)top)
		{
			windows[nWindows] = widget;
			nWindows++;
		}

		widget = widget->next == desktop->base.firstChild ? NULL : widget->next;
	}

	// Sort them in ascending order by draw priority.
	qsort(windows, nWindows, sizeof(struct wzWindow *), wz_compare_widget_draw_priorities);

	// Assign each window a new draw priority starting at WZ_DRAW_PRIORITY_WINDOW_START and ascending.
	for (i = 0; i < nWindows; i++)
	{
		windows[i]->drawPriority = WZ_DRAW_PRIORITY_WINDOW_START + i;
	}

	// Give the top window the highest priority.
	if (top)
	{
		((struct wzWidget *)top)->drawPriority = WZ_DRAW_PRIORITY_WINDOW_START + i;
	}
}

static void wz_widget_mouse_button_down_recursive(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->vtable.mouse_button_down)
	{
		widget->vtable.mouse_button_down(widget, mouseButton, mouseX, mouseY);
	}

	child = widget->firstChild;

	while (child)
	{
		if (child->hover)
		{
			wz_widget_mouse_button_down_recursive(child, mouseButton, mouseX, mouseY);
		}

		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_mouse_button_down(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	assert(desktop);

	desktop->lockInputWindow = wz_desktop_get_hover_window(desktop, mouseX, mouseY);

	if (stb_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		wz_desktop_update_window_draw_priorities(desktop, desktop->lockInputWindow);
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_mouse_button_down_recursive(widget, mouseButton, mouseX, mouseY);

	// Need a special case for dock icons.
	wz_desktop_update_desktop_preview_visible(desktop, mouseX, mouseY);
}

static void wz_widget_mouse_button_up_recursive(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->vtable.mouse_button_up)
	{
		widget->vtable.mouse_button_up(widget, mouseButton, mouseX, mouseY);
	}

	child = widget->firstChild;

	while (child)
	{
		wz_widget_mouse_button_up_recursive(child, mouseButton, mouseX, mouseY);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_mouse_button_up(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY)
{
	struct wzWidget *widget;

	assert(desktop);

	// Need a special case for dock icons.
	if (desktop->movingWindow)
	{
		if (wz_widget_get_visible((struct wzWidget *)desktop->dockPreview))
		{
			// Dock the window.
			wz_widget_set_rect((struct wzWidget *)desktop->movingWindow, wz_widget_get_rect((struct wzWidget *)desktop->dockPreview));
			wz_window_set_dock(desktop->movingWindow, desktop->windowDock);
		}

		wz_widget_set_visible((struct wzWidget *)desktop->dockPreview, false);
	}

	if (stb_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_mouse_button_up_recursive(widget, mouseButton, mouseX, mouseY);
}

// If window is not NULL, only call mouse_move in widgets that are children of the window and the window itself.
static void wz_widget_mouse_move_recursive(struct wzWindow *window, struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	wzRect rect;
	bool widgetIsChildOfWindow;
	bool oldHover;
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	rect = wz_widget_get_absolute_rect(widget);
	widgetIsChildOfWindow = !window || (window && widget->window == window);
	oldHover = widget->hover;
	widget->hover = widgetIsChildOfWindow && WZ_POINT_IN_RECT(mouseX, mouseY, rect);

	if (!oldHover && widget->hover && widget->vtable.mouse_hover_on)
	{
		widget->vtable.mouse_hover_on(widget);
	}
	else if (oldHover && !widget->hover && widget->vtable.mouse_hover_off)
	{
		widget->vtable.mouse_hover_off(widget);
	}

	if (widgetIsChildOfWindow && widget->vtable.mouse_move)
	{
		widget->vtable.mouse_move(widget, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
	}

	child = widget->firstChild;

	while (child)
	{
		wz_widget_mouse_move_recursive(window, child, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_mouse_move(struct wzDesktop *desktop, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	assert(desktop);

	// Need a special case for dock icons.
	wz_desktop_update_desktop_preview_visible(desktop, mouseX, mouseY);

	if (stb_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		wz_widget_mouse_move_recursive(NULL, desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)], mouseX, mouseY, mouseDeltaX, mouseDeltaY);
		return;
	}

	desktop->lockInputWindow = wz_desktop_get_hover_window(desktop, mouseX, mouseY);
	wz_widget_mouse_move_recursive(desktop->lockInputWindow, (struct wzWidget *)desktop, mouseX, mouseY, mouseDeltaX, mouseDeltaY);
}

static void wz_widget_mouse_wheel_move_recursive(struct wzWidget *widget, int x, int y)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->vtable.mouse_wheel_move)
	{
		widget->vtable.mouse_wheel_move(widget, x, y);
	}

	child = widget->firstChild;

	while (child)
	{
		if (child->hover)
		{
			wz_widget_mouse_wheel_move_recursive(child, x, y);
		}

		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_mouse_wheel_move(struct wzDesktop *desktop, int x, int y)
{
	struct wzWidget *widget;

	assert(desktop);

	if (stb_arr_len(desktop->lockInputWidgetStack) > 0)
	{
		// Lock input to the top/last item on the stack.
		widget = desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)];
	}
	else if (desktop->lockInputWindow)
	{
		widget = (struct wzWidget *)desktop->lockInputWindow;
	}
	else
	{
		widget = (struct wzWidget *)desktop;
	}

	wz_widget_mouse_wheel_move_recursive(widget, x, y);
}

static void wz_widget_calculate_unique_draw_priorities_recursive(int *drawPriorities, int *nDrawPriorities, struct wzWidget *widget)
{
	struct wzWidget *child;
	int i;

	assert(widget);

	if (widget->hidden)
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

	child = widget->firstChild;

	while (child)
	{
		wz_widget_calculate_unique_draw_priorities_recursive(drawPriorities, nDrawPriorities, child);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

static int wz_compare_draw_priorities(const void *a, const void *b)
{
	return *(const int *)a - *(const int *)b;
}

static void wz_widget_draw_by_less_than_or_equals_priority_recursive(int priority, struct wzWidget *widget)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->drawPriority <= priority && widget->vtable.draw)
	{
		widget->vtable.draw(widget);
	}

	child = widget->firstChild;

	while (child)
	{
		wz_widget_draw_by_less_than_or_equals_priority_recursive(priority, child);
		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

static void wz_widget_draw_by_priority_recursive(int priority, struct wzWidget *widget)
{
	struct wzWidget *child;

	assert(widget);

	if (widget->hidden)
		return;

	if (widget->drawPriority == priority && widget->vtable.draw)
	{
		widget->vtable.draw(widget);
	}

	child = widget->firstChild;

	while (child)
	{
		// If the priority is a match, draw children with <= priority.
		if (widget->drawPriority == priority)
		{
			wz_widget_draw_by_less_than_or_equals_priority_recursive(priority, child);
		}
		else
		{
			wz_widget_draw_by_priority_recursive(priority, child);
		}

		child = child->next == widget->firstChild ? NULL : child->next;
	}
}

void wz_desktop_draw(struct wzDesktop *desktop)
{
	int drawPriorities[WZ_DRAW_PRIORITY_MAX];
	int nDrawPriorities;
	int i;

	assert(desktop);

	// Calculate unique draw priorities.
	nDrawPriorities = 0;
	wz_widget_calculate_unique_draw_priorities_recursive(drawPriorities, &nDrawPriorities, (struct wzWidget *)desktop);

	// Sort draw priorities in ascending order.
	qsort(drawPriorities, nDrawPriorities, sizeof(int), wz_compare_draw_priorities);

	// Do one draw pass per draw priority.
	for (i = 0; i < nDrawPriorities; i++)
	{
		wz_widget_draw_by_priority_recursive(drawPriorities[i], (struct wzWidget *)desktop);
	}
}

void wz_desktop_push_lock_input_widget(struct wzDesktop *desktop, struct wzWidget *widget)
{
	assert(desktop);
	stb_arr_push(desktop->lockInputWidgetStack, widget);
}

void wz_desktop_pop_lock_input_widget(struct wzDesktop *desktop, struct wzWidget *widget)
{
	// Only pop if the widget is on the top of the stack.
	if (stb_arr_len(desktop->lockInputWidgetStack) == 0)
		return;

	if (widget == desktop->lockInputWidgetStack[stb_arr_lastn(desktop->lockInputWidgetStack)])
	{	
		stb_arr_pop(desktop->lockInputWidgetStack);
	}
}

void wz_desktop_set_moving_window(struct wzDesktop *desktop, struct wzWindow *window)
{
	int i;

	assert(desktop);
	desktop->movingWindow = window;

	// Show the dock icons if movingWindow is not NULL.
	for (i = 0; i < WZ_NUM_DOCK_ICONS; i++)
	{
		wz_widget_set_visible((struct wzWidget *)desktop->dockIcons[i], desktop->movingWindow != NULL);
	}
}
