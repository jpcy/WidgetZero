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
#include <wz_cpp_p.h>

namespace wz {

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
		WidgetPrivate *wp = (WidgetPrivate *)metadata;

		for (size_t i = 0; i < wp->eventHandlers.size(); i++)
		{
			if (wp->eventHandlers[i]->eventType == e->base.type)
			{
				wp->eventHandlers[i]->call(e);
			}
		}

		wp->handleEvent(e);
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

	for (size_t i = 0; i < eventHandlers.size(); i++)
	{
		delete eventHandlers[i];
	}

	eventHandlers.clear();
}

void WidgetPrivate::add(Widget *widget)
{
	wz_widget_add_child_widget(getWidget(), widget->p->getWidget());
	children.push_back(widget);
}

void WidgetPrivate::remove(Widget *widget)
{
	size_t i;

	for (i = 0; i < children.size(); i++)
	{
		if (children[i] == widget)
			break;
	}

	if (i < children.size())
	{
		wz_widget_remove_child_widget(getWidget(), widget->p->getWidget());
		children.erase(children.begin() + i);
	}
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

Widget *Widget::setWidth(int w)
{
	wz_widget_set_width(p->getWidget(), w);
	return this;
}

Widget *Widget::setHeight(int h)
{
	wz_widget_set_height(p->getWidget(), h);
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

Widget *Widget::setFontFace(const std::string &fontFace)
{
	p->fontFace = fontFace;
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

Widget *Widget::setFontSize(float fontSize)
{
	p->fontSize = fontSize;
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

Widget *Widget::setFont(const std::string &fontFace, float fontSize)
{
	p->fontFace = fontFace;
	p->fontSize = fontSize;
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

Widget *Widget::setVisible(bool visible)
{
	wz_widget_set_visible(p->getWidget(), visible);
	return this;
}

Widget *Widget::addEventHandler(IEventHandler *eventHandler)
{
	p->eventHandlers.push_back(eventHandler);
	return this;
}

//------------------------------------------------------------------------------

ButtonPrivate::ButtonPrivate(wzRenderer *renderer) : drawStyle(Normal)
{
	WZ_ASSERT(renderer);
	padding.left = padding.right = 8;
	padding.top = padding.bottom = 4;
	this->renderer = renderer;
	button = wz_button_create();
	wz_widget_set_metadata((wzWidget *)button, this);
	wz_widget_set_draw_callback((wzWidget *)button, DrawWidget);
	wz_widget_set_measure_callback((wzWidget *)button, MeasureWidget);
}

ButtonPrivate::~ButtonPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)button))
	{
		wz_widget_destroy((wzWidget *)button);
	}
}

wzSize ButtonPrivate::measure()
{
	return renderer->measure_button(renderer, padding, icon.c_str(), fontFace.c_str(), fontSize, label.c_str());
}

void ButtonPrivate::draw(wzRect clip)
{
	if (drawStyle == Normal)
	{
		renderer->draw_button(renderer, clip, button, padding, icon.c_str(), fontFace.c_str(), fontSize, label.c_str());
	}
	else if (drawStyle == Tab)
	{
		renderer->draw_tab_button(renderer, clip, button, padding, fontFace.c_str(), fontSize, label.c_str());
	}
}

//------------------------------------------------------------------------------

Button::Button(wzRenderer *renderer)
{
	p = new ButtonPrivate(renderer);
}

Button::Button(wzRenderer *renderer, const std::string &label, const std::string &icon)
{
	p = new ButtonPrivate(renderer);

	if (!label.empty())
		setLabel(label);

	if (!icon.empty())
		setIcon(icon);
}

Button::~Button()
{
	delete p;
}

wzBorder Button::getPadding() const
{
	return ((ButtonPrivate *)p)->padding;
}

Button *Button::setPadding(wzBorder padding)
{
	((ButtonPrivate *)p)->padding = padding;
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

Button *Button::setPadding(int top, int right, int bottom, int left)
{
	ButtonPrivate *bp = (ButtonPrivate *)p;
	bp->padding.top = top;
	bp->padding.right = right;
	bp->padding.bottom = bottom;
	bp->padding.left = left;
	wz_widget_resize_to_measured(p->getWidget());
	return this;
}

std::string Button::getIcon() const
{
	return ((ButtonPrivate *)p)->icon;
}

Button *Button::setIcon(const std::string &icon)
{
	ButtonPrivate *bp = (ButtonPrivate *)p;
	bp->icon = icon;
	wz_widget_resize_to_measured(p->getWidget());
	return this;
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

CheckboxPrivate::CheckboxPrivate(wzRenderer *renderer) : boundValue(NULL)
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
	if (!wz_widget_get_main_window((wzWidget *)button))
	{
		wz_widget_destroy((wzWidget *)button);
	}
}

wzSize CheckboxPrivate::measure()
{
	return renderer->measure_checkbox(renderer, fontFace.c_str(), fontSize, label.c_str());
}

void CheckboxPrivate::draw(wzRect clip)
{
	renderer->draw_checkbox(renderer, clip, button, fontFace.c_str(), fontSize, label.c_str());
}

void CheckboxPrivate::handleEvent(wzEvent *e)
{
	if (boundValue && e->button.type == WZ_EVENT_BUTTON_CLICKED)
	{
		*boundValue = wz_button_is_set(button);
	}
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

Checkbox *Checkbox::bindValue(bool *value)
{
	CheckboxPrivate *cp = (CheckboxPrivate *)p;
	cp->boundValue = value;
	wz_button_set(cp->button, *value);
	return this;
}

//------------------------------------------------------------------------------

ComboPrivate::ComboPrivate(wzRenderer *renderer) : itemData(NULL), itemStride(0)
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
	if (!wz_widget_get_main_window((wzWidget *)combo))
	{
		wz_widget_destroy((wzWidget *)combo);
	}
}

wzSize ComboPrivate::measure()
{
	return renderer->measure_combo(renderer, fontFace.c_str(), fontSize, itemData, itemStride, wz_list_get_num_items(wz_combo_get_list(combo)));
}

void ComboPrivate::draw(wzRect clip)
{
	int itemIndex = wz_list_get_selected_item((const wzList *)list->p->getWidget());
	renderer->draw_combo(renderer, clip, combo, fontFace.c_str(), fontSize, itemIndex >= 0 ? *((const char **)&itemData[itemIndex * itemStride]) : NULL);
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

Widget *Combo::setFontFace(const std::string &fontFace)
{
	Widget::setFontFace(fontFace);
	((ComboPrivate *)p)->list->setFontFace(fontFace);
	return (Widget *)this;
}

Widget *Combo::setFontSize(float fontSize)
{
	Widget::setFontSize(fontSize);
	((ComboPrivate *)p)->list->setFontSize(fontSize);
	return (Widget *)this;
}

Widget *Combo::setFont(const std::string &fontFace, float fontSize)
{
	Widget::setFont(fontFace, fontSize);
	((ComboPrivate *)p)->list->setFont(fontFace, fontSize);
	return (Widget *)this;
}

Combo *Combo::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	ComboPrivate *cp = (ComboPrivate *)p;
	cp->itemData = itemData;
	cp->itemStride = itemStride;
	cp->list->setItems(itemData, itemStride, nItems);
	wz_widget_resize_to_measured(p->getWidget());
	return this;
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

FramePrivate::FramePrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	frame = wz_frame_create();
	wzWidget *widget = (wzWidget *)frame;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_size_args(widget, 200, 200);
}

FramePrivate::~FramePrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)frame))
	{
		wz_widget_destroy((wzWidget *)frame);
	}
}

