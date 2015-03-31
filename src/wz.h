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
IED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <stdint.h>
#include <vector>
#include <string>

#ifndef WZ_ASSERT
#ifdef _MSC_VER
#include <crtdbg.h>
#define WZ_ASSERT _ASSERTE
#else
#include <assert.h>
#define WZ_ASSERT assert
#endif
#endif

#ifdef WZ_USE_PCH
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#endif

#define WZCPP_CALL_OBJECT_METHOD(object, method) ((object)->*(method)) 

#define WZ_MAX_WINDOWS 256

namespace wz {

class Button;
class CheckBox;
class Combo;
class DockIcon;
class DockPreview;
class GroupBox;
class Label;
class List;
class MainWindow;
class MenuBarButton;
class MenuBar;
class RadioButton;
class Scroller;
class Spinner;
class StackLayout;
class TabBar;
class TabButton;
class Tabbed;
class TextEdit;
class Widget;
class Window;

struct WidgetType
{
	enum Enum
	{
		Widget,
		MainWindow,
		Window,
		Button,
		CheckBox,
		Combo,
		DockIcon,
		Frame,
		GroupBox,
		Label,
		List,
		MenuBar,
		MenuBarButton,
		RadioButton,
		Scroller,
		Spinner,
		StackLayout,
		TabBar,
		TabPage,
		Tabbed,
		TextEdit
	};
};

struct Border
{
	Border() : top(0), right(0), bottom(0), left(0) {}
	Border(int top, int right, int bottom, int left) : top(top), right(right), bottom(bottom), left(left) {}
	Border(int uniform) : top(uniform), right(uniform), bottom(uniform), left(uniform) {}
	Border operator+(Border b) const { return Border(top + b.top, right + b.right, bottom + b.bottom, left + b.left); }
	int top, right, bottom, left;
};

struct Color
{
	Color() : r(0), g(0), b(0), a(0) {}
	Color(float r, float g, float b, float a = 1) : r(r), g(g), b(b), a(a) {}
	Color(float *rgba) { r = rgba[0]; g = rgba[1]; b = rgba[2]; a = rgba[3]; }
	float r, g, b, a;
};

struct Position
{
	Position() : x(0), y(0) {}
	Position(int x, int y) : x(x), y(y) {}
	int x, y;
};

struct Rect
{
	Rect() : x(0), y(0), w(0), h(0) {}
	Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
	bool isEmpty() const { return x == 0 && y == 0 && w == 0 && h == 0; }
	Rect operator-(Border b) const { return Rect(x + b.left, y + b.top, w - (b.left + b.right), h - (b.top + b.bottom)); }
	bool operator==(Rect r) const { return x == r.x && y == r.y && w == r.w && h == r.h; }
	bool operator!=(Rect r) const { return x != r.x || y != r.y || w != r.w || h != r.h; }
	static bool intersect(const Rect A, const Rect B, Rect *result);

	int x, y, w, h;
};

struct Size
{
	Size() : w(0), h(0) {}
	Size(int w, int h) : w(w), h(h) {}
	bool operator==(Size s) const { return w == s.w && h == s.h; }
	Size operator+(Size s) { return Size(w + s.w, h + s.h); }
	Size operator+=(Size s) { w += s.w; h += s.h; return *this; }
	int w, h;
};

struct Stretch
{
	enum Enum
	{
		None,
		Width = 1,
		Height = 2,
		All = Width | Height
	};
};

inline Stretch::Enum operator|(Stretch::Enum a, Stretch::Enum b)
{
	return Stretch::Enum(int(a) | int(b));
}

struct Align
{
	enum Enum
	{
		None,

		// Horizontal.
		Left = 1,
		Center = 2,
		Right = 4,

