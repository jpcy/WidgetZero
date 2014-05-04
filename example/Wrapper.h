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
#include <memory>
#include <widgetzero/wz.h>

class Context
{
public:
	Context();
	~Context();
	wzContext *get() const { return context_; }

private:
	wzContext *context_;
};

class Widget
{
public:
	virtual ~Widget() {}
	virtual wzWidget *getWidget() = 0;
	
	wzContext *getContext()
	{
		return wz_widget_get_context(getWidget());
	}
};

class Desktop : public Widget
{
public:
	Desktop(Context *context);
	~Desktop();
	virtual wzWidget *getWidget() { return (struct wzWidget *)desktop_; }
	void mouseMove(int x, int y, int dx, int dy);
	void mouseButtonDown(int button, int x, int y);
	void mouseButtonUp(int button, int x, int y);
	void draw();

private:
	wzDesktop *desktop_;
};

class Window : public Widget
{
public:
	Window(Widget *parent, char *title);
	virtual wzWidget *getWidget() { return (struct wzWidget *)window_; }
	void setRect(int x, int y, int w, int h);
	void draw();
	
private:
	wzWindow *window_;
	char title_[64];
};

class Button : public Widget
{
public:
	Button(Widget *parent, const char *label);
	Button(wzButton *button, const char *label);
	virtual wzWidget *getWidget() { return (struct wzWidget *)button_; }
	void setPosition(int x, int y);
	wzRect getRect();
	void draw();

private:
	wzButton *button_;
	char label_[64];
};

class Checkbox : public Widget
{
public:
	Checkbox(Widget *parent, const char *label);
	virtual wzWidget *getWidget() { return (struct wzWidget *)button_; }
	void setPosition(int x, int y);
	void draw();

private:
	wzButton *button_;
	char label_[64];

	static const int boxSize = 16;
	static const int boxRightMargin = 8;
};

class GroupBox : public Widget
{
public:
	GroupBox(Widget *parent, const char *label);
	virtual wzWidget *getWidget() { return (struct wzWidget *)groupBox_; }
	void setPosition(int x, int y);
	void draw();

private:
	wzGroupBox *groupBox_;
	char label_[64];
};

class Scroller : public Widget
{
public:
	Scroller(Widget *parent, wzScrollerType type, int value, int stepValue, int maxValue);
	Scroller(wzScroller *scroller);
	virtual wzWidget *getWidget() { return (struct wzWidget *)scroller_; }
	void setRect(int x, int y, int w, int h);
	void draw();
	int getValue() const;

private:
	wzScroller *scroller_;
	std::auto_ptr<Button> decrementButton;
	std::auto_ptr<Button> incrementButton;
};

class List : public Widget
{
public:
	List(Widget *parent, const char **items, int nItems);
	virtual wzWidget *getWidget() { return (struct wzWidget *)list_; }
	void setRect(int x, int y, int w, int h);
	void draw();

private:
	wzList *list_;
	const char **items_;
	std::auto_ptr<Scroller> scroller_;

	static const int itemsMargin = 2;
	static const int itemHeight = 18;
	static const int itemLeftPadding = 4;
};