//------------------------------------------------------------------------------

Frame::Frame(wzRenderer *renderer)
{
	p = new FramePrivate(renderer);
}

Frame::~Frame()
{
	delete p;
}

Widget *Frame::add(Widget *widget)
{
	p->add(widget);
	return widget;
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
	if (!wz_widget_get_main_window((wzWidget *)frame))
	{
		wz_widget_destroy((wzWidget *)frame);
	}
}

void GroupBoxPrivate::refreshMargin()
{
	wz_widget_set_margin(wz_widget_get_content_widget(getWidget()), renderer->measure_group_box_margin(renderer, fontFace.c_str(), fontSize, label.c_str()));
}

void GroupBoxPrivate::draw(wzRect clip)
{
	renderer->draw_group_box(renderer, clip, frame, fontFace.c_str(), fontSize, label.c_str());
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
	if (!wz_widget_get_main_window((wzWidget *)label))
	{
		wz_widget_destroy((wzWidget *)label);
	}
}

wzSize LabelPrivate::measure()
{
	return renderer->measure_label(renderer, fontFace.c_str(), fontSize, text.c_str());
}

void LabelPrivate::draw(wzRect clip)
{
	renderer->draw_label(renderer, clip, label, fontFace.c_str(), fontSize, text.c_str(), r, g, b);
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

ListPrivate::ListPrivate(wzRenderer *renderer) : itemData(NULL), itemStride(0)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	scroller.reset(new Scroller(renderer));

	list = wz_list_create((wzScroller *)scroller->p->getWidget());
	wz_list_set_items_border(list, renderer->get_list_items_border(renderer, list));
	wz_widget_set_metadata((wzWidget *)list, this);
	wz_widget_set_draw_callback((wzWidget *)list, DrawWidget);
	refreshItemHeight();
}

