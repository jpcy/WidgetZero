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
#include <assert.h>
#define WZ_ASSERT assert
#endif

#include <nanovg.h>

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

enum WidgetType
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
};

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

enum WidgetEventType
{
	WZ_EVENT_UNKNOWN,
	WZ_EVENT_BUTTON_PRESSED,
	WZ_EVENT_BUTTON_CLICKED,
	WZ_EVENT_LIST_ITEM_SELECTED,
	WZ_EVENT_SCROLLER_VALUE_CHANGED,
	WZ_EVENT_TAB_BAR_TAB_CHANGED,
	WZ_EVENT_TAB_BAR_TAB_ADDED,
	WZ_EVENT_TAB_BAR_TAB_REMOVED
};

struct EventBase
{
	WidgetEventType type;
	Widget *widget;
};

struct CreateWidgetEvent
{
	WidgetEventType type;
	Widget *parent;
	Widget *widget; 
	Widget *extra;
};

struct DestroyWidgetEvent
{
	WidgetEventType type;
	Widget *parent;
	Widget *widget;
};

struct ButtonEvent
{
	WidgetEventType type;
	Button *button;
	bool isSet;
};

struct ListEvent
{
	WidgetEventType type;
	List *list;
	int selectedItem;
};

struct ScrollerEvent
{
	WidgetEventType type;
	Scroller *scroller;
	int oldValue;
	int value;
};

struct TabBarEvent
{
	WidgetEventType type;
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

enum Cursor
{
	WZ_CURSOR_DEFAULT,
	WZ_CURSOR_IBEAM,
	WZ_CURSOR_RESIZE_N_S,
	WZ_CURSOR_RESIZE_E_W,
	WZ_CURSOR_RESIZE_NE_SW,
	WZ_CURSOR_RESIZE_NW_SE,
	WZ_NUM_CURSORS
};

enum DockPosition
{
	WZ_DOCK_POSITION_NONE = -1,
	WZ_DOCK_POSITION_NORTH,
	WZ_DOCK_POSITION_SOUTH,
	WZ_DOCK_POSITION_EAST,
	WZ_DOCK_POSITION_WEST,
	WZ_NUM_DOCK_POSITIONS
};

#define WZ_MIN(a, b) ((a) < (b) ? (a) : (b))
#define WZ_MAX(a, b) ((a) > (b) ? (a) : (b))
#define WZ_CLAMPED(min, value, max) WZ_MAX(min, WZ_MIN(max, value))
#define WZ_ABS(a) ((a) >= 0 ? (a) : (-a))
#define WZ_SIGN(a) ((a) >= 0 ? 1 : -1)
#define WZ_POINT_IN_RECT(px, py, rect) ((px) >= rect.x && (px) < rect.x + rect.w && (py) >= rect.y && (py) < rect.y + rect.h)
#define WZ_RECTS_OVERLAP(rect1, rect2) (rect1.x < rect2.x + rect2.w && rect1.x + rect1.w > rect2.x && rect1.y < rect2.y + rect2.h && rect1.y + rect1.h > rect2.y) 

enum Key
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
};

#define WZ_KEY_MOD_OFF(key) ((key) & ~(WZ_KEY_SHIFT_BIT | WZ_KEY_CONTROL_BIT))

struct IEventHandler
{
	virtual ~IEventHandler() {}
	virtual void call(Event e) = 0;

	WidgetEventType eventType;
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
	~IRenderer() {};
	virtual const char *getError() = 0;
	virtual int getLineHeight(const char *fontFace, float fontSize) = 0;

	// width or height can be NULL.
	virtual void measureText(const char *fontFace, float fontSize, const char *text, int n, int *width, int *height) = 0;

