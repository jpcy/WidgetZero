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

#include <stdint.h>
#include <vector>

#ifndef WZ_ASSERT
#include <assert.h>
#define WZ_ASSERT assert
#endif

#include <nanovg.h>

typedef struct NVGcontext NVGcontext;

#define WZCPP_CALL_OBJECT_METHOD(object, method) ((object)->*(method)) 

namespace wz {

struct wzRenderer;
struct MainWindowImpl;
struct WindowImpl;
struct WidgetImpl;
struct ButtonImpl;
struct CheckBoxImpl;
struct ComboImpl;
struct FrameImpl;
struct GroupBoxImpl;
struct LabelImpl;
struct ListImpl;
struct MenuBarImpl;
struct MenuBarButtonImpl;
struct RadioButtonImpl;
struct ScrollerImpl;
struct SpinnerImpl;
struct StackLayoutImpl;
struct TextEditImpl;

typedef enum
{
	WZ_TYPE_WIDGET,
	WZ_TYPE_MAIN_WINDOW,
	WZ_TYPE_WINDOW,
	WZ_TYPE_BUTTON,
	WZ_TYPE_COMBO,
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
WidgetType;

struct Position
{
	Position() : x(0), y(0) {}
	int x, y;
};

struct Size
{
	Size() : w(0), h(0) {}
	int w, h;
};

struct Rect
{
	Rect() : x(0), y(0), w(0), h(0) {}
	bool isEmpty() const { return x == 0 && y == 0 && w == 0 && h == 0; }
	static bool intersect(const Rect A, const Rect B, Rect *result);

	int x, y, w, h;
};

struct Border
{
	Border() : top(0), right(0), bottom(0), left(0) {}
	int top, right, bottom, left;
};

extern const Border Border_zero;

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
WidgetEventType;

typedef struct
{
	WidgetEventType type;
	struct WidgetImpl *widget;
}
EventBase;

typedef struct
{
	WidgetEventType type;
	struct WidgetImpl *parent;
	struct WidgetImpl *widget; 
	struct WidgetImpl *extra;
}
CreateWidgetEvent;

typedef struct
{
	WidgetEventType type;
	struct WidgetImpl *parent;
	struct WidgetImpl *widget;
}
DestroyWidgetEvent;

typedef struct
{
	WidgetEventType type;
	struct ButtonImpl *button;
	bool isSet;
}
ButtonEvent;

typedef struct
{
	WidgetEventType type;
	struct ListImpl *list;
	int selectedItem;
}
ListEvent;

typedef struct
{
	WidgetEventType type;
	struct ScrollerImpl *scroller;
	int oldValue;
	int value;
}
ScrollerEvent;

typedef struct
{
	WidgetEventType type;
	struct TabBarImpl *tabBar;
	struct ButtonImpl *tab;
}
TabBarEvent;

typedef union
{
	EventBase base;
	CreateWidgetEvent create;
	DestroyWidgetEvent destroy;
	ButtonEvent button;
	ListEvent list;
	ScrollerEvent scroller;
	TabBarEvent tabBar;
}
Event;

typedef void (*EventCallback)(Event *e);

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
Cursor;

typedef enum
{
	WZ_DOCK_POSITION_NONE = -1,
	WZ_DOCK_POSITION_NORTH,
	WZ_DOCK_POSITION_SOUTH,
	WZ_DOCK_POSITION_EAST,
	WZ_DOCK_POSITION_WEST,
	WZ_NUM_DOCK_POSITIONS
}
DockPosition;

#define WZ_MIN(a, b) ((a) < (b) ? (a) : (b))
#define WZ_MAX(a, b) ((a) > (b) ? (a) : (b))
#define WZ_CLAMPED(min, value, max) WZ_MAX(min, WZ_MIN(max, value))
#define WZ_ABS(a) ((a) >= 0 ? (a) : (-a))
#define WZ_SIGN(a) ((a) >= 0 ? 1 : -1)
#define WZ_POINT_IN_RECT(px, py, rect) ((px) >= rect.x && (px) < rect.x + rect.w && (py) >= rect.y && (py) < rect.y + rect.h)
#define WZ_RECTS_OVERLAP(rect1, rect2) (rect1.x < rect2.x + rect2.w && rect1.x + rect1.w > rect2.x && rect1.y < rect2.y + rect2.h && rect1.y + rect1.h > rect2.y) 

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
Key;

#define WZ_KEY_MOD_OFF(key) ((key) & ~(WZ_KEY_SHIFT_BIT | WZ_KEY_CONTROL_BIT))

struct IEventHandler
{
	virtual ~IEventHandler() {}
	virtual void call(Event *e) = 0;

	WidgetEventType eventType;
};

template<class Object>
struct EventHandler : public IEventHandler
{
	typedef void (Object::*Method)(Event *);

	virtual void call(Event *e)
	{
		WZCPP_CALL_OBJECT_METHOD(object, method)(e);
	}

	Object *object;
	Method method;
};

// Set the centralized event handler. All events invoked by the ancestor widgets of this mainWindow will call the callback function.
void wz_main_window_set_event_callback(struct MainWindowImpl *mainWindow, EventCallback callback);

void wz_main_window_mouse_button_down(struct MainWindowImpl *mainWindow, int mouseButton, int mouseX, int mouseY);
void wz_main_window_mouse_button_up(struct MainWindowImpl *mainWindow, int mouseButton, int mouseX, int mouseY);
void wz_main_window_mouse_move(struct MainWindowImpl *mainWindow, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
void wz_main_window_mouse_wheel_move(struct MainWindowImpl *mainWindow, int x, int y);
void wz_main_window_key_down(struct MainWindowImpl *mainWindow, Key key);
void wz_main_window_key_up(struct MainWindowImpl *mainWindow, Key key);
void wz_main_window_text_input(struct MainWindowImpl *mainWindow, const char *text);
void wz_main_window_draw(struct MainWindowImpl *mainWindow);
void wz_main_window_draw_frame(struct MainWindowImpl *mainWindow);
void wz_main_window_set_menu_bar(struct MainWindowImpl *mainWindow, struct MenuBarImpl *menuBar);
void wz_main_window_add(struct MainWindowImpl *mainWindow, struct WidgetImpl *widget);
void wz_main_window_remove(struct MainWindowImpl *mainWindow, struct WidgetImpl *widget);
void wz_main_window_toggle_text_cursor(struct MainWindowImpl *mainWindow);
Cursor wz_main_window_get_cursor(const struct MainWindowImpl *mainWindow);
const struct WidgetImpl *wz_main_window_get_keyboard_focus_widget(const struct MainWindowImpl *mainWindow);
void wz_main_window_dock_window(struct MainWindowImpl *mainWindow, struct WindowImpl *window, DockPosition dockPosition);

typedef void (*WidgetDrawCallback)(struct WidgetImpl *widget, Rect clip);
typedef Size (*WidgetMeasureCallback)(struct WidgetImpl *widget);

void wz_widget_destroy(struct WidgetImpl *widget);
struct MainWindowImpl *wz_widget_get_main_window(struct WidgetImpl *widget);
WidgetType wz_widget_get_type(const struct WidgetImpl *widget);
bool wz_widget_is_layout(const struct WidgetImpl *widget);

// Resize the widget to the result of calling the widget "measure" callback.
void wz_widget_resize_to_measured(struct WidgetImpl *widget);

void wz_widget_set_position_args(struct WidgetImpl *widget, int x, int y);
void wz_widget_set_position(struct WidgetImpl *widget, Position position);
Position wz_widget_get_position(const struct WidgetImpl *widget);
Position wz_widget_get_absolute_position(const struct WidgetImpl *widget);
void wz_widget_set_width(struct WidgetImpl *widget, int w);
void wz_widget_set_height(struct WidgetImpl *widget, int h);
void wz_widget_set_size_args(struct WidgetImpl *widget, int w, int h);
void wz_widget_set_size(struct WidgetImpl *widget, Size size);
int wz_widget_get_width(const struct WidgetImpl *widget);
int wz_widget_get_height(const struct WidgetImpl *widget);
Size wz_widget_get_size(const struct WidgetImpl *widget);
void wz_widget_set_rect_args(struct WidgetImpl *widget, int x, int y, int w, int h);
void wz_widget_set_rect(struct WidgetImpl *widget, Rect rect);
Rect wz_widget_get_rect(const struct WidgetImpl *widget);
Rect wz_widget_get_absolute_rect(const struct WidgetImpl *widget);
void wz_widget_set_margin(struct WidgetImpl *widget, Border margin);
void wz_widget_set_margin_args(struct WidgetImpl *widget, int top, int right, int bottom, int left);
void wz_widget_set_margin_uniform(struct WidgetImpl *widget, int value);
Border wz_widget_get_margin(const struct WidgetImpl *widget);
void wz_widget_set_stretch(struct WidgetImpl *widget, int stretch);
int wz_widget_get_stretch(const struct WidgetImpl *widget);
void wz_widget_set_stretch_scale(struct WidgetImpl *widget, float width, float height);
float wz_widget_get_stretch_width_scale(const struct WidgetImpl *widget);
float wz_widget_get_stretch_height_scale(const struct WidgetImpl *widget);
void wz_widget_set_align(struct WidgetImpl *widget, int align);
int wz_widget_get_align(const struct WidgetImpl *widget);
void wz_widget_set_font_face(struct WidgetImpl *widget, const char *fontFace);
const char *wz_widget_get_font_face(const struct WidgetImpl *widget);
void wz_widget_set_font_size(struct WidgetImpl *widget, float fontSize);
float wz_widget_get_font_size(const struct WidgetImpl *widget);
void wz_widget_set_font(struct WidgetImpl *widget, const char *fontFace, float fontSize);
bool wz_widget_get_hover(const struct WidgetImpl *widget);
void wz_widget_set_visible(struct WidgetImpl *widget, bool visible);
bool wz_widget_get_visible(const struct WidgetImpl *widget);
bool wz_widget_has_keyboard_focus(const struct WidgetImpl *widget);
void wz_widget_set_metadata(struct WidgetImpl *widget, void *metadata);
void *wz_widget_get_metadata(struct WidgetImpl *widget);
void wz_widget_set_draw_callback(struct WidgetImpl *widget, WidgetDrawCallback draw);
void wz_widget_set_measure_callback(struct WidgetImpl *widget, WidgetMeasureCallback measure);

// Determine whether the widget is an descendant of a widget with the provided type.
bool wz_widget_is_descendant_of(struct WidgetImpl *widget, WidgetType type);

struct WidgetImpl *wz_widget_get_parent(struct WidgetImpl *widget);

struct WindowImpl *wz_widget_get_parent_window(struct WidgetImpl *widget);

int wz_window_get_header_height(const struct WindowImpl *window);
int wz_window_get_border_size(const struct WindowImpl *window);
Rect wz_window_get_header_rect(const struct WindowImpl *window);
void wz_window_set_title(struct WindowImpl *window, const char *title);
const char *wz_window_get_title(const struct WindowImpl *window);
void wz_window_add(struct WindowImpl *window, struct WidgetImpl *widget);
void wz_window_remove(struct WindowImpl *window, struct WidgetImpl *widget);

void wz_button_set_label(struct ButtonImpl *button, const char *label);
const char *wz_button_get_label(const struct ButtonImpl *button);
void wz_button_set_icon(struct ButtonImpl *button, const char *icon);
const char *wz_button_get_icon(const struct ButtonImpl *button);
void wz_button_set_padding(struct ButtonImpl *button, Border padding);
void wz_button_set_padding_args(struct ButtonImpl *button, int top, int right, int bottom, int left);
Border wz_button_get_padding(const struct ButtonImpl *button);
bool wz_button_is_pressed(const struct ButtonImpl *button);
bool wz_button_is_set(const struct ButtonImpl *button);
void wz_button_set(struct ButtonImpl *button, bool value);
void wz_button_bind_value(struct ButtonImpl *button, bool *value);
void wz_button_add_callback_pressed(struct ButtonImpl *button, EventCallback callback);
void wz_button_add_callback_clicked(struct ButtonImpl *button, EventCallback callback);

void wz_check_box_set_label(struct CheckBoxImpl *checkBox, const char *label);
const char *wz_check_box_get_label(const struct CheckBoxImpl *checkBox);
bool wz_check_box_is_checked(const struct CheckBoxImpl *checkBox);
void wz_check_box_check(struct CheckBoxImpl *checkBox, bool value);
void wz_check_box_bind_value(struct CheckBoxImpl *checkBox, bool *value);
void wz_check_box_add_callback_checked(struct CheckBoxImpl *checkBox, EventCallback callback);

struct ListImpl *wz_combo_get_list(const struct ComboImpl *combo);
bool wz_combo_is_open(struct ComboImpl *combo);

void wz_frame_add(struct FrameImpl *frame, struct WidgetImpl *widget);
void wz_frame_remove(struct FrameImpl *frame, struct WidgetImpl *widget);

void wz_group_box_set_label(struct GroupBoxImpl *groupBox, const char *label);
const char *wz_group_box_get_label(const struct GroupBoxImpl *groupBox);
void wz_group_box_add(struct GroupBoxImpl *groupBox, struct WidgetImpl *widget);
void wz_group_box_remove(struct GroupBoxImpl *groupBox, struct WidgetImpl *widget);

void wz_label_set_multiline(struct LabelImpl *label, bool multiline);
bool wz_label_get_multiline(const struct LabelImpl *label);
void wz_label_set_text(struct LabelImpl *label, const char *text);
const char *wz_label_get_text(const struct LabelImpl *label);
void wz_label_set_text_color(struct LabelImpl *label, NVGcolor color);
NVGcolor wz_label_get_text_color(const struct LabelImpl *label);

typedef void (*DrawListItemCallback)(struct wzRenderer *renderer, Rect clip, const struct ListImpl *list, const char *fontFace, float fontSize, int itemIndex, const uint8_t *itemData);

Border wz_list_get_items_border(const struct ListImpl *list);
Rect wz_list_get_items_rect(const struct ListImpl *list);

// rect will be absolute - ancestor window position is taken into account.
Rect wz_list_get_absolute_items_rect(const struct ListImpl *list);

void wz_list_set_draw_item_callback(struct ListImpl *list, DrawListItemCallback callback);
DrawListItemCallback wz_list_get_draw_item_callback(const struct ListImpl *list);
void wz_list_set_item_data(struct ListImpl *list, uint8_t *itemData);
uint8_t *wz_list_get_item_data(const struct ListImpl *list);
void wz_list_set_item_stride(struct ListImpl *list, int itemStride);
int wz_list_get_item_stride(const struct ListImpl *list);
void wz_list_set_item_height(struct ListImpl *list, int itemHeight);
int wz_list_get_item_height(const struct ListImpl *list);
void wz_list_set_num_items(struct ListImpl *list, int nItems);
int wz_list_get_num_items(const struct ListImpl *list);
int wz_list_get_first_item(const struct ListImpl *list);
void wz_list_set_selected_item(struct ListImpl *list, int selectedItem);
int wz_list_get_selected_item(const struct ListImpl *list);
int wz_list_get_pressed_item(const struct ListImpl *list);
int wz_list_get_hovered_item(const struct ListImpl *list);
int wz_list_get_scroll_value(const struct ListImpl *list);
void wz_list_add_callback_item_selected(struct ListImpl *list, EventCallback callback);

struct MenuBarButtonImpl *wz_menu_bar_create_button(struct MenuBarImpl *menuBar);

void wz_menu_bar_button_set_label(struct MenuBarButtonImpl *button, const char *label);
const char *wz_menu_bar_button_get_label(const struct MenuBarButtonImpl *button);
bool wz_menu_bar_button_is_pressed(const struct MenuBarButtonImpl *button);

void wz_radio_button_set_label(struct RadioButtonImpl *radioButton, const char *label);
const char *wz_radio_button_get_label(const struct RadioButtonImpl *radioButton);
bool wz_radio_button_is_set(const struct RadioButtonImpl *radioButton);
void wz_radio_button_set(struct RadioButtonImpl *radioButton, bool value);
void wz_radio_button_add_callback_clicked(struct RadioButtonImpl *radioButton, EventCallback callback);

typedef enum
{
	WZ_SCROLLER_VERTICAL,
	WZ_SCROLLER_HORIZONTAL
}
ScrollerType;

ScrollerType wz_scroller_get_type(const struct ScrollerImpl *scroller);
int wz_scroller_get_value(const struct ScrollerImpl *scroller);
void wz_scroller_set_value(struct ScrollerImpl *scroller, int value);
void wz_scroller_decrement_value(struct ScrollerImpl *scroller);
void wz_scroller_increment_value(struct ScrollerImpl *scroller);
void wz_scroller_set_step_value(struct ScrollerImpl *scroller, int stepValue);
int wz_scroller_get_step_value(struct ScrollerImpl *scroller);
void wz_scroller_set_max_value(struct ScrollerImpl *scroller, int maxValue);
void wz_scroller_set_nub_scale(struct ScrollerImpl *scroller, float nubScale);
void wz_scroller_get_nub_state(const struct ScrollerImpl *scroller, Rect *containerRect, Rect *rect, bool *hover, bool *pressed);
void wz_scroller_add_callback_value_changed(struct ScrollerImpl *scroller, EventCallback callback);

int wz_spinner_get_value(const struct SpinnerImpl *spinner);
void wz_spinner_set_value(struct SpinnerImpl *spinner, int value);

typedef enum
{
	WZ_STACK_LAYOUT_VERTICAL,
	WZ_STACK_LAYOUT_HORIZONTAL,
}
StackLayoutDirection;

void wz_stack_layout_set_direction(struct StackLayoutImpl *stackLayout, StackLayoutDirection direction);
void wz_stack_layout_set_spacing(struct StackLayoutImpl *stackLayout, int spacing);
int wz_stack_layout_get_spacing(const struct StackLayoutImpl *stackLayout);
void wz_stack_layout_add(struct StackLayoutImpl *stackLayout, struct WidgetImpl *widget);
void wz_stack_layout_remove(struct StackLayoutImpl *stackLayout, struct WidgetImpl *widget);

struct ButtonImpl *wz_tab_bar_create_tab(struct TabBarImpl *tabBar);
void wz_tab_bar_destroy_tab(struct TabBarImpl *tabBar, struct ButtonImpl *tab);
void wz_tab_bar_clear_tabs(struct TabBarImpl *tabBar);
struct ButtonImpl *wz_tab_bar_get_decrement_button(struct TabBarImpl *tabBar);
struct ButtonImpl *wz_tab_bar_get_increment_button(struct TabBarImpl *tabBar);
struct ButtonImpl *wz_tab_bar_get_selected_tab(struct TabBarImpl *tabBar);
void wz_tab_bar_select_tab(struct TabBarImpl *tabBar, struct ButtonImpl *tab);
void wz_tab_bar_add_callback_tab_changed(struct TabBarImpl *tabBar, EventCallback callback);
int wz_tab_bar_get_scroll_value(const struct TabBarImpl *tabBar);

void wz_tabbed_add_tab(struct TabbedImpl *tabbed, struct ButtonImpl **tab, struct WidgetImpl **page);
void wz_tab_page_add(struct WidgetImpl *tabPage, struct WidgetImpl *widget);
void wz_tab_page_remove(struct WidgetImpl *tabPage, struct WidgetImpl *widget);

typedef bool (*TextEditValidateTextCallback)(const char *text);

void wz_text_edit_set_validate_text_callback(struct TextEditImpl *textEdit, TextEditValidateTextCallback callback);
bool wz_text_edit_is_multiline(const struct TextEditImpl *textEdit);
Border wz_text_edit_get_border(const struct TextEditImpl *textEdit);
Rect wz_text_edit_get_text_rect(const struct TextEditImpl *textEdit);
const char *wz_text_edit_get_text(const struct TextEditImpl *textEdit);
void wz_text_edit_set_text(struct TextEditImpl *textEdit, const char *text);
int wz_text_edit_get_scroll_value(const struct TextEditImpl *textEdit);
const char *wz_text_edit_get_visible_text(const struct TextEditImpl *textEdit);

// y is centered on the line.
Position wz_text_edit_get_cursor_position(const struct TextEditImpl *textEdit);

bool wz_text_edit_has_selection(const struct TextEditImpl *textEdit);

// start is always < end if wz_text_edit_has_selection
int wz_text_edit_get_selection_start_index(const struct TextEditImpl *textEdit);

// y is centered on the line.
Position wz_text_edit_get_selection_start_position(const struct TextEditImpl *textEdit);

// end is always > start if wz_text_edit_has_selection
int wz_text_edit_get_selection_end_index(const struct TextEditImpl *textEdit);

// y is centered on the line.
Position wz_text_edit_get_selection_end_position(const struct TextEditImpl *textEdit);

Position wz_text_edit_position_from_index(const struct TextEditImpl *textEdit, int index);

typedef struct NVGcontext *(*wzNanoVgGlCreate)(int flags);
typedef void (*wzNanoVgGlDestroy)(struct NVGcontext* ctx);

struct wzRenderer *wz_renderer_create(wzNanoVgGlCreate create, wzNanoVgGlDestroy destroy, int flags, const char *fontDirectory, const char *defaultFontFace, float defaultFontSize);
void wz_renderer_destroy(struct wzRenderer *renderer);
const char *wz_renderer_get_error();
struct NVGcontext *wz_renderer_get_context(struct wzRenderer *renderer);

int wz_renderer_create_image(struct wzRenderer *renderer, const char *filename, int *width, int *height);
void wz_renderer_set_font_face(struct wzRenderer *renderer, const char *face);
void wz_renderer_print_box(struct wzRenderer *renderer, Rect rect, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength);
void wz_renderer_print(struct wzRenderer *renderer, int x, int y, int align, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength);
void wz_renderer_clip_to_rect(struct NVGcontext *vg, Rect rect);
bool wz_renderer_clip_to_rect_intersection(struct NVGcontext *vg, Rect rect1, Rect rect2);
void wz_renderer_draw_filled_rect(struct NVGcontext *vg, Rect rect, struct NVGcolor color);
void wz_renderer_draw_rect(struct NVGcontext *vg, Rect rect, struct NVGcolor color);
void wz_renderer_draw_line(struct NVGcontext *vg, int x1, int y1, int x2, int y2, struct NVGcolor color);
void wz_renderer_draw_image(struct NVGcontext *vg, Rect rect, int image);

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

void wz_renderer_create_rect_path(struct NVGcontext *vg, Rect rect, float r, int sides, int roundedCorners);

int wz_renderer_get_line_height(struct wzRenderer *renderer, const char *fontFace, float fontSize);

// width or height can be NULL.
void wz_renderer_measure_text(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int *width, int *height);

typedef struct
{
	const char *start;
	size_t length;
	const char *next;
}
LineBreakResult;

LineBreakResult wz_renderer_line_break_text(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int lineWidth);

class Button;
class Checkbox;
class Combo;
class GroupBox;
class Label;
class List;
class RadioButton;
class Scroller;
class StackLayout;
class Tab;
struct TabImpl;
class Tabbed;
class TextEdit;
class ToggleButton;
class Widget;
class Window;

class Widget
{
public:
	virtual ~Widget();
	Rect getRect() const;
	Widget *setPosition(int x, int y);
	Widget *setWidth(int w);
	Widget *setHeight(int h);
	Widget *setSize(int w, int h);
	Widget *setRect(int x, int y, int w, int h);
	Widget *setStretch(int stretch);
	Widget *setAlign(int align);
	Widget *setMargin(int margin);
	Widget *setMargin(int top, int right, int bottom, int left);
	Widget *setMargin(Border margin);
	Widget *setFontFace(const std::string &fontFace);
	Widget *setFontSize(float fontSize);
	Widget *setFont(const std::string &fontFace, float fontSize);
	Widget *setVisible(bool visible);

	Widget *addEventHandler(IEventHandler *eventHandler);

	template<class Object>
	Widget *addEventHandler(WidgetEventType eventType, Object *object, void (Object::*method)(Event *))
	{
		EventHandler<Object> *eventHandler = new EventHandler<Object>();
		eventHandler->eventType = eventType;
		eventHandler->object = object;
		eventHandler->method = method;
		addEventHandler(eventHandler);
		return this;
	}

	WidgetImpl *impl;
};

class Button : public Widget
{
public:
	Button();
	Button(const std::string &label, const std::string &icon = std::string());
	~Button();
	Border getPadding() const;
	Button *setPadding(Border padding);
	Button *setPadding(int top, int right, int bottom, int left);
	const char *getIcon() const;
	Button *setIcon(const std::string &icon);
	const char *getLabel() const;
	Button *setLabel(const std::string &label);
};

class Checkbox : public Widget
{
public:
	Checkbox();
	Checkbox( const std::string &label);
	~Checkbox();
	const char *getLabel() const;
	Checkbox *setLabel(const std::string &label);
	Checkbox *bindValue(bool *value);
};

class Combo : public Widget
{
public:
	Combo();
	~Combo();
	Combo *setItems(uint8_t *itemData, size_t itemStride, int nItems);
};

class Frame : public Widget
{
public:
	Frame();
	~Frame();
	Widget *add(Widget *widget);
	void remove(Widget *widget);
};

class GroupBox : public Widget
{
public:
	GroupBox();
	GroupBox(const std::string &label);
	~GroupBox();
	const char *getLabel() const;
	GroupBox *setLabel(const std::string &label);
	Widget *add(Widget *widget);
	void remove(Widget *widget);
};

class Label : public Widget
{
public:
	Label();
	Label(const std::string &text);
	~Label();
	Label *setText(const char *format, ...);
	Label *setTextColor(float r, float g, float b, float a = 1.0f);
	Label *setMultiline(bool multiline);
};

class List : public Widget
{
public:
	List();
	~List();
	List *setItems(uint8_t *itemData, size_t itemStride, int nItems);
	List *setSelectedItem(int index);
	List *setItemHeight(int height);
	List *setDrawItemCallback(DrawListItemCallback callback);
};

class MainWindow
{
public:
	MainWindow(wzRenderer *renderer);
	~MainWindow();
	int getWidth() const;
	int getHeight() const;
	void setSize(int w, int h);
	void mouseMove(int x, int y, int dx, int dy);
	void mouseButtonDown(int button, int x, int y);
	void mouseButtonUp(int button, int x, int y);
	void mouseWheelMove(int x, int y);
	void keyDown(Key key);
	void keyUp(Key key);
	void textInput(const char *text);
	void beginFrame();
	void draw();
	void drawFrame();
	void endFrame();
	void toggleTextCursor();
	Cursor getCursor() const;
	Widget *add(Widget *widget);
	void remove(Widget *widget);
	void createMenuButton(const std::string &label);
	void dockWindow(Window *window, DockPosition dockPosition);

	MainWindowImpl *impl;
};

class RadioButton : public Widget
{
public:
	RadioButton();
	RadioButton(const std::string &label);
	~RadioButton();
	const char *getLabel() const;
	RadioButton *setLabel(const std::string &label);
};

class Scroller : public Widget
{
public:
	Scroller(ScrollerType type);
	~Scroller();
	Scroller *setValue(int value);
	Scroller *setStepValue(int stepValue);
	Scroller *setMaxValue(int maxValue);	
	int getValue() const;
};

class Spinner : public Widget
{
public:
	Spinner();
	~Spinner();
	Spinner *setValue(int value);
	int getValue() const;
};

class StackLayout : public Widget
{
public:
	StackLayout();
	StackLayout(StackLayoutDirection direction);
	~StackLayout();
	StackLayout *setDirection(StackLayoutDirection direction);
	StackLayout *setSpacing(int spacing);
	int getSpacing() const;
	Widget *add(Widget *widget);
	void remove(Widget *widget);
};

class Tab
{
public:
	Tab();
	~Tab();
	Tab *setLabel(const std::string &label);
	Widget *add(Widget *widget);
	void remove(Widget *widget);

	TabImpl *impl;
};

class Tabbed : public Widget
{
public:
	Tabbed();
	~Tabbed();
	Tab *addTab(Tab *tab);
};

class TextEdit : public Widget
{
public:
	TextEdit(bool multiline);
	TextEdit(const std::string &text, bool multiline);
	~TextEdit();
	TextEdit *setText(const std::string &text);
};

class ToggleButton : public Widget
{
public:
	ToggleButton();
	ToggleButton(const std::string &label, const std::string &icon = std::string());
	~ToggleButton();
	Border getPadding() const;
	ToggleButton *setPadding(Border padding);
	ToggleButton *setPadding(int top, int right, int bottom, int left);
	const char *getIcon() const;
	ToggleButton *setIcon(const std::string &icon);
	const char *getLabel() const;
	ToggleButton *setLabel(const std::string &label);
};

class Window : public Widget
{
public:
	Window();
	Window(const std::string &title);
	~Window();
	const char *getTitle() const;
	Window *setTitle(const std::string &title);
	Widget *add(Widget *widget);
	void remove(Widget *widget);
};

} // namespace wz
