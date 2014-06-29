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
#include <stdarg.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <wz_cpp.h>

#define IMPLEMENT_STANDARD_WIDGET_INTERFACE(name) \
	wzRect name::getRect() const { return internal_->getRect(); } \
	name name::setPosition(int x, int y) { internal_->setPosition(x, y); return *this; } \
	name name::setSize(int w, int h) { internal_->setSize(w, h); return *this; } \
	name name::setRect(int x, int y, int w, int h) { internal_->setRect(x, y, w, h); return *this; } \
	name name::setAutosize(int autosize) { internal_->setAutosize(autosize); return *this; } \
	name name::setStretch(int stretch) { internal_->setStretch(stretch); return *this; } \
	name name::setAlign(int align) { internal_->setAlign(align); return *this; } \
	name name::setMargin(int margin) { internal_->setMargin(margin); return *this; } \
	name name::setMargin(int top, int right, int bottom, int left) { internal_->setMargin(top, right, bottom, left); return *this; } \
	name name::setMargin(wzBorder margin) { internal_->setMargin(margin); return *this; }

#define IMPLEMENT_CHILD_WIDGET_CREATE(className, parent, childClassName) \
	childClassName className::create##childClassName() { childClassName c; c.internal_ = new childClassName##Internal(parent); children_.push_back(c.internal_); return c; }

namespace wz {

class TabButton;
class DockTabBar;

class Widget
{
public:
	virtual ~Widget() {}
	virtual const wzWidget *getWidget() const = 0;
	virtual wzWidget *getWidget() = 0;
	virtual wzWidget *getContentWidget() { return getWidget(); }
	virtual void draw(wzRect clip) {};
	virtual void handleEvent(wzEvent e) {};
	wzRect getRect() const;
	void setPosition(int x, int y);
	void setSize(int w, int h);
	void setRect(int x, int y, int w, int h);
	void setAutosize(int autosize);
	void setStretch(int stretch);
	void setAlign(int align);
	void setMargin(int margin);
	void setMargin(int top, int right, int bottom, int left);
	void setMargin(wzBorder margin);
	
	wzRenderer *getRenderer()
	{
		return renderer_;
	}

	wzDesktop *getDesktop()
	{
		return wz_widget_get_desktop(getWidget());
	}

protected:
	wzRenderer *renderer_;
};

class ButtonInternal : public Widget
{
public:
	ButtonInternal(Widget *parent);
	ButtonInternal(wzButton *button, const std::string &label);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button_; }
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);
	std::string getLabel() const;
	void setLabel(const std::string &label);

private:
	void initialize();

	wzButton *button_;
	std::string label_;
};

class CheckboxInternal : public Widget
{
public:
	CheckboxInternal(Widget *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button_; }
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);
	std::string getLabel() const;
	void setLabel(const std::string &label);

private:
	wzButton *button_;
	std::string label_;

	static const int boxSize = 16;
	static const int boxRightMargin = 8;
};

class ComboInternal : public Widget
{
public:
	ComboInternal(Widget *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)combo_; }
	virtual wzWidget *getWidget() { return (wzWidget *)combo_; }
	void draw(wzRect clip);
	void setItems(const char **items, int nItems);

private:
	wzCombo *combo_;
	const char **items_;
	std::auto_ptr<ListInternal> list_;
};

class DesktopInternal : public Widget
{
public:
	DesktopInternal(wzRenderer *renderer);
	~DesktopInternal();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)desktop_; }
	virtual wzWidget *getWidget() { return (wzWidget *)desktop_; }
	virtual wzWidget *getContentWidget() { return wz_desktop_get_content_widget(desktop_); }
	void setSize(int w, int h);
	void mouseMove(int x, int y, int dx, int dy);
	void mouseButtonDown(int button, int x, int y);
	void mouseButtonUp(int button, int x, int y);
	void mouseWheelMove(int x, int y);
	void keyDown(wzKey key);
	void keyUp(wzKey key);
	void textInput(const char *text);
	void setShowCursor(bool showCursor) { showCursor_ = showCursor; }
	void draw();
	void drawDockIcon(wzRect rect);
	void drawDockPreview(wzRect rect);

	wzCursor getCursor() const
	{
		return wz_desktop_get_cursor(desktop_);
	}

	static DesktopInternal *fromWidget(wzWidget *widget)
	{
		return (DesktopInternal *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	}

	bool getShowCursor() const { return showCursor_; }

	Button createButton();
	Checkbox createCheckbox();
	Combo createCombo();
	GroupBox createGroupBox();
	Label createLabel();
	List createList();
	RadioButton createRadioButton();
	Scroller createScroller();
	StackLayout createStackLayout();
	TextEdit createTextEdit();
	Tabbed createTabbed();
	Window createWindow();

private:
	wzDesktop *desktop_;
	DockTabBar *dockTabBars_[WZ_NUM_DOCK_POSITIONS];
	bool showCursor_;
	std::vector<Widget *> children_;
};

class DockTabBar : public Widget
{
public:
	DockTabBar(wzTabBar *tabBar);
	~DockTabBar();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabBar_; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabBar_; }
	void draw(wzRect clip);
	void handleEvent(wzEvent e);

private:
	wzTabBar *tabBar_;
	std::vector<TabButton *> tabs_;
	std::auto_ptr<ButtonInternal> decrementButton;
	std::auto_ptr<ButtonInternal> incrementButton;
};