		// Vertical.
		Top = 8,
		Middle = 16,
		Bottom = 32
	};
};

inline Align::Enum operator|(Align::Enum a, Align::Enum b)
{
	return Align::Enum(int(a) | int(b));
}

struct EventType
{
	enum Enum
	{
		Unknown,
		ButtonPressed,
		ButtonClicked,
		ListItemSelected,
		ScrollerValueChanged,
		TabBarTabChanged,
		TabBarTabAdded,
		TabBarTabRemoved
	};
};

struct EventBase
{
	EventType::Enum type;
	Widget *widget;
};

struct CreateWidgetEvent
{
	EventType::Enum type;
	Widget *parent;
	Widget *widget; 
	Widget *extra;
};

struct DestroyWidgetEvent
{
	EventType::Enum type;
	Widget *parent;
	Widget *widget;
};

struct ButtonEvent
{
	EventType::Enum type;
	Button *button;
	bool isSet;
};

struct ListEvent
{
	EventType::Enum type;
	List *list;
	int selectedItem;
};

struct ScrollerEvent
{
	EventType::Enum type;
	Scroller *scroller;
	int oldValue;
	int value;
};

struct TabBarEvent
{
	EventType::Enum type;
	TabBar *tabBar;
	TabButton *tab;
};

union Event
{
	EventBase base;
	CreateWidgetEvent create;
	DestroyWidgetEvent destroy;
	ButtonEvent button;
	ListEvent list;
	ScrollerEvent scroller;
	TabBarEvent tabBar;
};

typedef void (*EventCallback)(Event e);

struct Cursor
{
	enum Enum
	{
		Default,
		Ibeam,
		Resize_N_S,
		Resize_E_W,
		Resize_NE_SW,
		Resize_NW_SE,
		NumCursors
	};
};

struct DockPosition
{
	enum Enum
	{
		None = -1,
		North,
		South,
		East,
		West,
		NumDockPositions
	};
};

#define WZ_MIN(a, b) ((a) < (b) ? (a) : (b))
#define WZ_MAX(a, b) ((a) > (b) ? (a) : (b))
#define WZ_CLAMPED(min, value, max) WZ_MAX(min, WZ_MIN(max, value))
#define WZ_ABS(a) ((a) >= 0 ? (a) : (-a))
#define WZ_SIGN(a) ((a) >= 0 ? 1 : -1)
#define WZ_POINT_IN_RECT(px, py, rect) ((px) >= rect.x && (px) < rect.x + rect.w && (py) >= rect.y && (py) < rect.y + rect.h)
#define WZ_RECTS_OVERLAP(rect1, rect2) (rect1.x < rect2.x + rect2.w && rect1.x + rect1.w > rect2.x && rect1.y < rect2.y + rect2.h && rect1.y + rect1.h > rect2.y) 

struct Key
{
	enum Enum
	{
		Unknown,
		LeftArrow,
		RightArrow,
		UpArrow,
		DownArrow,
		Home,
		End,
		Enter,
		Delete,
		Backspace,
		LeftShift,
		RightShift,
		LeftControl,
		RightControl,
		NumKeys,

		ShiftBit = (1 << 10),
		ControlBit = (1 << 11)
	};
};

#define WZ_KEY_MOD_OFF(key) ((key) & ~(Key::ShiftBit | Key::ControlBit))

struct IEventHandler
{
	virtual ~IEventHandler() {}
	virtual void call(Event e) = 0;

	EventType::Enum eventType;
};

template<class Object>
struct EventHandler : public IEventHandler
{
	typedef void (Object::*Method)(Event);

	virtual void call(Event e)
	{
		WZCPP_CALL_OBJECT_METHOD(object, method)(e);
	}

	Object *object;
	Method method;
};

struct LineBreakResult
{
	const char *start;
	size_t length;
	const char *next;
};

class IRenderer
{
public:
	virtual ~IRenderer();
	virtual Color getClearColor();
	virtual void beginFrame(int windowWidth, int windowHeight);
	virtual void endFrame();
	virtual void drawButton(Button *button, Rect clip);
	virtual Size measureButton(Button *button);
	virtual void drawCheckBox(CheckBox *checkBox, Rect clip);
	virtual Size measureCheckBox(CheckBox *checkBox);
	virtual void drawCombo(Combo *combo, Rect clip);
	virtual Size measureCombo(Combo *combo);
	virtual void drawDockIcon(DockIcon *dockIcon, Rect clip);
	virtual void drawDockPreview(DockPreview *dockPreview, Rect clip);
	virtual Border getGroupBoxMargin(GroupBox *groupBox);
	virtual void drawGroupBox(GroupBox *groupBox, Rect clip);
	virtual Size measureGroupBox(GroupBox *groupBox);
	virtual Color getLabelTextColor(Label *label);
	virtual void drawLabel(Label *label, Rect clip);
	virtual Size measureLabel(Label *label);
	virtual void drawList(List *list, Rect clip);
	virtual Size measureList(List *list);
	virtual void drawMenuBarButton(MenuBarButton *button, Rect clip);
	virtual Size measureMenuBarButton(MenuBarButton *button);
	virtual int getMenuBarPadding(MenuBar *menuBar);
	virtual void drawMenuBar(MenuBar *menuBar, Rect clip);
	virtual Size measureMenuBar(MenuBar *menuBar);
	virtual void drawRadioButton(RadioButton *button, Rect clip);
	virtual Size measureRadioButton(RadioButton *button);
	virtual void drawScrollerDecrementButton(Button *button, Rect clip);
	virtual void drawScrollerIncrementButton(Button *button, Rect clip);
	virtual void drawScroller(Scroller *scroller, Rect clip);
	virtual Size measureScroller(Scroller *scroller);
	virtual int getSpinnerButtonWidth(Spinner *spinner);
	virtual void drawSpinnerDecrementButton(Button *button, Rect clip);
	virtual void drawSpinnerIncrementButton(Button *button, Rect clip);
	virtual void drawSpinner(Spinner *spinner, Rect clip);
	virtual Size measureSpinner(Spinner *spinner);
	virtual void drawTabButton(TabButton *button, Rect clip);
	virtual int getTabBarScrollButtonWidth(TabBar *tabBar);
	virtual void drawTabBarDecrementButton(Button *button, Rect clip);
	virtual void drawTabBarIncrementButton(Button *button, Rect clip);
	virtual void drawTabBar(TabBar *tabBar, Rect clip);
	virtual Size measureTabBar(TabBar *tabBar);
	virtual void drawTabbed(Tabbed *tabbed, Rect clip);
	virtual Size measureTabbed(Tabbed *tabbed);
	virtual void drawTextEdit(TextEdit *textEdit, Rect clip);
	virtual Size measureTextEdit(TextEdit *textEdit);
	virtual void drawWindow(Window *window, Rect clip);
	virtual Size measureWindow(Window *window);

