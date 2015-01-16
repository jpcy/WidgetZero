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
#ifndef WIDGET_ZERO_H
#define WIDGET_ZERO_H

#include <stdint.h>

#ifndef __cplusplus
#ifdef _MSC_VER
typedef unsigned char bool;
#define false 0
#define true 1
#else
#include <stdbool.h>
#endif
#endif

#ifndef WZ_ASSERT
#include <assert.h>
#define WZ_ASSERT assert
#endif

#include <nanovg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NVGcontext NVGcontext;

struct wzRenderer;
struct wzMainWindow;
struct wzWindow;
struct wzWidget;
struct wzButton;
struct wzCheckBox;
struct wzCombo;
struct wzDummy;
struct wzFrame;
struct wzGroupBox;
struct wzLabel;
struct wzList;
struct wzMenuBar;
struct wzMenuBarButton;
struct wzRadioButton;
struct wzScroller;
struct wzSpinner;
struct wzStackLayout;
struct wzTextEdit;

typedef enum
{
	WZ_TYPE_UNKNOWN,
	WZ_TYPE_MAIN_WINDOW,
	WZ_TYPE_WINDOW,
	WZ_TYPE_BUTTON,
	WZ_TYPE_COMBO,
	WZ_TYPE_DUMMY,
	WZ_TYPE_FRAME,
	WZ_TYPE_GROUP_BOX,
	WZ_TYPE_LABEL,
	WZ_TYPE_LIST,
	WZ_TYPE_MENU_BAR,
	WZ_TYPE_MENU_BAR_BUTTON,
	WZ_TYPE_RADIO_BUTTON,
	WZ_TYPE_SCROLLER,
	WZ_TYPE_SPINNER,
	WZ_TYPE_STACK_LAYOUT,
	WZ_TYPE_TAB_BAR,
	WZ_TYPE_TAB_PAGE,
	WZ_TYPE_TABBED,
	WZ_TYPE_TEXT_EDIT,
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

extern const wzBorder wzBorder_zero;

enum
{
	WZ_STRETCH_NONE,
	WZ_STRETCH_WIDTH = 1,
	WZ_STRETCH_HEIGHT = 2,
	WZ_STRETCH = WZ_STRETCH_WIDTH | WZ_STRETCH_HEIGHT
};

enum
{
	WZ_ALIGN_NONE,

	// Horizontal.
	WZ_ALIGN_LEFT = 1,
	WZ_ALIGN_CENTER = 2,
	WZ_ALIGN_RIGHT = 4,

	// Vertical.
	WZ_ALIGN_TOP = 8,
	WZ_ALIGN_MIDDLE = 16,
	WZ_ALIGN_BOTTOM = 32
};

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
	struct wzWidget *parent;
	struct wzWidget *widget; 
	struct wzWidget *extra;
}
wzCreateWidgetEvent;

typedef struct
{
	wzWidgetEventType type;
	struct wzWidget *parent;
	struct wzWidget *widget;
}
wzDestroyWidgetEvent;

typedef struct
{
	wzWidgetEventType type;
	struct wzButton *button;
	bool isSet;
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
	wzCreateWidgetEvent create;
	wzDestroyWidgetEvent destroy;
	wzButtonEvent button;
	wzListEvent list;
	wzScrollerEvent scroller;
	wzTabBarEvent tabBar;
}
wzEvent;

typedef void (*wzEventCallback)(wzEvent *e);

