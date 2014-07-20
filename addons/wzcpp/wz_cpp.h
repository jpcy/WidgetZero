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
#include <wz_renderer.h>

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

class Widget
{
public:
	virtual ~Widget();
	virtual wzRect getRect() const;
	virtual Widget *setPosition(int x, int y);
	virtual Widget *setWidth(int w);
	virtual Widget *setHeight(int h);
	virtual Widget *setSize(int w, int h);
	virtual Widget *setRect(int x, int y, int w, int h);
	virtual Widget *setStretch(int stretch);
	virtual Widget *setAlign(int align);
	virtual Widget *setMargin(int margin);
	virtual Widget *setMargin(int top, int right, int bottom, int left);
	virtual Widget *setMargin(wzBorder margin);
	virtual Widget *setFontFace(const std::string &fontFace);
	virtual Widget *setFontSize(float fontSize);
	virtual Widget *setFont(const std::string &fontFace, float fontSize);

	WidgetPrivate *p;
};

class Button : public Widget
{
public:
	Button(wzRenderer *renderer);
	Button(wzRenderer *renderer, const std::string &label);
	~Button();
	wzBorder getPadding() const;
	Button *setPadding(wzBorder padding);
	Button *setPadding(int top, int right, int bottom, int left);
	std::string getLabel() const;
	Button *setLabel(const std::string &label);
};

class Checkbox : public Widget
{
public:
	Checkbox(wzRenderer *renderer);
	Checkbox(wzRenderer *renderer, const std::string &label);
	~Checkbox();
	std::string getLabel() const;
	Checkbox *setLabel(const std::string &label);
};

class Combo : public Widget
{
public:
	Combo(wzRenderer *renderer);
	~Combo();
	virtual Widget *setFontFace(const std::string &fontFace);
	virtual Widget *setFontSize(float fontSize);
	virtual Widget *setFont(const std::string &fontFace, float fontSize);
	Combo *setItems(const char **items, int nItems);
};

class GroupBox : public Widget
{
public:
	GroupBox(wzRenderer *renderer);
	GroupBox(wzRenderer *renderer, const std::string &label);
	~GroupBox();
	std::string getLabel() const;
	GroupBox *setLabel(const std::string &label);
	Widget *add(Widget *widget);
};

class Label : public Widget
{
public:
	Label(wzRenderer *renderer);
	Label(wzRenderer *renderer, const std::string &text);
	~Label();
	Label *setText(const char *format, ...);
	Label *setTextColor(uint8_t r, uint8_t g, uint8_t b);
};

class List : public Widget
{
public:
	List(wzRenderer *renderer);
	~List();
	virtual Widget *setFontFace(const std::string &fontFace);
	virtual Widget *setFontSize(float fontSize);
	virtual Widget *setFont(const std::string &fontFace, float fontSize);
	List *setItems(const char **items, int nItems);
};

class MainWindow
{
public:
	MainWindow(wzRenderer *renderer);
	~MainWindow();
	void setSize(int w, int h);
	void mouseMove(int x, int y, int dx, int dy);
	void mouseButtonDown(int button, int x, int y);
	void mouseButtonUp(int button, int x, int y);
	void mouseWheelMove(int x, int y);
	void keyDown(wzKey key);
	void keyUp(wzKey key);
	void textInput(const char *text);
	void setShowCursor(bool showCursor);
	void beginFrame();
	void drawFrame();
	void endFrame();
	bool getShowCursor() const;
	wzCursor getCursor() const;
	Widget *add(Widget *widget);
	void dockWindow(Window *window, wzDockPosition dockPosition);

	MainWindowPrivate *p;
};

class RadioButtonGroup
{
public:
	RadioButtonGroup();
	~RadioButtonGroup();
	wzRadioButtonGroup *get() { return group_; }

private:
	wzRadioButtonGroup *group_;
};

class RadioButton : public Widget
{
public:
	RadioButton(wzRenderer *renderer);
	RadioButton(wzRenderer *renderer, const std::string &label);
	~RadioButton();
	std::string getLabel() const;
	RadioButton *setLabel(const std::string &label);

	// NULL to remove the radio button from it's group.
	RadioButton *setGroup(RadioButtonGroup *group);
};

class Scroller : public Widget
{
public:
	Scroller(wzRenderer *renderer);
	~Scroller();
	Scroller *setType(wzScrollerType type);
	Scroller *setValue(int value);
	Scroller *setStepValue(int stepValue);
	Scroller *setMaxValue(int maxValue);	
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
};

class Tab
{
public:
	Tab();
	~Tab();
	Tab *setLabel(const std::string &label);
	Widget *add(Widget *widget);

	TabPrivate *p;
};

class Tabbed : public Widget
{
public:
	Tabbed(wzRenderer *renderer);
	~Tabbed();
	Tab *addTab(Tab *tab);
};

class TextEdit : public Widget
{
public:
	TextEdit(wzRenderer *renderer);
	TextEdit(wzRenderer *renderer, const std::string &text);
	~TextEdit();
	TextEdit *setText(const std::string &text);
};

class Window : public Widget
{
public:
	Window(wzRenderer *renderer);
	Window(wzRenderer *renderer, const std::string &title);
	~Window();
	virtual Widget *setFontFace(const std::string &fontFace);
	virtual Widget *setFontSize(float fontSize);
	virtual Widget *setFont(const std::string &fontFace, float fontSize);
	std::string getTitle() const;
	Window *setTitle(const std::string &title);
	Widget *add(Widget *widget);
};

} // namespace wz