class GroupBoxInternal : public Widget
{
public:
	GroupBoxInternal(Widget *parent);
	~GroupBoxInternal();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)frame_; }
	virtual wzWidget *getWidget() { return (wzWidget *)frame_; }
	virtual wzWidget *getContentWidget() { return wz_frame_get_content_widget(frame_); }
	void draw(wzRect clip);
	std::string getLabel() const;
	void setLabel(const std::string &label);
	Button createButton();
	Checkbox createCheckbox();
	Combo createCombo();
	GroupBox createGroupBox();
	Label createLabel();
	List createList();
	RadioButton createRadioButton();
	Scroller createScroller();
	StackLayout createStackLayout();
	TextEdit createTextEdit();
	Tabbed createTabbed();

private:
	wzFrame *frame_;
	std::string label_;
	std::vector<Widget *> children_;
};

class LabelInternal : public Widget
{
public:
	LabelInternal(Widget *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)label_; }
	virtual wzWidget *getWidget() { return (wzWidget *)label_; }
	void setText(const char *text);
	void setTextColor(uint8_t r, uint8_t g, uint8_t b);
	void draw(wzRect clip);

private:
	wzLabel *label_;
	std::string text_;
	uint8_t r_, g_, b_;
};

class ListInternal : public Widget
{
public:
	ListInternal(Widget *parent);
	ListInternal(wzList *list);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)list_; }
	virtual wzWidget *getWidget() { return (wzWidget *)list_; }
	void draw(wzRect clip);
	void setItems(const char **items, int nItems);

private:
	void initialize();

	wzList *list_;
	const char **items_;
	std::auto_ptr<ScrollerInternal> scroller_;

	static const int itemsMargin = 2;
	static const int itemHeight = 18;
	static const int itemLeftPadding = 4;
};

class RadioButtonInternal : public Widget
{
public:
	RadioButtonInternal(Widget *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button_; }
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);
	std::string RadioButtonInternal::getLabel() const;
	void setLabel(const std::string &label);
	void setGroup(RadioButtonGroup *group);

private:
	wzButton *button_;
	std::string label_;
};

class ScrollerInternal : public Widget
{
public:
	ScrollerInternal(Widget *parent);
	ScrollerInternal(wzScroller *scroller);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)scroller_; }
	virtual wzWidget *getWidget() { return (wzWidget *)scroller_; }
	void draw(wzRect clip);

	void setType(wzScrollerType type);
	void setValue(int value);
	void setStepValue(int stepValue);
	void setMaxValue(int maxValue);
	int getValue() const;

private:
	void initialize();

	wzScroller *scroller_;
	std::auto_ptr<ButtonInternal> decrementButton;
	std::auto_ptr<ButtonInternal> incrementButton;
};

class StackLayoutInternal : public Widget
{
public:
	StackLayoutInternal(Widget *parent);
	~StackLayoutInternal();
	virtual const wzWidget *getWidget() const { return layout_; }
	virtual wzWidget *getWidget() { return layout_; }
	void setDirection(StackLayout::Direction direction);

	Button createButton();
	Checkbox createCheckbox();
	Combo createCombo();
	GroupBox createGroupBox();
	Label createLabel();
	List createList();
	RadioButton createRadioButton();
	Scroller createScroller();
	StackLayout createStackLayout();
	TextEdit createTextEdit();
	Tabbed createTabbed();
	
private:
	wzWidget *layout_;
	Widget *parent_;
	std::vector<Widget *> children_;
};

class TabButton : public Widget
{
public:
	TabButton(wzButton *button);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button_; }
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);
	void setLabel(const std::string &label);

private:
	wzButton *button_;
	std::string label_;
};

class TabBar : public Widget
{
public:
	TabBar(Widget *parent);
	TabBar(wzTabBar *tabBar);
	~TabBar();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabBar_; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabBar_; }
	void draw(wzRect clip);
	TabButton *addTab();
	TabButton *addTab(wzButton *button);

private:
	void initialize();

	wzTabBar *tabBar_;
	std::vector<TabButton *> tabs_;
	std::auto_ptr<ButtonInternal> decrementButton;
	std::auto_ptr<ButtonInternal> incrementButton;
};

class TabPage : public Widget
{
public:
	TabPage(wzWidget *widget);
	virtual const wzWidget *getWidget() const { return widget_; }
	virtual wzWidget *getWidget() { return widget_; }
	void draw(wzRect clip);

private:
	wzWidget *widget_;
};

// Wraps tab button and page.
class TabInternal
{
public:
	TabInternal(TabButton *button, TabPage *page);
	~TabInternal();
	void setLabel(const std::string &label);
	Button createButton();
	Checkbox createCheckbox();
	Combo createCombo();
	GroupBox createGroupBox();
	Label createLabel();
	List createList();
	RadioButton createRadioButton();
	Scroller createScroller();
	StackLayout createStackLayout();
	TextEdit createTextEdit();

private:
	TabButton *button_;
	TabPage *page_;
	std::vector<Widget *> children_;
};

class TabbedInternal : public Widget
{
public:
	TabbedInternal(Widget *parent);
	~TabbedInternal();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabbed_; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabbed_; }
	void draw(wzRect clip);
	TabInternal *createTab();

private:
	wzTabbed *tabbed_;
	std::auto_ptr<TabBar> tabBar_;
	std::vector<TabPage *> tabPages_;
	std::vector<TabInternal *> tabs_;
};

class TextEditInternal : public Widget
{
public:
	TextEditInternal(Widget *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)textEdit_; }
	virtual wzWidget *getWidget() { return (wzWidget *)textEdit_; }
	void draw(wzRect clip);
	void setText(const std::string &text);

private:
	wzTextEdit *textEdit_;

	static const int borderSize_ = 4;
};