	virtual int getLineHeight(const char *fontFace, float fontSize);

	// width or height can be NULL.
	virtual void measureText(const char *fontFace, float fontSize, const char *text, int n, int *width, int *height);

	virtual LineBreakResult lineBreakText(const char *fontFace, float fontSize, const char *text, int n, int lineWidth);
};

struct WidgetFlags
{
	enum Enum
	{
		None = 0,
		DrawLast = 1 << 0,
		MeasureDirty = 1 << 1,
		RectDirty = 1 << 2
	};
};

inline WidgetFlags::Enum operator|(WidgetFlags::Enum a, WidgetFlags::Enum b)
{
	return WidgetFlags::Enum(int(a) | int(b));
}

class Widget
{
	friend class MainWindow;

public:
	Widget();
	virtual ~Widget();
	WidgetType::Enum getType() const;
	bool isLayoutWidget() const;
	const MainWindow *getMainWindow() const;
	MainWindow *getMainWindow();
	void setPosition(int x, int y);
	void setPosition(Position position);
	Position getPosition() const;
	Position getAbsolutePosition() const;
	void setWidth(int w);
	void setHeight(int h);
	void setSize(int w, int h);
	void setSize(Size size);
	int getWidth() const;
	int getHeight() const;
	Size getSize() const;
	void setRect(int x, int y, int w, int h);
	void setRect(Rect rect);
	Rect getRect() const;
	Rect getAbsoluteRect() const;
	void setMargin(Border margin);
	void setMargin(int top, int right, int bottom, int left);
	Border getMargin() const;
	void setPadding(Border padding);
	void setPadding(int top, int right, int bottom, int left);
	Border getPadding() const;
	void setStretch(Stretch::Enum stretch);
	int getStretch() const;
	void setStretchScale(float width, float height);
	float getStretchWidthScale() const;
	float getStretchHeightScale() const;
	void setAlign(Align::Enum align);
	int getAlign() const;
	void setFontFace(const char *fontFace);
	const char *getFontFace() const;
	void setFontSize(float fontSize);
	float getFontSize() const;
	void setFont(const char *fontFace, float fontSize);
	bool getHover() const;
	void setVisible(bool visible);
	bool isVisible() const;
	bool hasKeyboardFocus() const;
	const Widget *getParent() const;
	Widget *getParent();
	const std::vector<Widget *> &getChildren() const;
	void setMetadata(void *metadata);
	void *getMetadata();
	void addChildWidget(Widget *child);
	void removeChildWidget(Widget *child);
	void destroyChildWidget(Widget *child);
	bool isRectDirty() const;
	bool isMeasureDirty() const;
	Rect getUserRect() const;
	Size getMeasuredSize() const;

	// Returns a measured dimension if the user dimension isn't set.
	Size getUserOrMeasuredSize() const;

	void setRectInternal(Rect rect);
	const Widget *findClosestAncestor(WidgetType::Enum type) const;
	Widget *findClosestAncestor(WidgetType::Enum type);
	void setDrawManually(bool value);
	void setDrawLast(bool value);
	bool getDrawLast() const;
	void setOverlap(bool value);
	bool overlapsParentWindow() const;
	void setClipInputToParent(bool value);

	Widget *addEventHandler(IEventHandler *eventHandler);

	template<class Object>
	Widget *addEventHandler(EventType::Enum eventType, Object *object, void (Object::*method)(Event))
	{
		EventHandler<Object> *eventHandler = new EventHandler<Object>();
		eventHandler->eventType = eventType;
		eventHandler->object = object;
		eventHandler->method = method;
		addEventHandler(eventHandler);
		return this;
	}

	// Shortcut for IRenderer::getLineHeight, using the widget's renderer, font face and font size.
	int getLineHeight() const;

	// Shortcut for IRenderer::measureText, using the widget's renderer, font face and font size.
	void measureText(const char *text, int n, int *width, int *height) const;

	// Shortcut for IRenderer::lineBreakText, using the widget's renderer, font face and font size.
	LineBreakResult lineBreakText(const char *text, int n, int lineWidth) const;

protected:
	virtual void doLayout();

	// The widget was added to a parent widget (see Widget::addChildWidget).
	virtual void onParented(Widget *parent);

	// Widget::renderer has been changed.
	virtual void onRendererChanged();

