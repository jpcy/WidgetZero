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

#define IMPLEMENT_CHILD_WIDGET_CREATE(className, object, parent, childClassName) \
	childClassName className::create##childClassName() { childClassName c; c.p = new childClassName##Private(parent); (object)->children.push_back(c.p); return c; }

namespace wz {

class DockTabBar;
struct ListPrivate;
class TabButton;
struct ScrollerPrivate;

struct WidgetPrivate
{
	virtual ~WidgetPrivate() {}
	virtual const wzWidget *getWidget() const = 0;
	virtual wzWidget *getWidget() = 0;
	virtual wzWidget *getContentWidget() { return getWidget(); }
	virtual void draw(wzRect clip) {};
	virtual void handleEvent(wzEvent e) {};
	
	wzRenderer *renderer;
	wzDesktop *desktop;
};

struct ButtonPrivate : public WidgetPrivate
{
	ButtonPrivate(WidgetPrivate *parent);
	ButtonPrivate(wzButton *button, const std::string &label);
	void initialize();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }
	void draw(wzRect clip);

	wzButton *button;
	std::string label;
};

struct CheckboxPrivate : public WidgetPrivate
{
	CheckboxPrivate(WidgetPrivate *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }
	void draw(wzRect clip);

	wzButton *button;
	std::string label;
};

struct ComboPrivate : public WidgetPrivate
{
	ComboPrivate(WidgetPrivate *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)combo; }
	virtual wzWidget *getWidget() { return (wzWidget *)combo; }
	void draw(wzRect clip);

	wzCombo *combo;
	const char **items;
	std::auto_ptr<ListPrivate> list;
};

struct DesktopPrivate : public WidgetPrivate
{
	DesktopPrivate(wzRenderer *renderer);
	~DesktopPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)desktop; }
	virtual wzWidget *getWidget() { return (wzWidget *)desktop; }
	virtual wzWidget *getContentWidget() { return wz_desktop_get_content_widget(desktop); }
	void draw();
	void drawDockIcon(wzRect rect);
	void drawDockPreview(wzRect rect);

	static DesktopPrivate *fromWidget(wzWidget *widget)
	{
		return (DesktopPrivate *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	}

	DockTabBar *dockTabBars[WZ_NUM_DOCK_POSITIONS];
	bool showCursor;
	std::vector<WidgetPrivate *> children;
};

class DockTabBar : public WidgetPrivate
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
	std::auto_ptr<ButtonPrivate> decrementButton_;
	std::auto_ptr<ButtonPrivate> incrementButton_;
};

struct GroupBoxPrivate : public WidgetPrivate
{
	GroupBoxPrivate(WidgetPrivate *parent);
	~GroupBoxPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)frame; }
	virtual wzWidget *getWidget() { return (wzWidget *)frame; }
	virtual wzWidget *getContentWidget() { return wz_frame_get_content_widget(frame); }
	void draw(wzRect clip);

	wzFrame *frame;
	std::string label;
	std::vector<WidgetPrivate *> children;
};

struct LabelPrivate : public WidgetPrivate
{
	LabelPrivate(WidgetPrivate *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)label; }
	virtual wzWidget *getWidget() { return (wzWidget *)label; }
	void draw(wzRect clip);

	wzLabel *label;
	std::string text;
	uint8_t r, g, b;
};

struct ListPrivate : public WidgetPrivate
{
	ListPrivate(WidgetPrivate *parent);
	ListPrivate(wzList *list);
	void initialize();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)list; }
	virtual wzWidget *getWidget() { return (wzWidget *)list; }
	void draw(wzRect clip);

	void setItems(const char **items, int nItems);

	wzList *list;
	const char **items;
	std::auto_ptr<ScrollerPrivate> scroller;

	static const int itemsMargin = 2;
	static const int itemHeight = 18;
	static const int itemLeftPadding = 4;
};

struct RadioButtonPrivate : public WidgetPrivate
{
	RadioButtonPrivate(WidgetPrivate *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }
	void draw(wzRect clip);