typedef enum
{
	WZ_CURSOR_DEFAULT,
	WZ_CURSOR_IBEAM,
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
#define WZ_ABS(a) ((a) >= 0 ? (a) : (-a))
#define WZ_SIGN(a) ((a) >= 0 ? 1 : -1)
#define WZ_POINT_IN_RECT(px, py, rect) ((px) >= rect.x && (px) < rect.x + rect.w && (py) >= rect.y && (py) < rect.y + rect.h)
#define WZ_RECTS_OVERLAP(rect1, rect2) (rect1.x < rect2.x + rect2.w && rect1.x + rect1.w > rect2.x && rect1.y < rect2.y + rect2.h && rect1.y + rect1.h > rect2.y) 

bool wz_is_rect_empty(wzRect rect);
bool wz_intersect_rects(wzRect A, wzRect B, wzRect *result);

typedef enum
{
	WZ_KEY_UNKNOWN,
	WZ_KEY_LEFT,
	WZ_KEY_RIGHT,
	WZ_KEY_UP,
	WZ_KEY_DOWN, 
	WZ_KEY_HOME,
	WZ_KEY_END,
	WZ_KEY_ENTER,
	WZ_KEY_DELETE,
	WZ_KEY_BACKSPACE,
	WZ_KEY_LSHIFT,
	WZ_KEY_RSHIFT,
	WZ_KEY_LCONTROL,
	WZ_KEY_RCONTROL,
	WZ_NUM_KEYS,

	WZ_KEY_SHIFT_BIT = (1<<10),
	WZ_KEY_CONTROL_BIT = (1<<11)
}
wzKey;

#define WZ_KEY_MOD_OFF(key) ((key) & ~(WZ_KEY_SHIFT_BIT | WZ_KEY_CONTROL_BIT))

struct wzMainWindow *wz_main_window_create(struct wzRenderer *renderer);

// Set the centralized event handler. All events invoked by the ancestor widgets of this mainWindow will call the callback function.
void wz_main_window_set_event_callback(struct wzMainWindow *mainWindow, wzEventCallback callback);

void wz_main_window_mouse_button_down(struct wzMainWindow *mainWindow, int mouseButton, int mouseX, int mouseY);
void wz_main_window_mouse_button_up(struct wzMainWindow *mainWindow, int mouseButton, int mouseX, int mouseY);
void wz_main_window_mouse_move(struct wzMainWindow *mainWindow, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
void wz_main_window_mouse_wheel_move(struct wzMainWindow *mainWindow, int x, int y);
void wz_main_window_key_down(struct wzMainWindow *mainWindow, wzKey key);
void wz_main_window_key_up(struct wzMainWindow *mainWindow, wzKey key);
void wz_main_window_text_input(struct wzMainWindow *mainWindow, const char *text);
void wz_main_window_draw(struct wzMainWindow *mainWindow);
void wz_main_window_draw_frame(struct wzMainWindow *mainWindow);
void wz_main_window_set_menu_bar(struct wzMainWindow *mainWindow, struct wzMenuBar *menuBar);
void wz_main_window_add(struct wzMainWindow *mainWindow, struct wzWidget *widget);
void wz_main_window_remove(struct wzMainWindow *mainWindow, struct wzWidget *widget);
void wz_main_window_toggle_text_cursor(struct wzMainWindow *mainWindow);
wzCursor wz_main_window_get_cursor(const struct wzMainWindow *mainWindow);
const struct wzWidget *wz_main_window_get_keyboard_focus_widget(const struct wzMainWindow *mainWindow);
void wz_main_window_dock_window(struct wzMainWindow *mainWindow, struct wzWindow *window, wzDockPosition dockPosition);

typedef void (*wzWidgetDrawCallback)(struct wzWidget *widget, wzRect clip);
typedef wzSize (*wzWidgetMeasureCallback)(struct wzWidget *widget);

void wz_widget_destroy(struct wzWidget *widget);
struct wzMainWindow *wz_widget_get_main_window(struct wzWidget *widget);
wzWidgetType wz_widget_get_type(const struct wzWidget *widget);
bool wz_widget_is_layout(const struct wzWidget *widget);

// Resize the widget to the result of calling the widget "measure" callback.
void wz_widget_resize_to_measured(struct wzWidget *widget);

void wz_widget_set_position_args(struct wzWidget *widget, int x, int y);
void wz_widget_set_position(struct wzWidget *widget, wzPosition position);
wzPosition wz_widget_get_position(const struct wzWidget *widget);
wzPosition wz_widget_get_absolute_position(const struct wzWidget *widget);
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
wzRect wz_widget_get_absolute_rect(const struct wzWidget *widget);
void wz_widget_set_margin(struct wzWidget *widget, wzBorder margin);
void wz_widget_set_margin_args(struct wzWidget *widget, int top, int right, int bottom, int left);
void wz_widget_set_margin_uniform(struct wzWidget *widget, int value);
wzBorder wz_widget_get_margin(const struct wzWidget *widget);
void wz_widget_set_stretch(struct wzWidget *widget, int stretch);
int wz_widget_get_stretch(const struct wzWidget *widget);
void wz_widget_set_stretch_scale(struct wzWidget *widget, float width, float height);
float wz_widget_get_stretch_width_scale(const struct wzWidget *widget);
float wz_widget_get_stretch_height_scale(const struct wzWidget *widget);
void wz_widget_set_align(struct wzWidget *widget, int align);
int wz_widget_get_align(const struct wzWidget *widget);
void wz_widget_set_font_face(struct wzWidget *widget, const char *fontFace);
const char *wz_widget_get_font_face(const struct wzWidget *widget);
void wz_widget_set_font_size(struct wzWidget *widget, float fontSize);
float wz_widget_get_font_size(const struct wzWidget *widget);
void wz_widget_set_font(struct wzWidget *widget, const char *fontFace, float fontSize);
bool wz_widget_get_hover(const struct wzWidget *widget);
void wz_widget_set_visible(struct wzWidget *widget, bool visible);
bool wz_widget_get_visible(const struct wzWidget *widget);
bool wz_widget_has_keyboard_focus(const struct wzWidget *widget);
void wz_widget_set_metadata(struct wzWidget *widget, void *metadata);
void *wz_widget_get_metadata(struct wzWidget *widget);
void wz_widget_set_draw_callback(struct wzWidget *widget, wzWidgetDrawCallback draw);
void wz_widget_set_measure_callback(struct wzWidget *widget, wzWidgetMeasureCallback measure);

// Determine whether the widget is an descendant of a widget with the provided type.
bool wz_widget_is_descendant_of(struct wzWidget *widget, wzWidgetType type);

struct wzWidget *wz_widget_get_parent(struct wzWidget *widget);

struct wzWindow *wz_widget_get_parent_window(struct wzWidget *widget);

struct wzWindow *wz_window_create(const char *title);
int wz_window_get_header_height(const struct wzWindow *window);
int wz_window_get_border_size(const struct wzWindow *window);
wzRect wz_window_get_header_rect(const struct wzWindow *window);
void wz_window_set_title(struct wzWindow *window, const char *title);
const char *wz_window_get_title(const struct wzWindow *window);
void wz_window_add(struct wzWindow *window, struct wzWidget *widget);
void wz_window_remove(struct wzWindow *window, struct wzWidget *widget);

struct wzButton *wz_button_create(const char *label, const char *icon);
struct wzButton *wz_toggle_button_create(const char *label, const char *icon);
void wz_button_set_label(struct wzButton *button, const char *label);
const char *wz_button_get_label(const struct wzButton *button);
void wz_button_set_icon(struct wzButton *button, const char *icon);
const char *wz_button_get_icon(const struct wzButton *button);
void wz_button_set_padding(struct wzButton *button, wzBorder padding);
void wz_button_set_padding_args(struct wzButton *button, int top, int right, int bottom, int left);
wzBorder wz_button_get_padding(const struct wzButton *button);
bool wz_button_is_pressed(const struct wzButton *button);
bool wz_button_is_set(const struct wzButton *button);
void wz_button_set(struct wzButton *button, bool value);
void wz_button_bind_value(struct wzButton *button, bool *value);
void wz_button_add_callback_pressed(struct wzButton *button, wzEventCallback callback);
void wz_button_add_callback_clicked(struct wzButton *button, wzEventCallback callback);

struct wzCheckBox *wz_check_box_create(const char *label);
void wz_check_box_set_label(struct wzCheckBox *checkBox, const char *label);
const char *wz_check_box_get_label(const struct wzCheckBox *checkBox);
bool wz_check_box_is_checked(const struct wzCheckBox *checkBox);
void wz_check_box_check(struct wzCheckBox *checkBox, bool value);
void wz_check_box_bind_value(struct wzCheckBox *checkBox, bool *value);
void wz_check_box_add_callback_checked(struct wzCheckBox *checkBox, wzEventCallback callback);

struct wzCombo *wz_combo_create(uint8_t *itemData, int itemStride, int nItems);
struct wzList *wz_combo_get_list(const struct wzCombo *combo);
bool wz_combo_is_open(struct wzCombo *combo);

struct wzDummy *wz_dummy_create();

struct wzFrame *wz_frame_create();
void wz_frame_add(struct wzFrame *frame, struct wzWidget *widget);
void wz_frame_remove(struct wzFrame *frame, struct wzWidget *widget);

struct wzGroupBox *wz_group_box_create(const char *label);
void wz_group_box_set_label(struct wzGroupBox *groupBox, const char *label);
const char *wz_group_box_get_label(const struct wzGroupBox *groupBox);
void wz_group_box_add(struct wzGroupBox *groupBox, struct wzWidget *widget);
void wz_group_box_remove(struct wzGroupBox *groupBox, struct wzWidget *widget);

struct wzLabel *wz_label_create(const char *text);
void wz_label_set_multiline(struct wzLabel *label, bool multiline);
bool wz_label_get_multiline(const struct wzLabel *label);
void wz_label_set_text(struct wzLabel *label, const char *text);
const char *wz_label_get_text(const struct wzLabel *label);
void wz_label_set_text_color(struct wzLabel *label, NVGcolor color);
NVGcolor wz_label_get_text_color(const struct wzLabel *label);

typedef void (*wzDrawListItemCallback)(struct wzRenderer *renderer, wzRect clip, const struct wzList *list, const char *fontFace, float fontSize, int itemIndex, const uint8_t *itemData);

struct wzList *wz_list_create(uint8_t *itemData, int itemStride, int nItems);
wzBorder wz_list_get_items_border(const struct wzList *list);
wzRect wz_list_get_items_rect(const struct wzList *list);

// rect will be absolute - ancestor window position is taken into account.
wzRect wz_list_get_absolute_items_rect(const struct wzList *list);

void wz_list_set_draw_item_callback(struct wzList *list, wzDrawListItemCallback callback);
wzDrawListItemCallback wz_list_get_draw_item_callback(const struct wzList *list);
void wz_list_set_item_data(struct wzList *list, uint8_t *itemData);
uint8_t *wz_list_get_item_data(const struct wzList *list);
void wz_list_set_item_stride(struct wzList *list, int itemStride);
int wz_list_get_item_stride(const struct wzList *list);
void wz_list_set_item_height(struct wzList *list, int itemHeight);
int wz_list_get_item_height(const struct wzList *list);
void wz_list_set_num_items(struct wzList *list, int nItems);
int wz_list_get_num_items(const struct wzList *list);
int wz_list_get_first_item(const struct wzList *list);
void wz_list_set_selected_item(struct wzList *list, int selectedItem);
int wz_list_get_selected_item(const struct wzList *list);
int wz_list_get_pressed_item(const struct wzList *list);
int wz_list_get_hovered_item(const struct wzList *list);
int wz_list_get_scroll_value(const struct wzList *list);
void wz_list_add_callback_item_selected(struct wzList *list, wzEventCallback callback);

struct wzMenuBar *wz_menu_bar_create();
struct wzMenuBarButton *wz_menu_bar_create_button(struct wzMenuBar *menuBar);

void wz_menu_bar_button_set_label(struct wzMenuBarButton *button, const char *label);
const char *wz_menu_bar_button_get_label(const struct wzMenuBarButton *button);
bool wz_menu_bar_button_is_pressed(const struct wzMenuBarButton *button);

struct wzRadioButton *wz_radio_button_create(const char *label);
void wz_radio_button_set_label(struct wzRadioButton *radioButton, const char *label);
const char *wz_radio_button_get_label(const struct wzRadioButton *radioButton);
bool wz_radio_button_is_set(const struct wzRadioButton *radioButton);
void wz_radio_button_set(struct wzRadioButton *radioButton, bool value);
void wz_radio_button_add_callback_clicked(struct wzRadioButton *radioButton, wzEventCallback callback);

typedef enum
{
	WZ_SCROLLER_VERTICAL,
	WZ_SCROLLER_HORIZONTAL
}
wzScrollerType;

struct wzScroller *wz_scroller_create(wzScrollerType scrollerType, int value, int stepValue, int maxValue);
wzScrollerType wz_scroller_get_type(const struct wzScroller *scroller);
int wz_scroller_get_value(const struct wzScroller *scroller);
void wz_scroller_set_value(struct wzScroller *scroller, int value);
void wz_scroller_decrement_value(struct wzScroller *scroller);
void wz_scroller_increment_value(struct wzScroller *scroller);
void wz_scroller_set_step_value(struct wzScroller *scroller, int stepValue);
int wz_scroller_get_step_value(struct wzScroller *scroller);
void wz_scroller_set_max_value(struct wzScroller *scroller, int maxValue);
void wz_scroller_set_nub_scale(struct wzScroller *scroller, float nubScale);
void wz_scroller_get_nub_state(const struct wzScroller *scroller, wzRect *containerRect, wzRect *rect, bool *hover, bool *pressed);
void wz_scroller_add_callback_value_changed(struct wzScroller *scroller, wzEventCallback callback);

struct wzSpinner *wz_spinner_create();
int wz_spinner_get_value(const struct wzSpinner *spinner);
void wz_spinner_set_value(struct wzSpinner *spinner, int value);

typedef enum
{
	WZ_STACK_LAYOUT_VERTICAL,
	WZ_STACK_LAYOUT_HORIZONTAL,
}
wzStackLayoutDirection;

struct wzStackLayout *wz_stack_layout_create(wzStackLayoutDirection direction, int spacing);
void wz_stack_layout_set_direction(struct wzStackLayout *stackLayout, wzStackLayoutDirection direction);
void wz_stack_layout_set_spacing(struct wzStackLayout *stackLayout, int spacing);
int wz_stack_layout_get_spacing(const struct wzStackLayout *stackLayout);
void wz_stack_layout_add(struct wzStackLayout *stackLayout, struct wzWidget *widget);
void wz_stack_layout_remove(struct wzStackLayout *stackLayout, struct wzWidget *widget);

struct wzTabBar *wz_tab_bar_create();
struct wzButton *wz_tab_bar_create_tab(struct wzTabBar *tabBar);
void wz_tab_bar_destroy_tab(struct wzTabBar *tabBar, struct wzButton *tab);
void wz_tab_bar_clear_tabs(struct wzTabBar *tabBar);
struct wzButton *wz_tab_bar_get_decrement_button(struct wzTabBar *tabBar);
struct wzButton *wz_tab_bar_get_increment_button(struct wzTabBar *tabBar);
struct wzButton *wz_tab_bar_get_selected_tab(struct wzTabBar *tabBar);
void wz_tab_bar_select_tab(struct wzTabBar *tabBar, struct wzButton *tab);
void wz_tab_bar_add_callback_tab_changed(struct wzTabBar *tabBar, wzEventCallback callback);
int wz_tab_bar_get_scroll_value(const struct wzTabBar *tabBar);

struct wzTabbed *wz_tabbed_create();
void wz_tabbed_add_tab(struct wzTabbed *tabbed, struct wzButton **tab, struct wzWidget **page);
void wz_tab_page_add(struct wzWidget *tabPage, struct wzWidget *widget);
void wz_tab_page_remove(struct wzWidget *tabPage, struct wzWidget *widget);

typedef bool (*wzTextEditValidateTextCallback)(const char *text);

struct wzTextEdit *wz_text_edit_create(bool multiline, int maximumTextLength);
void wz_text_edit_set_validate_text_callback(struct wzTextEdit *textEdit, wzTextEditValidateTextCallback callback);
bool wz_text_edit_is_multiline(const struct wzTextEdit *textEdit);
wzBorder wz_text_edit_get_border(const struct wzTextEdit *textEdit);
wzRect wz_text_edit_get_text_rect(const struct wzTextEdit *textEdit);
const char *wz_text_edit_get_text(const struct wzTextEdit *textEdit);
void wz_text_edit_set_text(struct wzTextEdit *textEdit, const char *text);
int wz_text_edit_get_scroll_value(const struct wzTextEdit *textEdit);
const char *wz_text_edit_get_visible_text(const struct wzTextEdit *textEdit);

// y is centered on the line.
wzPosition wz_text_edit_get_cursor_position(const struct wzTextEdit *textEdit);

bool wz_text_edit_has_selection(const struct wzTextEdit *textEdit);

// start is always < end if wz_text_edit_has_selection
int wz_text_edit_get_selection_start_index(const struct wzTextEdit *textEdit);

// y is centered on the line.
wzPosition wz_text_edit_get_selection_start_position(const struct wzTextEdit *textEdit);

// end is always > start if wz_text_edit_has_selection
int wz_text_edit_get_selection_end_index(const struct wzTextEdit *textEdit);

// y is centered on the line.
wzPosition wz_text_edit_get_selection_end_position(const struct wzTextEdit *textEdit);

wzPosition wz_text_edit_position_from_index(const struct wzTextEdit *textEdit, int index);

typedef struct NVGcontext *(*wzNanoVgGlCreate)(int flags);
typedef void (*wzNanoVgGlDestroy)(struct NVGcontext* ctx);

struct wzRenderer *wz_renderer_create(wzNanoVgGlCreate create, wzNanoVgGlDestroy destroy, int flags, const char *fontDirectory, const char *defaultFontFace, float defaultFontSize);
void wz_renderer_destroy(struct wzRenderer *renderer);
const char *wz_renderer_get_error();
struct NVGcontext *wz_renderer_get_context(struct wzRenderer *renderer);

int wz_renderer_create_image(struct wzRenderer *renderer, const char *filename, int *width, int *height);
void wz_renderer_set_font_face(struct wzRenderer *renderer, const char *face);
void wz_renderer_print_box(struct wzRenderer *renderer, wzRect rect, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength);
void wz_renderer_print(struct wzRenderer *renderer, int x, int y, int align, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength);
void wz_renderer_clip_to_rect(struct NVGcontext *vg, wzRect rect);
bool wz_renderer_clip_to_rect_intersection(struct NVGcontext *vg, wzRect rect1, wzRect rect2);
void wz_renderer_draw_filled_rect(struct NVGcontext *vg, wzRect rect, struct NVGcolor color);
void wz_renderer_draw_rect(struct NVGcontext *vg, wzRect rect, struct NVGcolor color);
void wz_renderer_draw_line(struct NVGcontext *vg, int x1, int y1, int x2, int y2, struct NVGcolor color);
void wz_renderer_draw_image(struct NVGcontext *vg, wzRect rect, int image);

enum
{
	WZ_SIDE_TOP = 1<<0,
	WZ_SIDE_RIGHT = 1<<1,
	WZ_SIDE_BOTTOM = 1<<2,
	WZ_SIDE_LEFT = 1<<3,
	WZ_SIDE_ALL = WZ_SIDE_TOP | WZ_SIDE_RIGHT | WZ_SIDE_BOTTOM | WZ_SIDE_LEFT,

	WZ_CORNER_TL = 1<<0,
	WZ_CORNER_TR = 1<<1,
	WZ_CORNER_BR = 1<<2,
	WZ_CORNER_BL = 1<<3,
	WZ_CORNER_ALL = WZ_CORNER_TL | WZ_CORNER_TR | WZ_CORNER_BR | WZ_CORNER_BL
};

void wz_renderer_create_rect_path(struct NVGcontext *vg, wzRect rect, float r, int sides, int roundedCorners);

int wz_renderer_get_line_height(struct wzRenderer *renderer, const char *fontFace, float fontSize);

// width or height can be NULL.
void wz_renderer_measure_text(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int *width, int *height);

typedef struct
{
	const char *start;
	size_t length;
	const char *next;
}
wzLineBreakResult;

wzLineBreakResult wz_renderer_line_break_text(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int lineWidth);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
