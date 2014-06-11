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
	WZ_TYPE_TAB_BAR,
	WZ_TYPE_TAB_PAGE,
	WZ_TYPE_TABBED,
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

typedef enum
{
	WZ_EVENT_UNKNOWN,
	WZ_EVENT_BUTTON_PRESSED,
	WZ_EVENT_BUTTON_CLICKED,
	WZ_EVENT_LIST_ITEM_SELECTED,
	WZ_EVENT_SCROLLER_VALUE_CHANGED,
	WZ_EVENT_TAB_BAR_TAB_CHANGED,
	WZ_EVENT_TAB_BAR_TAB_ADDED,
	WZ_EVENT_TAB_BAR_TAB_REMOVED
}
wzWidgetEventType;

typedef struct
{
	wzWidgetEventType type;
	struct wzWidget *widget;
}
wzEventBase;

typedef struct
{
	wzWidgetEventType type;
	struct wzButton *button;
}
wzButtonEvent;

typedef struct
{
	wzWidgetEventType type;
	struct wzList *list;
	int selectedItem;
}
wzListEvent;

typedef struct
{
	wzWidgetEventType type;
	struct wzScroller *scroller;
	int oldValue;
	int value;
}
wzScrollerEvent;

typedef struct
{
	wzWidgetEventType type;
	struct wzTabBar *tabBar;
	struct wzButton *tab;
}
wzTabBarEvent;

typedef union
{
	wzEventBase base;
	wzButtonEvent button;
	wzListEvent list;
	wzScrollerEvent scroller;
	wzTabBarEvent tabBar;
}
wzEvent;

typedef void (*wzEventCallback)(wzEvent e);

typedef enum
{
	WZ_CURSOR_DEFAULT,
	WZ_CURSOR_RESIZE_N_S,
	WZ_CURSOR_RESIZE_E_W,
	WZ_CURSOR_RESIZE_NE_SW,
	WZ_CURSOR_RESIZE_NW_SE,
	WZ_NUM_CURSORS
}
wzCursor;

typedef enum
{
	WZ_DOCK_POSITION_NONE = -1,
	WZ_DOCK_POSITION_NORTH,
	WZ_DOCK_POSITION_SOUTH,
	WZ_DOCK_POSITION_EAST,
	WZ_DOCK_POSITION_WEST,
	WZ_NUM_DOCK_POSITIONS
}
wzDockPosition;

#define WZ_MIN(a, b) ((a) < (b) ? (a) : (b))
#define WZ_MAX(a, b) ((a) > (b) ? (a) : (b))
#define WZ_CLAMPED(min, value, max) WZ_MAX(min, WZ_MIN(max, value))
#define WZ_POINT_IN_RECT(px, py, rect) ((px) >= rect.x && (px) < rect.x + rect.w && (py) >= rect.y && (py) < rect.y + rect.h)
#define WZ_RECTS_OVERLAP(rect1, rect2) (rect1.x < rect2.x + rect2.w && rect1.x + rect1.w > rect2.x && rect1.y < rect2.y + rect2.h && rect1.y + rect1.h > rect2.y) 

bool wz_is_rect_empty(wzRect rect);
bool wz_intersect_rects(wzRect A, wzRect B, wzRect *result);

typedef void (*wzDesktopDrawDockIconCallback)(wzRect rect, void *metadata);
typedef void (*wzDesktopDrawDockPreviewCallback)(wzRect rect, void *metadata);

struct wzDesktop *wz_desktop_create();

// Set the centralized event handler. All events invoked by the ancestor widgets of this desktop will call the callback function.
void wz_desktop_set_event_callback(struct wzDesktop *desktop, wzEventCallback callback);