	wzButton *button;
	std::string label;
	RadioButtonGroup *group;
};

struct ScrollerPrivate : public WidgetPrivate
{
	ScrollerPrivate(WidgetPrivate *parent);
	ScrollerPrivate(wzScroller *scroller);
	void initialize();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)scroller; }
	virtual wzWidget *getWidget() { return (wzWidget *)scroller; }
	void draw(wzRect clip);

	wzScroller *scroller;
	std::auto_ptr<ButtonPrivate> decrementButton;
	std::auto_ptr<ButtonPrivate> incrementButton;
};

struct StackLayoutPrivate : public WidgetPrivate
{
	StackLayoutPrivate(WidgetPrivate *parent);
	~StackLayoutPrivate();
	virtual const wzWidget *getWidget() const { return (wzWidget *)layout; }
	virtual wzWidget *getWidget() { return (wzWidget *)layout; }
	
	wzStackLayout *layout;
	WidgetPrivate *parent;
	std::vector<WidgetPrivate *> children;
};

class TabButton : public WidgetPrivate
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

class TabBar : public WidgetPrivate
{
public:
	TabBar(WidgetPrivate *parent);
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
	std::auto_ptr<ButtonPrivate> decrementButton_;
	std::auto_ptr<ButtonPrivate> incrementButton_;
};

class TabPage : public WidgetPrivate
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
struct TabPrivate
{
	TabPrivate(TabButton *button, TabPage *page);
	~TabPrivate();

	TabButton *button;
	TabPage *page;
	std::vector<WidgetPrivate *> children;
};

struct TabbedPrivate : public WidgetPrivate
{
	TabbedPrivate(WidgetPrivate *parent);
	~TabbedPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabbed; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabbed; }
	void draw(wzRect clip);

	wzTabbed *tabbed;
	std::auto_ptr<TabBar> tabBar;
	std::vector<TabPage *> tabPages;
	std::vector<TabPrivate *> tabs;
};

struct TextEditPrivate : public WidgetPrivate
{
	TextEditPrivate(WidgetPrivate *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)textEdit; }
	virtual wzWidget *getWidget() { return (wzWidget *)textEdit; }
	void draw(wzRect clip);

	wzTextEdit *textEdit;

	static const int borderSize = 4;
};

struct WindowPrivate : public WidgetPrivate
{
	WindowPrivate(WidgetPrivate *parent);
	~WindowPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)window; }
	virtual wzWidget *getWidget() { return (wzWidget *)window; }
	virtual wzWidget *getContentWidget() { return wz_window_get_content_widget(window); }
	void draw(wzRect clip);

	wzWindow *window;
	std::string title;
	std::vector<WidgetPrivate *> children;
};

//------------------------------------------------------------------------------

static void DrawWidget(wzWidget *widget, wzRect clip)
{
	((WidgetPrivate *)wz_widget_get_metadata(widget))->draw(clip);
}

static void HandleEvent(wzEvent e)
{
	void *metadata = wz_widget_get_metadata(e.base.widget);

	if (metadata)
	{
		((WidgetPrivate *)metadata)->handleEvent(e);
	}
}

//------------------------------------------------------------------------------

wzRect Widget::getRect() const
{
	return wz_widget_get_absolute_rect(p->getWidget());
}

Widget Widget::setPosition(int x, int y)
{ 
	wz_widget_set_position_args(p->getWidget(), x, y);
	return *this;
}

Widget Widget::setSize(int w, int h)
{
	wz_widget_set_size_args(p->getWidget(), w, h);
	return *this;
}

Widget Widget::setRect(int x, int y, int w, int h)
{
	wz_widget_set_rect_args(p->getWidget(), x, y, w, h);
	return *this;
}

Widget Widget::setAutosize(int autosize)
{
	wz_widget_set_autosize(p->getWidget(), autosize);
	return *this;
}

Widget Widget::setStretch(int stretch)
{
	wz_widget_set_stretch(p->getWidget(), stretch);
	return *this;
}