	virtual LineBreakResult lineBreakText(const char *fontFace, float fontSize, const char *text, int n, int lineWidth) = 0;
	virtual void drawButton(Button *button, Rect clip) = 0;
	virtual Size measureButton(Button *button) = 0;
	virtual void drawCheckBox(CheckBox *checkBox, Rect clip) = 0;
	virtual Size measureCheckBox(CheckBox *checkBox) = 0;
	virtual void drawCombo(Combo *combo, Rect clip) = 0;
	virtual Size measureCombo(Combo *combo) = 0;
	virtual void drawDockIcon(DockIcon *dockIcon, Rect clip) = 0;
	virtual void drawDockPreview(DockPreview *dockPreview, Rect clip) = 0;
	virtual void drawGroupBox(GroupBox *groupBox, Rect clip) = 0;
	virtual Size measureGroupBox(GroupBox *groupBox) = 0;
	virtual void drawLabel(Label *label, Rect clip) = 0;
	virtual Size measureLabel(Label *label) = 0;
	virtual void drawList(List *list, Rect clip) = 0;
	virtual Size measureList(List *list) = 0;
	virtual void drawMenuBarButton(MenuBarButton *button, Rect clip) = 0;
	virtual Size measureMenuBarButton(MenuBarButton *button) = 0;
	virtual void drawMenuBar(MenuBar *menuBar, Rect clip) = 0;
	virtual Size measureMenuBar(MenuBar *menuBar) = 0;
	virtual void drawRadioButton(RadioButton *button, Rect clip) = 0;
	virtual Size measureRadioButton(RadioButton *button) = 0;
	virtual void drawScrollerDecrementButton(Button *button, Rect clip) = 0;
	virtual void drawScrollerIncrementButton(Button *button, Rect clip) = 0;
	virtual void drawScroller(Scroller *scroller, Rect clip) = 0;
	virtual Size measureScroller(Scroller *scroller) = 0;
	virtual void drawSpinnerDecrementButton(Button *button, Rect clip) = 0;
	virtual void drawSpinnerIncrementButton(Button *button, Rect clip) = 0;
	virtual void drawSpinner(Spinner *spinner, Rect clip) = 0;
	virtual Size measureSpinner(Spinner *spinner) = 0;
	virtual void drawTabButton(TabButton *button, Rect clip) = 0;
	virtual void drawTabBar(TabBar *tabBar, Rect clip) = 0;
	virtual Size measureTabBar(TabBar *tabBar) = 0;
	virtual void drawTabbed(Tabbed *tabbed, Rect clip) = 0;
	virtual Size measureTabbed(Tabbed *tabbed) = 0;
	virtual void drawTextEdit(TextEdit *textEdit, Rect clip) = 0;
	virtual Size measureTextEdit(TextEdit *textEdit) = 0;
	virtual void drawWindow(Window *window, Rect clip) = 0;
	virtual Size measureWindow(Window *window) = 0;
};

enum
{
	WZ_WIDGET_FLAG_DRAW_LAST = 1 << 0,
};

class Widget
{
public:
	Widget();
	virtual ~Widget();
	Widget *addEventHandler(IEventHandler *eventHandler);

	template<class Object>
	Widget *addEventHandler(WidgetEventType eventType, Object *object, void (Object::*method)(Event))
	{
		EventHandler<Object> *eventHandler = new EventHandler<Object>();
		eventHandler->eventType = eventType;
		eventHandler->object = object;
		eventHandler->method = method;
		addEventHandler(eventHandler);
		return this;
	}

	// The widget was added to a parent widget (see Widget::addChildWidget).
	virtual void onParented(Widget *parent) {}

	// Widget::renderer has been changed.
	virtual void onRendererChanged() {}

	virtual void onFontChanged(const char *fontFace, float fontSize) {}

	// Some additional widget state may been to be cleared when a widget is hidden.
	virtual void onVisibilityChanged() {}

