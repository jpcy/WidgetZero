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

static void DrawDockIcon(wzRect rect, void *metadata)
{
	Desktop *desktop = (Desktop *)metadata;
	desktop->drawDockIcon(rect);
}

static void DrawDockPreview(wzRect rect, void *metadata)
{
	Desktop *desktop = (Desktop *)metadata;
	desktop->drawDockPreview(rect);
}

Desktop::Desktop(wzRenderer *renderer)
{
	desktop_ = wz_desktop_create();
	renderer_ = renderer;
	wz_widget_set_metadata((wzWidget *)desktop_, this);
	wz_desktop_set_event_callback(desktop_, HandleEvent);
	wz_desktop_set_draw_dock_icon_callback(desktop_, DrawDockIcon, this);
	wz_desktop_set_draw_dock_preview_callback(desktop_, DrawDockPreview, this);

	struct wzTabBar **dockTabBars = wz_desktop_get_dock_tab_bars(desktop_);

	for (int i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		dockTabBars_[i] = new DockTabBar(dockTabBars[i]);
		wz_widget_set_height((wzWidget *)dockTabBars[i], 20);
	}
}

Desktop::~Desktop()
{
	for (size_t i = 0; i < WZ_NUM_DOCK_POSITIONS; i++)
	{
		delete dockTabBars_[i];
	}

	wz_widget_destroy((wzWidget *)desktop_);
}

void Desktop::setSize(int w, int h)
{
	wz_widget_set_size_args((wzWidget *)desktop_, w, h);
}

void Desktop::mouseMove(int x, int y, int dx, int dy)
{
	wz_desktop_mouse_move(desktop_, x, y, dx, dy);
}

void Desktop::mouseButtonDown(int button, int x, int y)
{
	wz_desktop_mouse_button_down(desktop_, button, x, y);
}

void Desktop::mouseButtonUp(int button, int x, int y)
{
	wz_desktop_mouse_button_up(desktop_, button, x, y);
}

void Desktop::mouseWheelMove(int x, int y)
{
	wz_desktop_mouse_wheel_move(desktop_, x, y);
}

void Desktop::draw()
{
	wzRect rect = wz_widget_get_rect((const wzWidget *)desktop_);
	renderer_->begin_frame(renderer_, rect.w, rect.h);
	wz_desktop_draw(desktop_);
	renderer_->end_frame(renderer_);
}

void Desktop::drawDockIcon(wzRect rect)
{
	renderer_->draw_dock_icon(renderer_, rect);
}

void Desktop::drawDockPreview(wzRect rect)
{
	renderer_->draw_dock_preview(renderer_, rect);
}

//------------------------------------------------------------------------------

Window::Window(Widget *parent, const std::string &title) : title_(title)
{
	renderer_ = parent->getRenderer();

	wzWidget *widget;
	wzSize size;

	window_ = wz_window_create(parent->getDesktop());
	widget = (wzWidget *)window_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_window_set_border_size(window_, 4);

	// Calculate header height based on label text plus padding.
	renderer_->measure_text(renderer_, title_.c_str(), &size.w, &size.h);
	wz_window_set_header_height(window_, size.h + 6);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void Window::draw(wzRect clip)
{
	renderer_->draw_window(renderer_, clip, window_, title_.c_str());
}

//------------------------------------------------------------------------------

VerticalStackLayout::VerticalStackLayout(Widget *parent)
{
	renderer_ = parent->getRenderer();
	layout_ = wz_vertical_stack_layout_create(parent->getDesktop());
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)layout_);
}

//------------------------------------------------------------------------------

Button::Button(Widget *parent, const std::string &label) : label_(label)
{
	renderer_ = parent->getRenderer();
	button_ = wz_button_create(parent->getDesktop());
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)button_);

	// Calculate size based on label text plus padding.
	wzSize size;
	renderer_->measure_text(renderer_, label_.c_str(), &size.w, &size.h);
	size.w += 16;
	size.h += 8;
	wz_widget_set_size((wzWidget *)button_, size);

	initialize();
}

Button::Button(wzButton *button, const std::string &label) : button_(button), label_(label)
{
	renderer_ = Desktop::fromWidget((wzWidget *)button_)->getRenderer();
	initialize();
}

void Button::draw(wzRect clip)
{
	renderer_->draw_button(renderer_, clip, button_, label_.c_str());
}

void Button::initialize()
{
	wz_widget_set_metadata((wzWidget *)button_, this);
	wz_widget_set_draw_function((wzWidget *)button_, DrawWidget);
}

//------------------------------------------------------------------------------