	virtual void onFontChanged(const char *fontFace, float fontSize);

	// Some additional widget state may been to be cleared when a widget is hidden.
	virtual void onVisibilityChanged();

	virtual void onRectChanged();

	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	virtual void onMouseWheelMove(int x, int y);
	virtual void onMouseHoverOn();
	virtual void onMouseHoverOff();
	virtual void onKeyDown(Key::Enum key);
	virtual void onKeyUp(Key::Enum key);
	virtual void onTextInput(const char *text);

	// Returns the rect to clip the children of this widget against. Return an empty rect to disable clipping of children.
	virtual Rect getChildrenClipRect() const;

	virtual void draw(Rect clip);

	virtual Size measure();

	void setMeasureDirty(bool value = true);
	void setRectDirty(bool value = true);
	void doMeasurePassRecursive(bool dirty);
	void doLayoutPassRecursive(bool dirty);

	// Draw without clipping if visible.
	void drawIfVisible();

	void invokeEvent(Event e);
	void invokeEvent(Event e, const std::vector<EventCallback> &callbacks);

	MainWindow *findMainWindow();
	void setRenderer(IRenderer *renderer);
	void setMainWindowAndWindowRecursive(MainWindow *mainWindow, Window *window);

	void debugPrintf(const char *format, ...) const;

	WidgetType::Enum type_;

	// Explicitly set by the user.
	Rect userRect_;

	Rect rect_;

	// Only used if the widget is the child of a layout.
	Stretch::Enum stretch_;

	float stretchWidthScale_;
	float stretchHeightScale_;

	// Only used if the widget is the child of a layout.
	Align::Enum align_;

	// Only used when userSetSize w and/or h are set to WZ_AUTOSIZE, or the widget is the child of a layout.
	Border margin_;

	// Parent's padding is equivalent to this widget's margin.
	Border padding_;

	Size measuredSize_;

	// User-set metadata.
	void *metadata_;

	WidgetFlags::Enum flags_;

	bool hover_;

	// Don't draw this widget if false.
	bool visible_;

	// Used internally to ignore siblings that overlap at the mouse cursor.
	bool ignore_;

	// This widget should overlap other widgets when doing mouse cursor logic. e.g. tab bar scroll buttons.
	bool overlap_;

	// Don't draw automatically when MainWindow::draw walks through the widget hierarchy.
	bool drawManually_;

	// Clip to the parent widget rect in mouse move calculations. Used by the combo widget dropdown list (false).
	bool inputClippedToParent_;

	char fontFace_[256];
	float fontSize_;

	IRenderer *renderer_;

	MainWindow *mainWindow_;

	// The closest ancestor window. NULL if the widget is the descendant of a mainWindow. Set in Widget::addChildWidget.
	Window *window_;

	Widget *parent_;
	std::vector<Widget *> children_;

	std::vector<IEventHandler *> eventHandlers_;

private:
#ifndef NDEBUG
	void debugPrintWidgetDetailsRecursive() const;
#endif
};

struct ButtonClickBehavior
{
	enum Enum
	{
		// Click the button on mouse up (default).
		Up,

		// Click the button on mouse down
		Down
	};
};

struct ButtonSetBehavior
{
	enum Enum
	{
		// Button is never set.
		Default,

		// Click to toggle whether the button is set.
		Toggle,

		// Click to set the button. Clicking again does nothing.
		Sticky
	};
};

class Button : public Widget
{
public:
	Button(const std::string &label = std::string(), const std::string &icon = std::string());
	void setLabel(const char *label);
	const char *getLabel() const;
	void setIcon(const char *icon);
	const char *getIcon() const;
	bool isPressed() const;
	bool isSet() const;
	void set(bool value);
	void bindValue(bool *value);
	void addCallbackPressed(EventCallback callback);
	void addCallbackClicked(EventCallback callback);

protected:
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void draw(Rect clip);
	virtual Size measure();
	void setClickBehavior(ButtonClickBehavior::Enum clickBehavior);
	void setSetBehavior(ButtonSetBehavior::Enum setBehavior);
	void click();

	ButtonClickBehavior::Enum clickBehavior_;
	ButtonSetBehavior::Enum setBehavior_;
	std::string label_;
	std::string icon_;
	bool isPressed_;
	bool isSet_;
	bool *boundValue_;
	std::vector<EventCallback> pressedCallbacks_;
	std::vector<EventCallback> clickedCallbacks_;
};

class CheckBox : public Button
{
public:
	CheckBox(const std::string &label = std::string());
	bool isChecked() const;
	void check(bool value);
	void addCallbackChecked(EventCallback callback);

protected:
	virtual void draw(Rect clip);
	virtual Size measure();
};

class Combo : public Widget
{
public:
	Combo(uint8_t *itemData = NULL, int itemStride = 0, int nItems = 0);
	void setItems(uint8_t *itemData, size_t itemStride, int nItems);
	List *getList();
	const List *getList() const;
	bool isOpen() const;

protected:
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void onRectChanged();
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual Rect getChildrenClipRect() const;
	virtual void draw(Rect clip);
	virtual Size measure();
	void onListItemSelected(Event e);
	void updateListRect();

