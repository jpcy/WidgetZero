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
#ifndef _WIDGET_ZERO_H_
#define _WIDGET_ZERO_H_

#ifndef __cplusplus
#ifdef _MSC_VER
typedef int bool;
#define false 0
#define true 1
#else
#include <stdbool.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct wzContext;
struct wzDesktop;
struct wzWindow;
struct wzWidget;
struct wzButton;
struct wzCombo;
struct wzScroller;
struct wzGroupBox;
struct wzLabel;
struct wzList;

typedef enum
{
	WZ_TYPE_UNKNOWN,
	WZ_TYPE_DESKTOP,
	WZ_TYPE_WINDOW,
	WZ_TYPE_BUTTON,
	WZ_TYPE_COMBO,
	WZ_TYPE_GROUPBOX,
	WZ_TYPE_LABEL,
	WZ_TYPE_LIST,
	WZ_TYPE_SCROLLER,
	WZ_MAX_WIDGET_TYPES = 64
}
wzWidgetType;

typedef struct
{
	int x, y;
}
wzPosition;

typedef struct
{
	int w, h;
}
wzSize;

typedef struct
{
	int x, y, w, h;
}
wzRect;

typedef struct
{
	int top, right, bottom, left;
}
wzBorder;

#define WZ_MIN(a, b) ((a) < (b) ? (a) : (b))
#define WZ_MAX(a, b) ((a) > (b) ? (a) : (b))
#define WZ_CLAMPED(min, value, max) WZ_MAX(min, WZ_MIN(max, value))
#define WZ_POINT_IN_RECT(px, py, rect) ((px) >= rect.x && (px) < rect.x + rect.w && (py) >= rect.y && (py) < rect.y + rect.h)

// clip the child rect to the parent
wzRect wzClippedRect(wzRect parent, wzRect child);

struct wzContext *wz_context_create(void);
void wz_context_destroy(struct wzContext *context);

struct wzDesktop *wz_desktop_create(struct wzContext *context);
void wz_desktop_mouse_button_down(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY);
void wz_desktop_mouse_button_up(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY);
void wz_desktop_mouse_move(struct wzDesktop *desktop, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
void wz_desktop_draw(struct wzDesktop *desktop);

void wz_widget_destroy(struct wzWidget *widget);
struct wzContext *wz_widget_get_context(struct wzWidget *widget);
wzWidgetType wz_widget_get_type(const struct wzWidget *widget);
void wz_widget_set_position_args(struct wzWidget *widget, int x, int y);
void wz_widget_set_position(struct wzWidget *widget, wzPosition position);
wzPosition wz_widget_get_position(const struct wzWidget *widget);
void wz_widget_set_size_args(struct wzWidget *widget, int w, int h);
void wz_widget_set_size(struct wzWidget *widget, wzSize size);
wzSize wz_widget_get_size(const struct wzWidget *widget);
void wz_widget_set_rect_args(struct wzWidget *widget, int x, int y, int w, int h);
void wz_widget_set_rect(struct wzWidget *widget, wzRect rect);
wzRect wz_widget_get_rect(const struct wzWidget *widget);

// rect will be absolute - ancestor window position is taken into account.
wzRect wz_widget_get_absolute_rect(const struct wzWidget *widget);

bool wz_widget_get_hover(const struct wzWidget *widget);
void wz_widget_set_visible(struct wzWidget *widget, bool visible);
bool wz_widget_get_visible(const struct wzWidget *widget);
void wz_widget_set_metadata(struct wzWidget *widget, void *metadata);
void *wz_widget_get_metadata(struct wzWidget *widget);
void wz_widget_set_draw_function(struct wzWidget *widget, void (*draw)(struct wzWidget *));
void wz_widget_add_child_widget(struct wzWidget *widget, struct wzWidget *child);

struct wzWindow *wz_window_create(struct wzContext *context);
void wz_window_set_header_height(struct wzWindow *window, int height);
int wz_window_get_header_height(struct wzWindow *window);
void wz_window_set_border_size(struct wzWindow *window, int size);
int wz_window_get_border_size(struct wzWindow *window);
wzRect wz_window_get_header_rect(struct wzWindow *window);
wzRect wz_window_get_content_rect(struct wzWindow *window);

typedef void (*wzButtonPressedCallback)(struct wzButton *);

struct wzButton *wz_button_create(struct wzContext *context);
void wz_button_set_toggle_behavior(struct wzButton *button, bool enabled);
bool wz_button_is_pressed(const struct wzButton *button);
bool wz_button_is_set(const struct wzButton *button);
void wz_button_add_callback_pressed(struct wzButton *button, wzButtonPressedCallback callback);

struct wzCombo *wz_combo_create(struct wzContext *context);
struct wzList *wz_combo_get_list(struct wzCombo *combo);
bool wz_combo_is_open(struct wzCombo *combo);

struct wzGroupBox *wz_groupbox_create(struct wzContext *context);

struct wzLabel *wz_label_create(struct wzContext *context);

typedef void (*wzListItemSelectedCallback)(struct wzList *);

struct wzList *wz_list_create(struct wzContext *context);
struct wzScroller *wz_list_get_scroller(struct wzList *list);
void wz_list_set_items_border(struct wzList *list, wzBorder itemsBorder);
wzRect wz_list_get_items_rect(const struct wzList *list);

// rect will be absolute - ancestor window position is taken into account.
wzRect wz_list_get_absolute_items_rect(const struct wzList *list);

void wz_list_set_item_height(struct wzList *list, int itemHeight);
int wz_list_get_item_height(const struct wzList *list);
void wz_list_set_num_items(struct wzList *list, int nItems);
int wz_list_get_num_items(const struct wzList *list);
int wz_list_get_first_item(const struct wzList *list);
void wz_list_set_selected_item(struct wzList *list, int selectedItem);
int wz_list_get_selected_item(const struct wzList *list);
int wz_list_get_pressed_item(const struct wzList *list);
int wz_list_get_hovered_item(const struct wzList *list);
void wz_list_add_callback_item_selected(struct wzList *list, wzListItemSelectedCallback callback);

typedef enum
{
	WZ_SCROLLER_VERTICAL,
	WZ_SCROLLER_HORIZONTAL
}
wzScrollerType;

typedef void (*wzScrollerValueChangedCallback)(struct wzScroller *, int value);

struct wzScroller *wz_scroller_create(struct wzContext *context, wzScrollerType scrollerType);
int wz_scroller_get_value(const struct wzScroller *scroller);
void wz_scroller_set_value(struct wzScroller *scroller, int value);
void wz_scroller_decrement_value(struct wzScroller *scroller);
void wz_scroller_increment_value(struct wzScroller *scroller);
void wz_scroller_set_step_value(struct wzScroller *scroller, int stepValue);
void wz_scroller_set_max_value(struct wzScroller *scroller, int maxValue);
struct wzButton *wz_scroller_get_decrement_button(struct wzScroller *scroller);
struct wzButton *wz_scroller_get_increment_button(struct wzScroller *scroller);
int wz_scroller_get_nub_size(struct wzScroller *scroller);
void wz_scroller_set_nub_size(struct wzScroller *scroller, int size);
wzRect wz_scroller_get_nub_rect(struct wzScroller *scroller);
void wz_scroller_add_callback_value_changed(struct wzScroller *scroller, wzScrollerValueChangedCallback callback);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