Checkbox::Checkbox(Widget *parent, const std::string &label) : label_(label)
{
	renderer_ = parent->getRenderer();
	button_ = wz_button_create(parent->getDesktop());
	wzWidget *widget = (wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_button_set_set_behavior(button_, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);

	// Calculate size.
	wzSize size;
	renderer_->measure_text(renderer_, label_.c_str(), &size.w, &size.h);
	size.w += boxSize + boxRightMargin;
	size.w += 16;
	size.h += 8;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

void Checkbox::draw(wzRect clip)
{
	renderer_->draw_checkbox(renderer_, clip, button_, label_.c_str());
}

//------------------------------------------------------------------------------

Combo::Combo(Widget *parent, const char **items, int nItems) : items_(items)
{
	renderer_ = parent->getRenderer();
	combo_ = wz_combo_create(parent->getDesktop());
	wzWidget *widget = (wzWidget *)combo_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
	
	list_.reset(new List(wz_combo_get_list(combo_), items, nItems));
}

void Combo::draw(wzRect clip)
{
	int itemIndex = wz_list_get_selected_item((const wzList *)list_->getWidget());
	renderer_->draw_combo(renderer_, clip, combo_, itemIndex >= 0 ? items_[itemIndex] : NULL);
}

//------------------------------------------------------------------------------

GroupBox::GroupBox(Widget *parent, const std::string &label) : label_(label)
{
	renderer_ = parent->getRenderer();
	groupBox_ = wz_groupbox_create(parent->getDesktop());
	wzWidget *widget = (wzWidget *)groupBox_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_set_size_args(widget, 200, 200);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

void GroupBox::draw(wzRect clip)
{
	renderer_->draw_groupbox(renderer_, clip, groupBox_, label_.c_str());
}

//------------------------------------------------------------------------------

Scroller::Scroller(Widget *parent, wzScrollerType type, int value, int stepValue, int maxValue)
{
	renderer_ = parent->getRenderer();
	scroller_ = wz_scroller_create(parent->getDesktop(), type);
	wz_scroller_set_max_value(scroller_, maxValue);
	wz_scroller_set_value(scroller_, value);
	wz_scroller_set_step_value(scroller_, stepValue);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)scroller_);
	initialize();
}

Scroller::Scroller(wzScroller *scroller) : scroller_(scroller)
{
	renderer_ = Desktop::fromWidget((wzWidget *)scroller_)->getRenderer();
	initialize();
}

void Scroller::draw(wzRect clip)
{
	renderer_->draw_scroller(renderer_, clip, scroller_);
}

int Scroller::getValue() const
{
	return wz_scroller_get_value(scroller_);
}

void Scroller::initialize()
{
	wz_widget_set_metadata((wzWidget *)scroller_, this);
	wz_widget_set_draw_function((wzWidget *)scroller_, DrawWidget);
	wz_scroller_set_nub_size(scroller_, 16);

	decrementButton.reset(new Button(wz_scroller_get_decrement_button(scroller_), "-"));
	incrementButton.reset(new Button(wz_scroller_get_increment_button(scroller_), "+"));

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wz_widget_set_size_args((wzWidget *)wz_scroller_get_decrement_button(scroller_), 16, 16);
	wz_widget_set_size_args((wzWidget *)wz_scroller_get_increment_button(scroller_), 16, 16);
}

//------------------------------------------------------------------------------

Label::Label(Widget *parent) : r_(255), g_(255), b_(255)
{
	renderer_ = parent->getRenderer();
	label_ = wz_label_create(parent->getDesktop());
	wzWidget *widget = (wzWidget *)label_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getContentWidget(), widget);
}

void Label::setText(const char *format, ...)
{
	static char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(text_), format, args);
	va_end(args);
	text_ = buffer;

	wzSize size;
	renderer_->measure_text(renderer_, text_.c_str(), &size.w, &size.h);
	wz_widget_set_size((wzWidget *)label_, size);
}

void Label::setTextColor(uint8_t r, uint8_t g, uint8_t b)
{
	r_ = r;
	g_ = g;
	b_ = b;
}

void Label::draw(wzRect clip)
{
	renderer_->draw_label(renderer_, clip, label_, text_.c_str(), r_, g_, b_);
}

//------------------------------------------------------------------------------

List::List(Widget *parent, const char **items, int nItems) : items_(items)
{
	renderer_ = parent->getRenderer();
	list_ = wz_list_create(parent->getDesktop());
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)list_);
	initialize(nItems);
}

List::List(wzList *list, const char **items, int nItems) : list_(list), items_(items)
{
	renderer_ = Desktop::fromWidget((wzWidget *)list_)->getRenderer();
	initialize(nItems);	
}

void List::draw(wzRect clip)
{
	renderer_->draw_list(renderer_, clip, list_, items_);
}

