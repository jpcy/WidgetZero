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

struct ButtomImpl;
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
struct TabbedImpl;
struct TextEditImpl;
struct WindowImpl;
class Window;

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

typedef void (*WidgetDrawCallback)(struct WidgetImpl *widget, Rect clip);
typedef Size (*WidgetMeasureCallback)(struct WidgetImpl *widget);

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
	virtual void drawButton(ButtonImpl *button, Rect clip) = 0;
	virtual Size measureButton(ButtonImpl *button) = 0;
	virtual void drawCheckBox(CheckBoxImpl *checkBox, Rect clip) = 0;
	virtual Size measureCheckBox(CheckBoxImpl *checkBox) = 0;
	virtual void drawCombo(ComboImpl *combo, Rect clip) = 0;
	virtual Size measureCombo(ComboImpl *combo) = 0;
	virtual void drawGroupBox(GroupBoxImpl *groupBox, Rect clip) = 0;
	virtual Size measureGroupBox(GroupBoxImpl *groupBox) = 0;
	virtual void drawLabel(LabelImpl *label, Rect clip) = 0;
	virtual Size measureLabel(LabelImpl *label) = 0;
	virtual void drawList(ListImpl *list, Rect clip) = 0;
	virtual Size measureList(ListImpl *list) = 0;
	virtual void drawMenuBarButton(MenuBarButtonImpl *button, Rect clip) = 0;
	virtual Size measureMenuBarButton(MenuBarButtonImpl *button) = 0;
	virtual void drawMenuBar(MenuBarImpl *menuBar, Rect clip) = 0;
	virtual Size measureMenuBar(MenuBarImpl *menuBar) = 0;
	virtual void drawRadioButton(RadioButtonImpl *button, Rect clip) = 0;
	virtual Size measureRadioButton(RadioButtonImpl *button) = 0;
	virtual void drawScroller(ScrollerImpl *scroller, Rect clip) = 0;
	virtual Size measureScroller(ScrollerImpl *scroller) = 0;
	virtual void drawSpinner(SpinnerImpl *spinner, Rect clip) = 0;
	virtual Size measureSpinner(SpinnerImpl *spinner) = 0;
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

	WidgetImpl *getImpl();
	const WidgetImpl *getImpl() const;

protected:
	std::auto_ptr<WidgetImpl> impl;
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
	ButtonImpl *getImpl();
	const ButtonImpl *getImpl() const;
};

class CheckBox : public Widget
{
public:
	CheckBox();
	CheckBox(const std::string &label);
	~CheckBox();
	const char *getLabel() const;
	CheckBox *setLabel(const std::string &label);
	CheckBox *bindValue(bool *value);
	CheckBoxImpl *getImpl();
	const CheckBoxImpl *getImpl() const;
};

class Combo : public Widget
{
public:
	Combo();
	~Combo();
	Combo *setItems(uint8_t *itemData, size_t itemStride, int nItems);
	ComboImpl *getImpl();
	const ComboImpl *getImpl() const;
};

class Frame : public Widget
{
public:
	Frame();
	~Frame();
	Widget *add(Widget *widget);
	void remove(Widget *widget);
	FrameImpl *getImpl();
	const FrameImpl *getImpl() const;
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
	GroupBoxImpl *getImpl();
	const GroupBoxImpl *getImpl() const;
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
	LabelImpl *getImpl();
	const LabelImpl *getImpl() const;
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
	ListImpl *getImpl();
	const ListImpl *getImpl() const;
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
	std::auto_ptr<struct MainWindowImpl> impl;
};

class RadioButton : public Widget
{
public:
	RadioButton();
	RadioButton(const std::string &label);
	~RadioButton();
	const char *getLabel() const;
	RadioButton *setLabel(const std::string &label);
	RadioButtonImpl *getImpl();
	const RadioButtonImpl *getImpl() const;
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
	ScrollerImpl *getImpl();
	const ScrollerImpl *getImpl() const;
};

class Spinner : public Widget
{
public:
	Spinner();
	~Spinner();
	Spinner *setValue(int value);
	int getValue() const;
	SpinnerImpl *getImpl();
	const SpinnerImpl *getImpl() const;
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
	StackLayoutImpl *getImpl();
	const StackLayoutImpl *getImpl() const;
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

	std::auto_ptr<TabImpl> impl;
};

class Tabbed : public Widget
{
public:
	Tabbed();
	~Tabbed();
	Tab *addTab(Tab *tab);
	TabbedImpl *getImpl();
	const TabbedImpl *getImpl() const;
};

typedef bool (*TextEditValidateTextCallback)(const char *text);

class TextEdit : public Widget
{
public:
	TextEdit(bool multiline);
	TextEdit(const std::string &text, bool multiline);
	~TextEdit();
	TextEdit *setText(const std::string &text);
	TextEditImpl *getImpl();
	const TextEditImpl *getImpl() const;
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
	ButtonImpl *getImpl();
	const ButtonImpl *getImpl() const;
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
	WindowImpl *getImpl();
	const WindowImpl *getImpl() const;
};

} // namespace wz
