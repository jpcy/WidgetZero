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
#include <widgetzero/wz.h>
#include <widgetzero/wz_renderer.h>

namespace wz {

class List;

class Widget
{
public:
	virtual ~Widget() {}
	virtual wzWidget *getWidget() = 0;
	virtual void draw(wzRect clip) {};
	void setPosition(int x, int y);
	void setRect(int x, int y, int w, int h);
	
	wzRenderer *getRenderer()
	{
		return renderer_;
	}

protected:
	wzRenderer *renderer_;
};

class Desktop : public Widget
{
public:
	Desktop(wzRenderer *renderer);
	~Desktop();
	virtual wzWidget *getWidget() { return (wzWidget *)desktop_; }
	void setSize(int w, int h);
	void mouseMove(int x, int y, int dx, int dy);
	void mouseButtonDown(int button, int x, int y);
	void mouseButtonUp(int button, int x, int y);
	void mouseWheelMove(int x, int y);
	void draw();
	void drawDockIcon(wzRect rect);
	void drawDockPreview(wzRect rect);

private:
	wzDesktop *desktop_;
};

class Window : public Widget
{
public:
	Window(Widget *parent, char *title);
	virtual wzWidget *getWidget() { return (wzWidget *)window_; }
	void draw(wzRect clip);
	
private:
	wzWindow *window_;
	char title_[64];
};

class Button : public Widget
{
public:
	Button(Widget *parent, const char *label);
	Button(wzButton *button, const char *label);
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	wzRect getRect();
	void draw(wzRect clip);

private:
	wzButton *button_;
	char label_[64];
};

class Checkbox : public Widget
{
public:
	Checkbox(Widget *parent, const char *label);
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);

private:
	wzButton *button_;
	char label_[64];

	static const int boxSize = 16;
	static const int boxRightMargin = 8;
};

class Combo : public Widget
{
public:
	Combo(Widget *parent, const char **items, int nItems);
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
	GroupBox(Widget *parent, const char *label);
	virtual wzWidget *getWidget() { return (wzWidget *)groupBox_; }
	void draw(wzRect clip);

private:
	wzGroupBox *groupBox_;
	char label_[64];
};

class Scroller : public Widget
{
public:
	Scroller(Widget *parent, wzScrollerType type, int value, int stepValue, int maxValue);
	Scroller(wzScroller *scroller);
	virtual wzWidget *getWidget() { return (wzWidget *)scroller_; }
	void draw(wzRect clip);
	int getValue() const;

private:
	wzScroller *scroller_;
	std::auto_ptr<Button> decrementButton;
	std::auto_ptr<Button> incrementButton;
};

class Label : public Widget
{
public:
	Label(Widget *parent);
	virtual wzWidget *getWidget() { return (wzWidget *)label_; }
	void setText(const char *format, ...);
	void setTextColor(uint8_t r, uint8_t g, uint8_t b);
	void draw(wzRect clip);

private:
	wzLabel *label_;
	char text_[256];
	uint8_t r, g, b;
};

class List : public Widget
{
public:
	List(Widget *parent, const char **items, int nItems);
	List(wzList *list, const char **items, int nItems);
	virtual wzWidget *getWidget() { return (wzWidget *)list_; }
	void draw(wzRect clip);

private:
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
	TabButton(wzButton *button, const char *label);
	virtual wzWidget *getWidget() { return (wzWidget *)button_; }
	void draw(wzRect clip);

private:
	wzButton *button_;
	char label_[64];
};

class TabBar : public Widget
{
public:
	TabBar(Widget *parent);
	TabBar(wzTabBar *tabBar);
	~TabBar();
	virtual wzWidget *getWidget() { return (wzWidget *)tabBar_; }
	void draw(wzRect clip);
	void addTab(const char *label);
	void addTab(wzButton *button, const char *label);

private:
	wzTabBar *tabBar_;
	std::vector<TabButton *> tabs_;
	std::auto_ptr<Button> decrementButton;
	std::auto_ptr<Button> incrementButton;
};

class TabPage : public Widget
{
public:
	TabPage(wzWidget *widget);
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
	virtual wzWidget *getWidget() { return (wzWidget *)tabbed_; }
	void draw(wzRect clip);
	TabPage *addTab(const char *label);

private:
	wzTabbed *tabbed_;
	std::auto_ptr<TabBar> tabBar_;
	std::vector<TabPage *> tabs_;
};

} // namespace wz
