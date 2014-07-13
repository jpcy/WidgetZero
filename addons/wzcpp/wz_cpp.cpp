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

namespace wz {

class DockTabBar;
struct ListPrivate;

struct WidgetPrivate
{
	virtual ~WidgetPrivate();
	virtual const wzWidget *getWidget() const { return NULL; }
	virtual wzWidget *getWidget() { return NULL; }
	virtual wzSize measure() { wzSize s; s.w = s.h = 0; return s; }
	virtual void draw(wzRect clip) {}
	virtual void handleEvent(wzEvent *e) {}

	void add(Widget *widget);
	
	wzRenderer *renderer;
	std::vector<Widget *> children;
};

struct ButtonPrivate : public WidgetPrivate
{
	ButtonPrivate(wzRenderer *renderer);
	~ButtonPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	enum DrawStyle
	{
		Normal,
		Tab
	};

	wzButton *button;
	DrawStyle drawStyle;
	std::string label;
};

struct CheckboxPrivate : public WidgetPrivate
{
	CheckboxPrivate(wzRenderer *renderer);
	~CheckboxPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzButton *button;
	std::string label;
};

struct ComboPrivate : public WidgetPrivate
{
	ComboPrivate(wzRenderer *renderer);
	~ComboPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)combo; }
	virtual wzWidget *getWidget() { return (wzWidget *)combo; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzCombo *combo;
	const char **items;
	std::auto_ptr<List> list;
};

struct DesktopPrivate : public WidgetPrivate
{
	DesktopPrivate(wzRenderer *renderer);
	~DesktopPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)desktop; }
	virtual wzWidget *getWidget() { return (wzWidget *)desktop; }
	void draw();
	void drawDockIcon(wzRect rect);
	void drawDockPreview(wzRect rect);

	wzDesktop *desktop;
	wzRenderer *renderer;
	DockTabBar *dockTabBars[WZ_NUM_DOCK_POSITIONS];
	bool showCursor;
};

struct GroupBoxPrivate : public WidgetPrivate
{
	GroupBoxPrivate(wzRenderer *renderer);
	~GroupBoxPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)frame; }
	virtual wzWidget *getWidget() { return (wzWidget *)frame; }
	virtual void draw(wzRect clip);

	void refreshMargin();

	wzFrame *frame;
	std::string label;
};

struct LabelPrivate : public WidgetPrivate
{
	LabelPrivate(wzRenderer *renderer);
	~LabelPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)label; }
	virtual wzWidget *getWidget() { return (wzWidget *)label; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzLabel *label;
	std::string text;
	uint8_t r, g, b;
};

struct ListPrivate : public WidgetPrivate
{
	ListPrivate(wzRenderer *renderer);
	~ListPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)list; }
	virtual wzWidget *getWidget() { return (wzWidget *)list; }
	virtual void draw(wzRect clip);

	void setItems(const char **items, int nItems);

	wzList *list;
	const char **items;
	std::auto_ptr<Scroller> scroller;

	static const int itemsMargin = 2;
	static const int itemHeight = 18;
	static const int itemLeftPadding = 4;
};

struct RadioButtonPrivate : public WidgetPrivate
{
	RadioButtonPrivate(wzRenderer *renderer);
	~RadioButtonPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzButton *button;
	std::string label;
	RadioButtonGroup *group;
};

struct ScrollerPrivate : public WidgetPrivate
{
	ScrollerPrivate(wzRenderer *renderer);
	~ScrollerPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)scroller; }
	virtual wzWidget *getWidget() { return (wzWidget *)scroller; }
	virtual void draw(wzRect clip);

	wzScroller *scroller;
	std::auto_ptr<Button> decrementButton;
	std::auto_ptr<Button> incrementButton;
};

struct StackLayoutPrivate : public WidgetPrivate
{
	StackLayoutPrivate();
	~StackLayoutPrivate();
	virtual const wzWidget *getWidget() const { return (wzWidget *)layout; }
	virtual wzWidget *getWidget() { return (wzWidget *)layout; }
	