Widget Widget::setAlign(int align)
{
	wz_widget_set_align(p->getWidget(), align);
	return *this;
}

Widget Widget::setMargin(int margin)
{
	wz_widget_set_margin_args(p->getWidget(), margin, margin, margin, margin);
	return *this;
}

Widget Widget::setMargin(int top, int right, int bottom, int left)
{
	wz_widget_set_margin_args(p->getWidget(), top, right, bottom, left);
	return *this;
}

Widget Widget::setMargin(wzBorder margin)
{
	wz_widget_set_margin(p->getWidget(), margin);
	return *this;
}

//------------------------------------------------------------------------------

ButtonPrivate::ButtonPrivate(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	button = wz_button_create(desktop);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)button);
	initialize();
}

ButtonPrivate::ButtonPrivate(wzButton *button, const std::string &label) : button(button), label(label)
{
	renderer = DesktopPrivate::fromWidget((wzWidget *)button)->renderer;
	initialize();
}

void ButtonPrivate::initialize()
{
	wz_widget_set_metadata((wzWidget *)button, this);
	wz_widget_set_draw_function((wzWidget *)button, DrawWidget);
}

void ButtonPrivate::draw(wzRect clip)
{
	renderer->draw_button(renderer, clip, button, label.c_str());
}

//------------------------------------------------------------------------------

std::string Button::getLabel() const
{
	return ((ButtonPrivate *)p)->label;
}

Button Button::setLabel(const std::string &label)
{
	ButtonPrivate *bp = (ButtonPrivate *)p;
	bp->label = label;

	// Calculate size based on label text plus padding.
	wzSize size;
	p->renderer->measure_text(p->renderer, bp->label.c_str(), 0, &size.w, &size.h);
	size.w += 16;
	size.h += 8;
	wz_widget_set_size((wzWidget *)bp->button, size);

	return *this;
}

//------------------------------------------------------------------------------