	virtual void onRectChanged() {}

	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY) {}
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY) {}
	virtual void onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY) {}
	virtual void onMouseWheelMove(int x, int y) {}
	virtual void onMouseHoverOn() {}
	virtual void onMouseHoverOff() {}
	virtual void onKeyDown(Key key) {}
	virtual void onKeyUp(Key key) {}
	virtual void onTextInput(const char *text) {}

	// Returns the rect to clip the children of this widget against. Return an empty rect to disable clipping of children.
	virtual Rect getChildrenClipRect() const { return getAbsoluteRect(); }

	virtual void draw(Rect clip) {}
	virtual Size measure() { return Size(); }

	WidgetType getType() const;
	bool isLayout() const;
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
	void setUniformMargin(int value);
	Border getMargin() const;
	void setStretch(int stretch);
	int getStretch() const;
	void setStretchScale(float width, float height);
	float getStretchWidthScale() const;
	float getStretchHeightScale() const;
	void setAlign(int align);
	int getAlign() const;
	void setFontFace(const char *fontFace);
	const char *getFontFace() const;
	void setFontSize(float fontSize);
	float getFontSize() const;
	void setFont(const char *fontFace, float fontSize);
	bool getHover() const;
	void setVisible(bool visible);
	bool getVisible() const;
	bool hasKeyboardFocus() const;
	void setMetadata(void *metadata);
	void *getMetadata();

	// Resize the widget to the result of calling the widget "measure" callback.
	void resizeToMeasured();

public:
	void addChildWidget(Widget *child);
	void removeChildWidget(Widget *child);
	void destroyChildWidget(Widget *child);
	void setPositionInternal(int x, int y);
	void setPositionInternal(Position position);
	void setWidthInternal(int w);
	void setHeightInternal(int h);
	void setSizeInternal(int w, int h);
	void setSizeInternal(Size size);

private:
	void setRectInternalRecursive(Rect rect);

public:
	void setRectInternal(int x, int y, int w, int h);
	void setRectInternal(Rect rect);
	void refreshRect();
	const Widget *findClosestAncestor(WidgetType type) const;
	Widget *findClosestAncestor(WidgetType type);
	void setDrawManually(bool value);
	void setDrawLast(bool value);
	void setOverlap(bool value);
	bool overlapsParentWindow() const;
	void setClipInputToParent(bool value);
	void setInternalMetadata(void *metadata);
	void *getInternalMetadata();

	// Shortcut for IRenderer::getLineHeight, using the widget's renderer, font face and font size.
	int getLineHeight() const;

	// Shortcut for IRenderer::measureText, using the widget's renderer, font face and font size.
	void measureText(const char *text, int n, int *width, int *height) const;

	// Shortcut for IRenderer::lineBreakText, using the widget's renderer, font face and font size.
	LineBreakResult lineBreakText(const char *text, int n, int lineWidth) const;

	// Draw without clipping if visible.
	void drawIfVisible();

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

	// Don't draw automatically when MainWindow::draw walks through the widget hierarchy.
	bool drawManually;

	// True if not clipped to the parent widget rect in mouse move calculations. Used by the combo widget dropdown list.
	bool inputNotClippedToParent;

	char fontFace[256];
	float fontSize;

	IRenderer *renderer;

	MainWindow *mainWindow;

	// The closest ancestor window. NULL if the widget is the descendant of a mainWindow. Set in Widget::addChildWidget.
	Window *window;

	Widget *parent;
	std::vector<Widget *> children;

	std::vector<IEventHandler *> eventHandlers;

	void invokeEvent(Event e);
	void invokeEvent(Event e, const std::vector<EventCallback> &callbacks);

private:
	// Applies alignment and stretching to the provided rect, relative to the widget's parent rect.
	Rect calculateAlignedStretchedRect(Rect rect) const;

	MainWindow *findMainWindow();
	void setRenderer(IRenderer *renderer);
	void setMainWindowAndWindowRecursive(MainWindow *mainWindow, Window *window);
	void resizeToMeasuredRecursive();
};

enum ButtonClickBehavior
{
	// Click the button on mouse up (default).
	WZ_BUTTON_CLICK_BEHAVIOR_UP,

	// Click the button on mouse down
	WZ_BUTTON_CLICK_BEHAVIOR_DOWN
};

enum ButtonSetBehavior
{
	// Button is never set.
	WZ_BUTTON_SET_BEHAVIOR_DEFAULT,