	wzStackLayout *layout;
};

class TabBar : public WidgetPrivate
{
public:
	TabBar(wzRenderer *renderer);
	~TabBar();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabBar_; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabBar_; }
	virtual void draw(wzRect clip);
	Button *createTab();

protected:
	wzTabBar *tabBar_;
	std::auto_ptr<Button> decrementButton_;
	std::auto_ptr<Button> incrementButton_;
};

class DockTabBar : public TabBar
{
public:
	DockTabBar(wzRenderer *renderer);
	virtual void handleEvent(wzEvent *e);
};

class TabPage : public WidgetPrivate
{
public:
	TabPage(wzRenderer *renderer);
	virtual const wzWidget *getWidget() const { return widget_; }
	virtual wzWidget *getWidget() { return widget_; }
	virtual void draw(wzRect clip);

private:
	wzWidget *widget_;
};

// Wraps tab button and page.
struct TabPrivate
{
	TabPrivate();
	~TabPrivate();

	Button *button;
	TabPage *page;
	std::vector<Widget *> children;
};

struct TabbedPrivate : public WidgetPrivate
{
	TabbedPrivate(wzRenderer *renderer);
	~TabbedPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabbed; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabbed; }
	virtual void draw(wzRect clip);

	wzTabbed *tabbed;
	std::auto_ptr<TabBar> tabBar;
	std::vector<TabPage *> tabPages;
};

struct TextEditPrivate : public WidgetPrivate
{
	TextEditPrivate(wzRenderer *renderer);
	~TextEditPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)textEdit; }
	virtual wzWidget *getWidget() { return (wzWidget *)textEdit; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzTextEdit *textEdit;
};

struct WindowPrivate : public WidgetPrivate
{
	WindowPrivate(wzRenderer *renderer);
	virtual ~WindowPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)window; }
	virtual wzWidget *getWidget() { return (wzWidget *)window; }
	virtual void draw(wzRect clip);

	void refreshHeaderHeight();

	wzWindow *window;
	std::string title;
};

//------------------------------------------------------------------------------

static void DrawWidget(wzWidget *widget, wzRect clip)
{
	((WidgetPrivate *)wz_widget_get_metadata(widget))->draw(clip);
}

static wzSize MeasureWidget(wzWidget *widget)
{
	return ((WidgetPrivate *)wz_widget_get_metadata(widget))->measure();
}

static void HandleEvent(wzEvent *e)
{
	void *metadata = wz_widget_get_metadata(e->base.widget);

	if (metadata)
	{
		((WidgetPrivate *)metadata)->handleEvent(e);
	}
}

//------------------------------------------------------------------------------

WidgetPrivate::~WidgetPrivate()
{
	for (size_t i = 0; i < children.size(); i++)
	{
		delete children[i];
	}

	children.clear();
}

void WidgetPrivate::add(Widget *widget)
{
	wz_widget_add_child_widget(getWidget(), widget->p->getWidget());
	children.push_back(widget);
}

//------------------------------------------------------------------------------

Widget::~Widget()
{
}

wzRect Widget::getRect() const
{
	return wz_widget_get_absolute_rect(p->getWidget());
}

Widget *Widget::setPosition(int x, int y)
{ 
	wz_widget_set_position_args(p->getWidget(), x, y);
	return this;
}

Widget *Widget::setSize(int w, int h)
{
	wz_widget_set_size_args(p->getWidget(), w, h);
	return this;
}

Widget *Widget::setRect(int x, int y, int w, int h)
{
	wz_widget_set_rect_args(p->getWidget(), x, y, w, h);
	return this;
}

Widget *Widget::setStretch(int stretch)
{
	wz_widget_set_stretch(p->getWidget(), stretch);
	return this;
}