ListPrivate::~ListPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)list))
	{
		wz_widget_destroy((wzWidget *)list);
	}
}

void ListPrivate::draw(wzRect clip)
{
	renderer->draw_list(renderer, clip, list, fontFace.c_str(), fontSize, itemData, itemStride);
}

void ListPrivate::refreshItemHeight()
{
	wz_list_set_item_height(list, renderer->measure_list_item_height(renderer, list, fontFace.c_str(), fontSize));
}

void ListPrivate::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	this->itemData = itemData;
	this->itemStride = itemStride;
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

Widget *List::setFontFace(const std::string &fontFace)
{
	Widget::setFontFace(fontFace);
	((ListPrivate *)p)->refreshItemHeight();
	return (Widget *)this;
}

Widget *List::setFontSize(float fontSize)
{
	Widget::setFontSize(fontSize);
	((ListPrivate *)p)->refreshItemHeight();
	return (Widget *)this;
}

Widget *List::setFont(const std::string &fontFace, float fontSize)
{
	Widget::setFont(fontFace, fontSize);
	((ListPrivate *)p)->refreshItemHeight();
	return (Widget *)this;
}

List *List::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	((ListPrivate *)p)->setItems(itemData, itemStride, nItems);
	return this;
}

List *List::setSelectedItem(int index)
{
	wz_list_set_selected_item((wzList *)p->getWidget(), index);
	return this;
}

//------------------------------------------------------------------------------

static void MeasureText(wzMainWindow *mainWindow, wzWidget *widget, const char *text, int n, int *width, int *height)
{
	wzRenderer *renderer = ((MainWindowPrivate *)wz_widget_get_metadata((wzWidget *)mainWindow))->renderer;
	WidgetPrivate *wp = (WidgetPrivate *)wz_widget_get_metadata(widget);
	renderer->measure_text(renderer, wp->fontFace.c_str(), wp->fontSize, text, n, width, height);
}

static int TextGetPixelDelta(wzMainWindow *mainWindow, wzWidget *widget, const char *text, int index)
{
	wzRenderer *renderer = ((MainWindowPrivate *)wz_widget_get_metadata((wzWidget *)mainWindow))->renderer;
	WidgetPrivate *wp = (WidgetPrivate *)wz_widget_get_metadata(widget);
	return renderer->text_get_pixel_delta(renderer, wp->fontFace.c_str(), wp->fontSize, text, index);
}

