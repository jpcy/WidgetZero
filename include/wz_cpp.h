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

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>
#include <wz.h>

#define WZCPP_CALL_OBJECT_METHOD(object, method) ((object)->*(method)) 

namespace wz {

class Button;
class Checkbox;
class Combo;
struct MainWindowPrivate;
class GroupBox;
class Label;
class List;
class RadioButton;
class Scroller;
class StackLayout;
class Tab;
struct TabPrivate;
class Tabbed;
class TextEdit;
struct WidgetPrivate;
class Widget;
class Window;

struct IEventHandler
{
	virtual ~IEventHandler() {}
	virtual void call(wzEvent *e) = 0;

	wzWidgetEventType eventType;
};

template<class Object>
struct EventHandler : public IEventHandler
{
	typedef void (Object::*Method)(wzEvent *);

	virtual void call(wzEvent *e)
	{
		WZCPP_CALL_OBJECT_METHOD(object, method)(e);
	}

	Object *object;
	Method method;
};

class Widget
{
public:
	virtual ~Widget();
	wzRect getRect() const;
	Widget *setPosition(int x, int y);
	Widget *setWidth(int w);
	Widget *setHeight(int h);
	Widget *setSize(int w, int h);
	Widget *setRect(int x, int y, int w, int h);
	Widget *setStretch(int stretch);
	Widget *setAlign(int align);
	Widget *setMargin(int margin);
	Widget *setMargin(int top, int right, int bottom, int left);
	Widget *setMargin(wzBorder margin);
	Widget *setStyle(wzWidgetStyle style);
	wzWidgetStyle getStyle() const;
	Widget *setFontFace(const std::string &fontFace);
	Widget *setFontSize(float fontSize);
	Widget *setFont(const std::string &fontFace, float fontSize);
	Widget *setVisible(bool visible);

	Widget *addEventHandler(IEventHandler *eventHandler);

	template<class Object>
	Widget *addEventHandler(wzWidgetEventType eventType, Object *object, void (Object::*method)(wzEvent *))
	{
		EventHandler<Object> *eventHandler = new EventHandler<Object>();
		eventHandler->eventType = eventType;
		eventHandler->object = object;
		eventHandler->method = method;
		addEventHandler(eventHandler);
		return this;
	}

	WidgetPrivate *p;
};

class Button : public Widget
{
public:
	Button();
	Button(const std::string &label, const std::string &icon = std::string());
	~Button();
	wzBorder getPadding() const;
	Button *setPadding(wzBorder padding);
	Button *setPadding(int top, int right, int bottom, int left);
	const char *getIcon() const;
	Button *setIcon(const std::string &icon);
	const char *getLabel() const;
	Button *setLabel(const std::string &label);
	Button *setToggle(bool toggle);
};

class Checkbox : public Widget
{
public:
	Checkbox();
	Checkbox( const std::string &label);
	~Checkbox();
	const char *getLabel() const;
	Checkbox *setLabel(const std::string &label);
	Checkbox *bindValue(bool *value);
};

class Combo : public Widget
{
public:
	Combo();
	~Combo();
	Combo *setItems(uint8_t *itemData, size_t itemStride, int nItems);
};

class Frame : public Widget
{
public:
	Frame(wzRenderer *renderer);
	~Frame();
	Widget *add(Widget *widget);
	void remove(Widget *widget);
};

class GroupBox : public Widget
{
public:
	GroupBox();
	GroupBox(const std::string &label);
	~GroupBox();
	const char *getLabel() const;
	GroupBox *setLabel(const std::string &label);
	Widget *add(Widget *widget);
	void remove(Widget *widget);
};

class Label : public Widget
{
public:
	Label();
	Label(const std::string &text);
	~Label();
	Label *setText(const char *format, ...);
	Label *setTextColor(float r, float g, float b, float a = 1.0f);
	Label *setMultiline(bool multiline);
};

class List : public Widget
{
public:
	List();
	~List();
	List *setItems(uint8_t *itemData, size_t itemStride, int nItems);
	List *setSelectedItem(int index);
	List *setItemHeight(int height);
	List *setDrawItemCallback(wzDrawListItemCallback callback);
};

class MainWindow
{
public:
	MainWindow(wzRenderer *renderer);
	~MainWindow();
	int getWidth() const;
	int getHeight() const;
	void setSize(int w, int h);
	void mouseMove(int x, int y, int dx, int dy);
	void mouseButtonDown(int button, int x, int y);
	void mouseButtonUp(int button, int x, int y);
	void mouseWheelMove(int x, int y);
	void keyDown(wzKey key);
	void keyUp(wzKey key);
	void textInput(const char *text);
	void beginFrame();
	void draw();
	void drawFrame();
	void endFrame();
	void toggleTextCursor();
	wzCursor getCursor() const;
	Widget *add(Widget *widget);
	void remove(Widget *widget);
	void createMenuButton(const std::string &label);
	void dockWindow(Window *window, wzDockPosition dockPosition);

