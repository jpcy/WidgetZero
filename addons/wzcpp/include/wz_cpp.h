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
class ButtonInternal;
class Checkbox;
class CheckboxInternal;
class Combo;
class ComboInternal;
class Desktop;
class DesktopInternal;
class GroupBox;
class GroupBoxInternal;
class Label;
class LabelInternal;
class List;
class ListInternal;
class RadioButton;
class RadioButtonInternal;
class Scroller;
class ScrollerInternal;
class StackLayout;
class StackLayoutInternal;
class Tab;
class TabInternal;
class Tabbed;
class TabbedInternal;
class TextEdit;
class TextEditInternal;
class Window;
class WindowInternal;

class Desktop
{
public:
	Desktop(wzRenderer *renderer);
	~Desktop();
	void setSize(int w, int h);
	void mouseMove(int x, int y, int dx, int dy);
	void mouseButtonDown(int button, int x, int y);
	void mouseButtonUp(int button, int x, int y);
	void mouseWheelMove(int x, int y);
	void keyDown(wzKey key);
	void keyUp(wzKey key);
	void textInput(const char *text);
	void setShowCursor(bool showCursor);
	void draw();
	bool getShowCursor() const;
	wzCursor getCursor() const;

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

	DesktopInternal *internal_;
};

class Window
{
public:
	std::string getTitle() const;
	Window setTitle(const std::string &title);

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

	wzRect getRect() const;
	Window setPosition(int x, int y);
	Window setSize(int w, int h);
	Window setRect(int x, int y, int w, int h);
	Window setAutosize(int autosize);
	Window setStretch(int stretch);
	Window setAlign(int align);
	Window setMargin(int margin);
	Window setMargin(int top, int right, int bottom, int left);
	Window setMargin(wzBorder margin);

	WindowInternal *internal_;
};

class Button
{
public:
	std::string getLabel() const;
	Button setLabel(const std::string &label);
	wzRect getRect() const;
	Button setPosition(int x, int y);
	Button setSize(int w, int h);
	Button setRect(int x, int y, int w, int h);
	Button setAutosize(int autosize);
	Button setStretch(int stretch);
	Button setAlign(int align);
	Button setMargin(int margin);
	Button setMargin(int top, int right, int bottom, int left);
	Button setMargin(wzBorder margin);

	ButtonInternal *internal_;
};

class Checkbox
{
public:
	std::string getLabel() const;
	Checkbox setLabel(const std::string &label);
	wzRect getRect() const;
	Checkbox setPosition(int x, int y);
	Checkbox setSize(int w, int h);
	Checkbox setRect(int x, int y, int w, int h);
	Checkbox setAutosize(int autosize);
	Checkbox setStretch(int stretch);
	Checkbox setAlign(int align);
	Checkbox setMargin(int margin);
	Checkbox setMargin(int top, int right, int bottom, int left);
	Checkbox setMargin(wzBorder margin);

	CheckboxInternal *internal_;
};

class Combo
{
public:
	Combo setItems(const char **items, int nItems);
	wzRect getRect() const;
	Combo setPosition(int x, int y);
	Combo setSize(int w, int h);
	Combo setRect(int x, int y, int w, int h);
	Combo setAutosize(int autosize);
	Combo setStretch(int stretch);
	Combo setAlign(int align);
	Combo setMargin(int margin);
	Combo setMargin(int top, int right, int bottom, int left);
	Combo setMargin(wzBorder margin);

	ComboInternal *internal_;
};

class GroupBox
{
public:
	std::string getLabel() const;
	GroupBox setLabel(const std::string &label);

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

	wzRect getRect() const;
	GroupBox setPosition(int x, int y);
	GroupBox setSize(int w, int h);
	GroupBox setRect(int x, int y, int w, int h);
	GroupBox setAutosize(int autosize);
	GroupBox setStretch(int stretch);
	GroupBox setAlign(int align);
	GroupBox setMargin(int margin);
	GroupBox setMargin(int top, int right, int bottom, int left);
	GroupBox setMargin(wzBorder margin);

	GroupBoxInternal *internal_;
};

class Label
{
public:
	Label setText(const char *format, ...);
	Label setTextColor(uint8_t r, uint8_t g, uint8_t b);
	wzRect getRect() const;
	Label setPosition(int x, int y);
	Label setSize(int w, int h);
	Label setRect(int x, int y, int w, int h);
	Label setAutosize(int autosize);
	Label setStretch(int stretch);
	Label setAlign(int align);
	Label setMargin(int margin);
	Label setMargin(int top, int right, int bottom, int left);
	Label setMargin(wzBorder margin);