static void DrawDockIcon(wzRect rect, void *metadata)
{
	MainWindowPrivate *mainWindow = (MainWindowPrivate *)metadata;
	mainWindow->drawDockIcon(rect);
}

static void DrawDockPreview(wzRect rect, void *metadata)
{
	MainWindowPrivate *mainWindow = (MainWindowPrivate *)metadata;
	mainWindow->drawDockPreview(rect);
}

MainWindowPrivate::MainWindowPrivate(wzRenderer *renderer) : showCursor(false)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	mainWindow = wz_main_window_create();
	wz_widget_set_metadata((wzWidget *)mainWindow, this);
	wz_main_window_set_event_callback(mainWindow, HandleEvent);
	wz_main_window_set_measure_text_callback(mainWindow, MeasureText);
	wz_main_window_set_text_get_pixel_delta_callback(mainWindow, TextGetPixelDelta);
	wz_main_window_set_draw_dock_icon_callback(mainWindow, DrawDockIcon, this);
	wz_main_window_set_draw_dock_preview_callback(mainWindow, DrawDockPreview, this);

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		dockTabBars[i] = new DockTabBar(renderer);
		wz_main_window_set_dock_tab_bar(mainWindow, (wzDockPosition)i, (wzTabBar *)dockTabBars[i]->getWidget());
		wz_widget_set_height(dockTabBars[i]->getWidget(), 20);
	}
}

MainWindowPrivate::~MainWindowPrivate()
{
	for (size_t i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		delete dockTabBars[i];
	}

	wz_widget_destroy((wzWidget *)mainWindow);
}

void MainWindowPrivate::drawDockIcon(wzRect rect)
{
	renderer->draw_dock_icon(renderer, rect);
}

void MainWindowPrivate::drawDockPreview(wzRect rect)
{
	renderer->draw_dock_preview(renderer, rect);
}

//------------------------------------------------------------------------------

MainWindow::MainWindow(wzRenderer *renderer)
{
	p = new MainWindowPrivate(renderer);
}

MainWindow::~MainWindow()
{
	delete p;
}

int MainWindow::getWidth() const
{
	return wz_widget_get_width((const wzWidget *)p->mainWindow);
}

int MainWindow::getHeight() const
{
	return wz_widget_get_height((const wzWidget *)p->mainWindow);
}

void MainWindow::setSize(int w, int h)
{
	wz_widget_set_size_args((wzWidget *)p->mainWindow, w, h);
}

void MainWindow::mouseMove(int x, int y, int dx, int dy)
{
	wz_main_window_mouse_move(p->mainWindow, x, y, dx, dy);
}

void MainWindow::mouseButtonDown(int button, int x, int y)
{
	wz_main_window_mouse_button_down(p->mainWindow, button, x, y);
}

void MainWindow::mouseButtonUp(int button, int x, int y)
{
	wz_main_window_mouse_button_up(p->mainWindow, button, x, y);
}

void MainWindow::mouseWheelMove(int x, int y)
{
	wz_main_window_mouse_wheel_move(p->mainWindow, x, y);
}

void MainWindow::keyDown(wzKey key)
{
	wz_main_window_key_down(p->mainWindow, key);
}

void MainWindow::keyUp(wzKey key)
{
	wz_main_window_key_up(p->mainWindow, key);
}

void MainWindow::textInput(const char *text)
{
	wz_main_window_text_input(p->mainWindow, text);
}

void MainWindow::setShowCursor(bool showCursor)
{
	p->showCursor = showCursor;
}

void MainWindow::beginFrame()
{
	p->renderer->begin_frame(p->renderer, p->mainWindow);
}

void MainWindow::drawFrame()
{	
	wz_main_window_draw(p->mainWindow);
}

void MainWindow::endFrame()
{
	p->renderer->end_frame(p->renderer);
}

bool MainWindow::getShowCursor() const
{
	return p->showCursor;
}