class WindowInternal : public Widget
{
public:
	WindowInternal(Widget *parent);
	~WindowInternal();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)window_; }
	virtual wzWidget *getWidget() { return (wzWidget *)window_; }
	virtual wzWidget *getContentWidget() { return wz_window_get_content_widget(window_); }
	void setTitle(const std::string &title);
	std::string getTitle() const { return title_; }
	void draw(wzRect clip);

	Button createButton();
	Checkbox createCheckbox();
	Combo createCombo();
	GroupBox createGroupBox();
	Label createLabel();
	List createList();
	RadioButton createRadioButton();
	Scroller createScroller();
	StackLayout createStackLayout();
	TextEdit createTextEdit();
	Tabbed createTabbed();
	
private:
	wzWindow *window_;
	std::string title_;
	std::vector<Widget *> children_;
};

//------------------------------------------------------------------------------

static void DrawWidget(wzWidget *widget, wzRect clip)
{
	((Widget *)wz_widget_get_metadata(widget))->draw(clip);
}

static void HandleEvent(wzEvent e)
{
	void *metadata = wz_widget_get_metadata(e.base.widget);

	if (metadata)
	{
		((Widget *)metadata)->handleEvent(e);
	}
}

//------------------------------------------------------------------------------

wzRect Widget::getRect() const
{
	return wz_widget_get_absolute_rect(getWidget());
}

void Widget::setPosition(int x, int y)
{
	wz_widget_set_position_args(getWidget(), x, y);
}

void Widget::setSize(int w, int h)
{
	wz_widget_set_size_args(getWidget(), w, h);
}

void Widget::setRect(int x, int y, int w, int h)
{
	wzRect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	wz_widget_set_rect(getWidget(), rect);
}

void Widget::setAutosize(int autosize)
{
	wz_widget_set_autosize(getWidget(), autosize);
}

void Widget::setStretch(int stretch)
{
	wz_widget_set_stretch(getWidget(), stretch);
}

void Widget::setAlign(int align)
{
	wz_widget_set_align(getWidget(), align);
}

void Widget::setMargin(int margin)
{
	wzBorder m;
	m.top = m.right = m.bottom = m.left = margin;
	wz_widget_set_margin(getWidget(), m);
}

void Widget::setMargin(int top, int right, int bottom, int left)
{
	wzBorder m;
	m.top = top;
	m.right = right;
	m.bottom = bottom;
	m.left = left;
	wz_widget_set_margin(getWidget(), m);
}

void Widget::setMargin(wzBorder margin)
{
	wz_widget_set_margin(getWidget(), margin);
}

//------------------------------------------------------------------------------

ButtonInternal::ButtonInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	button_ = wz_button_create(parent->getDesktop());
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)button_);
	initialize();
}

ButtonInternal::ButtonInternal(wzButton *button, const std::string &label) : button_(button), label_(label)
{
	renderer_ = DesktopInternal::fromWidget((wzWidget *)button_)->getRenderer();
	initialize();
}

void ButtonInternal::draw(wzRect clip)
{
	renderer_->draw_button(renderer_, clip, button_, label_.c_str());
}

void ButtonInternal::initialize()
{
	wz_widget_set_metadata((wzWidget *)button_, this);
	wz_widget_set_draw_function((wzWidget *)button_, DrawWidget);
}

std::string ButtonInternal::getLabel() const
{
	return label_;
}

void ButtonInternal::setLabel(const std::string &label)
{
	label_ = label;

	// Calculate size based on label text plus padding.
	wzSize size;
	renderer_->measure_text(renderer_, label_.c_str(), 0, &size.w, &size.h);
	size.w += 16;
	size.h += 8;
	wz_widget_set_size((wzWidget *)button_, size);
}

//------------------------------------------------------------------------------

std::string Button::getLabel() const
{
	return internal_->getLabel();
}