	MainWindowPrivate *p;
};

class RadioButton : public Widget
{
public:
	RadioButton();
	RadioButton(const std::string &label);
	~RadioButton();
	const char *getLabel() const;
	RadioButton *setLabel(const std::string &label);
};

class Scroller : public Widget
{
public:
	Scroller();
	~Scroller();
	Scroller *setType(wzScrollerType type);
	Scroller *setValue(int value);
	Scroller *setStepValue(int stepValue);
	Scroller *setMaxValue(int maxValue);	
	int getValue() const;
};

class Spinner : public Widget
{
public:
	Spinner();
	~Spinner();
	Spinner *setValue(int value);
	int getValue() const;
};

class StackLayout : public Widget
{
public:
	StackLayout();
	StackLayout(wzStackLayoutDirection direction);
	~StackLayout();
	StackLayout *setDirection(wzStackLayoutDirection direction);
	StackLayout *setSpacing(int spacing);
	int getSpacing() const;
	Widget *add(Widget *widget);
	void remove(Widget *widget);
};

class Tab
{
public:
	Tab();
	~Tab();
	Tab *setLabel(const std::string &label);
	Widget *add(Widget *widget);
	void remove(Widget *widget);

	TabPrivate *p;
};

class Tabbed : public Widget
{
public:
	Tabbed();
	~Tabbed();
	Tab *addTab(Tab *tab);
};

class TextEdit : public Widget
{
public:
	TextEdit(bool multiline);
	TextEdit(const std::string &text, bool multiline);
	~TextEdit();
	TextEdit *setText(const std::string &text);
};

class Window : public Widget
{
public:
	Window();
	Window(const std::string &title);
	~Window();
	const char *getTitle() const;
	Window *setTitle(const std::string &title);
	Widget *add(Widget *widget);
	void remove(Widget *widget);
};

#ifdef WZ_CPP_IMPLEMENTATION

struct WidgetPrivate
{
	virtual ~WidgetPrivate()
	{
		for (size_t i = 0; i < eventHandlers.size(); i++)
		{
			delete eventHandlers[i];
		}

		eventHandlers.clear();
	}

	virtual const wzWidget *getWidget() const = 0;
	virtual wzWidget *getWidget() = 0;

	std::vector<IEventHandler *> eventHandlers;
};

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
	}
}

struct ButtonPrivate : public WidgetPrivate
{
	ButtonPrivate()
	{
		button = wz_button_create(NULL, NULL);
		wz_widget_set_metadata((wzWidget *)button, this);
	}

	~ButtonPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)button))
		{
			wz_widget_destroy((wzWidget *)button);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }

	wzButton *button;
};

struct CheckboxPrivate : public WidgetPrivate
{
	CheckboxPrivate()
	{
		button = wz_check_box_create(NULL);
		wz_widget_set_metadata((wzWidget *)button, this);
	}

	~CheckboxPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)button))
		{
			wz_widget_destroy((wzWidget *)button);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }

	wzButton *button;
};

struct ComboPrivate : public WidgetPrivate
{
	ComboPrivate()
	{
		combo = wz_combo_create();
		wz_widget_set_metadata((wzWidget *)combo, this);
	}

	~ComboPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)combo))
		{
			wz_widget_destroy((wzWidget *)combo);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)combo; }
	virtual wzWidget *getWidget() { return (wzWidget *)combo; }

	wzCombo *combo;
};

struct FramePrivate : public WidgetPrivate
{
	FramePrivate(wzRenderer *renderer)
	{
		WZ_ASSERT(renderer);
		frame = wz_frame_create();
		wzWidget *widget = (wzWidget *)frame;
		wz_widget_set_metadata(widget, this);
		wz_widget_set_size_args(widget, 200, 200);
	}

	~FramePrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)frame))
		{
			wz_widget_destroy((wzWidget *)frame);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)frame; }
	virtual wzWidget *getWidget() { return (wzWidget *)frame; }

	wzFrame *frame;
};

struct GroupBoxPrivate : public WidgetPrivate
{
	GroupBoxPrivate()
	{
		groupBox = wz_group_box_create();
		wz_widget_set_metadata((wzWidget *)groupBox, this);
		wz_widget_set_size_args((wzWidget *)groupBox, 200, 200);
	}

