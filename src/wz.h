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

#define WZCPP_CALL_OBJECT_METHOD(object, method) ((object)->*(method)) 

namespace wz {

struct WindowImpl;
class Window;
struct MenuBarImpl;

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
	Position(int x, int y) : x(x), y(y) {}
	int x, y;
};

struct Size
{
	Size() : w(0), h(0) {}
	Size(int w, int h) : w(w), h(h) {}
	int w, h;
};

struct Rect
{
	Rect() : x(0), y(0), w(0), h(0) {}
	Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
	bool isEmpty() const { return x == 0 && y == 0 && w == 0 && h == 0; }
	static bool intersect(const Rect A, const Rect B, Rect *result);

	int x, y, w, h;
};

struct Border
{
	Border() : top(0), right(0), bottom(0), left(0) {}
	Border(int top, int right, int bottom, int left) : top(top), right(right), bottom(bottom), left(left) {}
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

void wz_tab_page_add(struct WidgetImpl *tabPage, struct WidgetImpl *widget);
void wz_tab_page_remove(struct WidgetImpl *tabPage, struct WidgetImpl *widget);

typedef struct
{
	const char *start;
	size_t length;
	const char *next;
}
LineBreakResult;

class IRenderer
{
public:
	~IRenderer() {};
	virtual const char *getError() = 0;
	virtual int getLineHeight(const char *fontFace, float fontSize) = 0;

	// width or height can be NULL.
	virtual void measureText(const char *fontFace, float fontSize, const char *text, int n, int *width, int *height) = 0;

	virtual LineBreakResult lineBreakText(const char *fontFace, float fontSize, const char *text, int n, int lineWidth) = 0;
	virtual void drawButton(const ButtonImpl *button, Rect clip) = 0;
	virtual Size measureButton(const ButtonImpl *button) = 0;
};

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

typedef void (*DrawListItemCallback)(IRenderer *renderer, Rect clip, const struct ListImpl *list, const char *fontFace, float fontSize, int itemIndex, const uint8_t *itemData);

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
	MainWindow(IRenderer *renderer);
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

private:
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

typedef enum
{
	WZ_SCROLLER_VERTICAL,
	WZ_SCROLLER_HORIZONTAL
}
ScrollerType;

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

typedef enum
{
	WZ_STACK_LAYOUT_VERTICAL,
	WZ_STACK_LAYOUT_HORIZONTAL,
}
StackLayoutDirection;

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

struct TabImpl;

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

typedef bool (*TextEditValidateTextCallback)(const char *text);

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