Button Button::setLabel(const std::string &label)
{
	internal_->setLabel(label);
	return *this;
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(Button)

//------------------------------------------------------------------------------

CheckboxInternal::CheckboxInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	button_ = wz_button_create(parent->getDesktop());
	wzWidget *widget = (wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_button_set_set_behavior(button_, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

void CheckboxInternal::draw(wzRect clip)
{
	renderer_->draw_checkbox(renderer_, clip, button_, label_.c_str());
}

std::string CheckboxInternal::getLabel() const
{
	return label_;
}

void CheckboxInternal::setLabel(const std::string &label)
{
	label_ = label;

	// Calculate size.
	wzSize size;
	renderer_->measure_text(renderer_, label_.c_str(), 0, &size.w, &size.h);
	size.w += boxSize + boxRightMargin;
	size.w += 16;
	size.h += 8;
	wz_widget_set_size((wzWidget *)button_, size);
}

//------------------------------------------------------------------------------

std::string Checkbox::getLabel() const
{
	return internal_->getLabel();
}

Checkbox Checkbox::setLabel(const std::string &label)
{
	internal_->setLabel(label);
	return *this;
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(Checkbox)

//------------------------------------------------------------------------------

ComboInternal::ComboInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	combo_ = wz_combo_create(parent->getDesktop());
	wzWidget *widget = (wzWidget *)combo_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
	list_.reset(new ListInternal(wz_combo_get_list(combo_)));
}

void ComboInternal::draw(wzRect clip)
{
	int itemIndex = wz_list_get_selected_item((const wzList *)list_->getWidget());
	renderer_->draw_combo(renderer_, clip, combo_, itemIndex >= 0 ? items_[itemIndex] : NULL);
}

void ComboInternal::setItems(const char **items, int nItems)
{
	// Calculate size based on the biggest item text plus padding.
	wzSize size;
	size.w = size.h = 0;

	for (int i = 0; i < nItems; i++)
	{
		wzSize textSize;
		renderer_->measure_text(renderer_, items[i], 0, &textSize.w, &textSize.h);
		size.w = WZ_MAX(size.w, textSize.w);
		size.h = WZ_MAX(size.h, textSize.h);
	}

	size.w += 50;
	size.h += 4;
	wz_widget_set_size((wzWidget *)combo_, size);

	items_ = items;
	list_->setItems(items, nItems);
}

//------------------------------------------------------------------------------

Combo Combo::setItems(const char **items, int nItems)
{
	internal_->setItems(items, nItems);
	return *this;
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(Combo)

//------------------------------------------------------------------------------

static void MeasureText(struct wzDesktop *desktop, const char *text, int n, int *width, int *height)
{
	wzRenderer *renderer = ((Widget *)wz_widget_get_metadata((wzWidget *)desktop))->getRenderer();
	renderer->measure_text(renderer, text, n, width, height);
}

static int TextGetPixelDelta(struct wzDesktop *desktop, const char *text, int index)
{
	wzRenderer *renderer = ((Widget *)wz_widget_get_metadata((wzWidget *)desktop))->getRenderer();
	return renderer->text_get_pixel_delta(renderer, text, index);
}

static void DrawDockIcon(wzRect rect, void *metadata)
{
	DesktopInternal *desktop = (DesktopInternal *)metadata;
	desktop->drawDockIcon(rect);
}

static void DrawDockPreview(wzRect rect, void *metadata)
{
	DesktopInternal *desktop = (DesktopInternal *)metadata;
	desktop->drawDockPreview(rect);
}

DesktopInternal::DesktopInternal(wzRenderer *renderer) : showCursor_(false)
{
	desktop_ = wz_desktop_create();
	renderer_ = renderer;
	wz_widget_set_metadata((wzWidget *)desktop_, this);
	wz_desktop_set_event_callback(desktop_, HandleEvent);
	wz_desktop_set_measure_text_callback(desktop_, MeasureText);
	wz_desktop_set_text_get_pixel_delta_callback(desktop_, TextGetPixelDelta);
	wz_desktop_set_draw_dock_icon_callback(desktop_, DrawDockIcon, this);
	wz_desktop_set_draw_dock_preview_callback(desktop_, DrawDockPreview, this);

	struct wzTabBar **dockTabBars = wz_desktop_get_dock_tab_bars(desktop_);

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		dockTabBars_[i] = new DockTabBar(dockTabBars[i]);
		wz_widget_set_height((wzWidget *)dockTabBars[i], 20);
	}
}

DesktopInternal::~DesktopInternal()
{
	for (size_t i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		delete dockTabBars_[i];
	}

	for (size_t i = 0; i < children_.size(); i++)
	{
		delete children_[i];
	}

	children_.clear();

	wz_widget_destroy((wzWidget *)desktop_);
}

void DesktopInternal::setSize(int w, int h)
{
	wz_widget_set_size_args((wzWidget *)desktop_, w, h);
}

void DesktopInternal::mouseMove(int x, int y, int dx, int dy)
{
	wz_desktop_mouse_move(desktop_, x, y, dx, dy);
}

void DesktopInternal::mouseButtonDown(int button, int x, int y)
{
	wz_desktop_mouse_button_down(desktop_, button, x, y);
}

void DesktopInternal::mouseButtonUp(int button, int x, int y)
{
	wz_desktop_mouse_button_up(desktop_, button, x, y);
}

void DesktopInternal::mouseWheelMove(int x, int y)
{
	wz_desktop_mouse_wheel_move(desktop_, x, y);
}

void DesktopInternal::keyDown(wzKey key)
{
	wz_desktop_key_down(desktop_, key);
}

void DesktopInternal::keyUp(wzKey key)
{
	wz_desktop_key_up(desktop_, key);
}

void DesktopInternal::textInput(const char *text)
{
	wz_desktop_text_input(desktop_, text);
}

void DesktopInternal::draw()
{
	wzRect rect = wz_widget_get_rect((const wzWidget *)desktop_);
	renderer_->begin_frame(renderer_, rect.w, rect.h);
	wz_desktop_draw(desktop_);
	renderer_->end_frame(renderer_);
}

void DesktopInternal::drawDockIcon(wzRect rect)
{
	renderer_->draw_dock_icon(renderer_, rect);
}

void DesktopInternal::drawDockPreview(wzRect rect)
{
	renderer_->draw_dock_preview(renderer_, rect);
}

IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, List)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, TextEdit)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, Tabbed)
IMPLEMENT_CHILD_WIDGET_CREATE(DesktopInternal, this, Window)

//------------------------------------------------------------------------------

Desktop::Desktop(wzRenderer *renderer)
{
	internal_ = new DesktopInternal(renderer);
}

Desktop::~Desktop()
{
	delete internal_;
}

void Desktop::setSize(int w, int h)
{
	internal_->setSize(w, h);
}

void Desktop::mouseMove(int x, int y, int dx, int dy)
{
	internal_->mouseMove(x, y, dx, dy);
}

void Desktop::mouseButtonDown(int button, int x, int y)
{
	internal_->mouseButtonDown(button, x, y);
}

void Desktop::mouseButtonUp(int button, int x, int y)
{
	internal_->mouseButtonUp(button, x, y);
}

void Desktop::mouseWheelMove(int x, int y)
{
	internal_->mouseWheelMove(x, y);
}

void Desktop::keyDown(wzKey key)
{
	internal_->keyDown(key);
}

void Desktop::keyUp(wzKey key)
{
	internal_->keyUp(key);
}

