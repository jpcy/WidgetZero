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

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "wz.h"
#include "wz_renderer_nanovg.h"
#include "wz_skin.h"

#define WZ_MAX_WINDOWS 256

namespace wz {

struct ListImpl;
struct ScrollerImpl;
struct WindowImpl;

typedef struct
{
	WidgetDrawCallback draw;
	WidgetMeasureCallback measure;

	// If NULL, WidgetImpl.rect will be set to rect, otherwise this function is called.
	void (*set_rect)(struct WidgetImpl *widget, Rect rect);

	// Some additional widget state may been to be cleared when a widget is hidden.
	void (*set_visible)(struct WidgetImpl *widget, bool visible);

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

class WidgetChildren
{
public:
	~WidgetChildren();
	size_t size() const;
	bool empty() const;
	const struct WidgetImpl *operator[](size_t i) const;
	struct WidgetImpl *operator[](size_t i);
	void clear();
	void push_back(Widget *widget);
	void push_back(struct WidgetImpl *widgetImpl);
	void erase(size_t i);
	void erase(Widget *widget);
	void erase(struct WidgetImpl *widget);

private:
	std::vector<Widget *> widgets_;
	std::vector<struct WidgetImpl *> impls_;
};

struct WidgetImpl
{
	WidgetImpl();
	virtual ~WidgetImpl();
	
	// The widget was added to a parent widget (see WidgetImpl::addChildWidget).
	virtual void onParented(struct WidgetImpl *parent) {}

	// WidgetImpl::renderer has been changed.
	virtual void onRendererChanged() {}

	virtual void onFontChanged(const char *fontFace, float fontSize) {}
	
	virtual void draw(Rect clip) {}
	virtual Size measure() { return Size(); }

	WidgetType getType() const;
	bool isLayout() const;
	struct MainWindowImpl *getMainWindow();
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
	void setDrawCallback(WidgetDrawCallback draw);
	void setMeasureCallback(WidgetMeasureCallback measure);

	// Resize the widget to the result of calling the widget "measure" callback.
	void resizeToMeasured();

private:
	// Does everything but actually add the widget to WidgetImpl::children. Used by both WidgetImpl::addChildWidget overloads.
	void addChildWidgetInternal(struct WidgetImpl *child);

public:
	void addChildWidget(Widget *child);
	void addChildWidget(struct WidgetImpl *child);
	void removeChildWidget(Widget *child);
	void removeChildWidget(struct WidgetImpl *child);
	void destroyChildWidget(struct WidgetImpl *child);
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
	const struct WidgetImpl *findClosestAncestor(WidgetType type) const;
	struct WidgetImpl *findClosestAncestor(WidgetType type);
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

	// Don't draw automatically when MainWindowImpl::draw walks through the widget hierarchy.
	bool drawManually;

	// True if not clipped to the parent widget rect in mouse move calculations. Used by the combo widget dropdown list.
	bool inputNotClippedToParent;

	char fontFace[256];
	float fontSize;

	WidgetVtable vtable;

	IRenderer *renderer;

	struct MainWindowImpl *mainWindow;

	// The closest ancestor window. NULL if the widget is the descendant of a mainWindow. Set in WidgetImpl::addChildWidget.
	struct WindowImpl *window;

	struct WidgetImpl *parent;
	WidgetChildren children;

	std::vector<IEventHandler *> eventHandlers;

private:
	// Applies alignment and stretching to the provided rect, relative to the widget's parent rect.
	Rect calculateAlignedStretchedRect(Rect rect) const;

	struct MainWindowImpl *findMainWindow();
	void setRenderer(IRenderer *renderer);
	void setMainWindowAndWindowRecursive(struct MainWindowImpl *mainWindow, struct WindowImpl *window);
	void resizeToMeasuredRecursive();
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

typedef enum
{
	// Click the button on mouse up (default).
	WZ_BUTTON_CLICK_BEHAVIOR_UP,

	// Click the button on mouse down
	WZ_BUTTON_CLICK_BEHAVIOR_DOWN
}
ButtonClickBehavior;

typedef enum
{
	// Button is never set.
	WZ_BUTTON_SET_BEHAVIOR_DEFAULT,

	// Click to toggle whether the button is set.
	WZ_BUTTON_SET_BEHAVIOR_TOGGLE,

	// Click to set the button. Clicking again does nothing.
	WZ_BUTTON_SET_BEHAVIOR_STICKY
}
ButtonSetBehavior;

struct ButtonImpl : public WidgetImpl
{
	ButtonImpl(const std::string &label = std::string(), const std::string &icon = std::string());
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

	ButtonClickBehavior clickBehavior;
	ButtonSetBehavior setBehavior;
	Border padding;
	std::string label;
	std::string icon;
	bool isPressed_;
	bool isSet_;
	bool *boundValue;
	std::vector<EventCallback> pressed_callbacks;
	std::vector<EventCallback> clicked_callbacks;
};

struct CheckBoxImpl : public ButtonImpl
{
	CheckBoxImpl(const std::string &label = std::string());
	virtual void draw(Rect clip);
	virtual Size measure();
	bool isChecked() const;
	void check(bool value);
	void addCallbackChecked(EventCallback callback);
};

struct ComboImpl : public WidgetImpl
{
	ComboImpl(uint8_t *itemData, int itemStride, int nItems);
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void draw(Rect clip);
	virtual Size measure();
	struct ListImpl *getList();
	const struct ListImpl *getList() const;
	bool isOpen() const;

	bool isOpen_;
	struct ListImpl *list;
};

struct FrameImpl : public WidgetImpl
{
	FrameImpl();
	void add(Widget *widget);
	void add(struct WidgetImpl *widget);
	void remove(Widget *widget);
	void remove(struct WidgetImpl *widget);
};

struct GroupBoxImpl : public WidgetImpl
{
	GroupBoxImpl(const std::string &label = std::string());
	virtual void onRendererChanged();
	virtual void draw(Rect clip);
	virtual Size measure();
	void setLabel(const char *label);
	const char *getLabel() const;
	void add(Widget *widget);
	void add(struct WidgetImpl *widget);
	void remove(Widget *widget);
	void remove(struct WidgetImpl *widget);

	struct WidgetImpl *content;
	std::string label;

private:
	void refreshMargin();
};

struct LabelImpl : public WidgetImpl
{
	LabelImpl(const std::string &text = std::string());
	virtual void draw(Rect clip);
	virtual Size measure();
	void setMultiline(bool multiline);
	bool getMultiline() const;
	void setText(const char *text);
	const char *getText() const;
	void setTextColor(NVGcolor color);
	NVGcolor getTextColor() const;

private:
	std::string text_;
	bool multiline_;
	NVGcolor textColor_;
	bool isTextColorUserSet_;
};

struct ListImpl : public WidgetImpl
{
	ListImpl(uint8_t *itemData, int itemStride, int nItems);
	virtual void onRendererChanged();
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void draw(Rect clip);
	virtual Size measure();
	Border getItemsBorder() const;
	Rect getItemsRect() const;

	// rect will be absolute - ancestor window position is taken into account.
	Rect getAbsoluteItemsRect() const;

	void setDrawItemCallback(DrawListItemCallback callback);
	DrawListItemCallback getDrawItemCallback() const;
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
	struct ScrollerImpl *getScroller();
	const struct ScrollerImpl *getScroller() const;
	void addCallbackItemSelected(EventCallback callback);

	Border itemsBorder;
	DrawListItemCallback draw_item;
	uint8_t *itemData;
	int itemStride;
	int itemHeight;
	bool isItemHeightUserSet;
	int nItems;
	int firstItem;
	int selectedItem;
	int pressedItem;
	int hoveredItem;

	// The same as hoveredItem, except when pressedItem != -1.
	int mouseOverItem;

	struct ScrollerImpl *scroller;

	std::vector<EventCallback> item_selected_callbacks;

	// Set when the mouse moves. Used to refresh the hovered item when scrolling via the mouse wheel.
	Position lastMousePosition;
};

struct MainWindowImpl : public WidgetImpl
{
	MainWindowImpl(IRenderer *renderer);

	bool isShiftKeyDown() const;
	bool isControlKeyDown() const;

	// Set the centralized event handler. All events invoked by the ancestor widgets of this mainWindow will call the callback function.
	void setEventCallback(EventCallback callback);

	void mouseButtonDown(int mouseButton, int mouseX, int mouseY);
	void mouseButtonUp(int mouseButton, int mouseX, int mouseY);
	void mouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	void mouseWheelMove(int x, int y);

private:
	void keyDelta(Key key, bool down);

public:
	void keyDown(Key key);
	void keyUp(Key key);
	void textInput(const char *text);
	void draw();
	void drawFrame();
	void setMenuBar(struct MenuBarImpl *menuBar);
	void add(Widget *widget);
	void add(struct WidgetImpl *widget);
	void remove(Widget *widget);
	void remove(struct WidgetImpl *widget);
	bool isTextCursorVisible() const;
	void toggleTextCursor();
	void setCursor(Cursor cursor);
	Cursor getCursor() const;
	const struct WidgetImpl *getKeyboardFocusWidget() const;

	// Set keyboard focus to this widget.
	void setKeyboardFocusWidget(struct WidgetImpl *widget);

	DockPosition getWindowDockPosition(const struct WindowImpl *window) const;
	void dockWindow(struct WindowImpl *window, DockPosition dockPosition);
	void undockWindow(struct WindowImpl *window);

	// The docked window has been resized, update the rects of other windows docked at the same position.
	void updateDockedWindowRect(struct WindowImpl *window);

	// Lock input to this widget.
	void pushLockInputWidget(struct WidgetImpl *widget);

	// Stop locking input to this widget.
	void popLockInputWidget(struct WidgetImpl *widget);

	void setMovingWindow(struct WindowImpl *window);
	void updateContentRect();

	struct WidgetImpl *content;

	// Centralized event handler.
	EventCallback handle_event;

	bool isTextCursorVisible_;

	Cursor cursor;

	bool isShiftKeyDown_, isControlKeyDown_;

	std::vector<struct WidgetImpl *> lockInputWidgetStack;

	// Lock input to this window, i.e. don't call mouse_move, mouse_button_down or mouse_button_up on any widget that isn't this window or it's descendants.
	struct WindowImpl *lockInputWindow;

	struct WidgetImpl *keyboardFocusWidget;

	// This window is currently being moved and may be docked.
	struct WindowImpl *movingWindow;

	// Hidden from the consumer.
	struct WidgetImpl *dockIcons[WZ_NUM_DOCK_POSITIONS];
	struct WidgetImpl *dockPreview;

	std::vector<struct WindowImpl *> dockedWindows[WZ_NUM_DOCK_POSITIONS];

	// A window being dragged will be docked to this position on mouse up. Set when the cursor hovers over a dock icon.
	DockPosition windowDockPosition;

	// Each dock position has a tab bar which is visible when multiple windows are docked at the same position.
	struct TabBarImpl *dockTabBars[WZ_NUM_DOCK_POSITIONS];

	bool ignoreDockTabBarChangedEvent;

	struct MenuBarImpl *menuBar;

private:
	// Returns the window that the mouse cursor is hovering over. NULL if there isn't one.
	struct WindowImpl *getHoverWindow(int mouseX, int mouseY);

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
	void updateWindowDrawPriorities(struct WindowImpl *top);
};

struct MenuBarButtonImpl : public WidgetImpl
{
	MenuBarButtonImpl();
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
	struct MenuBarImpl *menuBar;
};

struct MenuBarImpl : public WidgetImpl
{
	MenuBarImpl();
	virtual void onRendererChanged();
	virtual void draw(Rect clip);
	virtual Size measure();
	struct MenuBarButtonImpl *createButton();

	struct StackLayoutImpl *layout;
};

struct RadioButtonImpl : public ButtonImpl
{
	RadioButtonImpl(const std::string &label = std::string());
	virtual void onParented(struct WidgetImpl *parent);
	virtual void draw(Rect clip);
	virtual Size measure();
};

struct ScrollerNub : public WidgetImpl
{
	ScrollerNub();

	struct ScrollerImpl *scroller;
	bool isPressed;

	// The position of the nub when the it was pressed.
	Position pressPosition;

	// The position of the mouse when the nub was pressed.
	Position pressMousePosition;
};

struct ScrollerImpl : public WidgetImpl
{
	ScrollerImpl(ScrollerType scrollerType, int value, int stepValue, int maxValue);
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
	struct ScrollerNub *nub;
	std::vector<EventCallback> value_changed_callbacks;
};

struct SpinnerImpl : public WidgetImpl
{
	SpinnerImpl();
	virtual void onRendererChanged();
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void draw(Rect clip);
	virtual Size measure();
	int getValue() const;
	void setValue(int value);

	struct TextEditImpl *textEdit;
	struct ButtonImpl *decrementButton;
	struct ButtonImpl *incrementButton;
};

struct StackLayoutImpl : public WidgetImpl
{
	StackLayoutImpl(StackLayoutDirection direction, int spacing);
	void setDirection(StackLayoutDirection direction);
	void setSpacing(int spacing);
	int getSpacing() const;
	void add(Widget *widget);
	void add(struct WidgetImpl *widget);
	void remove(Widget *widget);
	void remove(struct WidgetImpl *widget);

	StackLayoutDirection direction;

	// Spacing between child widgets. Applied to the top/left of children.
	int spacing;
};

struct TabBarImpl : public WidgetImpl
{
	TabBarImpl();
	virtual void draw(Rect clip);
	virtual Size measure();
	struct ButtonImpl *createTab();
	void destroyTab(struct ButtonImpl *tab);
	void clearTabs();
	struct ButtonImpl *getDecrementButton();
	struct ButtonImpl *getIncrementButton();
	struct ButtonImpl *getSelectedTab();
	void selectTab(struct ButtonImpl *tab);
	void addCallbackTabChanged(EventCallback callback);
	int getScrollValue() const;

	struct ButtonImpl *selectedTab;
	std::vector<struct ButtonImpl *> tabs;

	int scrollValue;
	struct ButtonImpl *decrementButton;
	struct ButtonImpl *incrementButton;

	std::vector<EventCallback> tab_changed_callbacks;
};

typedef struct
{
	struct ButtonImpl *tab;
	struct WidgetImpl *page;
}
TabbedPage;

struct TabbedImpl : public WidgetImpl
{
	TabbedImpl();
	virtual void draw(Rect clip);
	virtual Size measure();
	void addTab(struct ButtonImpl **tab, struct WidgetImpl **page);

	struct TabBarImpl *tabBar;
	std::vector<TabbedPage> pages;
};

struct TextEditImpl : public WidgetImpl
{
	TextEditImpl(bool multiline, int maximumTextLength);
	virtual void onRendererChanged();
	virtual void draw(Rect clip);
	virtual Size measure();
	void setValidateTextCallback(TextEditValidateTextCallback callback);
	bool isMultiline() const;
	Border getBorder() const;
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

	struct ScrollerImpl *scroller;
	bool multiline;
	int maximumTextLength;
	TextEditValidateTextCallback validate_text;
	Border border;
	bool pressed;
	int cursorIndex;
	int scrollValue;
	int selectionStartIndex;
	int selectionEndIndex;
	std::string text;
};

typedef enum
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
}
WindowDrag;

struct WindowImpl : public WidgetImpl
{
	WindowImpl(const std::string &title);
	virtual void onRendererChanged();
	virtual void onFontChanged(const char *fontFace, float fontSize);
	virtual void draw(Rect clip);
	virtual Size measure();
	int getHeaderHeight() const;
	int getBorderSize() const;
	Rect getHeaderRect() const;
	void setTitle(const char *title);
	const char *getTitle() const;
	struct WidgetImpl *getContentWidget();
	int getDrawPriority() const;
	void setDrawPriority(int drawPriority);

	// Tell the window it's being docked.
	void dock();

	void add(Widget *widget);
	void add(struct WidgetImpl *widget);
	void remove(Widget *widget);
	void remove(struct WidgetImpl *widget);

	int drawPriority;
	int headerHeight;
	int borderSize;
	std::string title;
	
	struct WidgetImpl *content;

	WindowDrag drag;

	// Dragging a docked window header doesn't undock the window until the mouse has moved WZ_WINDOW_UNDOCK_DISTANCE.
	Position undockStartPosition;

	Position resizeStartPosition;
	Rect resizeStartRect;

	// Remember the window size when it is docked, so when the window is undocked the size can be restored.
	Size sizeBeforeDocking;

private:
	void refreshHeaderHeight();
};

void wz_invoke_event(Event *e);
void wz_invoke_event(Event *e, const std::vector<EventCallback> &callbacks);

void wz_text_edit_set_border(struct TextEditImpl *textEdit, Border border);

} // namespace wz
