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
#pragma once

#include <vector>
#include <wz.h>

namespace wz {

typedef struct
{
	WidgetDrawCallback draw;
	WidgetMeasureCallback measure;

	void (*destroy)(struct WidgetImpl *widget);

	// The widget was added to a parent widget (see wz_widget_add_child_widget). 
	void (*added)(struct WidgetImpl *parent, struct WidgetImpl *widget);

	// WidgetImpl.renderer has been changed
	void (*renderer_changed)(struct WidgetImpl *widget);

	// If NULL, WidgetImpl.rect will be set to rect, otherwise this function is called.
	void (*set_rect)(struct WidgetImpl *widget, Rect rect);

	// Some additional widget state may been to be cleared when a widget is hidden.
	void (*set_visible)(struct WidgetImpl *widget, bool visible);

	void (*font_changed)(struct WidgetImpl *widget, const char *fontFace, float fontSize);

	void (*mouse_button_down)(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY);
	void (*mouse_button_up)(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY);
	void (*mouse_move)(struct WidgetImpl *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	void (*mouse_wheel_move)(struct WidgetImpl *widget, int x, int y);
	void (*mouse_hover_on)(struct WidgetImpl *widget);
	void (*mouse_hover_off)(struct WidgetImpl *widget);
	void (*key_down)(struct WidgetImpl *widget, Key key);
	void (*key_up)(struct WidgetImpl *widget, Key key);
	void (*text_input)(struct WidgetImpl *widget, const char *text);

	// Returns the rect to clip the children of this widget against. Return an empty rect to disable clipping of children.
	Rect (*get_children_clip_rect)(struct WidgetImpl *widget);
}
WidgetVtable;

enum
{
	WZ_WIDGET_FLAG_DRAW_LAST = 1<<0,
};

struct WidgetImpl
{
	WidgetImpl();

	virtual ~WidgetImpl()
	{
		for (size_t i = 0; i < eventHandlers.size(); i++)
		{
			delete eventHandlers[i];
		}

		eventHandlers.clear();
	}

	virtual const WidgetImpl *getWidget() const { return this; }
	virtual WidgetImpl *getWidget() { return this; }

	WidgetType type;

	// Explicitly set by the user.
	Rect userRect;

	Rect rect;

	// Only used if the widget is the child of a layout.
	int stretch;

	float stretchWidthScale;
	float stretchHeightScale;

	// Only used if the widget is the child of a layout.
	int align;

	// Only used when userSetSize w and/or h are set to WZ_AUTOSIZE, or the widget is the child of a layout.
	Border margin;

	// Like metadata, but used internally.
	void *internalMetadata;

	// User-set metadata.
	void *metadata;

	int flags;

	bool hover;

	// Don't draw this widget.
	bool hidden;

	// Used internally to ignore siblings that overlap at the mouse cursor.
	bool ignore;

	// This widget should overlap other widgets when doing mouse cursor logic. e.g. tab bar scroll buttons.
	bool overlap;

	// Don't draw automatically when wz_main_window_draw walks through the widget hierarchy.
	bool drawManually;

	// True if not clipped to the parent widget rect in mouse move calculations. Used by the combo widget dropdown list.
	bool inputNotClippedToParent;

	char fontFace[256];
	float fontSize;

	WidgetVtable vtable;

	struct wzRenderer *renderer;

	struct MainWindowImpl *mainWindow;

	// The closest ancestor window. NULL if the widget is the descendant of a mainWindow. Set in wz_widget_add_child_widget.
	struct WindowImpl *window;

	struct WidgetImpl *parent;
	std::vector<struct WidgetImpl *> children;

	std::vector<IEventHandler *> eventHandlers;
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

void wz_widget_add_child_widget(struct WidgetImpl *widget, struct WidgetImpl *child);
void wz_widget_remove_child_widget(struct WidgetImpl *widget, struct WidgetImpl *child);
void wz_widget_destroy_child_widget(struct WidgetImpl *widget, struct WidgetImpl *child);

void wz_widget_set_position_args_internal(struct WidgetImpl *widget, int x, int y);
void wz_widget_set_position_internal(struct WidgetImpl *widget, Position position);
void wz_widget_set_width_internal(struct WidgetImpl *widget, int w);
void wz_widget_set_height_internal(struct WidgetImpl *widget, int h);
void wz_widget_set_size_args_internal(struct WidgetImpl *widget, int w, int h);
void wz_widget_set_size_internal(struct WidgetImpl *widget, Size size);
void wz_widget_set_rect_args_internal(struct WidgetImpl *widget, int x, int y, int w, int h);
void wz_widget_set_rect_internal(struct WidgetImpl *widget, Rect rect);

void wz_widget_refresh_rect(struct WidgetImpl *widget);

struct WidgetImpl *wz_widget_find_closest_ancestor(const struct WidgetImpl *widget, WidgetType type);

void wz_widget_set_draw_manually(struct WidgetImpl *widget, bool value);

void wz_widget_set_draw_last(struct WidgetImpl *widget, bool value);

void wz_widget_set_overlap(struct WidgetImpl *widget, bool value);

bool wz_widget_overlaps_parent_window(const struct WidgetImpl *widget);

void wz_widget_set_clip_input_to_parent(struct WidgetImpl *widget, bool value);

void wz_widget_set_internal_metadata(struct WidgetImpl *widget, void *metadata);
void *wz_widget_get_internal_metadata(struct WidgetImpl *widget);

// Shortcut for wz_renderer_get_line_height, using the widget's renderer, font face and font size.
int wz_widget_get_line_height(const struct WidgetImpl *widget);

// Shortcut for wz_renderer_measure_text, using the widget's renderer, font face and font size.
void wz_widget_measure_text(const struct WidgetImpl *widget, const char *text, int n, int *width, int *height);

} // namespace wz
