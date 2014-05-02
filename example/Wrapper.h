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

class Window
{
public:
	Window(Context *context);
	~Window();
	wzWindow *get() const { return window_; }
	wzContext *getContext() { return wz_widget_get_context((struct wzWidget *)window_); }
	void mouseMove(int x, int y, int dx, int dy);
	void mouseButtonDown(int button, int x, int y);
	void mouseButtonUp(int button, int x, int y);
	void draw();

private:
	wzWindow *window_;
};

class Button
{
public:
	Button(Window *window, const char *label);
	Button(wzButton *button, const char *label);
	void setPosition(int x, int y);
	wzRect getRect();
	void draw();

private:
	wzButton *button_;
	char label_[64];
};

class Checkbox
{
public:
	Checkbox(Window *window, const char *label);
	void setPosition(int x, int y);
	void draw();

private:
	wzButton *button_;
	char label_[64];

	static const int boxSize = 16;
	static const int boxRightMargin = 8;
};

class GroupBox
{
public:
	GroupBox(Window *window, const char *label);
	void setPosition(int x, int y);
	void draw();

private:
	wzGroupBox *groupBox_;
	char label_[64];
};

class Scroller
{
public:
	Scroller(Window *window, wzScrollerType type, int value, int stepValue, int maxValue);
	Scroller(wzScroller *scroller);
	void setRect(int x, int y, int w, int h);
	void draw();
	int getValue() const;
	wzScroller *get() { return scroller_; }

private:
	wzScroller *scroller_;
	std::auto_ptr<Button> decrementButton;
	std::auto_ptr<Button> incrementButton;
};

class List
{
public:
	List(Window *window, char **items, int nItems);
	void setRect(int x, int y, int w, int h);
	void draw();

private:
	wzList *list_;
	char **items_;
	std::auto_ptr<Scroller> scroller_;

	static const int itemsMargin = 2;
	static const int itemHeight = 18;
	static const int itemLeftPadding = 4;
};