CheckboxPrivate::CheckboxPrivate(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	button = wz_button_create(desktop);
	wzWidget *widget = (wzWidget *)button;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_button_set_set_behavior(button, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

void CheckboxPrivate::draw(wzRect clip)
{
	renderer->draw_checkbox(renderer, clip, button, label.c_str());
}

//------------------------------------------------------------------------------

std::string Checkbox::getLabel() const
{
	return ((CheckboxPrivate *)p)->label;
}

Checkbox Checkbox::setLabel(const std::string &label)
{
	CheckboxPrivate *cp = (CheckboxPrivate *)p;
	cp->label = label;
	wz_widget_set_size((wzWidget *)cp->button, p->renderer->measure_checkbox(p->renderer, cp->label.c_str()));
	return *this;
}

//------------------------------------------------------------------------------

ComboPrivate::ComboPrivate(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	combo = wz_combo_create(desktop);
	wzWidget *widget = (wzWidget *)combo;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
	list.reset(new ListPrivate(wz_combo_get_list(combo)));
}

void ComboPrivate::draw(wzRect clip)
{
	int itemIndex = wz_list_get_selected_item((const wzList *)list->getWidget());
	renderer->draw_combo(renderer, clip, combo, itemIndex >= 0 ? items[itemIndex] : NULL);
}

//------------------------------------------------------------------------------

Combo Combo::setItems(const char **items, int nItems)
{
	// Calculate size based on the biggest item text plus padding.
	wzSize size;
	size.w = size.h = 0;

	for (int i = 0; i < nItems; i++)
	{
		wzSize textSize;
		p->renderer->measure_text(p->renderer, items[i], 0, &textSize.w, &textSize.h);
		size.w = WZ_MAX(size.w, textSize.w);
		size.h = WZ_MAX(size.h, textSize.h);
	}

	ComboPrivate *cp = (ComboPrivate *)p;

	size.w += 50;
	size.h += 4;
	wz_widget_set_size((wzWidget *)cp->combo, size);

	cp->items = items;
	cp->list->setItems(items, nItems);
	return *this;
}


//------------------------------------------------------------------------------

static void MeasureText(struct wzDesktop *desktop, const char *text, int n, int *width, int *height)
{
	wzRenderer *renderer = ((WidgetPrivate *)wz_widget_get_metadata((wzWidget *)desktop))->renderer;
	renderer->measure_text(renderer, text, n, width, height);
}

static int TextGetPixelDelta(struct wzDesktop *desktop, const char *text, int index)
{
	wzRenderer *renderer = ((WidgetPrivate *)wz_widget_get_metadata((wzWidget *)desktop))->renderer;
	return renderer->text_get_pixel_delta(renderer, text, index);
}

static void DrawDockIcon(wzRect rect, void *metadata)
{
	DesktopPrivate *desktop = (DesktopPrivate *)metadata;
	desktop->drawDockIcon(rect);
}

static void DrawDockPreview(wzRect rect, void *metadata)
{
	DesktopPrivate *desktop = (DesktopPrivate *)metadata;
	desktop->drawDockPreview(rect);
}

DesktopPrivate::DesktopPrivate(wzRenderer *renderer) : showCursor(false)
{
	desktop = wz_desktop_create();
	this->renderer = renderer;
	wz_widget_set_metadata((wzWidget *)desktop, this);
	wz_desktop_set_event_callback(desktop, HandleEvent);
	wz_desktop_set_measure_text_callback(desktop, MeasureText);
	wz_desktop_set_text_get_pixel_delta_callback(desktop, TextGetPixelDelta);
	wz_desktop_set_draw_dock_icon_callback(desktop, DrawDockIcon, this);
	wz_desktop_set_draw_dock_preview_callback(desktop, DrawDockPreview, this);

	struct wzTabBar **dockTabBars = wz_desktop_get_dock_tab_bars(desktop);

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		this->dockTabBars[i] = new DockTabBar(dockTabBars[i]);
		wz_widget_set_height((wzWidget *)dockTabBars[i], 20);
	}
}

DesktopPrivate::~DesktopPrivate()
{
	for (size_t i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		delete dockTabBars[i];
	}

	for (size_t i = 0; i < children.size(); i++)
	{
		delete children[i];
	}

	children.clear();

	wz_widget_destroy((wzWidget *)desktop);
}

void DesktopPrivate::drawDockIcon(wzRect rect)
{
	renderer->draw_dock_icon(renderer, rect);
}

void DesktopPrivate::drawDockPreview(wzRect rect)
{
	renderer->draw_dock_preview(renderer, rect);
}

//------------------------------------------------------------------------------

Desktop::Desktop(wzRenderer *renderer)
{
	p = new DesktopPrivate(renderer);
}

Desktop::~Desktop()
{
	delete p;
}

void Desktop::setSize(int w, int h)
{
	wz_widget_set_size_args((wzWidget *)p->desktop, w, h);
}

void Desktop::mouseMove(int x, int y, int dx, int dy)
{
	wz_desktop_mouse_move(p->desktop, x, y, dx, dy);
}

void Desktop::mouseButtonDown(int button, int x, int y)
{
	wz_desktop_mouse_button_down(p->desktop, button, x, y);
}

void Desktop::mouseButtonUp(int button, int x, int y)
{
	wz_desktop_mouse_button_up(p->desktop, button, x, y);
}

void Desktop::mouseWheelMove(int x, int y)
{
	wz_desktop_mouse_wheel_move(p->desktop, x, y);
}

void Desktop::keyDown(wzKey key)
{
	wz_desktop_key_down(p->desktop, key);
}

void Desktop::keyUp(wzKey key)
{
	wz_desktop_key_up(p->desktop, key);
}

void Desktop::textInput(const char *text)
{
	wz_desktop_text_input(p->desktop, text);
}

void Desktop::setShowCursor(bool showCursor)
{
	p->showCursor = showCursor;
}

void Desktop::draw()
{
	wzRect rect = wz_widget_get_rect((const wzWidget *)p->desktop);
	p->renderer->begin_frame(p->renderer, rect.w, rect.h);
	wz_desktop_draw(p->desktop);
	p->renderer->end_frame(p->renderer);
}

bool Desktop::getShowCursor() const
{
	return p->showCursor;
}

wzCursor Desktop::getCursor() const
{
	return wz_desktop_get_cursor(p->desktop);
}

IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, List)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, TextEdit)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, Tabbed)
IMPLEMENT_CHILD_WIDGET_CREATE(Desktop, p, p, Window)