Widget *Widget::setAlign(int align)
{
	wz_widget_set_align(p->getWidget(), align);
	return this;
}

Widget *Widget::setMargin(int margin)
{
	wz_widget_set_margin_args(p->getWidget(), margin, margin, margin, margin);
	return this;
}

Widget *Widget::setMargin(int top, int right, int bottom, int left)
{
	wz_widget_set_margin_args(p->getWidget(), top, right, bottom, left);
	return this;
}

Widget *Widget::setMargin(wzBorder margin)
{
	wz_widget_set_margin(p->getWidget(), margin);
	return this;
}

//------------------------------------------------------------------------------

ButtonPrivate::ButtonPrivate(wzRenderer *renderer) : drawStyle(Normal)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	button = wz_button_create();
	wz_widget_set_metadata((wzWidget *)button, this);
	wz_widget_set_draw_callback((wzWidget *)button, DrawWidget);
	wz_widget_set_measure_callback((wzWidget *)button, MeasureWidget);
}

ButtonPrivate::~ButtonPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)button))
	{
		wz_widget_destroy((wzWidget *)button);
	}
}

wzSize ButtonPrivate::measure()
{
	return renderer->measure_button(renderer, label.c_str());
}

void ButtonPrivate::draw(wzRect clip)
{
	if (drawStyle == Normal)
	{
		renderer->draw_button(renderer, clip, button, label.c_str());
	}
	else if (drawStyle == Tab)
	{
		renderer->draw_tab_button(renderer, clip, button, label.c_str());
	}
}

//------------------------------------------------------------------------------

Button::Button(wzRenderer *renderer)
{
	p = new ButtonPrivate(renderer);
}

Button::Button(wzRenderer *renderer, const std::string &label)
{
	p = new ButtonPrivate(renderer);
	setLabel(label);
}

Button::~Button()
{
	delete p;
}

std::string Button::getLabel() const
{
	return ((ButtonPrivate *)p)->label;
}

Button *Button::setLabel(const std::string &label)
{
	ButtonPrivate *bp = (ButtonPrivate *)p;
	bp->label = label;
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

//------------------------------------------------------------------------------

CheckboxPrivate::CheckboxPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	button = wz_button_create();
	wzWidget *widget = (wzWidget *)button;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_callback(widget, DrawWidget);
	wz_widget_set_measure_callback(widget, MeasureWidget);
	wz_button_set_set_behavior(button, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
}

CheckboxPrivate::~CheckboxPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)button))
	{
		wz_widget_destroy((wzWidget *)button);
	}
}

wzSize CheckboxPrivate::measure()
{
	return renderer->measure_checkbox(renderer, label.c_str());
}

void CheckboxPrivate::draw(wzRect clip)
{
	renderer->draw_checkbox(renderer, clip, button, label.c_str());
}

//------------------------------------------------------------------------------

Checkbox::Checkbox(wzRenderer *renderer)
{
	p = new CheckboxPrivate(renderer);
}

Checkbox::Checkbox(wzRenderer *renderer, const std::string &label)
{
	p = new CheckboxPrivate(renderer);
	setLabel(label);
}

Checkbox::~Checkbox()
{
	delete p;
}

std::string Checkbox::getLabel() const
{
	return ((CheckboxPrivate *)p)->label;
}

Checkbox *Checkbox::setLabel(const std::string &label)
{
	CheckboxPrivate *cp = (CheckboxPrivate *)p;
	cp->label = label;
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

//------------------------------------------------------------------------------

ComboPrivate::ComboPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	combo = wz_combo_create();
	wzWidget *widget = (wzWidget *)combo;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_callback(widget, DrawWidget);
	wz_widget_set_measure_callback(widget, MeasureWidget);

	list.reset(new List(renderer));
	wz_combo_set_list(combo, (wzList *)list->p->getWidget());
}

ComboPrivate::~ComboPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)combo))
	{
		wz_widget_destroy((wzWidget *)combo);
	}
}