	~GroupBoxPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)groupBox))
		{
			wz_widget_destroy((wzWidget *)groupBox);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)groupBox; }
	virtual wzWidget *getWidget() { return (wzWidget *)groupBox; }

	wzGroupBox *groupBox;
};

struct LabelPrivate : public WidgetPrivate
{
	LabelPrivate()
	{
		label = wz_label_create();
		wz_widget_set_metadata((wzWidget *)label, this);
	}

	~LabelPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)label))
		{
			wz_widget_destroy((wzWidget *)label);
		}
	}
	virtual const wzWidget *getWidget() const { return (const wzWidget *)label; }
	virtual wzWidget *getWidget() { return (wzWidget *)label; }

	wzLabel *label;
};

struct ListPrivate : public WidgetPrivate
{
	ListPrivate()
	{
		list = wz_list_create();
		wz_widget_set_metadata((wzWidget *)list, this);
	}

	~ListPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)list))
		{
			wz_widget_destroy((wzWidget *)list);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)list; }
	virtual wzWidget *getWidget() { return (wzWidget *)list; }

	wzList *list;
};

struct MainWindowPrivate : public WidgetPrivate
{
	MainWindowPrivate(wzRenderer *renderer)
	{
		WZ_ASSERT(renderer);
		this->renderer = renderer;
		mainWindow = wz_main_window_create(renderer);
		wz_widget_set_metadata((wzWidget *)mainWindow, this);
		wz_main_window_set_event_callback(mainWindow, HandleEvent);

		menuBar = wz_menu_bar_create();
		wz_main_window_set_menu_bar(mainWindow, menuBar);
	}

	~MainWindowPrivate()
	{
		wz_widget_destroy((wzWidget *)mainWindow);
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)mainWindow; }
	virtual wzWidget *getWidget() { return (wzWidget *)mainWindow; }

	wzMainWindow *mainWindow;
	wzMenuBar *menuBar;
	wzRenderer *renderer;
};

struct RadioButtonPrivate : public WidgetPrivate
{
	RadioButtonPrivate()
	{
		button = wz_radio_button_create(NULL);
		wz_widget_set_metadata((wzWidget *)button, this);
	}

	~RadioButtonPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)button))
		{
			wz_widget_destroy((wzWidget *)button);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }

	wzButton *button;
};

struct ScrollerPrivate : public WidgetPrivate
{
	ScrollerPrivate()
	{
		scroller = wz_scroller_create();
		wz_widget_set_metadata((wzWidget *)scroller, this);
	}

	~ScrollerPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)scroller))
		{
			wz_widget_destroy((wzWidget *)scroller);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)scroller; }
	virtual wzWidget *getWidget() { return (wzWidget *)scroller; }

	wzScroller *scroller;
};

struct SpinnerPrivate : public WidgetPrivate
{
	SpinnerPrivate()
	{
		spinner = wz_spinner_create();
		wz_widget_set_metadata((wzWidget *)spinner, this);
	}

	~SpinnerPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)spinner))
		{
			wz_widget_destroy((wzWidget *)spinner);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)spinner; }
	virtual wzWidget *getWidget() { return (wzWidget *)spinner; }

	wzSpinner *spinner;
};

struct StackLayoutPrivate : public WidgetPrivate
{
	StackLayoutPrivate()
	{
		layout = wz_stack_layout_create();
	}

	~StackLayoutPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)layout))
		{
			wz_widget_destroy((wzWidget *)layout);
		}
	}

	virtual const wzWidget *getWidget() const { return (wzWidget *)layout; }
	virtual wzWidget *getWidget() { return (wzWidget *)layout; }
	
	wzStackLayout *layout;
};

// Wraps tab button and page.
struct TabPrivate
{
	TabPrivate() : button(NULL), page(NULL) {}
	wzButton *button;
	wzWidget *page;
};

struct TabbedPrivate : public WidgetPrivate
{
	TabbedPrivate()
	{
		tabbed = wz_tabbed_create();
		wz_widget_set_metadata((wzWidget *)tabbed, this);
	}

	~TabbedPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)tabbed))
		{
			wz_widget_destroy((wzWidget *)tabbed);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabbed; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabbed; }

	wzTabbed *tabbed;
};

struct TextEditPrivate : public WidgetPrivate
{
	TextEditPrivate(bool multiline)
	{
		textEdit = wz_text_edit_create(multiline, 256);
		wz_widget_set_metadata((wzWidget *)textEdit, this);
	}

	~TextEditPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)textEdit))
		{
			wz_widget_destroy((wzWidget *)textEdit);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)textEdit; }
	virtual wzWidget *getWidget() { return (wzWidget *)textEdit; }

	wzTextEdit *textEdit;
};