	// Click to toggle whether the button is set.
	WZ_BUTTON_SET_BEHAVIOR_TOGGLE,

	// Click to set the button. Clicking again does nothing.
	WZ_BUTTON_SET_BEHAVIOR_STICKY
};

class Button : public Widget
{
public:
	Button(const std::string &label = std::string(), const std::string &icon = std::string());
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void draw(Rect clip);
	virtual Size measure();
	void setLabel(const char *label);
	const char *getLabel() const;
	void setIcon(const char *icon);
	const char *getIcon() const;
	void setPadding(Border padding);
	void setPadding(int top, int right, int bottom, int left);
	Border getPadding() const;
	bool isPressed() const;
	bool isSet() const;
	void set(bool value);
	void bindValue(bool *value);
	void addCallbackPressed(EventCallback callback);
	void addCallbackClicked(EventCallback callback);
	void setClickBehavior(ButtonClickBehavior clickBehavior);
	void setSetBehavior(ButtonSetBehavior setBehavior);

protected:
	void click();

	ButtonClickBehavior clickBehavior_;
	ButtonSetBehavior setBehavior_;
	Border padding_;
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
	virtual void draw(Rect clip);
	virtual Size measure();
	bool isChecked() const;
	void check(bool value);
	void addCallbackChecked(EventCallback callback);
};

class Combo : public Widget
{
public:
	Combo(uint8_t *itemData = NULL, int itemStride = 0, int nItems = 0);
	void setItems(uint8_t *itemData, size_t itemStride, int nItems);
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void onRectChanged();
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual Rect getChildrenClipRect() const;
	virtual void draw(Rect clip);
	virtual Size measure();
	List *getList();
	const List *getList() const;
	bool isOpen() const;

protected:
	void onListItemSelected(Event e);
	void updateListRect();

	bool isOpen_;
	List *list_;
};

class DockIcon : public Widget
{
public:
	DockIcon();
	virtual void draw(Rect clip);
};

class DockPreview : public Widget
{
public:
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
	virtual void onRendererChanged();
	virtual void draw(Rect clip);
	virtual Size measure();
	void setLabel(const char *label);
	const char *getLabel() const;
	void add(Widget *widget);
	void remove(Widget *widget);

protected:
	void refreshMargin();

	Widget *content_;
	std::string label_;
};

class Label : public Widget
{
public:
	Label(const std::string &text = std::string());
	virtual void draw(Rect clip);
	virtual Size measure();
	void setMultiline(bool multiline);
	bool getMultiline() const;
	void setText(const char *text);
	void setTextf(const char *format, ...);
	const char *getText() const;
	void setTextColor(NVGcolor color);
	void setTextColor(float r, float g, float b);
	NVGcolor getTextColor() const;

protected:
	std::string text_;
	bool multiline_;
	NVGcolor textColor_;
	bool isTextColorUserSet_;
};

typedef void(*DrawListItemCallback)(IRenderer *renderer, Rect clip, const List *list, const char *fontFace, float fontSize, int itemIndex, const uint8_t *itemData);

class List : public Widget
{
public:
	List(uint8_t *itemData = NULL, int itemStride = 0, int nItems = 0);
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

typedef bool(*WidgetPredicate)(const Widget *);

class MainWindow : public Widget
{
public:
	MainWindow(IRenderer *renderer);
	void createMenuButton(const std::string &label);
	virtual void onRectChanged();

	bool isShiftKeyDown() const;
	bool isControlKeyDown() const;

	// Set the centralized event handler. All events invoked by the ancestor widgets of this mainWindow will call the callback function.
	void setEventCallback(EventCallback callback);

	void mouseButtonDown(int mouseButton, int mouseX, int mouseY);
	void mouseButtonUp(int mouseButton, int mouseX, int mouseY);
	void mouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	void mouseWheelMove(int x, int y);
	void keyDown(Key key);
	void keyUp(Key key);
	void textInput(const char *text);
	void draw();
	void drawFrame();
	void setMenuBar(MenuBar *menuBar);
	void add(Widget *widget);
	void remove(Widget *widget);
	bool isTextCursorVisible() const;
	void toggleTextCursor();
	void setCursor(Cursor cursor);
	Cursor getCursor() const;
	const Widget *getKeyboardFocusWidget() const;