wzSize ComboPrivate::measure()
{
	return renderer->measure_combo(renderer, items, wz_list_get_num_items(wz_combo_get_list(combo)));
}

void ComboPrivate::draw(wzRect clip)
{
	int itemIndex = wz_list_get_selected_item((const wzList *)list->p->getWidget());
	renderer->draw_combo(renderer, clip, combo, itemIndex >= 0 ? items[itemIndex] : NULL);
}

//------------------------------------------------------------------------------

Combo::Combo(wzRenderer *renderer)
{
	p = new ComboPrivate(renderer);
}

Combo::~Combo()
{
	delete p;
}

Combo *Combo::setItems(const char **items, int nItems)
{
	ComboPrivate *cp = (ComboPrivate *)p;
	cp->items = items;
	cp->list->setItems(items, nItems);
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

//------------------------------------------------------------------------------

static void MeasureText(struct wzDesktop *desktop, const char *text, int n, int *width, int *height)
{
	wzRenderer *renderer = ((DesktopPrivate *)wz_widget_get_metadata((wzWidget *)desktop))->renderer;
	renderer->measure_text(renderer, text, n, width, height);
}

static int TextGetPixelDelta(struct wzDesktop *desktop, const char *text, int index)
{
	wzRenderer *renderer = ((DesktopPrivate *)wz_widget_get_metadata((wzWidget *)desktop))->renderer;
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
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	desktop = wz_desktop_create();
	wz_widget_set_metadata((wzWidget *)desktop, this);
	wz_desktop_set_event_callback(desktop, HandleEvent);
	wz_desktop_set_measure_text_callback(desktop, MeasureText);
	wz_desktop_set_text_get_pixel_delta_callback(desktop, TextGetPixelDelta);
	wz_desktop_set_draw_dock_icon_callback(desktop, DrawDockIcon, this);
	wz_desktop_set_draw_dock_preview_callback(desktop, DrawDockPreview, this);

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		dockTabBars[i] = new DockTabBar(renderer);
		wz_desktop_set_dock_tab_bar(desktop, (wzDockPosition)i, (wzTabBar *)dockTabBars[i]->getWidget());
		wz_widget_set_height(dockTabBars[i]->getWidget(), 20);
	}
}

DesktopPrivate::~DesktopPrivate()
{
	for (size_t i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		delete dockTabBars[i];
	}

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

void Desktop::beginFrame()
{
	wzRect rect = wz_widget_get_rect((const wzWidget *)p->desktop);
	p->renderer->begin_frame(p->renderer, rect.w, rect.h);
}

void Desktop::drawFrame()
{	
	wz_desktop_draw(p->desktop);
}

void Desktop::endFrame()
{
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

Widget *Desktop::add(Widget *widget)
{
	p->add(widget);
	return widget;
}

//------------------------------------------------------------------------------

DockTabBar::DockTabBar(wzRenderer *renderer) : TabBar(renderer)
{
}

void DockTabBar::handleEvent(wzEvent *e)
{
	if (e->base.type == WZ_EVENT_CREATE_WIDGET)
	{
		// Create a new tab.
		WindowPrivate *window = (WindowPrivate *)wz_widget_get_metadata(e->create.extra);
		Button *tab = new Button(renderer);
		((ButtonPrivate *)tab->p)->drawStyle = ButtonPrivate::Tab;
		tab->setLabel(window->title);
		children.push_back(tab);
		e->create.widget = tab->p->getWidget();
	}
	else if (e->base.type == WZ_EVENT_DESTROY_WIDGET)
	{
		// Remove the corresponding Button instance.
		for (size_t i = 0; i < children.size(); i++)
		{
			if (children[i]->p->getWidget() == (wzWidget *)e->tabBar.tab)
			{
				Widget *tab = children[i];
				children.erase(children.begin() + i);
				delete tab;
				return;
			}
		}
	}
	else if (e->base.type == WZ_EVENT_TAB_BAR_TAB_ADDED)
	{
		ButtonPrivate *tab = (ButtonPrivate *)wz_widget_get_metadata((wzWidget *)e->tabBar.tab);
		wz_widget_resize_to_measured(tab->getWidget());
	}
}

//------------------------------------------------------------------------------

GroupBoxPrivate::GroupBoxPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	frame = wz_frame_create();
	wzWidget *widget = (wzWidget *)frame;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_callback(widget, DrawWidget);
	wz_widget_set_size_args(widget, 200, 200);
	refreshMargin();
}

GroupBoxPrivate::~GroupBoxPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)frame))
	{
		wz_widget_destroy((wzWidget *)frame);
	}
}