wzCursor MainWindow::getCursor() const
{
	return wz_main_window_get_cursor(p->mainWindow);
}

Widget *MainWindow::add(Widget *widget)
{
	p->add(widget);
	return widget;
}

void MainWindow::remove(Widget *widget)
{
	p->remove(widget);
}

void MainWindow::dockWindow(Window *window, wzDockPosition dockPosition)
{
	wz_main_window_dock_window(p->mainWindow, (wzWindow *)window->p->getWidget(), dockPosition);
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
	if (!wz_widget_get_main_window((wzWidget *)button))
	{
		wz_widget_destroy((wzWidget *)button);
	}
}

wzSize RadioButtonPrivate::measure()
{
	return renderer->measure_radio_button(renderer, fontFace.c_str(), fontSize, label.c_str());
}

void RadioButtonPrivate::draw(wzRect clip)
{
	renderer->draw_radio_button(renderer, clip, button, fontFace.c_str(), fontSize, label.c_str());
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
	wz_widget_set_measure_callback((wzWidget *)scroller, MeasureWidget);
	wz_scroller_set_nub_size(scroller, 16);

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wz_widget_set_size_args(decrementButton->p->getWidget(), 16, 16);
	wz_widget_set_size_args(incrementButton->p->getWidget(), 16, 16);
}

ScrollerPrivate::~ScrollerPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)scroller))
	{
		wz_widget_destroy((wzWidget *)scroller);
	}
}

wzSize ScrollerPrivate::measure()
{
	return renderer->measure_scroller(renderer, wz_scroller_get_type(scroller));
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
	if (!wz_widget_get_main_window((wzWidget *)layout))
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
	if (!wz_widget_get_main_window((wzWidget *)tabBar_))
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

	if (!wz_widget_get_main_window((wzWidget *)tabbed))
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
	wz_text_edit_set_border(textEdit, renderer->get_text_edit_border(renderer, textEdit));
	wzWidget *widget = (wzWidget *)textEdit;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_callback(widget, DrawWidget);
	wz_widget_set_measure_callback(widget, MeasureWidget);
}

TextEditPrivate::~TextEditPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)textEdit))
	{
		wz_widget_destroy((wzWidget *)textEdit);
	}
}

wzSize TextEditPrivate::measure()
{
	return renderer->measure_text_edit(renderer, wz_text_edit_get_border(textEdit), fontFace.c_str(), fontSize, wz_text_edit_get_text(textEdit));
}

void TextEditPrivate::draw(wzRect clip)
{
	MainWindowPrivate *mainWindow = (MainWindowPrivate *)wz_widget_get_metadata((wzWidget *)wz_widget_get_main_window(getWidget()));
	renderer->draw_text_edit(renderer, clip, textEdit, fontFace.c_str(), fontSize, mainWindow->showCursor);
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
	if (!wz_widget_get_main_window((wzWidget *)window))
	{
		wz_widget_destroy((wzWidget *)window);
	}
}

void WindowPrivate::draw(wzRect clip)
{
	renderer->draw_window(renderer, clip, window, fontFace.c_str(), fontSize, title.c_str());
}

void WindowPrivate::refreshHeaderHeight()
{
	wz_window_set_header_height(window, renderer->measure_window_header_height(renderer, fontFace.c_str(), fontSize, title.c_str()));
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

Widget *Window::setFontFace(const std::string &fontFace)
{
	Widget::setFontFace(fontFace);
	((WindowPrivate *)p)->refreshHeaderHeight();
	return (Widget *)this;
}

Widget *Window::setFontSize(float fontSize)
{
	Widget::setFontSize(fontSize);
	((WindowPrivate *)p)->refreshHeaderHeight();
	return (Widget *)this;
}

Widget *Window::setFont(const std::string &fontFace, float fontSize)
{
	Widget::setFont(fontFace, fontSize);
	((WindowPrivate *)p)->refreshHeaderHeight();
	return (Widget *)this;
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