void wz_desktop_set_draw_dock_icon_callback(struct wzDesktop *desktop, wzDesktopDrawDockIconCallback callback, void *metadata);
void wz_desktop_set_draw_dock_preview_callback(struct wzDesktop *desktop, wzDesktopDrawDockPreviewCallback callback, void *metadata);
void wz_desktop_set_dock_icon_size(struct wzDesktop *desktop, wzSize size);
void wz_desktop_set_dock_icon_size_args(struct wzDesktop *desktop, int w, int h);
void wz_desktop_mouse_button_down(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY);
void wz_desktop_mouse_button_up(struct wzDesktop *desktop, int mouseButton, int mouseX, int mouseY);
void wz_desktop_mouse_move(struct wzDesktop *desktop, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
void wz_desktop_mouse_wheel_move(struct wzDesktop *desktop, int x, int y);
void wz_desktop_draw(struct wzDesktop *desktop);
struct wzTabBar **wz_desktop_get_dock_tab_bars(struct wzDesktop *desktop);
struct wzWindow *wz_desktop_get_dock_tab_window(struct wzDesktop *desktop, struct wzButton *tab);
wzCursor wz_desktop_get_cursor(struct wzDesktop *desktop);

void wz_widget_destroy(struct wzWidget *widget);
struct wzDesktop *wz_widget_get_desktop(struct wzWidget *widget);
wzWidgetType wz_widget_get_type(const struct wzWidget *widget);
void wz_widget_set_position_args(struct wzWidget *widget, int x, int y);
void wz_widget_set_position(struct wzWidget *widget, wzPosition position);
wzPosition wz_widget_get_position(const struct wzWidget *widget);
void wz_widget_set_width(struct wzWidget *widget, int w);
void wz_widget_set_height(struct wzWidget *widget, int h);
void wz_widget_set_size_args(struct wzWidget *widget, int w, int h);
void wz_widget_set_size(struct wzWidget *widget, wzSize size);
int wz_widget_get_width(const struct wzWidget *widget);
int wz_widget_get_height(const struct wzWidget *widget);
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
void wz_widget_set_draw_function(struct wzWidget *widget, void (*draw)(struct wzWidget *, wzRect));
void wz_widget_add_child_widget(struct wzWidget *widget, struct wzWidget *child);
void wz_widget_destroy_child_widget(struct wzWidget *widget, struct wzWidget *child);

// Determine whether the widget is an descendant of a widget with the provided type.
bool wz_widget_is_descendant_of(struct wzWidget *widget, wzWidgetType type);

struct wzWidget *wz_widget_get_parent(struct wzWidget *widget);

struct wzWindow *wz_widget_get_parent_window(struct wzWidget *widget);

struct wzWindow *wz_window_create(struct wzDesktop *desktop);
void wz_window_set_header_height(struct wzWindow *window, int height);
int wz_window_get_header_height(struct wzWindow *window);
void wz_window_set_border_size(struct wzWindow *window, int size);
int wz_window_get_border_size(struct wzWindow *window);
wzRect wz_window_get_header_rect(struct wzWindow *window);
wzRect wz_window_get_content_rect(struct wzWindow *window);

typedef enum
{
	// Click the button on mouse up (default).
	WZ_BUTTON_CLICK_BEHAVIOR_UP,

	// Click the button on mouse down
	WZ_BUTTON_CLICK_BEHAVIOR_DOWN
}
wzButtonClickBehavior;

typedef enum
{
	// Button is never set.
	WZ_BUTTON_SET_BEHAVIOR_DEFAULT,

	// Click to toggle whether the button is set.
	WZ_BUTTON_SET_BEHAVIOR_TOGGLE,

	// Click to set the button. Clicking again does nothing.
	WZ_BUTTON_SET_BEHAVIOR_STICKY
}
wzButtonSetBehavior;

struct wzButton *wz_button_create(struct wzDesktop *desktop);
void wz_button_set_click_behavior(struct wzButton *button, wzButtonClickBehavior clickBehavior);
void wz_button_set_set_behavior(struct wzButton *button, wzButtonSetBehavior clickBehavior);
bool wz_button_is_pressed(const struct wzButton *button);
bool wz_button_is_set(const struct wzButton *button);
void wz_button_set(struct wzButton *button, bool value);
void wz_button_add_callback_pressed(struct wzButton *button, wzEventCallback callback);
void wz_button_add_callback_clicked(struct wzButton *button, wzEventCallback callback);

struct wzCombo *wz_combo_create(struct wzDesktop *desktop);
struct wzList *wz_combo_get_list(struct wzCombo *combo);
bool wz_combo_is_open(struct wzCombo *combo);

struct wzGroupBox *wz_groupbox_create(struct wzDesktop *desktop);

struct wzLabel *wz_label_create(struct wzDesktop *desktop);

struct wzList *wz_list_create(struct wzDesktop *desktop);
struct wzScroller *wz_list_get_scroller(struct wzList *list);
void wz_list_set_items_border(struct wzList *list, wzBorder itemsBorder);
void wz_list_set_items_border_args(struct wzList *list, int top, int right, int bottom, int left);
wzBorder wz_list_get_items_border(const struct wzList *list);
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
void wz_list_add_callback_item_selected(struct wzList *list, wzEventCallback callback);

typedef enum
{
	WZ_SCROLLER_VERTICAL,
	WZ_SCROLLER_HORIZONTAL
}
wzScrollerType;

struct wzScroller *wz_scroller_create(struct wzDesktop *desktop, wzScrollerType scrollerType);
int wz_scroller_get_value(const struct wzScroller *scroller);
void wz_scroller_set_value(struct wzScroller *scroller, int value);
void wz_scroller_decrement_value(struct wzScroller *scroller);
void wz_scroller_increment_value(struct wzScroller *scroller);
void wz_scroller_set_step_value(struct wzScroller *scroller, int stepValue);
int wz_scroller_get_step_value(struct wzScroller *scroller);
void wz_scroller_set_max_value(struct wzScroller *scroller, int maxValue);
struct wzButton *wz_scroller_get_decrement_button(struct wzScroller *scroller);
struct wzButton *wz_scroller_get_increment_button(struct wzScroller *scroller);
int wz_scroller_get_nub_size(struct wzScroller *scroller);
void wz_scroller_set_nub_size(struct wzScroller *scroller, int size);
wzRect wz_scroller_get_nub_rect(struct wzScroller *scroller);
void wz_scroller_add_callback_value_changed(struct wzScroller *scroller, wzEventCallback callback);

struct wzTabBar *wz_tab_bar_create(struct wzDesktop *desktop);
struct wzButton *wz_tab_bar_add_tab(struct wzTabBar *tabBar);
struct wzButton *wz_tab_bar_add_existing_tab(struct wzTabBar *tabBar, struct wzButton *tab);
void wz_tab_bar_remove_tab(struct wzTabBar *tabBar, struct wzButton *tab);
void wz_tab_bar_clear_tabs(struct wzTabBar *tabBar);
struct wzButton *wz_tab_bar_get_decrement_button(struct wzTabBar *tabBar);
struct wzButton *wz_tab_bar_get_increment_button(struct wzTabBar *tabBar);
struct wzButton *wz_tab_bar_get_selected_tab(struct wzTabBar *tabBar);
void wz_tab_bar_select_tab(struct wzTabBar *tabBar, struct wzButton *tab);
void wz_tab_bar_add_callback_tab_changed(struct wzTabBar *tabBar, wzEventCallback callback);

struct wzTabbed *wz_tabbed_create(struct wzDesktop *desktop);
void wz_tabbed_add_tab(struct wzTabbed *tabbed, struct wzButton **tab, struct wzWidget **page);
struct wzTabBar *wz_tabbed_get_tab_bar(struct wzTabbed *tabbed);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
