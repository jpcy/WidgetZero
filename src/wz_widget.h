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
#ifndef _WZ_WIDGET_H_
#define _WZ_WIDGET_H_

#include <wz.h>
#include "wz_arr.h"

typedef struct
{
	void (*destroy)(struct wzWidget *widget);

	void (*draw)(struct wzWidget *widget, wzRect clip);

	// If NULL, wzWidget.rect will be set to rect, otherwise this function is called.
	void (*set_rect)(struct wzWidget *widget, wzRect rect);

	// Some additional widget state may been to be cleared when a widget is hidden.
	void (*set_visible)(struct wzWidget *widget, bool visible);

	void (*mouse_button_down)(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY);
	void (*mouse_button_up)(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY);
	void (*mouse_move)(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	void (*mouse_wheel_move)(struct wzWidget *widget, int x, int y);
	void (*mouse_hover_on)(struct wzWidget *widget);
	void (*mouse_hover_off)(struct wzWidget *widget);
	void (*key_down)(struct wzWidget *widget, wzKey key);
	void (*key_up)(struct wzWidget *widget, wzKey key);
	void (*text_input)(struct wzWidget *widget, const char *text);

	// Returns the rect to clip the children of this widget against. Return an empty rect to disable clipping of children.
	wzRect (*get_children_clip_rect)(struct wzWidget *widget);

	struct wzWidget *(*get_content_widget)(struct wzWidget *widget);
}
wzWidgetVtable;

enum
{
	WZ_DRAW_PRIORITY_DEFAULT,

	// Widgets can use this range to draw on top over widgets with default draw priority.
	WZ_DRAW_PRIORITY_WIDGET_CUSTOM_START,
	WZ_DRAW_PRIORITY_WIDGET_CUSTOM_END = WZ_DRAW_PRIORITY_WIDGET_CUSTOM_START + 16,
	WZ_DRAW_PRIORITY_TAB_BAR_SCROLL_BUTTON,
	WZ_DRAW_PRIORITY_DOCK_TAB_BAR,
	WZ_DRAW_PRIORITY_DOCK_TAB_BAR_SCROLL_BUTTON,
	WZ_DRAW_PRIORITY_WINDOW,
	WZ_DRAW_PRIORITY_COMBO_DROPDOWN,
	WZ_DRAW_PRIORITY_DOCK_ICON,
	WZ_DRAW_PRIORITY_DOCK_PREVIEW,
	WZ_DRAW_PRIORITY_MAX
};

struct wzWidget
{
	wzWidgetType type;
	int drawPriority;
	wzRect rect;

	int autosize;

	// Only used if the widget is the child of a layout.
	int stretch;

	// Only used if the widget is the child of a layout.
	int align;

	// Only used when userSetSize w and/or h are set to WZ_AUTOSIZE, or the widget is the child of a layout.
	wzBorder margin;

	// Like metadata, but used internally.
	void *internalMetadata;

	// User-set metadata.
	void *metadata;

	bool hover;

	// Don't draw this widget.
	bool hidden;

	// Used internally to ignore siblings that overlap at the mouse cursor.
	bool ignore;

	// True if not clipped to the parent widget rect in mouse move calculations. Used by the combo widget dropdown list.
	bool inputNotClippedToParent;

	wzWidgetVtable vtable;

	struct wzDesktop *desktop;

	// The closest ancestor window. NULL if the widget is the descendant of a desktop. Set in wz_widget_add_child_widget_internal.
	struct wzWindow *window;

	struct wzWidget *parent;
	struct wzWidget **children;
};

enum
{
	WZ_COMPASS_N,
	WZ_COMPASS_NE,
	WZ_COMPASS_E,
	WZ_COMPASS_SE,
	WZ_COMPASS_S,
	WZ_COMPASS_SW,
	WZ_COMPASS_W,
	WZ_COMPASS_NW,
	WZ_NUM_COMPASS_POINTS
};

void wz_widget_add_child_widget_internal(struct wzWidget *widget, struct wzWidget *child);

void wz_widget_refresh_rect(struct wzWidget *widget);

struct wzWidget *wz_widget_find_closest_ancestor(struct wzWidget *widget, wzWidgetType type);

// Also sets all ancestor widgets to the same draw priority.
void wz_widget_set_draw_priority(struct wzWidget *widget, int drawPriority);

int wz_widget_get_draw_priority(const struct wzWidget *widget);

bool wz_widget_overlaps_parent_window(const struct wzWidget *widget);

void wz_widget_set_clip_input_to_parent(struct wzWidget *widget, bool value);

void wz_widget_set_internal_metadata(struct wzWidget *widget, void *metadata);
void *wz_widget_get_internal_metadata(struct wzWidget *widget);

// Calculate the highest draw priority out of this widget and all it's direct ancestors.
// Used when widgets need to be seen to inherit the draw priority of their ancestors. e.g. combo descendants: list, scroller, buttons used by scroller.
int wz_widget_calculate_inherited_draw_priority(const struct wzWidget *widget);

#endif