void Desktop::textInput(const char *text)
{
	internal_->textInput(text);
}

void Desktop::setShowCursor(bool showCursor)
{
	internal_->setShowCursor(showCursor);
}

void Desktop::draw()
{
	internal_->draw();
}

bool Desktop::getShowCursor() const
{
	return internal_->getShowCursor();
}

wzCursor Desktop::getCursor() const
{
	return internal_->getCursor();
}

Button Desktop::createButton()
{
	return internal_->createButton();
}

Checkbox Desktop::createCheckbox()
{
	return internal_->createCheckbox();
}

Combo Desktop::createCombo()
{
	return internal_->createCombo();
}

GroupBox Desktop::createGroupBox()
{
	return internal_->createGroupBox();
}

Label Desktop::createLabel()
{
	return internal_->createLabel();
}

List Desktop::createList()
{
	return internal_->createList();
}

RadioButton Desktop::createRadioButton()
{
	return internal_->createRadioButton();
}

Scroller Desktop::createScroller()
{
	return internal_->createScroller();
}

StackLayout Desktop::createStackLayout()
{
	return internal_->createStackLayout();
}

Tabbed Desktop::createTabbed()
{
	return internal_->createTabbed();
}

TextEdit Desktop::createTextEdit()
{
	return internal_->createTextEdit();
}

Window Desktop::createWindow()
{
	return internal_->createWindow();
}

//------------------------------------------------------------------------------

DockTabBar::DockTabBar(wzTabBar *tabBar) : tabBar_(tabBar)
{
	renderer_ = DesktopInternal::fromWidget((wzWidget *)tabBar)->getRenderer();
	wz_widget_set_metadata((wzWidget *)tabBar_, this);

	decrementButton.reset(new ButtonInternal(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton->getWidget(), 14);
	incrementButton.reset(new ButtonInternal(wz_tab_bar_get_increment_button(tabBar_), ">"));
	wz_widget_set_width((wzWidget *)incrementButton->getWidget(), 14);
}

DockTabBar::~DockTabBar()
{
	for (size_t i = 0; i < tabs_.size(); i++)
	{
		delete tabs_[i];
	}
}

void DockTabBar::draw(wzRect clip)
{
}

void DockTabBar::handleEvent(wzEvent e)
{
	if (e.base.type == WZ_EVENT_TAB_BAR_TAB_ADDED)
	{
		// Wrap the added tab (e.tabBar.tab) in a new TabButton instance.
		WindowInternal *window = (WindowInternal *)wz_widget_get_metadata((wzWidget *)wz_desktop_get_dock_tab_window(wz_widget_get_desktop((wzWidget *)tabBar_), e.tabBar.tab));
		TabButton *tabButton = new TabButton(e.tabBar.tab);
		tabButton->setLabel(window->getTitle());
		tabs_.push_back(tabButton);
	}
	else if (e.base.type == WZ_EVENT_TAB_BAR_TAB_REMOVED)
	{
		// Remove the corresponding TabButton instance.
		for (size_t i = 0; i < tabs_.size(); i++)
		{
			if (tabs_[i]->getWidget() == (wzWidget *)e.tabBar.tab)
			{
				TabButton *tab = tabs_[i];
				tabs_.erase(tabs_.begin() + i);
				delete tab;
				return;
			}
		}
	}
}

//------------------------------------------------------------------------------

GroupBoxInternal::GroupBoxInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	frame_ = wz_frame_create(parent->getDesktop());
	wzWidget *widget = (wzWidget *)frame_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_set_size_args(widget, 200, 200);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

GroupBoxInternal::~GroupBoxInternal()
{
	for (size_t i = 0; i < children_.size(); i++)
	{
		delete children_[i];
	}

	children_.clear();
}

void GroupBoxInternal::draw(wzRect clip)
{
	renderer_->draw_group_box(renderer_, clip, frame_, label_.c_str());
}

std::string GroupBoxInternal::getLabel() const
{
	return label_;
}

void GroupBoxInternal::setLabel(const std::string &label)
{
	label_ = label;
	wz_widget_set_margin(getContentWidget(), renderer_->measure_group_box_margin(renderer_, label_.c_str()));
}

IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, List)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, TextEdit)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBoxInternal, this, Tabbed)

//------------------------------------------------------------------------------

std::string GroupBox::getLabel() const
{
	return internal_->getLabel();
}

GroupBox GroupBox::setLabel(const std::string &label)
{
	internal_->setLabel(label);
	return *this;
}

Button GroupBox::createButton()
{
	return internal_->createButton();
}

Checkbox GroupBox::createCheckbox()
{
	return internal_->createCheckbox();
}

Combo GroupBox::createCombo()
{
	return internal_->createCombo();
}

GroupBox GroupBox::createGroupBox()
{
	return internal_->createGroupBox();
}

Label GroupBox::createLabel()
{
	return internal_->createLabel();
}

List GroupBox::createList()
{
	return internal_->createList();
}

RadioButton GroupBox::createRadioButton()
{
	return internal_->createRadioButton();
}

Scroller GroupBox::createScroller()
{
	return internal_->createScroller();
}

StackLayout GroupBox::createStackLayout()
{
	return internal_->createStackLayout();
}

TextEdit GroupBox::createTextEdit()
{
	return internal_->createTextEdit();
}