void GroupBoxPrivate::refreshMargin()
{
	wz_widget_set_margin(wz_widget_get_content_widget(getWidget()), renderer->measure_group_box_margin(renderer, label.c_str()));
}

void GroupBoxPrivate::draw(wzRect clip)
{
	renderer->draw_group_box(renderer, clip, frame, label.c_str());
}

//------------------------------------------------------------------------------

GroupBox::GroupBox(wzRenderer *renderer)
{
	p = new GroupBoxPrivate(renderer);
}

GroupBox::GroupBox(wzRenderer *renderer, const std::string &label)
{
	p = new GroupBoxPrivate(renderer);
	setLabel(label);
}

GroupBox::~GroupBox()
{
	delete p;
}

std::string GroupBox::getLabel() const
{
	return ((GroupBoxPrivate *)p)->label;
}

GroupBox *GroupBox::setLabel(const std::string &label)
{
	GroupBoxPrivate *gp = (GroupBoxPrivate *)p;
	gp->label = label;
	gp->refreshMargin();
	return this;
}

Widget *GroupBox::add(Widget *widget)
{
	p->add(widget);
	return widget;
}

//------------------------------------------------------------------------------

LabelPrivate::LabelPrivate(wzRenderer *renderer) : r(255), g(255), b(255)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	label = wz_label_create();
	wzWidget *widget = (wzWidget *)label;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_callback(widget, DrawWidget);
	wz_widget_set_measure_callback(widget, MeasureWidget);
}

LabelPrivate::~LabelPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)label))
	{
		wz_widget_destroy((wzWidget *)label);
	}
}

wzSize LabelPrivate::measure()
{
	return renderer->measure_label(renderer, text.c_str());
}

void LabelPrivate::draw(wzRect clip)
{
	renderer->draw_label(renderer, clip, label, text.c_str(), r, g, b);
}

//------------------------------------------------------------------------------

Label::Label(wzRenderer *renderer)
{
	p = new LabelPrivate(renderer);
}

Label::Label(wzRenderer *renderer, const std::string &text)
{
	p = new LabelPrivate(renderer);
	setText(text.c_str());
}

Label::~Label()
{
	delete p;
}

Label *Label::setText(const char *format, ...)
{
	static char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	LabelPrivate *lp = (LabelPrivate *)p;
	lp->text = buffer;
	wz_widget_resize_to_measured(p->getWidget());

	return this;
}

Label *Label::setTextColor(uint8_t r, uint8_t g, uint8_t b)
{
	LabelPrivate *lp = (LabelPrivate *)p;
	lp->r = r;
	lp->g = g;
	lp->b = b;
	return this;
}

//------------------------------------------------------------------------------

ListPrivate::ListPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	scroller.reset(new Scroller(renderer));
	wz_widget_set_size_args(scroller->p->getWidget(), 16, 0);

	list = wz_list_create((wzScroller *)scroller->p->getWidget());
	wz_list_set_item_height(list, itemHeight);
	wz_list_set_items_border_args(list, itemsMargin, itemsMargin, itemsMargin, itemsMargin);
	wz_widget_set_metadata((wzWidget *)list, this);
	wz_widget_set_draw_callback((wzWidget *)list, DrawWidget);
}