	// Set keyboard focus to this widget.
	void setKeyboardFocusWidget(Widget *widget);

	DockPosition getWindowDockPosition(const Window *window) const;
	void dockWindow(Window *window, DockPosition dockPosition);
	void undockWindow(Window *window);

	// The docked window has been resized, update the rects of other windows docked at the same position.
	void updateDockedWindowRect(Window *window);

	// Lock input to this widget.
	void pushLockInputWidget(Widget *widget);

	// Stop locking input to this widget.
	void popLockInputWidget(Widget *widget);

	void setMovingWindow(Window *window);
	void updateContentRect();

	Widget *content;

	// Centralized event handler.
	EventCallback handle_event;

	bool isTextCursorVisible_;

	Cursor cursor;

	bool isShiftKeyDown_, isControlKeyDown_;

	std::vector<Widget *> lockInputWidgetStack;

	// Lock input to this window, i.e. don't call onMouseMove, onMouseButtonDown or onMouseButtonUp on any widget that isn't this window or it's descendants.
	Window *lockInputWindow;

	Widget *keyboardFocusWidget;

	// This window is currently being moved and may be docked.
	Window *movingWindow;

	// Hidden from the consumer.
	DockIcon *dockIcons[WZ_NUM_DOCK_POSITIONS];
	DockPreview *dockPreview;

	std::vector<Window *> dockedWindows[WZ_NUM_DOCK_POSITIONS];

	// A window being dragged will be docked to this position on mouse up. Set when the cursor hovers over a dock icon.
	DockPosition windowDockPosition;

	// Each dock position has a tab bar which is visible when multiple windows are docked at the same position.
	TabBar *dockTabBars[WZ_NUM_DOCK_POSITIONS];

	bool ignoreDockTabBarChangedEvent;

	MenuBar *menuBar;

private:
	void mouseButtonDownRecursive(Widget *widget, int mouseButton, int mouseX, int mouseY);
	void mouseButtonUpRecursive(Widget *widget, int mouseButton, int mouseX, int mouseY);

	// Clear widget hover on everything but ignoreWindow and it's children.
	void clearHoverRecursive(Window *ignoreWindow, Widget *widget);

	// Sets Widget.ignore
	void ignoreOverlappingChildren(Widget *widget, int mouseX, int mouseY);

	// If window is not NULL, only call onMouseMove in widgets that are children of the window and the window itself.
	void mouseMoveRecursive(Window *window, Widget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);

	void mouseWheelMoveRecursive(Widget *widget, int x, int y);

	void keyDelta(Key key, bool down);
	void drawWidgetRecursive(Widget *widget, Rect clip, WidgetPredicate drawPredicate, WidgetPredicate recursePredicate);
	void drawWidget(Widget *widget, WidgetPredicate drawPredicate, WidgetPredicate recursePredicate);

	// Returns the window that the mouse cursor is hovering over. NULL if there isn't one.
	Window *getHoverWindow(int mouseX, int mouseY);

	// Used by all dock tab bars.
	void onDockTabBarTabChanged(Event e);

	void refreshDockTabBar(DockPosition dockPosition);

public: // FIXME
	void updateDockingRects();

private:
	Rect calculateDockWindowRect(DockPosition dockPosition, Size windowSize);

public: // FIXME
	void updateDockIconPositions();

private:
	void updateDockPreviewRect(DockPosition dockPosition);
	void updateDockPreviewVisible(int mouseX, int mouseY);

	// top can be NULL
	void updateWindowDrawPriorities(Window *top);
};

class MenuBarButton : public Widget
{
public:
	MenuBarButton();
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseHoverOn();
	virtual void draw(Rect clip);
	virtual Size measure();
	void setLabel(const char *label);
	const char *getLabel() const;
	bool isPressed() const;