Tabbed GroupBox::createTabbed()
{
	return internal_->createTabbed();
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(GroupBox)

//------------------------------------------------------------------------------

LabelInternal::LabelInternal(Widget *parent) : r_(255), g_(255), b_(255)
{
	renderer_ = parent->getRenderer();
	label_ = wz_label_create(parent->getDesktop());
	wzWidget *widget = (wzWidget *)label_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

void LabelInternal::setText(const char *text)
{
	text_ = text;

	wzSize size;
	renderer_->measure_text(renderer_, text_.c_str(), 0, &size.w, &size.h);
	wz_widget_set_size((wzWidget *)label_, size);
}

void LabelInternal::setTextColor(uint8_t r, uint8_t g, uint8_t b)
{
	r_ = r;
	g_ = g;
	b_ = b;
}

void LabelInternal::draw(wzRect clip)
{
	renderer_->draw_label(renderer_, clip, label_, text_.c_str(), r_, g_, b_);
}

//------------------------------------------------------------------------------

Label Label::setText(const char *format, ...)
{
	static char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	internal_->setText(buffer);
	return *this;
}

Label Label::setTextColor(uint8_t r, uint8_t g, uint8_t b)
{
	internal_->setTextColor(r, g, b);
	return *this;
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(Label)

//------------------------------------------------------------------------------

ListInternal::ListInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	list_ = wz_list_create(parent->getDesktop());
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)list_);
	initialize();
}

ListInternal::ListInternal(wzList *list) : list_(list)
{
	renderer_ = DesktopInternal::fromWidget((wzWidget *)list_)->getRenderer();
	initialize();
}

void ListInternal::draw(wzRect clip)
{
	renderer_->draw_list(renderer_, clip, list_, items_);
}

void ListInternal::initialize()
{
	wz_list_set_item_height(list_, itemHeight);
	wz_list_set_items_border_args(list_, itemsMargin, itemsMargin, itemsMargin, itemsMargin);
	wz_widget_set_metadata((wzWidget *)list_, this);
	wz_widget_set_draw_function((wzWidget *)list_, DrawWidget);

	scroller_.reset(new ScrollerInternal(wz_list_get_scroller(list_)));
	wz_widget_set_size_args(scroller_->getWidget(), 16, 0);
}

void ListInternal::setItems(const char **items, int nItems)
{
	items_ = items;
	wz_list_set_num_items(list_, nItems);
}

//------------------------------------------------------------------------------

List List::setItems(const char **items, int nItems)
{
	internal_->setItems(items, nItems);
	return *this;
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(List)

//------------------------------------------------------------------------------

RadioButtonGroup::RadioButtonGroup()
{
	group_ = wz_radio_button_group_create();
}

RadioButtonGroup::~RadioButtonGroup()
{
	wz_radio_button_group_destroy(group_);
}

//------------------------------------------------------------------------------

RadioButtonInternal::RadioButtonInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	button_ = wz_button_create(parent->getDesktop());
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)button_);
	wz_widget_set_metadata((wzWidget *)button_, this);
	wz_widget_set_draw_function((wzWidget *)button_, DrawWidget);
}

void RadioButtonInternal::draw(wzRect clip)
{
	renderer_->draw_radio_button(renderer_, clip, button_, label_.c_str());
}

std::string RadioButtonInternal::getLabel() const
{
	return label_;
}

void RadioButtonInternal::setLabel(const std::string &label)
{
	label_ = label;
	wz_widget_set_size((wzWidget *)button_, renderer_->measure_radio_button(renderer_, label_.c_str()));
}

void RadioButtonInternal::setGroup(RadioButtonGroup *group)
{
	wz_radio_button_group_add_button(group->get(), button_);
}

//------------------------------------------------------------------------------

std::string RadioButton::getLabel() const
{
	return internal_->getLabel();
}

RadioButton RadioButton::setLabel(const std::string &label)
{
	internal_->setLabel(label);
	return *this;
}

RadioButton RadioButton::setGroup(RadioButtonGroup *group)
{
	internal_->setGroup(group);
	return *this;
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(RadioButton)

//------------------------------------------------------------------------------

ScrollerInternal::ScrollerInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	scroller_ = wz_scroller_create(parent->getDesktop());
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)scroller_);
	initialize();
}

ScrollerInternal::ScrollerInternal(wzScroller *scroller) : scroller_(scroller)
{
	renderer_ = DesktopInternal::fromWidget((wzWidget *)scroller_)->getRenderer();
	initialize();
}

void ScrollerInternal::draw(wzRect clip)
{
	renderer_->draw_scroller(renderer_, clip, scroller_);
}

void ScrollerInternal::setType(wzScrollerType type)
{
	wz_scroller_set_type(scroller_, type);
}

void ScrollerInternal::setValue(int value)
{
	wz_scroller_set_value(scroller_, value);
}

void ScrollerInternal::setStepValue(int stepValue)
{
	wz_scroller_set_step_value(scroller_, stepValue);
}

void ScrollerInternal::setMaxValue(int maxValue)
{
	wz_scroller_set_max_value(scroller_, maxValue);
}

int ScrollerInternal::getValue() const
{
	return wz_scroller_get_value(scroller_);
}

void ScrollerInternal::initialize()
{
	wz_widget_set_metadata((wzWidget *)scroller_, this);
	wz_widget_set_draw_function((wzWidget *)scroller_, DrawWidget);
	wz_scroller_set_nub_size(scroller_, 16);

	decrementButton.reset(new ButtonInternal(wz_scroller_get_decrement_button(scroller_), "-"));
	incrementButton.reset(new ButtonInternal(wz_scroller_get_increment_button(scroller_), "+"));

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wz_widget_set_size_args((wzWidget *)wz_scroller_get_decrement_button(scroller_), 16, 16);
	wz_widget_set_size_args((wzWidget *)wz_scroller_get_increment_button(scroller_), 16, 16);
}

//------------------------------------------------------------------------------