//------------------------------------------------------------------------------

DockTabBar::DockTabBar(wzTabBar *tabBar) : tabBar_(tabBar)
{
	renderer = DesktopPrivate::fromWidget((wzWidget *)tabBar_)->renderer;
	wz_widget_set_metadata((wzWidget *)tabBar_, this);

	decrementButton_.reset(new ButtonPrivate(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton_->getWidget(), 14);
	incrementButton_.reset(new ButtonPrivate(wz_tab_bar_get_increment_button(tabBar_), ">"));
	wz_widget_set_width((wzWidget *)incrementButton_->getWidget(), 14);
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
		WindowPrivate *window = (WindowPrivate *)wz_widget_get_metadata((wzWidget *)wz_desktop_get_dock_tab_window(wz_widget_get_desktop((wzWidget *)tabBar_), e.tabBar.tab));
		TabButton *tabButton = new TabButton(e.tabBar.tab);
		tabButton->setLabel(window->title);
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

GroupBoxPrivate::GroupBoxPrivate(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	frame = wz_frame_create(desktop);
	wzWidget *widget = (wzWidget *)frame;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_set_size_args(widget, 200, 200);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

GroupBoxPrivate::~GroupBoxPrivate()
{
	for (size_t i = 0; i < children.size(); i++)
	{
		delete children[i];
	}

	children.clear();
}

void GroupBoxPrivate::draw(wzRect clip)
{
	renderer->draw_group_box(renderer, clip, frame, label.c_str());
}

//------------------------------------------------------------------------------

std::string GroupBox::getLabel() const
{
	return ((GroupBoxPrivate *)p)->label;
}

GroupBox GroupBox::setLabel(const std::string &label)
{
	GroupBoxPrivate *gp = (GroupBoxPrivate *)p;
	gp->label = label;
	wz_widget_set_margin(p->getContentWidget(), p->renderer->measure_group_box_margin(p->renderer, gp->label.c_str()));
	return *this;
}

IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, List)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, TextEdit)
IMPLEMENT_CHILD_WIDGET_CREATE(GroupBox, (GroupBoxPrivate *)p, p, Tabbed)

//------------------------------------------------------------------------------

LabelPrivate::LabelPrivate(WidgetPrivate *parent) : r(255), g(255), b(255)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	label = wz_label_create(desktop);
	wzWidget *widget = (wzWidget *)label;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

void LabelPrivate::draw(wzRect clip)
{
	renderer->draw_label(renderer, clip, label, text.c_str(), r, g, b);
}

//------------------------------------------------------------------------------

Label Label::setText(const char *format, ...)
{
	static char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	LabelPrivate *lp = (LabelPrivate *)p;
	lp->text = buffer;
	wzSize size;
	p->renderer->measure_text(p->renderer, lp->text.c_str(), 0, &size.w, &size.h);
	wz_widget_set_size((wzWidget *)lp->label, size);

	return *this;
}

Label Label::setTextColor(uint8_t r, uint8_t g, uint8_t b)
{
	LabelPrivate *lp = (LabelPrivate *)p;
	lp->r = r;
	lp->g = g;
	lp->b = b;
	return *this;
}

//------------------------------------------------------------------------------

ListPrivate::ListPrivate(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	list = wz_list_create(desktop);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)list);
	initialize();
}

ListPrivate::ListPrivate(wzList *list)
{
	this->list = list;
	renderer = DesktopPrivate::fromWidget((wzWidget *)list)->renderer;
	initialize();
}