	bool isOpen_;
	List *list_;
};

class DockIcon : public Widget
{
public:
	DockIcon();

protected:
	virtual void draw(Rect clip);
};

class DockPreview : public Widget
{
protected:
	virtual void draw(Rect clip);
};

class Frame : public Widget
{
public:
	Frame();
	void add(Widget *widget);
	void remove(Widget *widget);
};

class GroupBox : public Widget
{
public:
	GroupBox(const std::string &label = std::string());
	void setLabel(const char *label);
	const char *getLabel() const;
	void setContent(Widget *widget);
	void removeContent(Widget *widget);

protected:
	virtual void onRendererChanged();
	virtual void draw(Rect clip);
	virtual Size measure();
	void refreshPadding();

	std::string label_;
};

class Label : public Widget
{
public:
	Label(const std::string &text = std::string());
	void setMultiline(bool multiline);
	bool getMultiline() const;
	void setText(const char *text);
	void setTextf(const char *format, ...);
	const char *getText() const;
	void setTextColor(Color color);
	void setTextColor(float r, float g, float b);
	Color getTextColor() const;

protected:
	virtual void onRendererChanged();
	virtual void draw(Rect clip);
	virtual Size measure();

	std::string text_;
	bool multiline_;
	Color textColor_;
	bool isTextColorUserSet_;
};

typedef void(*DrawListItemCallback)(IRenderer *renderer, Rect clip, const List *list, const char *fontFace, float fontSize, int itemIndex, const uint8_t *itemData);

class List : public Widget
{
public:
	List(uint8_t *itemData = NULL, int itemStride = 0, int nItems = 0);
	Border getItemsBorder() const;
	Rect getItemsRect() const;

	// rect will be absolute - ancestor window position is taken into account.
	Rect getAbsoluteItemsRect() const;

	void setDrawItemCallback(DrawListItemCallback callback);
	DrawListItemCallback getDrawItemCallback() const;
	void setItems(uint8_t *itemData, size_t itemStride, int nItems);
	void setItemData(uint8_t *itemData);
	uint8_t *getItemData() const;
	void setItemStride(int itemStride);
	int getItemStride() const;
	void setItemHeight(int itemHeight);
	int getItemHeight() const;
	void setNumItems(int nItems);
	int getNumItems() const;
	int getFirstItem() const;
	void setSelectedItem(int selectedItem);
	int getSelectedItem() const;
	int getPressedItem() const;
	int getHoveredItem() const;
	int getScrollValue() const;
	Scroller *getScroller();
	const Scroller *getScroller() const;
	void addCallbackItemSelected(EventCallback callback);

protected:
	virtual void onRendererChanged();
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void onVisibilityChanged();
	virtual void onRectChanged();
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	virtual void onMouseWheelMove(int x, int y);
	virtual void onMouseHoverOff();
	virtual void draw(Rect clip);
	virtual Size measure();
	void onScrollerValueChanged(Event e);
	void setItemHeightInternal(int itemHeight);
	void refreshItemHeight();
	void updateMouseOverItem(int mouseX, int mouseY);
	void updateScroller();

	Border itemsBorder_;
	DrawListItemCallback drawItem_;
	uint8_t *itemData_;
	int itemStride_;
	int itemHeight_;
	bool isItemHeightUserSet_;
	int nItems_;
	int firstItem_;
	int selectedItem_;
	int pressedItem_;
	int hoveredItem_;

	// The same as hoveredItem, except when pressedItem != -1.
	int mouseOverItem_;

	Scroller *scroller_;

	std::vector<EventCallback> itemSelectedCallbacks_;

	// Set when the mouse moves. Used to refresh the hovered item when scrolling via the mouse wheel.
	Position lastMousePosition_;
};

struct MainWindowFlags
{
	enum Enum
	{
		None,
		DockingEnabled = 1<<0,
		MenuEnabled = 1<<1,

		// Used to avoid traversing the entire widget hierarchy.
		AnyWidgetMeasureDirty = 1<<2,

		// Used to avoid traversing the entire widget hierarchy.
		AnyWidgetRectDirty = 1 << 3
	};
};

inline MainWindowFlags::Enum operator|(MainWindowFlags::Enum a, MainWindowFlags::Enum b)
{
	return MainWindowFlags::Enum(int(a) | int(b));
}

class MainWindow : public Widget
{
public:
	MainWindow(IRenderer *renderer, MainWindowFlags::Enum flags = MainWindowFlags::None);
	bool isDockingEnabled() const;
	bool isMenuEnabled() const;
	void createMenuButton(const std::string &label);
	bool isShiftKeyDown() const;
	bool isControlKeyDown() const;
	void mouseButtonDown(int mouseButton, int mouseX, int mouseY);
	void mouseButtonUp(int mouseButton, int mouseX, int mouseY);
	void mouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	void mouseWheelMove(int x, int y);
	void keyDown(Key::Enum key);
	void keyUp(Key::Enum key);
	void textInput(const char *text);
	void draw();
	void drawFrame();
	void add(Widget *widget);
	void remove(Widget *widget);
	bool isTextCursorVisible() const;
	void toggleTextCursor();
	void setCursor(Cursor::Enum cursor);
	Cursor::Enum getCursor() const;