ListPrivate::~ListPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)list))
	{
		wz_widget_destroy((wzWidget *)list);
	}
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

List::List(wzRenderer *renderer)
{
	p = new ListPrivate(renderer);
}

List::~List()
{
	delete p;
}

List *List::setItems(const char **items, int nItems)
{
	((ListPrivate *)p)->setItems(items, nItems);
	return this;
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

RadioButtonPrivate::RadioButtonPrivate(wzRenderer *renderer) : group(NULL)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	button = wz_button_create();
	wz_widget_set_metadata((wzWidget *)button, this);
	wz_widget_set_draw_callback((wzWidget *)button, DrawWidget);
	wz_widget_set_measure_callback((wzWidget *)button, MeasureWidget);
}

RadioButtonPrivate::~RadioButtonPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)button))
	{
		wz_widget_destroy((wzWidget *)button);
	}
}

wzSize RadioButtonPrivate::measure()
{
	return renderer->measure_radio_button(renderer, label.c_str());
}

void RadioButtonPrivate::draw(wzRect clip)
{
	renderer->draw_radio_button(renderer, clip, button, label.c_str());
}

//------------------------------------------------------------------------------

RadioButton::RadioButton(wzRenderer *renderer)
{
	p = new RadioButtonPrivate(renderer);
}

RadioButton::RadioButton(wzRenderer *renderer, const std::string &label)
{
	p = new RadioButtonPrivate(renderer);
	setLabel(label);
}

RadioButton::~RadioButton()
{
	delete p;
}

std::string RadioButton::getLabel() const
{
	return ((RadioButtonPrivate *)p)->label;
}

RadioButton *RadioButton::setLabel(const std::string &label)
{
	RadioButtonPrivate *rp = (RadioButtonPrivate *)p;
	rp->label = label;
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

RadioButton *RadioButton::setGroup(RadioButtonGroup *group)
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

	return this;
}

//------------------------------------------------------------------------------

ScrollerPrivate::ScrollerPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	decrementButton.reset(new Button(renderer, "-"));
	incrementButton.reset(new Button(renderer, "+"));

	scroller = wz_scroller_create((wzButton *)decrementButton->p->getWidget(), (wzButton *)incrementButton->p->getWidget());
	wz_widget_set_metadata((wzWidget *)scroller, this);
	wz_widget_set_draw_callback((wzWidget *)scroller, DrawWidget);
	wz_scroller_set_nub_size(scroller, 16);

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wz_widget_set_size_args(decrementButton->p->getWidget(), 16, 16);
	wz_widget_set_size_args(incrementButton->p->getWidget(), 16, 16);
}

ScrollerPrivate::~ScrollerPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)scroller))
	{
		wz_widget_destroy((wzWidget *)scroller);
	}
}

void ScrollerPrivate::draw(wzRect clip)
{
	renderer->draw_scroller(renderer, clip, scroller);
}

//------------------------------------------------------------------------------

Scroller::Scroller(wzRenderer *renderer)
{
	p = new ScrollerPrivate(renderer);
}

Scroller::~Scroller()
{
	delete p;
}

Scroller *Scroller::setType(wzScrollerType type)
{
	wz_scroller_set_type(((ScrollerPrivate *)p)->scroller, type);
	return this;
}

Scroller *Scroller::setValue(int value)
{
	wz_scroller_set_value(((ScrollerPrivate *)p)->scroller, value);
	return this;
}

Scroller *Scroller::setStepValue(int stepValue)
{
	wz_scroller_set_step_value(((ScrollerPrivate *)p)->scroller, stepValue);
	return this;
}

Scroller *Scroller::setMaxValue(int maxValue)
{
	wz_scroller_set_max_value(((ScrollerPrivate *)p)->scroller, maxValue);
	return this;
}

