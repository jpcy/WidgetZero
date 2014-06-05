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
#include <widgetzero/wz_cpp.h>

namespace wz {

static void DrawWidget(wzWidget *widget, wzRect clip)
{
	((Widget *)wz_widget_get_metadata(widget))->draw(clip);
}

void Widget::setPosition(int x, int y)
{
	wz_widget_set_position_args(getWidget(), x, y);
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
	wz_desktop_set_draw_dock_icon_callback(desktop_, DrawDockIcon, this);
	wz_desktop_set_draw_dock_preview_callback(desktop_, DrawDockPreview, this);
}

Desktop::~Desktop()
{
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
	wz_desktop_draw(desktop_);
	renderer_->reset_clipping(renderer_);
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

Window::Window(Widget *parent, char *title)
{
	renderer_ = parent->getRenderer();

	wzWidget *widget;
	wzSize size;

	strcpy(title_, title);
	window_ = wz_window_create(wz_widget_get_desktop(parent->getWidget()));
	widget = (wzWidget *)window_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_window_set_border_size(window_, 4);

	// Calculate header height based on label text plus padding.
	renderer_->measure_text(renderer_, title_, &size.w, &size.h);
	size.h += 6;
	wz_window_set_header_height(window_, size.h);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void Window::draw(wzRect clip)
{
	renderer_->draw_window(renderer_, clip, window_, title_);
}

//------------------------------------------------------------------------------

Button::Button(Widget *parent, const char *label)
{
	renderer_ = parent->getRenderer();

	wzWidget *widget;
	wzSize size;

	strcpy(label_, label);
	button_ = wz_button_create(wz_widget_get_desktop(parent->getWidget()));
	widget = (wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);

	// Calculate size based on label text plus padding.
	renderer_->measure_text(renderer_, label_, &size.w, &size.h);
	size.w += 16;
	size.h += 8;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

Button::Button(wzButton *button, const char *label)
{
	wzWidget *widget = (wzWidget *)button;

	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	renderer_ = desktop->getRenderer();

	strcpy(label_, label);
	button_ = button;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
}

wzRect Button::getRect()
{
	return wz_widget_get_absolute_rect((wzWidget *)button_);
}

void Button::draw(wzRect clip)
{
	renderer_->draw_button(renderer_, clip, button_, label_);
}

//------------------------------------------------------------------------------

Checkbox::Checkbox(Widget *parent, const char *label)
{
	renderer_ = parent->getRenderer();

	wzWidget *widget;
	wzSize size;

	strcpy(label_, label);
	button_ = wz_button_create(wz_widget_get_desktop(parent->getWidget()));
	widget = (wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_button_set_set_behavior(button_, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);

	// Calculate size.
	renderer_->measure_text(renderer_, label_, &size.w, &size.h);
	size.w += boxSize + boxRightMargin;
	size.w += 16;
	size.h += 8;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void Checkbox::draw(wzRect clip)
{
	renderer_->draw_checkbox(renderer_, clip, button_, label_);
}

//------------------------------------------------------------------------------

Combo::Combo(Widget *parent, const char **items, int nItems)
{
	renderer_ = parent->getRenderer();
	items_ = items;
	combo_ = wz_combo_create(wz_widget_get_desktop(parent->getWidget()));
	wzWidget *widget = (wzWidget *)combo_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getWidget(), widget);
	
	list_.reset(new List(wz_combo_get_list(combo_), items, nItems));
}

void Combo::draw(wzRect clip)
{
	int itemIndex = wz_list_get_selected_item((const wzList *)list_->getWidget());
	renderer_->draw_combo(renderer_, clip, combo_, itemIndex >= 0 ? items_[itemIndex] : NULL);
}

//------------------------------------------------------------------------------

GroupBox::GroupBox(Widget *parent, const char *label)
{
	renderer_ = parent->getRenderer();

	wzWidget *widget;
	wzSize size;

	strcpy(label_, label);
	groupBox_ = wz_groupbox_create(wz_widget_get_desktop(parent->getWidget()));
	widget = (wzWidget *)groupBox_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);

	size.w = 200;
	size.h = 200;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void GroupBox::draw(wzRect clip)
{
	renderer_->draw_groupbox(renderer_, clip, groupBox_, label_);
}

//------------------------------------------------------------------------------

Scroller::Scroller(Widget *parent, wzScrollerType type, int value, int stepValue, int maxValue)
{
	renderer_ = parent->getRenderer();
	scroller_ = wz_scroller_create(wz_widget_get_desktop(parent->getWidget()), type);
	wz_scroller_set_max_value(scroller_, maxValue);
	wz_scroller_set_value(scroller_, value);
	wz_scroller_set_step_value(scroller_, stepValue);
	wzWidget *widget = (wzWidget *)scroller_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_scroller_set_nub_size(scroller_, 16);

	decrementButton.reset(new Button(wz_scroller_get_decrement_button(scroller_), "-"));
	incrementButton.reset(new Button(wz_scroller_get_increment_button(scroller_), "+"));

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wzSize buttonSize;
	buttonSize.w = 16;
	buttonSize.h = 16;
	wz_widget_set_size((wzWidget *)wz_scroller_get_decrement_button(scroller_), buttonSize);
	wz_widget_set_size((wzWidget *)wz_scroller_get_increment_button(scroller_), buttonSize);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

Scroller::Scroller(wzScroller *scroller)
{
	wzWidget *widget = (wzWidget *)scroller;

	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	renderer_ = desktop->getRenderer();

	scroller_ = scroller;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_scroller_set_nub_size(scroller_, 16);

	decrementButton.reset(new Button(wz_scroller_get_decrement_button(scroller_), "-"));
	incrementButton.reset(new Button(wz_scroller_get_increment_button(scroller_), "+"));

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wzSize buttonSize;
	buttonSize.w = 16;
	buttonSize.h = 16;
	wz_widget_set_size((wzWidget *)wz_scroller_get_decrement_button(scroller_), buttonSize);
	wz_widget_set_size((wzWidget *)wz_scroller_get_increment_button(scroller_), buttonSize);
}

void Scroller::draw(wzRect clip)
{
	renderer_->draw_scroller(renderer_, clip, scroller_);
}

int Scroller::getValue() const
{
	return wz_scroller_get_value(scroller_);
}

//------------------------------------------------------------------------------

Label::Label(Widget *parent)
{
	renderer_ = parent->getRenderer();
	text_[0] = r = g = b = 0;
	label_ = wz_label_create(wz_widget_get_desktop(parent->getWidget()));
	wzWidget *widget = (wzWidget *)label_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void Label::setText(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(text_, sizeof(text_), format, args);
	va_end(args);

	wzSize size;
	renderer_->measure_text(renderer_, text_, &size.w, &size.h);
	wz_widget_set_size((wzWidget *)label_, size);
}

void Label::setTextColor(uint8_t r, uint8_t g, uint8_t b)
{
	this->r = r;
	this->g = g;
	this->b = b;
}

void Label::draw(wzRect clip)
{
	renderer_->draw_label(renderer_, clip, label_, text_, r, g, b);
}

//------------------------------------------------------------------------------

List::List(Widget *parent, const char **items, int nItems)
{
	renderer_ = parent->getRenderer();
	list_ = wz_list_create(wz_widget_get_desktop(parent->getWidget()));
	wz_list_set_num_items(list_, nItems);
	wz_list_set_item_height(list_, itemHeight);
	wzWidget *widget = (wzWidget *)list_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getWidget(), widget);
	items_ = items;
	scroller_.reset(new Scroller(wz_list_get_scroller(list_)));

	wzBorder border;
	border.left = border.right = border.top = border.bottom = itemsMargin;
	wz_list_set_items_border(list_, border);

	wzSize scrollerSize;
	scrollerSize.w = 16;
	scrollerSize.h = 0;
	wz_widget_set_size(scroller_->getWidget(), scrollerSize);
}

List::List(wzList *list, const char **items, int nItems)
{
	wzWidget *widget = (wzWidget *)list;

	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	renderer_ = desktop->getRenderer();

	list_ = list;
	wz_list_set_num_items(list_, nItems);
	wz_list_set_item_height(list_, itemHeight);
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	items_ = items;
	scroller_.reset(new Scroller(wz_list_get_scroller(list_)));

	wzBorder border;
	border.left = border.right = border.top = border.bottom = itemsMargin;
	wz_list_set_items_border(list_, border);

	wzSize scrollerSize;
	scrollerSize.w = 16;
	scrollerSize.h = 0;
	wz_widget_set_size(scroller_->getWidget(), scrollerSize);
}

void List::draw(wzRect clip)
{
	renderer_->draw_list(renderer_, clip, list_, items_);
}

//------------------------------------------------------------------------------

TabButton::TabButton(wzButton *button, const char *label)
{
	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop((wzWidget *)button));
	renderer_ = desktop->getRenderer();

	button_ = button;
	strcpy(label_, label);
	wz_widget_set_metadata((wzWidget *)button_, this);
	wz_widget_set_draw_function((wzWidget *)button_, DrawWidget);

	// Calculate width based on label text plus padding.
	int width;
	renderer_->measure_text(renderer_, label_, &width, NULL);
	width += 16;
	wz_widget_set_width((wzWidget *)button_, width);
}

void TabButton::draw(wzRect clip)
{
	renderer_->draw_tab_button(renderer_, clip, button_, label_);
}

//------------------------------------------------------------------------------

TabBar::TabBar(Widget *parent)
{
	renderer_ = parent->getRenderer();
	tabBar_ = wz_tab_bar_create(wz_widget_get_desktop(parent->getWidget()));

	wz_widget_set_metadata((wzWidget *)tabBar_, this);
	wz_widget_add_child_widget(parent->getWidget(), (wzWidget *)tabBar_);

	decrementButton.reset(new Button(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton->getWidget(), 14);
	incrementButton.reset(new Button(wz_tab_bar_get_increment_button(tabBar_), ">"));
	wz_widget_set_width((wzWidget *)incrementButton->getWidget(), 14);
}

TabBar::TabBar(wzTabBar *tabBar)
{
	tabBar_ = tabBar;

	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop((wzWidget *)tabBar));
	renderer_ = desktop->getRenderer();

	wz_widget_set_metadata((wzWidget *)tabBar_, this);

	decrementButton.reset(new Button(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton->getWidget(), 14);
	incrementButton.reset(new Button(wz_tab_bar_get_increment_button(tabBar_), ">"));
	wz_widget_set_width((wzWidget *)incrementButton->getWidget(), 14);
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

void TabBar::addTab(const char *label)
{
	wzButton *tab = wz_tab_bar_add_tab(tabBar_);
	tabs_.push_back(new TabButton(tab, label));
}

void TabBar::addTab(wzButton *button, const char *label)
{
	tabs_.push_back(new TabButton(button, label));
}

//------------------------------------------------------------------------------

TabPage::TabPage(wzWidget *widget)
{
	widget_ = widget;
	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	renderer_ = desktop->getRenderer();
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
	tabbed_ = wz_tabbed_create(wz_widget_get_desktop(parent->getWidget()));
	wz_widget_set_metadata((wzWidget *)tabbed_, this);
	wz_widget_add_child_widget(parent->getWidget(), (wzWidget *)tabbed_);
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

TabPage *Tabbed::addTab(const char *label)
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