struct WindowPrivate : public WidgetPrivate
{
	WindowPrivate()
	{
		window = wz_window_create();
		wz_widget_set_metadata((wzWidget *)window, this);
	}

	~WindowPrivate()
	{
		if (!wz_widget_get_main_window((wzWidget *)window))
		{
			wz_widget_destroy((wzWidget *)window);
		}
	}

	virtual const wzWidget *getWidget() const { return (const wzWidget *)window; }
	virtual wzWidget *getWidget() { return (wzWidget *)window; }

	wzWindow *window;
};

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

Widget *Widget::setStyle(wzWidgetStyle style)
{
	wz_widget_set_style(p->getWidget(), style);
	return this;
}

wzWidgetStyle Widget::getStyle() const
{
	return wz_widget_get_style(p->getWidget());
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

Combo::Combo()
{
	p = new ComboPrivate();
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
	wz_frame_add((wzFrame *)p->getWidget(), widget->p->getWidget());
	return widget;
}

void Frame::remove(Widget *widget)
{
	wz_frame_remove((wzFrame *)p->getWidget(), widget->p->getWidget());
}

//------------------------------------------------------------------------------

GroupBox::GroupBox()
{
	p = new GroupBoxPrivate();
}

GroupBox::GroupBox(const std::string &label)
{
	p = new GroupBoxPrivate();
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
	wz_group_box_add((wzGroupBox *)p->getWidget(), widget->p->getWidget());
	return widget;
}

void GroupBox::remove(Widget *widget)
{
	wz_group_box_remove((wzGroupBox *)p->getWidget(), widget->p->getWidget());
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
	NVGcolor color;
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

List::List()
{
	p = new ListPrivate();
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

List *List::setItemHeight(int height)
{
	wz_list_set_item_height((wzList *)p->getWidget(), height);
	return this;
}

List *List::setDrawItemCallback(wzDrawListItemCallback callback)
{
	wz_list_set_draw_item_callback((wzList *)p->getWidget(), callback);
	return this;
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

void MainWindow::draw()
{	
	wz_main_window_draw(p->mainWindow);
}

void MainWindow::drawFrame()
{	
	wz_main_window_draw_frame(p->mainWindow);
}

void MainWindow::toggleTextCursor()
{
	wz_main_window_toggle_text_cursor(p->mainWindow);
}

wzCursor MainWindow::getCursor() const
{
	return wz_main_window_get_cursor(p->mainWindow);
}

Widget *MainWindow::add(Widget *widget)
{
	wz_main_window_add(p->mainWindow, widget->p->getWidget());
	return widget;
}

void MainWindow::remove(Widget *widget)
{
	wz_main_window_remove(p->mainWindow, widget->p->getWidget());
}

void MainWindow::createMenuButton(const std::string &label)
{
	wzMenuBarButton *button = wz_menu_bar_create_button(p->menuBar);
	wz_menu_bar_button_set_label(button, label.c_str());
}

void MainWindow::dockWindow(Window *window, wzDockPosition dockPosition)
{
	wz_main_window_dock_window(p->mainWindow, (wzWindow *)window->p->getWidget(), dockPosition);
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

//------------------------------------------------------------------------------

Scroller::Scroller()
{
	p = new ScrollerPrivate();
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

Spinner::Spinner()
{
	p = new SpinnerPrivate();
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
	wz_stack_layout_add((wzStackLayout *)p->getWidget(), widget->p->getWidget());
	return widget;
}

void StackLayout::remove(Widget *widget)
{
	wz_stack_layout_remove((wzStackLayout *)p->getWidget(), widget->p->getWidget());
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
	wz_tab_page_add(p->page, widget->p->getWidget());
	return widget;
}

void Tab::remove(Widget *widget)
{
	wz_tab_page_remove(p->page, widget->p->getWidget());
}

//------------------------------------------------------------------------------

Tabbed::Tabbed()
{
	p = new TabbedPrivate();
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

TextEdit::TextEdit(bool multiline)
{
	p = new TextEditPrivate(multiline);
}

TextEdit::TextEdit(const std::string &text, bool multiline)
{
	p = new TextEditPrivate(multiline);
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

Window::Window()
{
	p = new WindowPrivate();
}

Window::Window(const std::string &title)
{
	p = new WindowPrivate();
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
	wz_window_add((wzWindow *)p->getWidget(), widget->p->getWidget());
	return widget;
}

void Window::remove(Widget *widget)
{
	wz_window_remove((wzWindow *)p->getWidget(), widget->p->getWidget());
}

#endif // WZ_CPP_IMPLEMENTATION

} // namespace wz
