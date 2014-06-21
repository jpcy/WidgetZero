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

class List;
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

class Desktop : public Widget
{
public:
	Desktop(wzRenderer *renderer);
	~Desktop();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)desktop_; }
	virtual wzWidget *getWidget() { return (wzWidget *)desktop_; }
	virtual wzWidget *getContentWidget() { return wz_desktop_get_content_widget(desktop_); }
	void setSize(int w, int h);
	void mouseMove(int x, int y, int dx, int dy);
	void mouseButtonDown(int button, int x, int y);
	void mouseButtonUp(int button, int x, int y);
	void mouseWheelMove(int x, int y);
	void draw();
	void drawDockIcon(wzRect rect);
	void drawDockPreview(wzRect rect);

	static Desktop *fromWidget(wzWidget *widget)
	{
		return (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	}

private:
	wzDesktop *desktop_;
	DockTabBar *dockTabBars_[WZ_NUM_DOCK_POSITIONS];
};

class Window : public Widget
{
public:
	Window(Widget *parent, const std::string &title);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)window_; }
	virtual wzWidget *getWidget() { return (wzWidget *)window_; }
	virtual wzWidget *getContentWidget() { return wz_window_get_content_widget(window_); }
	std::string getTitle() const { return title_; }
	void draw(wzRect clip);
	
private:
	wzWindow *window_;
	std::string title_;
};

class StackLayout : public Widget
{
public:
	enum Direction
	{
		Horizontal,
		Vertical
	};

	StackLayout(Widget *parent, Direction direction);
	virtual const wzWidget *getWidget() const { return layout_; }
	virtual wzWidget *getWidget() { return layout_; }
	
private:
	wzWidget *layout_;
};

class Button : public Widget
{
public:
	Button(Widget *parent, const std::string &label);
	Button(wzButton *button, const std::string &label);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button_; }
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);

private:
	void initialize();

	wzButton *button_;
	std::string label_;
};

class Checkbox : public Widget
{
public:
	Checkbox(Widget *parent, const std::string &label);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button_; }
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);

private:
	wzButton *button_;
	std::string label_;

	static const int boxSize = 16;
	static const int boxRightMargin = 8;
};

class Combo : public Widget
{
public:
	Combo(Widget *parent, const char **items, int nItems);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)combo_; }
	virtual wzWidget *getWidget() { return (wzWidget *)combo_; }
	void draw(wzRect clip);

private:
	wzCombo *combo_;
	const char **items_;
	std::auto_ptr<List> list_;
};

class GroupBox : public Widget
{
public:
	GroupBox(Widget *parent, const std::string &label);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)frame_; }
	virtual wzWidget *getWidget() { return (wzWidget *)frame_; }
	virtual wzWidget *getContentWidget() { return wz_frame_get_content_widget(frame_); }
	void draw(wzRect clip);

private:
	wzFrame *frame_;
	std::string label_;
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
	RadioButton(Widget *parent, const std::string &label, RadioButtonGroup *group);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button_; }
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);

private:
	wzButton *button_;
	std::string label_;
};

class Scroller : public Widget
{
public:
	Scroller(Widget *parent, wzScrollerType type, int value, int stepValue, int maxValue);
	Scroller(wzScroller *scroller);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)scroller_; }
	virtual wzWidget *getWidget() { return (wzWidget *)scroller_; }
	void draw(wzRect clip);
	int getValue() const;

private:
	void initialize();

	wzScroller *scroller_;
	std::auto_ptr<Button> decrementButton;
	std::auto_ptr<Button> incrementButton;
};

class Label : public Widget
{
public:
	Label(Widget *parent);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)label_; }
	virtual wzWidget *getWidget() { return (wzWidget *)label_; }
	void setText(const char *format, ...);
	void setTextColor(uint8_t r, uint8_t g, uint8_t b);
	void draw(wzRect clip);

private:
	wzLabel *label_;
	std::string text_;
	uint8_t r_, g_, b_;
};

class List : public Widget
{
public:
	List(Widget *parent, const char **items, int nItems);
	List(wzList *list, const char **items, int nItems);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)list_; }
	virtual wzWidget *getWidget() { return (wzWidget *)list_; }
	void draw(wzRect clip);

private:
	void initialize(int nItems);

	wzList *list_;
	const char **items_;
	std::auto_ptr<Scroller> scroller_;

	static const int itemsMargin = 2;
	static const int itemHeight = 18;
	static const int itemLeftPadding = 4;
};

class TabButton : public Widget
{
public:
	TabButton(wzButton *button, const std::string &label);
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button_; }
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);

private:
	wzButton *button_;
	std::string label_;
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
	std::auto_ptr<Button> decrementButton;
	std::auto_ptr<Button> incrementButton;
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
	void addTab(const std::string &label);
	void addTab(wzButton *button, const std::string &label);

private:
	void initialize();

	wzTabBar *tabBar_;
	std::vector<TabButton *> tabs_;
	std::auto_ptr<Button> decrementButton;
	std::auto_ptr<Button> incrementButton;
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

class Tabbed : public Widget
{
public:
	Tabbed(Widget *parent);
	~Tabbed();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabbed_; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabbed_; }
	void draw(wzRect clip);
	TabPage *addTab(const std::string &label);

private:
	wzTabbed *tabbed_;
	std::auto_ptr<TabBar> tabBar_;
	std::vector<TabPage *> tabs_;
};

} // namespace wz