Scroller Scroller::setType(wzScrollerType type)
{
	internal_->setType(type);
	return *this;
}

Scroller Scroller::setValue(int value)
{
	internal_->setValue(value);
	return *this;
}

Scroller Scroller::setStepValue(int stepValue)
{
	internal_->setStepValue(stepValue);
	return *this;
}

Scroller Scroller::setMaxValue(int maxValue)
{
	internal_->setMaxValue(maxValue);
	return *this;
}

int Scroller::getValue() const
{
	return internal_->getValue();
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(Scroller)

//------------------------------------------------------------------------------

StackLayoutInternal::StackLayoutInternal(Widget *parent) : layout_(NULL), parent_(parent)
{
	renderer_ = parent->getRenderer();
}

StackLayoutInternal::~StackLayoutInternal()
{
	for (size_t i = 0; i < children_.size(); i++)
	{
		delete children_[i];
	}

	children_.clear();
}

void StackLayoutInternal::setDirection(StackLayout::Direction direction)
{
	if (direction == StackLayout::Horizontal)
	{
		layout_ = (wzWidget *)wz_horizontal_stack_layout_create(parent_->getDesktop());
	}
	else
	{
		layout_ = (wzWidget *)wz_vertical_stack_layout_create(parent_->getDesktop());
	}

	wz_widget_add_child_widget(parent_->getContentWidget(), layout_);
}

IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, List)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, TextEdit)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayoutInternal, this, Tabbed)

//------------------------------------------------------------------------------

StackLayout StackLayout::setDirection(Direction direction)
{
	internal_->setDirection(direction);
	return *this;
}

Button StackLayout::createButton()
{
	return internal_->createButton();
}

Checkbox StackLayout::createCheckbox()
{
	return internal_->createCheckbox();
}

Combo StackLayout::createCombo()
{
	return internal_->createCombo();
}

GroupBox StackLayout::createGroupBox()
{
	return internal_->createGroupBox();
}

Label StackLayout::createLabel()
{
	return internal_->createLabel();
}

List StackLayout::createList()
{
	return internal_->createList();
}

RadioButton StackLayout::createRadioButton()
{
	return internal_->createRadioButton();
}

Scroller StackLayout::createScroller()
{
	return internal_->createScroller();
}

StackLayout StackLayout::createStackLayout()
{
	return internal_->createStackLayout();
}

Tabbed StackLayout::createTabbed()
{
	return internal_->createTabbed();
}

TextEdit StackLayout::createTextEdit()
{
	return internal_->createTextEdit();
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(StackLayout)

//------------------------------------------------------------------------------

TabButton::TabButton(wzButton *button) : button_(button)
{
	renderer_ = DesktopInternal::fromWidget((wzWidget *)button)->getRenderer();
	wz_widget_set_metadata((wzWidget *)button_, this);
	wz_widget_set_draw_function((wzWidget *)button_, DrawWidget);
}

void TabButton::draw(wzRect clip)
{
	renderer_->draw_tab_button(renderer_, clip, button_, label_.c_str());
}

void TabButton::setLabel(const std::string &label)
{
	label_ = label;

	// Calculate width based on label text plus padding.
	int width;
	renderer_->measure_text(renderer_, label_.c_str(), 0, &width, NULL);
	width += 16;
	wz_widget_set_width((wzWidget *)button_, width);
}

//------------------------------------------------------------------------------

TabBar::TabBar(Widget *parent)
{
	renderer_ = parent->getRenderer();
	tabBar_ = wz_tab_bar_create(parent->getDesktop());
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)tabBar_);
	initialize();
}

TabBar::TabBar(wzTabBar *tabBar) : tabBar_(tabBar)
{
	renderer_ = DesktopInternal::fromWidget((wzWidget *)tabBar)->getRenderer();
	initialize();
}

TabBar::~TabBar()
{
	for (size_t i = 0; i < tabs_.size(); i++)
	{
		delete tabs_[i];
	}
}

void TabBar::draw(wzRect clip)
{
}

TabButton *TabBar::addTab()
{
	wzButton *button = wz_tab_bar_add_tab(tabBar_);
	TabButton *tabButton = new TabButton(button);
	tabs_.push_back(tabButton);
	return tabButton;
}

TabButton *TabBar::addTab(wzButton *button)
{
	TabButton *tabButton = new TabButton(button);
	tabs_.push_back(tabButton);
	return tabButton;
}

void TabBar::initialize()
{
	wz_widget_set_metadata((wzWidget *)tabBar_, this);

	decrementButton.reset(new ButtonInternal(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton->getWidget(), 14);
	incrementButton.reset(new ButtonInternal(wz_tab_bar_get_increment_button(tabBar_), ">"));
	wz_widget_set_width((wzWidget *)incrementButton->getWidget(), 14);
}

//------------------------------------------------------------------------------

TabPage::TabPage(wzWidget *widget)
{
	widget_ = widget;
	renderer_ = DesktopInternal::fromWidget(widget)->getRenderer();
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget_, DrawWidget);
}

void TabPage::draw(wzRect clip)
{
	renderer_->draw_tab_page(renderer_, clip, widget_);
}

//------------------------------------------------------------------------------

TabInternal::TabInternal(TabButton *button, TabPage *page) : button_(button), page_(page)
{
}

TabInternal::~TabInternal()
{
	for (size_t i = 0; i < children_.size(); i++)
	{
		delete children_[i];
	}

	children_.clear();
}

void TabInternal::setLabel(const std::string &label)
{
	button_->setLabel(label);
}

IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, List)
IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(TabInternal, page_, TextEdit)