	const Widget *getKeyboardFocusWidget() const;

	// Set keyboard focus to this widget.
	void setKeyboardFocusWidget(Widget *widget);

	DockPosition::Enum getWindowDockPosition(const Window *window) const;
	void dockWindow(Window *window, DockPosition::Enum dockPosition);
	void undockWindow(Window *window);

	// The docked window has been resized, update the rects of other windows docked at the same position.
	void updateDockedWindowRect(Window *window);

	// Lock input to this widget.
	void pushLockInputWidget(Widget *widget);

	// Stop locking input to this widget.
	void popLockInputWidget(Widget *widget);

	void setMovingWindow(Window *window);
	void updateContentRect();

	void setAnyWidgetMeasureDirty(bool value = true);
	void setAnyWidgetRectDirty(bool value = true);

protected:
	typedef bool(*WidgetPredicate)(const Widget *);

	virtual void onRectChanged();

	void doMeasureAndLayoutPasses();

	void mouseButtonDownRecursive(Widget *widget, int mouseButton, int mouseX, int mouseY);
	void mouseButtonUpRecursive(Widget *widget, int mouseButton, int mouseX, int mouseY);

	// Clear widget hover on everything but ignoreWindow and it's children.
	void clearHoverRecursive(Window *ignoreWindow, Widget *widget);

	// Sets Widget.ignore
	void ignoreOverlappingChildren(Widget *widget, int mouseX, int mouseY);

	// If window is not NULL, only call onMouseMove in widgets that are children of the window and the window itself.
	void mouseMoveRecursive(Window *window, Widget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);

	void mouseWheelMoveRecursive(Widget *widget, int x, int y);

	void keyDelta(Key::Enum key, bool down);
	void drawWidgetRecursive(Widget *widget, Rect clip, WidgetPredicate drawPredicate, WidgetPredicate recursePredicate);
	void drawWidget(Widget *widget, WidgetPredicate drawPredicate, WidgetPredicate recursePredicate);

	// Returns the window that the mouse cursor is hovering over. NULL if there isn't one.
	Window *getHoverWindow(int mouseX, int mouseY);

	// Used by all dock tab bars.
	void onDockTabBarTabChanged(Event e);

	void refreshDockTabBar(DockPosition::Enum dockPosition);

	void updateDockingRects();

	Rect calculateDockWindowRect(DockPosition::Enum dockPosition, Size windowSize);

	void updateDockIconPositions();

	void updateDockPreviewRect(DockPosition::Enum dockPosition);
	void updateDockPreviewVisible(int mouseX, int mouseY);

	// top can be NULL
	void updateWindowDrawPriorities(Window *top);

	MainWindowFlags::Enum flags_;

	Widget *content_;

	bool isTextCursorVisible_;

	Cursor::Enum cursor_;

	bool isShiftKeyDown_, isControlKeyDown_;

	std::vector<Widget *> lockInputWidgetStack_;

	// Lock input to this window, i.e. don't call onMouseMove, onMouseButtonDown or onMouseButtonUp on any widget that isn't this window or it's descendants.
	Window *lockInputWindow_;

	Widget *keyboardFocusWidget_;

	// This window is currently being moved and may be docked.
	Window *movingWindow_;

	// Hidden from the consumer.
	DockIcon *dockIcons_[DockPosition::NumDockPositions];
	DockPreview *dockPreview_;

	std::vector<Window *> dockedWindows_[DockPosition::NumDockPositions];

	// A window being dragged will be docked to this position on mouse up. Set when the cursor hovers over a dock icon.
	DockPosition::Enum windowDockPosition_;

	// Each dock position has a tab bar which is visible when multiple windows are docked at the same position.
	TabBar *dockTabBars_[DockPosition::NumDockPositions];

	bool ignoreDockTabBarChangedEvent_;

	MenuBar *menuBar_;
};

class MenuBarButton : public Widget
{
public:
	MenuBarButton(MenuBar *menuBar);
	void setLabel(const char *label);
	const char *getLabel() const;
	bool isPressed() const;

protected:
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseHoverOn();
	virtual void draw(Rect clip);
	virtual Size measure();

	Border padding_;
	std::string label_;
	bool isPressed_;
	bool isSet_;
	std::vector<EventCallback> pressedCallbacks_;
	MenuBar *menuBar_;
};

class MenuBar : public Widget
{
public:
	MenuBar();
	MenuBarButton *createButton();

protected:
	virtual void onRendererChanged();
	virtual void draw(Rect clip);
	virtual Size measure();