void ListPrivate::initialize()
{
	wz_list_set_item_height(list, itemHeight);
	wz_list_set_items_border_args(list, itemsMargin, itemsMargin, itemsMargin, itemsMargin);
	wz_widget_set_metadata((wzWidget *)list, this);
	wz_widget_set_draw_function((wzWidget *)list, DrawWidget);

	scroller.reset(new ScrollerPrivate(wz_list_get_scroller(list)));
	wz_widget_set_size_args(scroller->getWidget(), 16, 0);
}

void ListPrivate::draw(wzRect clip)
{
	renderer->draw_list(renderer, clip, list, items);
}

void ListPrivate::setItems(const char **items, int nItems)
{
	this->items = items;
	wz_list_set_num_items(list, nItems);
}

//------------------------------------------------------------------------------

List List::setItems(const char **items, int nItems)
{
	((ListPrivate *)p)->setItems(items, nItems);
	return *this;
}

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

RadioButtonPrivate::RadioButtonPrivate(WidgetPrivate *parent) : group(NULL)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	button = wz_button_create(desktop);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)button);
	wz_widget_set_metadata((wzWidget *)button, this);
	wz_widget_set_draw_function((wzWidget *)button, DrawWidget);
}

void RadioButtonPrivate::draw(wzRect clip)
{
	renderer->draw_radio_button(renderer, clip, button, label.c_str());
}

//------------------------------------------------------------------------------

std::string RadioButton::getLabel() const
{
	return ((RadioButtonPrivate *)p)->label;
}

RadioButton RadioButton::setLabel(const std::string &label)
{
	RadioButtonPrivate *rp = (RadioButtonPrivate *)p;
	rp->label = label;
	wz_widget_set_size((wzWidget *)rp->button, p->renderer->measure_radio_button(p->renderer, rp->label.c_str()));
	return *this;
}

RadioButton RadioButton::setGroup(RadioButtonGroup *group)
{
	RadioButtonPrivate *rp = (RadioButtonPrivate *)p;

	if (rp->group != NULL && rp->group != group)
	{
		// Switching groups: remove from the old group.
		wz_radio_button_group_remove_button(group->get(), rp->button);
	}

	rp->group = group;

	if (group != NULL)
	{
		wz_radio_button_group_add_button(group->get(), rp->button);
	}

	return *this;
}

//------------------------------------------------------------------------------

ScrollerPrivate::ScrollerPrivate(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	scroller = wz_scroller_create(desktop);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)scroller);
	initialize();
}

ScrollerPrivate::ScrollerPrivate(wzScroller *scroller)
{
	this->scroller = scroller;
	renderer = DesktopPrivate::fromWidget((wzWidget *)scroller)->renderer;
	initialize();
}

void ScrollerPrivate::draw(wzRect clip)
{
	renderer->draw_scroller(renderer, clip, scroller);
}

void ScrollerPrivate::initialize()
{
	wz_widget_set_metadata((wzWidget *)scroller, this);
	wz_widget_set_draw_function((wzWidget *)scroller, DrawWidget);
	wz_scroller_set_nub_size(scroller, 16);

	decrementButton.reset(new ButtonPrivate(wz_scroller_get_decrement_button(scroller), "-"));
	incrementButton.reset(new ButtonPrivate(wz_scroller_get_increment_button(scroller), "+"));

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wz_widget_set_size_args((wzWidget *)wz_scroller_get_decrement_button(scroller), 16, 16);
	wz_widget_set_size_args((wzWidget *)wz_scroller_get_increment_button(scroller), 16, 16);
}

//------------------------------------------------------------------------------

Scroller Scroller::setType(wzScrollerType type)
{
	wz_scroller_set_type(((ScrollerPrivate *)p)->scroller, type);
	return *this;
}

Scroller Scroller::setValue(int value)
{
	wz_scroller_set_value(((ScrollerPrivate *)p)->scroller, value);
	return *this;
}