//------------------------------------------------------------------------------

Tab Tab::setLabel(const std::string &label)
{
	internal_->setLabel(label);
	return *this;
}

Button Tab::createButton()
{
	return internal_->createButton();
}

Checkbox Tab::createCheckbox()
{
	return internal_->createCheckbox();
}

Combo Tab::createCombo()
{
	return internal_->createCombo();
}

GroupBox Tab::createGroupBox()
{
	return internal_->createGroupBox();
}

Label Tab::createLabel()
{
	return internal_->createLabel();
}

List Tab::createList()
{
	return internal_->createList();
}

RadioButton Tab::createRadioButton()
{
	return internal_->createRadioButton();
}

Scroller Tab::createScroller()
{
	return internal_->createScroller();
}

StackLayout Tab::createStackLayout()
{
	return internal_->createStackLayout();
}

TextEdit Tab::createTextEdit()
{
	return internal_->createTextEdit();
}

//------------------------------------------------------------------------------

TabbedInternal::TabbedInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	tabbed_ = wz_tabbed_create(parent->getDesktop());
	wz_widget_set_metadata((wzWidget *)tabbed_, this);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)tabbed_);
	tabBar_.reset(new TabBar(wz_tabbed_get_tab_bar(tabbed_)));
	tabBar_->setRect(0, 0, 0, 20);
}

TabbedInternal::~TabbedInternal()
{
	for (size_t i = 0; i < tabPages_.size(); i++)
	{
		delete tabPages_[i];
	}

	tabPages_.clear();

	for (size_t i = 0; i < tabPages_.size(); i++)
	{
		delete tabs_[i];
	}

	tabs_.clear();
}

void TabbedInternal::draw(wzRect clip)
{
}

TabInternal *TabbedInternal::createTab()
{
	wzButton *button;
	wzWidget *widget;
	wz_tabbed_add_tab(tabbed_, &button, &widget);
	TabButton *tabButton = tabBar_->addTab(button);
	
	TabPage *tabPage = new TabPage(widget);
	tabPages_.push_back(tabPage);

	TabInternal *tab = new TabInternal(tabButton, tabPage);
	tabs_.push_back(tab);
	
	return tab;
}

//------------------------------------------------------------------------------

Tab Tabbed::createTab()
{
	Tab tab;
	tab.internal_ = internal_->createTab();
	return tab;
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(Tabbed)

//------------------------------------------------------------------------------

TextEditInternal::TextEditInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	textEdit_ = wz_text_edit_create(parent->getDesktop(), 256);
	wz_text_edit_set_border_args(textEdit_, borderSize_, borderSize_, borderSize_, borderSize_);
	wzWidget *widget = (wzWidget *)textEdit_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

void TextEditInternal::draw(wzRect clip)
{
	renderer_->draw_text_edit(renderer_, clip, textEdit_, DesktopInternal::fromWidget(getWidget())->getShowCursor());
}

void TextEditInternal::setText(const std::string &text)
{
	wz_text_edit_set_text(textEdit_, text.c_str());

	int h;
	renderer_->measure_text(renderer_, text.c_str(), 0, NULL, &h);
	wz_widget_set_size_args((wzWidget *)textEdit_, 100, h + borderSize_ * 2);
}

//------------------------------------------------------------------------------

TextEdit TextEdit::setText(const std::string &text)
{
	internal_->setText(text);
	return *this;
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(TextEdit)

//------------------------------------------------------------------------------

WindowInternal::WindowInternal(Widget *parent)
{
	renderer_ = parent->getRenderer();
	window_ = wz_window_create(parent->getDesktop());
	wzWidget *widget = (wzWidget *)window_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_window_set_border_size(window_, 4);
	wz_widget_add_child_widget(parent->getWidget(), widget);
}

WindowInternal::~WindowInternal()
{
	for (size_t i = 0; i < children_.size(); i++)
	{
		delete children_[i];
	}

	children_.clear();
}

void WindowInternal::draw(wzRect clip)
{
	renderer_->draw_window(renderer_, clip, window_, title_.c_str());
}

void WindowInternal::setTitle(const std::string &title)
{ 
	title_ = title;

	// Calculate header height based on label text plus padding.
	wzSize size;
	renderer_->measure_text(renderer_, title_.c_str(), 0, &size.w, &size.h);
	wz_window_set_header_height(window_, size.h + 6);
}

IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, List)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, TextEdit)
IMPLEMENT_CHILD_WIDGET_CREATE(WindowInternal, this, Tabbed)

//------------------------------------------------------------------------------

Window Window::setTitle(const std::string &title)
{
	internal_->setTitle(title);
	return *this;
}

std::string Window::getTitle() const
{
	return internal_->getTitle();
}

Button Window::createButton()
{
	return internal_->createButton();
}

Checkbox Window::createCheckbox()
{
	return internal_->createCheckbox();
}

Combo Window::createCombo()
{
	return internal_->createCombo();
}

GroupBox Window::createGroupBox()
{
	return internal_->createGroupBox();
}

Label Window::createLabel()
{
	return internal_->createLabel();
}

List Window::createList()
{
	return internal_->createList();
}

RadioButton Window::createRadioButton()
{
	return internal_->createRadioButton();
}

Scroller Window::createScroller()
{
	return internal_->createScroller();
}

StackLayout Window::createStackLayout()
{
	return internal_->createStackLayout();
}

Tabbed Window::createTabbed()
{
	return internal_->createTabbed();
}

TextEdit Window::createTextEdit()
{
	return internal_->createTextEdit();
}

IMPLEMENT_STANDARD_WIDGET_INTERFACE(Window)

} // namespace wz