	Border padding;
	std::string label;
	bool isPressed_;
	bool isSet;
	std::vector<EventCallback> pressed_callbacks;
	MenuBar *menuBar;
};

class MenuBar : public Widget
{
public:
	MenuBar();
	virtual void onRendererChanged();
	virtual void draw(Rect clip);
	virtual Size measure();
	MenuBarButton *createButton();

	StackLayout *layout;
};

class RadioButton : public Button
{
public:
	RadioButton(const std::string &label = std::string());
	virtual void onParented(Widget *parent);
	virtual void draw(Rect clip);
	virtual Size measure();

protected:
	void onClicked(Event e);
};

class ScrollerNub : public Widget
{
public:
	ScrollerNub(Scroller *scroller);
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	void updateRect();

	Scroller *scroller;
	bool isPressed;

	// The position of the nub when the it was pressed.
	Position pressPosition;

	// The position of the mouse when the nub was pressed.
	Position pressMousePosition;
};

enum ScrollerType
{
	WZ_SCROLLER_VERTICAL,
	WZ_SCROLLER_HORIZONTAL
};

class Scroller : public Widget
{
public:
	Scroller(ScrollerType scrollerType, int value = 0, int stepValue = 1, int maxValue = 0);
	virtual void onRectChanged();
	virtual void onMouseWheelMove(int x, int y);
	virtual void draw(Rect clip);
	virtual Size measure();
	ScrollerType getType() const;
	int getValue() const;
	void setValue(int value);
	void decrementValue();
	void incrementValue();
	void setStepValue(int stepValue);
	int getStepValue();
	void setMaxValue(int maxValue);
	void setNubScale(float nubScale);
	void getNubState(Rect *containerRect, Rect *rect, bool *hover, bool *pressed) const;
	void addCallbackValueChanged(EventCallback callback);

	ScrollerType scrollerType;
	int value, stepValue, maxValue;
	float nubScale;
	ScrollerNub *nub;
	std::vector<EventCallback> value_changed_callbacks;

protected:
	void onDecrementButtonClicked(Event e);
	void onIncrementButtonClicked(Event e);
};

class SpinnerDecrementButton;
class SpinnerIncrementButton;

class Spinner : public Widget
{
public:
	Spinner();
	virtual void onRendererChanged();
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void draw(Rect clip);
	virtual Size measure();
	int getValue() const;
	void setValue(int value);

	TextEdit *textEdit;
	SpinnerDecrementButton *decrementButton;
	SpinnerIncrementButton *incrementButton;

protected:
	void onDecrementButtonClicked(Event e);
	void onIncrementButtonClicked(Event e);
};

enum StackLayoutDirection
{
	WZ_STACK_LAYOUT_VERTICAL,
	WZ_STACK_LAYOUT_HORIZONTAL,
};

class StackLayout : public Widget
{
public:
	StackLayout(StackLayoutDirection direction, int spacing = 0);
	virtual void onRectChanged();
	void setDirection(StackLayoutDirection direction);
	void setSpacing(int spacing);
	int getSpacing() const;
	void add(Widget *widget);
	void remove(Widget *widget);

	StackLayoutDirection direction;

	// Spacing between child widgets. Applied to the top/left of children.
	int spacing;

private:
	void layoutVertical();
	void layoutHorizontal();
};

class TabButton : public Button
{
public:
	TabButton(const std::string &label = std::string());
	virtual void draw(Rect clip);
};

class TabBar : public Widget
{
public:
	TabBar();
	virtual void onRectChanged();
	virtual void draw(Rect clip);
	virtual Size measure();
	TabButton *createTab();
	void destroyTab(TabButton *tab);
	void clearTabs();
	Button *getDecrementButton();
	Button *getIncrementButton();
	TabButton *getSelectedTab();
	void selectTab(TabButton *tab);
	void addCallbackTabChanged(EventCallback callback);
	int getScrollValue() const;

	TabButton *selectedTab;
	std::vector<TabButton *> tabs;