int Scroller::getValue() const
{
	return wz_scroller_get_value(((ScrollerPrivate *)p)->scroller);
}

//------------------------------------------------------------------------------

StackLayoutPrivate::StackLayoutPrivate()
{
	layout = wz_stack_layout_create();
}

StackLayoutPrivate::~StackLayoutPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)layout))
	{
		wz_widget_destroy((wzWidget *)layout);
	}
}

//------------------------------------------------------------------------------

StackLayout::StackLayout()
{
	p = new StackLayoutPrivate();
}

StackLayout::StackLayout(wzStackLayoutDirection direction)
{
	p = new StackLayoutPrivate();
	setDirection(direction);
}

StackLayout::~StackLayout()
{
	delete p;
}

StackLayout *StackLayout::setDirection(wzStackLayoutDirection direction)
{
	wz_stack_layout_set_direction(((StackLayoutPrivate *)p)->layout, direction);
	return this;
}

StackLayout *StackLayout::setSpacing(int spacing)
{
	wz_stack_layout_set_spacing(((StackLayoutPrivate *)p)->layout, spacing);
	return this;
}

int StackLayout::getSpacing() const
{
	return wz_stack_layout_get_spacing(((StackLayoutPrivate *)p)->layout);
}

Widget *StackLayout::add(Widget *widget)
{
	p->add(widget);
	return widget;
}

//------------------------------------------------------------------------------

TabBar::TabBar(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	decrementButton_.reset(new Button(renderer, "<"));
	wz_widget_set_width(decrementButton_->p->getWidget(), 14);

	incrementButton_.reset(new Button(renderer, ">"));
	wz_widget_set_width(incrementButton_->p->getWidget(), 14);

	tabBar_ = wz_tab_bar_create((wzButton *)decrementButton_->p->getWidget(), (wzButton *)incrementButton_->p->getWidget());
	wz_widget_set_metadata((wzWidget *)tabBar_, this);
}

TabBar::~TabBar()
{
	if (!wz_widget_get_desktop((wzWidget *)tabBar_))
	{
		wz_widget_destroy((wzWidget *)tabBar_);
	}
}

void TabBar::draw(wzRect clip)
{
}

Button *TabBar::createTab()
{
	Button *tab = new Button(renderer);
	((ButtonPrivate *)tab->p)->drawStyle = ButtonPrivate::Tab;
	children.push_back(tab);
	return tab;
}

//------------------------------------------------------------------------------

TabPage::TabPage(wzRenderer *renderer) 
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	widget_ = wz_tab_page_create();
	wz_widget_set_metadata(widget_, this);
	wz_widget_set_draw_callback(widget_, DrawWidget);
}

void TabPage::draw(wzRect clip)
{
	renderer->draw_tab_page(renderer, clip, widget_);
}

//------------------------------------------------------------------------------

TabPrivate::TabPrivate()
{
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

Tab::Tab()
{
	p = new TabPrivate();
}

Tab::~Tab()
{
	delete p;
}

Tab *Tab::setLabel(const std::string &label)
{
	p->button->setLabel(label);
	return this;
}

Widget *Tab::add(Widget *widget)
{
	wz_widget_add_child_widget(p->page->getWidget(), widget->p->getWidget());
	p->children.push_back(widget);
	return widget;
}

//------------------------------------------------------------------------------

TabbedPrivate::TabbedPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	tabBar.reset(new TabBar(renderer));
	wz_widget_set_rect_args(tabBar->getWidget(), 0, 0, 0, 20);

	tabbed = wz_tabbed_create((wzTabBar *)tabBar->getWidget());
	wz_widget_set_metadata((wzWidget *)tabbed, this);
}

TabbedPrivate::~TabbedPrivate()
{
	for (size_t i = 0; i < tabPages.size(); i++)
	{
		delete tabPages[i];
	}

	tabPages.clear();

	if (!wz_widget_get_desktop((wzWidget *)tabbed))
	{
		wz_widget_destroy((wzWidget *)tabbed);
	}
}