Scroller Scroller::setStepValue(int stepValue)
{
	wz_scroller_set_step_value(((ScrollerPrivate *)p)->scroller, stepValue);
	return *this;
}

Scroller Scroller::setMaxValue(int maxValue)
{
	wz_scroller_set_max_value(((ScrollerPrivate *)p)->scroller, maxValue);
	return *this;
}

int Scroller::getValue() const
{
	return wz_scroller_get_value(((ScrollerPrivate *)p)->scroller);
}

//------------------------------------------------------------------------------

StackLayoutPrivate::StackLayoutPrivate(WidgetPrivate *parent) : layout(NULL), parent(parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	layout = wz_stack_layout_create(desktop);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)layout);
}

StackLayoutPrivate::~StackLayoutPrivate()
{
	for (size_t i = 0; i < children.size(); i++)
	{
		delete children[i];
	}

	children.clear();
}

//------------------------------------------------------------------------------

StackLayout StackLayout::setDirection(wzStackLayoutDirection direction)
{
	wz_stack_layout_set_direction(((StackLayoutPrivate *)p)->layout, direction);
	return *this;
}

StackLayout StackLayout::setSpacing(int spacing)
{
	wz_stack_layout_set_spacing(((StackLayoutPrivate *)p)->layout, spacing);
	return *this;
}

int StackLayout::getSpacing() const
{
	return wz_stack_layout_get_spacing(((StackLayoutPrivate *)p)->layout);
}

IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, List)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, TextEdit)
IMPLEMENT_CHILD_WIDGET_CREATE(StackLayout, (StackLayoutPrivate *)p, p, Tabbed)

//------------------------------------------------------------------------------

TabButton::TabButton(wzButton *button) : button_(button)
{
	renderer = DesktopPrivate::fromWidget((wzWidget *)button)->renderer;
	wz_widget_set_metadata((wzWidget *)button_, this);
	wz_widget_set_draw_function((wzWidget *)button_, DrawWidget);
}

void TabButton::draw(wzRect clip)
{
	renderer->draw_tab_button(renderer, clip, button_, label_.c_str());
}

void TabButton::setLabel(const std::string &label)
{
	label_ = label;

	// Calculate width based on label text plus padding.
	int width;
	renderer->measure_text(renderer, label_.c_str(), 0, &width, NULL);
	width += 16;
	wz_widget_set_width((wzWidget *)button_, width);
}

//------------------------------------------------------------------------------

TabBar::TabBar(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	tabBar_ = wz_tab_bar_create(desktop);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)tabBar_);
	initialize();
}

TabBar::TabBar(wzTabBar *tabBar) : tabBar_(tabBar)
{
	renderer = DesktopPrivate::fromWidget((wzWidget *)tabBar_)->renderer;
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

	decrementButton_.reset(new ButtonPrivate(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton_->getWidget(), 14);
	incrementButton_.reset(new ButtonPrivate(wz_tab_bar_get_increment_button(tabBar_), ">"));
	wz_widget_set_width((wzWidget *)incrementButton_->getWidget(), 14);
}

//------------------------------------------------------------------------------

TabPage::TabPage(wzWidget *widget)
{
	widget_ = widget;
	renderer = DesktopPrivate::fromWidget(widget)->renderer;
	desktop = wz_widget_get_desktop(widget);
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget_, DrawWidget);
}

void TabPage::draw(wzRect clip)
{
	renderer->draw_tab_page(renderer, clip, widget_);
}

//------------------------------------------------------------------------------

TabPrivate::TabPrivate(TabButton *button, TabPage *page)
{
	this->button = button;
	this->page = page;
}

TabPrivate::~TabPrivate()
{
	for (size_t i = 0; i < children.size(); i++)
	{
		delete children[i];
	}

	children.clear();
}

//------------------------------------------------------------------------------

Tab Tab::setLabel(const std::string &label)
{
	((TabPrivate *)p)->button->setLabel(label);
	return *this;
}

IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, List)
IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(Tab, (TabPrivate *)p, ((TabPrivate *)p)->page, TextEdit)

