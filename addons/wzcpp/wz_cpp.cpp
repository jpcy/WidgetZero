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
	wz_widget_set_font_face(p->getWidget(), fontFace.c_str());
	return this;
}

Widget *Widget::setFontSize(float fontSize)
{
	wz_widget_set_font_size(p->getWidget(), fontSize);
	return this;
}

Widget *Widget::setFont(const std::string &fontFace, float fontSize)
{
	wz_widget_set_font(p->getWidget(), fontFace.c_str(), fontSize);
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

ButtonPrivate::ButtonPrivate()
{
	button = wz_button_create();
	wz_widget_set_metadata((wzWidget *)button, this);
}

ButtonPrivate::~ButtonPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)button))
	{
		wz_widget_destroy((wzWidget *)button);
	}
}

//------------------------------------------------------------------------------

Button::Button()
{
	p = new ButtonPrivate();
}

Button::Button(const std::string &label, const std::string &icon)
{
	p = new ButtonPrivate();

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
	return wz_button_get_padding((const wzButton *)p->getWidget());
}

Button *Button::setPadding(wzBorder padding)
{
	wz_button_set_padding((wzButton *)p->getWidget(), padding);
	return this;
}

Button *Button::setPadding(int top, int right, int bottom, int left)
{
	wzBorder padding;
	padding.top = top;
	padding.right = right;
	padding.bottom = bottom;
	padding.left = left;
	wz_button_set_padding((wzButton *)p->getWidget(), padding);
	return this;
}

const char *Button::getIcon() const
{
	return wz_button_get_icon((const wzButton *)p->getWidget());
}

Button *Button::setIcon(const std::string &icon)
{
	wz_button_set_icon((wzButton *)p->getWidget(), icon.c_str());
	return this;
}

const char *Button::getLabel() const
{
	return wz_button_get_label((const wzButton *)p->getWidget());
}

Button *Button::setLabel(const std::string &label)
{
	wz_button_set_label((wzButton *)p->getWidget(), label.c_str());
	return this;
}

Button *Button::setToggle(bool toggle)
{
	wz_button_set_set_behavior((wzButton *)p->getWidget(), toggle ? WZ_BUTTON_SET_BEHAVIOR_TOGGLE : WZ_BUTTON_SET_BEHAVIOR_DEFAULT);
	return this;
}

//------------------------------------------------------------------------------

CheckboxPrivate::CheckboxPrivate()
{
	button = wz_check_box_create();
	wz_widget_set_metadata((wzWidget *)button, this);
}

CheckboxPrivate::~CheckboxPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)button))
	{
		wz_widget_destroy((wzWidget *)button);
	}
}

//------------------------------------------------------------------------------

Checkbox::Checkbox()
{
	p = new CheckboxPrivate();
}

Checkbox::Checkbox(const std::string &label)
{
	p = new CheckboxPrivate();
	setLabel(label);
}

Checkbox::~Checkbox()
{
	delete p;
}

const char *Checkbox::getLabel() const
{
	return wz_button_get_label((const wzButton *)p->getWidget());
}

Checkbox *Checkbox::setLabel(const std::string &label)
{
	wz_button_set_label((wzButton *)p->getWidget(), label.c_str());
	return this;
}

Checkbox *Checkbox::bindValue(bool *value)
{
	wz_button_bind_value((wzButton *)p->getWidget(), value);
	return this;
}

//------------------------------------------------------------------------------

ComboPrivate::ComboPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	combo = wz_combo_create(renderer);
	wz_widget_set_metadata((wzWidget *)combo, this);
}

ComboPrivate::~ComboPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)combo))
	{
		wz_widget_destroy((wzWidget *)combo);
	}
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

Combo *Combo::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	wzList *list = wz_combo_get_list((wzCombo *)p->getWidget());
	wz_list_set_item_data(list, itemData);
	wz_list_set_item_stride(list, itemStride);
	wz_list_set_num_items(list, nItems);
	return this;
}

//------------------------------------------------------------------------------

FramePrivate::FramePrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
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
	groupBox = wz_group_box_create(renderer);
	wz_widget_set_metadata((wzWidget *)groupBox, this);
	wz_widget_set_size_args((wzWidget *)groupBox, 200, 200);
}

GroupBoxPrivate::~GroupBoxPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)groupBox))
	{
		wz_widget_destroy((wzWidget *)groupBox);
	}
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

const char *GroupBox::getLabel() const
{
	return wz_group_box_get_label((wzGroupBox *)p->getWidget());
}

GroupBox *GroupBox::setLabel(const std::string &label)
{
	wz_group_box_set_label((wzGroupBox *)p->getWidget(), label.c_str());
	return this;
}

Widget *GroupBox::add(Widget *widget)
{
	p->add(widget);
	return widget;
}

//------------------------------------------------------------------------------

LabelPrivate::LabelPrivate()
{
	label = wz_label_create();
	wz_widget_set_metadata((wzWidget *)label, this);
}

LabelPrivate::~LabelPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)label))
	{
		wz_widget_destroy((wzWidget *)label);
	}
}

//------------------------------------------------------------------------------

Label::Label()
{
	p = new LabelPrivate();
}

Label::Label(const std::string &text)
{
	p = new LabelPrivate();
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
	
	wz_label_set_text((wzLabel *)p->getWidget(), buffer);
	return this;
}

Label *Label::setTextColor(float r, float g, float b, float a)
{
	wzColor color;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	wz_label_set_text_color((wzLabel *)p->getWidget(), color);
	return this;
}

Label *Label::setMultiline(bool multiline)
{
	wz_label_set_multiline((wzLabel *)p->getWidget(), multiline);
	return this;
}

//------------------------------------------------------------------------------