void TabbedPrivate::draw(wzRect clip)
{
}

//------------------------------------------------------------------------------

Tabbed::Tabbed(wzRenderer *renderer)
{
	p = new TabbedPrivate(renderer);
}

Tabbed::~Tabbed()
{
	delete p;
}

Tab *Tabbed::addTab(Tab *tab)
{
	TabbedPrivate *tp = (TabbedPrivate *)p;
	
	Button *tabButton = tp->tabBar->createTab();

	TabPage *tabPage = new TabPage(p->renderer);
	tp->tabPages.push_back(tabPage);

	wz_tabbed_add_tab(tp->tabbed, (wzButton *)tabButton->p->getWidget(), tabPage->getWidget());
	tab->p->button = tabButton;
	tab->p->page = tabPage;
	return tab;
}

//------------------------------------------------------------------------------

TextEditPrivate::TextEditPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	textEdit = wz_text_edit_create(256);
	wz_text_edit_set_border(textEdit, renderer->get_text_edit_border(renderer));
	wzWidget *widget = (wzWidget *)textEdit;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_callback(widget, DrawWidget);
	wz_widget_set_measure_callback(widget, MeasureWidget);
}

TextEditPrivate::~TextEditPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)textEdit))
	{
		wz_widget_destroy((wzWidget *)textEdit);
	}
}

wzSize TextEditPrivate::measure()
{
	return renderer->measure_text_edit(renderer, wz_text_edit_get_border(textEdit), wz_text_edit_get_text(textEdit));
}

void TextEditPrivate::draw(wzRect clip)
{
	DesktopPrivate *desktop = (DesktopPrivate *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(getWidget()));
	renderer->draw_text_edit(renderer, clip, textEdit, desktop->showCursor);
}

//------------------------------------------------------------------------------

TextEdit::TextEdit(wzRenderer *renderer)
{
	p = new TextEditPrivate(renderer);
}

TextEdit::TextEdit(wzRenderer *renderer, const std::string &text)
{
	p = new TextEditPrivate(renderer);
	setText(text);
}

TextEdit::~TextEdit()
{
	delete p;
}

TextEdit *TextEdit::setText(const std::string &text)
{
	TextEditPrivate *tp = (TextEditPrivate *)p;
	wz_text_edit_set_text(tp->textEdit, text.c_str());
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

//------------------------------------------------------------------------------

WindowPrivate::WindowPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	window = wz_window_create();
	wzWidget *widget = (wzWidget *)window;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_callback(widget, DrawWidget);
	wz_window_set_border_size(window, 4);
	refreshHeaderHeight();
}

WindowPrivate::~WindowPrivate()
{
	if (!wz_widget_get_desktop((wzWidget *)window))
	{
		wz_widget_destroy((wzWidget *)window);
	}
}

void WindowPrivate::draw(wzRect clip)
{
	renderer->draw_window(renderer, clip, window, title.c_str());
}

void WindowPrivate::refreshHeaderHeight()
{
	wz_window_set_header_height(window, renderer->measure_window_header_height(renderer, title.c_str()));
}

//------------------------------------------------------------------------------

Window::Window(wzRenderer *renderer)
{
	p = new WindowPrivate(renderer);
}

Window::Window(wzRenderer *renderer, const std::string &title)
{
	p = new WindowPrivate(renderer);
	setTitle(title);
}

Window::~Window()
{
	delete p;
}

std::string Window::getTitle() const
{
	return ((WindowPrivate *)p)->title;
}

Window *Window::setTitle(const std::string &title)
{
	WindowPrivate *wp = (WindowPrivate *)p;
	wp->title = title;
	wp->refreshHeaderHeight();
	return this;
}

Widget *Window::add(Widget *widget)
{
	p->add(widget);
	return widget;
}

} // namespace wz