	StackLayout *layout_;
};

class RadioButton : public Button
{
public:
	RadioButton(const std::string &label = std::string());

protected:
	virtual void onParented(Widget *parent);
	virtual void draw(Rect clip);
	virtual Size measure();
	void onClicked(Event e);
};

class ScrollerNub : public Widget
{
public:
	ScrollerNub(Scroller *scroller);
	bool isPressed() const;
	void updateRect();

protected:
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);

	Scroller *scroller_;
	bool isPressed_;

	// The position of the nub when the it was pressed.
	Position pressPosition_;

	// The position of the mouse when the nub was pressed.
	Position pressMousePosition_;
};

struct ScrollerDirection
{
	enum Enum
	{
		Vertical,
		Horizontal
	};
};

class Scroller : public Widget
{
public:
	Scroller(ScrollerDirection::Enum direction, int value = 0, int stepValue = 1, int maxValue = 0);
	ScrollerDirection::Enum getDirection() const;
	int getValue() const;
	void setValue(int value);
	void decrementValue();
	void incrementValue();
	void setStepValue(int stepValue);
	int getStepValue();
	void setMaxValue(int maxValue);
	int getMaxValue() const;
	void setNubScale(float nubScale);
	float getNubScale() const;
	ScrollerNub *getNub();
	const ScrollerNub *getNub() const;
	void getNubState(Rect *containerRect, Rect *rect, bool *hover, bool *pressed) const;
	void addCallbackValueChanged(EventCallback callback);

protected:
	virtual void onRectChanged();
	virtual void onMouseWheelMove(int x, int y);
	virtual void draw(Rect clip);
	virtual Size measure();
	void onDecrementButtonClicked(Event e);
	void onIncrementButtonClicked(Event e);

	ScrollerDirection::Enum direction_;
	int value_, stepValue_, maxValue_;
	float nubScale_;
	Button *decrementButton_, *incrementButton_;
	ScrollerNub *nub_;
	std::vector<EventCallback> valueChangedCallbacks_;
};

class SpinnerDecrementButton;
class SpinnerIncrementButton;

class Spinner : public Widget
{
public:
	Spinner();
	int getValue() const;
	void setValue(int value);
	TextEdit *getTextEdit();
	const TextEdit *getTextEdit() const;

protected:
	virtual void onRendererChanged();
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void draw(Rect clip);
	virtual Size measure();
	void onDecrementButtonClicked(Event e);
	void onIncrementButtonClicked(Event e);

	TextEdit *textEdit_;
	SpinnerDecrementButton *decrementButton_;
	SpinnerIncrementButton *incrementButton_;
};

struct StackLayoutDirection
{
	enum Enum
	{
		Vertical,
		Horizontal
	};
};

class StackLayout : public Widget
{
public:
	StackLayout(StackLayoutDirection::Enum direction, int spacing = 0);
	void setDirection(StackLayoutDirection::Enum direction);
	void setSpacing(int spacing);
	int getSpacing() const;
	void add(Widget *widget);
	void remove(Widget *widget);

protected:
	virtual void doLayout();
	virtual Size measure();
	void layoutVertical();
	void layoutHorizontal();

	StackLayoutDirection::Enum direction_;

	// Spacing between child widgets. Applied to the top/left of children.
	int spacing_;
};

class TabButton : public Button
{
public:
	TabButton(const std::string &label = std::string(), const std::string &icon = std::string());

protected:
	virtual void draw(Rect clip);
};

class TabBar : public Widget
{
public:
	TabBar();
	void addTab(TabButton *tab);
	void destroyTab(TabButton *tab);
	void clearTabs();
	TabButton *getSelectedTab();
	void selectTab(TabButton *tab);
	void addCallbackTabChanged(EventCallback callback);
	int getScrollValue() const;

protected:
	virtual void onRendererChanged();
	virtual void onRectChanged();
	virtual void draw(Rect clip);
	virtual Size measure();

	// Sets the scroll value, and repositions and shows/hides the tabs accordingly.
	void setScrollValue(int value);

	void invokeTabChanged();

	void onTabButtonPressed(Event e);
	void onDecrementButtonClicked(Event e);
	void onIncrementButtonClicked(Event e);

	// Show the scroll buttons if they're required, hides them if they're not.
	void updateScrollButtons();

	// Hide tabs that are scrolled out of view.
	void updateTabVisibility();

	StackLayout *layout_;
	TabButton *selectedTab_;
	std::vector<TabButton *> tabs_;

	int scrollValue_;
	Button *decrementButton_;
	Button *incrementButton_;

	std::vector<EventCallback> tabChangedCallbacks_;
};

class TabPage : public Widget
{
public:
	TabPage();
	void add(Widget *widget);
	void remove(Widget *widget);
};