ListPrivate::ListPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	list = wz_list_create(renderer);
	wz_widget_set_metadata((wzWidget *)list, this);
}

ListPrivate::~ListPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)list))
	{
		wz_widget_destroy((wzWidget *)list);
	}
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

List *List::setItems(uint8_t *itemData, size_t itemStride, int nItems)
{
	wz_list_set_item_data((wzList *)p->getWidget(), itemData);
	wz_list_set_item_stride((wzList *)p->getWidget(), itemStride);
	wz_list_set_num_items((wzList *)p->getWidget(), nItems);
	return this;
}

List *List::setSelectedItem(int index)
{
	wz_list_set_selected_item((wzList *)p->getWidget(), index);
	return this;
}

List *List::setDrawItemCallback(wzDrawListItemCallback callback)
{
	wz_list_set_draw_item_callback((wzList *)p->getWidget(), callback);
	return this;
}

//------------------------------------------------------------------------------

MainWindowPrivate::MainWindowPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	this->renderer = renderer;
	mainWindow = wz_main_window_create(renderer);
	wz_widget_set_metadata((wzWidget *)mainWindow, this);
	wz_main_window_set_event_callback(mainWindow, HandleEvent);
}

MainWindowPrivate::~MainWindowPrivate()
{
	wz_widget_destroy((wzWidget *)mainWindow);
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

RadioButtonPrivate::RadioButtonPrivate() : group(NULL)
{
	button = wz_radio_button_create();
	wz_widget_set_metadata((wzWidget *)button, this);
}

RadioButtonPrivate::~RadioButtonPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)button))
	{
		wz_widget_destroy((wzWidget *)button);
	}
}

//------------------------------------------------------------------------------

RadioButton::RadioButton()
{
	p = new RadioButtonPrivate();
}

RadioButton::RadioButton(const std::string &label)
{
	p = new RadioButtonPrivate();
	setLabel(label);
}

RadioButton::~RadioButton()
{
	delete p;
}

const char *RadioButton::getLabel() const
{
	return wz_button_get_label((const wzButton *)p->getWidget());
}

RadioButton *RadioButton::setLabel(const std::string &label)
{
	wz_button_set_label((wzButton *)p->getWidget(), label.c_str());
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
	scroller = wz_scroller_create(renderer);
	wz_widget_set_metadata((wzWidget *)scroller, this);
}

ScrollerPrivate::~ScrollerPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)scroller))
	{
		wz_widget_destroy((wzWidget *)scroller);
	}
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

SpinnerPrivate::SpinnerPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	spinner = wz_spinner_create(renderer);
	wz_widget_set_metadata((wzWidget *)spinner, this);
}

SpinnerPrivate::~SpinnerPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)spinner))
	{
		wz_widget_destroy((wzWidget *)spinner);
	}
}

//------------------------------------------------------------------------------

Spinner::Spinner(wzRenderer *renderer)
{
	p = new SpinnerPrivate(renderer);
}

Spinner::~Spinner()
{
	delete p;
}

Spinner *Spinner::setValue(int value)
{
	wz_spinner_set_value(((SpinnerPrivate *)p)->spinner, value);
	return this;
}

int Spinner::getValue() const
{
	return wz_spinner_get_value(((SpinnerPrivate *)p)->spinner);
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
	wz_button_set_label(p->button, label.c_str());
	return this;
}

Widget *Tab::add(Widget *widget)
{
	wz_widget_add_child_widget(p->page, widget->p->getWidget());
	return widget;
}

//------------------------------------------------------------------------------

TabbedPrivate::TabbedPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	tabbed = wz_tabbed_create(renderer);
	wz_widget_set_metadata((wzWidget *)tabbed, this);
}

TabbedPrivate::~TabbedPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)tabbed))
	{
		wz_widget_destroy((wzWidget *)tabbed);
	}
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
	wz_tabbed_add_tab((wzTabbed *)p->getWidget(), &tab->p->button, &tab->p->page);
	return tab;
}

//------------------------------------------------------------------------------

TextEditPrivate::TextEditPrivate(wzRenderer *renderer, bool multiline)
{
	WZ_ASSERT(renderer);
	textEdit = wz_text_edit_create(renderer, multiline, 256);
	wz_widget_set_metadata((wzWidget *)textEdit, this);
}

TextEditPrivate::~TextEditPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)textEdit))
	{
		wz_widget_destroy((wzWidget *)textEdit);
	}
}

//------------------------------------------------------------------------------

TextEdit::TextEdit(wzRenderer *renderer, bool multiline)
{
	p = new TextEditPrivate(renderer, multiline);
}

TextEdit::TextEdit(wzRenderer *renderer, const std::string &text, bool multiline)
{
	p = new TextEditPrivate(renderer, multiline);
	setText(text);
}

TextEdit::~TextEdit()
{
	delete p;
}

TextEdit *TextEdit::setText(const std::string &text)
{
	wz_text_edit_set_text((wzTextEdit *)p->getWidget(), text.c_str());
	return this;
}

//------------------------------------------------------------------------------

WindowPrivate::WindowPrivate(wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	window = wz_window_create(renderer);
	wz_widget_set_metadata((wzWidget *)window, this);
}

WindowPrivate::~WindowPrivate()
{
	if (!wz_widget_get_main_window((wzWidget *)window))
	{
		wz_widget_destroy((wzWidget *)window);
	}
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

const char *Window::getTitle() const
{
	return wz_window_get_title((wzWindow *)p->getWidget());
}

Window *Window::setTitle(const std::string &title)
{
	wz_window_set_title((wzWindow *)p->getWidget(), title.c_str());
	return this;
}

Widget *Window::add(Widget *widget)
{
	p->add(widget);
	return widget;
}

} // namespace wz