//------------------------------------------------------------------------------

TabbedPrivate::TabbedPrivate(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	tabbed = wz_tabbed_create(desktop);
	wz_widget_set_metadata((wzWidget *)tabbed, this);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)tabbed);
	tabBar.reset(new TabBar(wz_tabbed_get_tab_bar(tabbed)));
	wz_widget_set_rect_args(tabBar->getWidget(), 0, 0, 0, 20);
}

TabbedPrivate::~TabbedPrivate()
{
	for (size_t i = 0; i < tabPages.size(); i++)
	{
		delete tabPages[i];
	}

	tabPages.clear();

	for (size_t i = 0; i < tabPages.size(); i++)
	{
		delete tabs[i];
	}

	tabs.clear();
}

void TabbedPrivate::draw(wzRect clip)
{
}

//------------------------------------------------------------------------------

Tab Tabbed::createTab()
{
	wzButton *button;
	wzWidget *widget;
	TabbedPrivate *tp = (TabbedPrivate *)p;
	wz_tabbed_add_tab(tp->tabbed, &button, &widget);
	TabButton *tabButton = tp->tabBar->addTab(button);
	
	TabPage *tabPage = new TabPage(widget);
	tp->tabPages.push_back(tabPage);

	TabPrivate *tabPrivate = new TabPrivate(tabButton, tabPage);
	tp->tabs.push_back(tabPrivate);

	Tab tab;
	tab.p = tabPrivate;
	return tab;
}

//------------------------------------------------------------------------------

TextEditPrivate::TextEditPrivate(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	textEdit = wz_text_edit_create(desktop, 256);
	wz_text_edit_set_border_args(textEdit, borderSize, borderSize, borderSize, borderSize);
	wzWidget *widget = (wzWidget *)textEdit;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

void TextEditPrivate::draw(wzRect clip)
{
	renderer->draw_text_edit(renderer, clip, textEdit, DesktopPrivate::fromWidget(getWidget())->showCursor);
}

//------------------------------------------------------------------------------

TextEdit TextEdit::setText(const std::string &text)
{
	TextEditPrivate *tp = (TextEditPrivate *)p;

	wz_text_edit_set_text(tp->textEdit, text.c_str());

	int h;
	p->renderer->measure_text(p->renderer, text.c_str(), 0, NULL, &h);
	wz_widget_set_size_args((wzWidget *)tp->textEdit, 100, h + tp->borderSize * 2);

	return *this;
}

//------------------------------------------------------------------------------

WindowPrivate::WindowPrivate(WidgetPrivate *parent)
{
	renderer = parent->renderer;
	desktop = parent->desktop;
	window = wz_window_create(desktop);
	wzWidget *widget = (wzWidget *)window;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_window_set_border_size(window, 4);
	wz_widget_add_child_widget(parent->getWidget(), widget);
}

WindowPrivate::~WindowPrivate()
{
	for (size_t i = 0; i < children.size(); i++)
	{
		delete children[i];
	}

	children.clear();
}

void WindowPrivate::draw(wzRect clip)
{
	renderer->draw_window(renderer, clip, window, title.c_str());
}

//------------------------------------------------------------------------------

Window Window::setTitle(const std::string &title)
{
	WindowPrivate *wp = (WindowPrivate *)p;
	wp->title = title;

	// Calculate header height based on label text plus padding.
	wzSize size;
	p->renderer->measure_text(p->renderer, wp->title.c_str(), 0, &size.w, &size.h);
	wz_window_set_header_height(wp->window, size.h + 6);

	return *this;
}

std::string Window::getTitle() const
{
	return ((WindowPrivate *)p)->title;
}

IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, Button)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, Checkbox)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, Combo)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, GroupBox)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, Label)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, List)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, RadioButton)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, Scroller)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, StackLayout)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, TextEdit)
IMPLEMENT_CHILD_WIDGET_CREATE(Window, (WindowPrivate *)p, p, Tabbed)

} // namespace wz