void List::initialize(int nItems)
{
	wz_list_set_num_items(list_, nItems);
	wz_list_set_item_height(list_, itemHeight);
	wz_list_set_items_border_args(list_, itemsMargin, itemsMargin, itemsMargin, itemsMargin);
	wz_widget_set_metadata((wzWidget *)list_, this);
	wz_widget_set_draw_function((wzWidget *)list_, DrawWidget);

	scroller_.reset(new Scroller(wz_list_get_scroller(list_)));
	wz_widget_set_size_args(scroller_->getWidget(), 16, 0);
}

//------------------------------------------------------------------------------

TabButton::TabButton(wzButton *button, const std::string &label) : label_(label)
{
	renderer_ = Desktop::fromWidget((wzWidget *)button)->getRenderer();

	button_ = button;
	wz_widget_set_metadata((wzWidget *)button_, this);
	wz_widget_set_draw_function((wzWidget *)button_, DrawWidget);

	// Calculate width based on label text plus padding.
	int width;
	renderer_->measure_text(renderer_, label_.c_str(), &width, NULL);
	width += 16;
	wz_widget_set_width((wzWidget *)button_, width);
}

void TabButton::draw(wzRect clip)
{
	renderer_->draw_tab_button(renderer_, clip, button_, label_.c_str());
}

//------------------------------------------------------------------------------

DockTabBar::DockTabBar(wzTabBar *tabBar) : tabBar_(tabBar)
{
	renderer_ = Desktop::fromWidget((wzWidget *)tabBar)->getRenderer();
	wz_widget_set_metadata((wzWidget *)tabBar_, this);

	decrementButton.reset(new Button(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton->getWidget(), 14);
	incrementButton.reset(new Button(wz_tab_bar_get_increment_button(tabBar_), ">"));
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
		Window *window = (Window *)wz_widget_get_metadata((wzWidget *)wz_desktop_get_dock_tab_window(wz_widget_get_desktop((wzWidget *)tabBar_), e.tabBar.tab));
		tabs_.push_back(new TabButton(e.tabBar.tab, window->getTitle()));
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

TabBar::TabBar(Widget *parent)
{
	renderer_ = parent->getRenderer();
	tabBar_ = wz_tab_bar_create(parent->getDesktop());
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)tabBar_);
	initialize();
}

TabBar::TabBar(wzTabBar *tabBar) : tabBar_(tabBar)
{
	renderer_ = Desktop::fromWidget((wzWidget *)tabBar)->getRenderer();
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

void TabBar::addTab(const std::string &label)
{
	wzButton *tab = wz_tab_bar_add_tab(tabBar_);
	tabs_.push_back(new TabButton(tab, label));
}

void TabBar::addTab(wzButton *button, const std::string &label)
{
	tabs_.push_back(new TabButton(button, label));
}

void TabBar::initialize()
{
	wz_widget_set_metadata((wzWidget *)tabBar_, this);

	decrementButton.reset(new Button(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton->getWidget(), 14);
	incrementButton.reset(new Button(wz_tab_bar_get_increment_button(tabBar_), ">"));
	wz_widget_set_width((wzWidget *)incrementButton->getWidget(), 14);
}

//------------------------------------------------------------------------------

TabPage::TabPage(wzWidget *widget)
{
	widget_ = widget;
	renderer_ = Desktop::fromWidget(widget)->getRenderer();
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget_, DrawWidget);
}

void TabPage::draw(wzRect clip)
{
	renderer_->draw_tab_page(renderer_, clip, widget_);
}

//------------------------------------------------------------------------------

Tabbed::Tabbed(Widget *parent)
{
	renderer_ = parent->getRenderer();
	tabbed_ = wz_tabbed_create(parent->getDesktop());
	wz_widget_set_metadata((wzWidget *)tabbed_, this);
	wz_widget_add_child_widget(parent->getContentWidget(), (wzWidget *)tabbed_);
	tabBar_.reset(new TabBar(wz_tabbed_get_tab_bar(tabbed_)));
	tabBar_->setRect(0, 0, 0, 20);
}

Tabbed::~Tabbed()
{
	for (size_t i = 0; i < tabs_.size(); i++)
	{
		delete tabs_[i];
	}
}

void Tabbed::draw(wzRect clip)
{
}

TabPage *Tabbed::addTab(const std::string &label)
{
	wzButton *button;
	wzWidget *widget;
	wz_tabbed_add_tab(tabbed_, &button, &widget);
	tabBar_->addTab(button, label);
	
	TabPage *tab = new TabPage(widget);
	tabs_.push_back(tab);
	
	return tab;
}

} // namespace wz