	int scrollValue;
	Button *decrementButton;
	Button *incrementButton;

	std::vector<EventCallback> tab_changed_callbacks;

protected:
	// Sets the scroll value, and repositions and shows/hides the tabs accordingly.
	void setScrollValue(int value);

	void invokeTabChanged();

	void onTabButtonPressed(Event e);
	void onDecrementButtonClicked(Event e);
	void onIncrementButtonClicked(Event e);

	// Show the scroll buttons if they're required, hides them if they're not.
	void updateScrollButtons();

	void updateTabs();
};

class TabPage : public Widget
{
public:
	TabPage();
	void add(Widget *widget);
	void remove(Widget *widget);
};

struct TabbedPage
{
	TabButton *tab;
	TabPage *page;
};

// Wraps tab button and page.
class Tab
{
public:
	Tab();
	~Tab();
	Tab *setLabel(const std::string &label);
	Widget *add(Widget *widget);
	void remove(Widget *widget);

	TabButton *button;
	TabPage *page;
};

class Tabbed : public Widget
{
public:
	Tabbed();
	virtual void onRectChanged();
	virtual void draw(Rect clip);
	virtual Size measure();
	void addTab(Tab *tab);
	void addTab(TabButton **tab, TabPage **page);

	TabBar *tabBar;
	std::vector<TabbedPage> pages;

protected:
	void onTabChanged(Event e);
};

typedef bool(*TextEditValidateTextCallback)(const char *text);

class TextEdit : public Widget
{
public:
	TextEdit(bool multiline, const std::string &text = std::string());
	virtual void onRendererChanged();
	virtual void onRectChanged();
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	virtual void onMouseWheelMove(int x, int y);
	virtual void onKeyDown(Key key);
	virtual void onTextInput(const char *text);
	virtual void draw(Rect clip);
	virtual Size measure();
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

	Scroller *scroller;
	bool multiline;
	TextEditValidateTextCallback validate_text;
	Border border;
	bool pressed;
	int cursorIndex;
	int scrollValue;
	int selectionStartIndex;
	int selectionEndIndex;
	std::string text;

protected:
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
};

class ToggleButton : public Button
{
public:
	ToggleButton(const std::string &label = std::string(), const std::string &icon = std::string());
};

enum WindowDrag
{
	WZ_DRAG_NONE,
	WZ_DRAG_HEADER,
	WZ_DRAG_RESIZE_N,
	WZ_DRAG_RESIZE_NE,
	WZ_DRAG_RESIZE_E,
	WZ_DRAG_RESIZE_SE,
	WZ_DRAG_RESIZE_S,
	WZ_DRAG_RESIZE_SW,
	WZ_DRAG_RESIZE_W,
	WZ_DRAG_RESIZE_NW,
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

class Window : public Widget
{
public:
	Window(const std::string &title);
	virtual void onRendererChanged();
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void onRectChanged();
	virtual void onMouseButtonDown(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseButtonUp(int mouseButton, int mouseX, int mouseY);
	virtual void onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	virtual Rect getChildrenClipRect() const;
	virtual void draw(Rect clip);
	virtual Size measure();
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

	int drawPriority;
	int headerHeight;
	int borderSize;
	std::string title;

	Widget *content;

	WindowDrag drag;

	// Dragging a docked window header doesn't undock the window until the mouse has moved WZ_WINDOW_UNDOCK_DISTANCE.
	Position undockStartPosition;

	Position resizeStartPosition;
	Rect resizeStartRect;

	// Remember the window size when it is docked, so when the window is undocked the size can be restored.
	Size sizeBeforeDocking;

private:
	void refreshHeaderHeight();

	// rects parameter size should be WZ_NUM_COMPASS_POINTS
	void calculateBorderRects(Rect *rects);

	// borderRects and mouseOverBorderRects parameter sizes should be WZ_NUM_COMPASS_POINTS
	void calculateMouseOverBorderRects(int mouseX, int mouseY, Rect *borderRects, bool *mouseOverBorderRects);
};

} // namespace wz