// Wraps tab button and page.
class Tab
{
	friend class Tabbed;

public:
	Tab(const std::string &label = std::string(), const std::string &icon = std::string());
	~Tab();
	const TabButton *getButton() const;
	const TabPage *getPage() const;
	Widget *add(Widget *widget);
	void remove(Widget *widget);

protected:
	TabButton *button_;
	TabPage *page_;
};

class Tabbed : public Widget
{
public:
	Tabbed();
	Tab *getSelectedTab();
	void add(Tab *tab);

protected:
	virtual void draw(Rect clip);
	virtual Size measure();
	void onTabChanged(Event e);

	StackLayout *layout_;
	TabBar *tabBar_;
	Widget *pageContainer_;
	std::vector<Tab *> tabs_;
};

typedef bool(*TextEditValidateTextCallback)(const char *text);

class TextEdit : public Widget
{
public:
	TextEdit(bool multiline, const std::string &text = std::string());
	void setValidateTextCallback(TextEditValidateTextCallback callback);
	bool isMultiline() const;
	Border getBorder() const;
	void setBorder(Border border);
	Rect getTextRect() const;
	const char *getText() const;
	void setText(const char *text);
	int getScrollValue() const;
	const char *getVisibleText() const;

	// y is centered on the line.
	Position getCursorPosition() const;

	bool hasSelection() const;

	// start is always < end if has_selection
	int getSelectionStartIndex() const;

	// y is centered on the line.
	Position getSelectionStartPosition() const;

	// end is always > start if has_selection
	int getSelectionEndIndex() const;

	// y is centered on the line.
	Position getSelectionEndPosition() const;

	// Calculate the position of the index - relative to text rect - based on the cursor index and scroll index. 
	Position positionFromIndex(int index) const;

protected:
	virtual void onRendererChanged();
	virtual void onRectChanged();
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	virtual void onMouseWheelMove(int x, int y);
	virtual void onKeyDown(Key::Enum key);
	virtual void onTextInput(const char *text);
	virtual void draw(Rect clip);
	virtual Size measure();
	void onScrollerValueChanged(Event e);
	int calculateNumLines(int lineWidth);
	void updateScroller();
	void insertText(int index, const char *text, int n);
	void enterText(const char *text);
	void deleteText(int index, int n);
	void deleteSelectedText();

	// Returns -1 if an index could not be calculated. e.g. if the position is outside the widget.
	int indexFromRelativePosition(Position pos) const;

	// Calculate the text index at the given absolute position.
	int indexFromPosition(int x, int y);

	// Update the scroll value so the cursor is visible.
	void updateScrollIndex();

	// Helper function for moving the cursor while the selection (shift) key is held.
	void moveCursorAndSelection(int newCursorIndex);

	Scroller *scroller_;
	bool multiline_;
	TextEditValidateTextCallback validateText_;
	Border border_;
	bool pressed_;
	int cursorIndex_;
	int scrollValue_;
	int selectionStartIndex_;
	int selectionEndIndex_;
	std::string text_;
};

class ToggleButton : public Button
{
public:
	ToggleButton(const std::string &label = std::string(), const std::string &icon = std::string());
};

struct WindowDrag
{
	enum Enum
	{
		None,
		Header,
		Resize_N,
		Resize_NE,
		Resize_E,
		Resize_SE,
		Resize_S,
		Resize_SW,
		Resize_W,
		Resize_NW
	};
};

struct Compass
{
	enum Enum
	{
		N,
		NE,
		E,
		SE,
		S,
		SW,
		W,
		NW,
		NumPoints
	};
};

class Window : public Widget
{
public:
	Window(const std::string &title);
	int getHeaderHeight() const;
	int getBorderSize() const;
	Rect getHeaderRect() const;
	void setTitle(const char *title);
	const char *getTitle() const;
	Widget *getContentWidget();
	int getDrawPriority() const;
	void setDrawPriority(int drawPriority);

	// Tell the window it's being docked.
	void dock();

	void add(Widget *widget);
	void remove(Widget *widget);

protected:
	virtual void onRendererChanged();
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	virtual Rect getChildrenClipRect() const;
	virtual void draw(Rect clip);
	virtual Size measure();

	void refreshHeaderHeightAndPadding();

	// rects parameter size should be Compass::NumPoints
	void calculateBorderRects(Rect *rects);

	// borderRects and mouseOverBorderRects parameter sizes should be Compass::NumPoints
	void calculateMouseOverBorderRects(int mouseX, int mouseY, Rect *borderRects, bool *mouseOverBorderRects);

	int drawPriority_;
	int headerHeight_;
	int borderSize_;
	std::string title_;
	Widget *content_;
	WindowDrag::Enum drag_;

	// Dragging a docked window header doesn't undock the window until the mouse has moved WZ_WINDOW_UNDOCK_DISTANCE.
	Position undockStartPosition_;

	Position resizeStartPosition_;
	Rect resizeStartRect_;

	// Remember the window size when it is docked, so when the window is undocked the size can be restored.
	Size sizeBeforeDocking_;
};

} // namespace wz