	LabelInternal *internal_;
};

class List
{
public:
	List setItems(const char **items, int nItems);
	wzRect getRect() const;
	List setPosition(int x, int y);
	List setSize(int w, int h);
	List setRect(int x, int y, int w, int h);
	List setAutosize(int autosize);
	List setStretch(int stretch);
	List setAlign(int align);
	List setMargin(int margin);
	List setMargin(int top, int right, int bottom, int left);
	List setMargin(wzBorder margin);

	ListInternal *internal_;
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

class RadioButton
{
public:
	std::string getLabel() const;
	RadioButton setLabel(const std::string &label);

	// NULL to remove the radio button from it's group.
	RadioButton setGroup(RadioButtonGroup *group);

	wzRect getRect() const;
	RadioButton setPosition(int x, int y);
	RadioButton setSize(int w, int h);
	RadioButton setRect(int x, int y, int w, int h);
	RadioButton setAutosize(int autosize);
	RadioButton setStretch(int stretch);
	RadioButton setAlign(int align);
	RadioButton setMargin(int margin);
	RadioButton setMargin(int top, int right, int bottom, int left);
	RadioButton setMargin(wzBorder margin);

	RadioButtonInternal *internal_;
};

class Scroller
{
public:
	Scroller setType(wzScrollerType type);
	Scroller setValue(int value);
	Scroller setStepValue(int stepValue);
	Scroller setMaxValue(int maxValue);	
	int getValue() const;
	wzRect getRect() const;
	Scroller setPosition(int x, int y);
	Scroller setSize(int w, int h);
	Scroller setRect(int x, int y, int w, int h);
	Scroller setAutosize(int autosize);
	Scroller setStretch(int stretch);
	Scroller setAlign(int align);
	Scroller setMargin(int margin);
	Scroller setMargin(int top, int right, int bottom, int left);
	Scroller setMargin(wzBorder margin);

	ScrollerInternal *internal_;
};

class StackLayout
{
public:
	StackLayout setDirection(wzStackLayoutDirection direction);
	StackLayout setSpacing(int spacing);
	int getSpacing() const;

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

	wzRect getRect() const;
	StackLayout setPosition(int x, int y);
	StackLayout setSize(int w, int h);
	StackLayout setRect(int x, int y, int w, int h);
	StackLayout setAutosize(int autosize);
	StackLayout setStretch(int stretch);
	StackLayout setAlign(int align);
	StackLayout setMargin(int margin);
	StackLayout setMargin(int top, int right, int bottom, int left);
	StackLayout setMargin(wzBorder margin);

	StackLayoutInternal *internal_;
};

class Tab
{
public:
	Tab setLabel(const std::string &label);

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

	wzRect getRect() const;
	Tab setPosition(int x, int y);
	Tab setSize(int w, int h);
	Tab setRect(int x, int y, int w, int h);
	Tab setAutosize(int autosize);
	Tab setStretch(int stretch);
	Tab setAlign(int align);
	Tab setMargin(int margin);
	Tab setMargin(int top, int right, int bottom, int left);
	Tab setMargin(wzBorder margin);

	TabInternal *internal_;
};

class Tabbed
{
public:
	Tab createTab();

	wzRect getRect() const;
	Tabbed setPosition(int x, int y);
	Tabbed setSize(int w, int h);
	Tabbed setRect(int x, int y, int w, int h);
	Tabbed setAutosize(int autosize);
	Tabbed setStretch(int stretch);
	Tabbed setAlign(int align);
	Tabbed setMargin(int margin);
	Tabbed setMargin(int top, int right, int bottom, int left);
	Tabbed setMargin(wzBorder margin);

	TabbedInternal *internal_;
};

class TextEdit
{
public:
	TextEdit setText(const std::string &text);
	wzRect getRect() const;
	TextEdit setPosition(int x, int y);
	TextEdit setSize(int w, int h);
	TextEdit setRect(int x, int y, int w, int h);
	TextEdit setAutosize(int autosize);
	TextEdit setStretch(int stretch);
	TextEdit setAlign(int align);
	TextEdit setMargin(int margin);
	TextEdit setMargin(int top, int right, int bottom, int left);
	TextEdit setMargin(wzBorder margin);

	TextEditInternal *internal_;
};

} // namespace wz
