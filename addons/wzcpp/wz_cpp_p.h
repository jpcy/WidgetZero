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

#include "wz_cpp.h"

namespace wz {

class DockTabBar;
struct ListPrivate;

struct WidgetPrivate
{
	WidgetPrivate() : fontSize(0) {}
	virtual ~WidgetPrivate();
	virtual const wzWidget *getWidget() const { return NULL; }
	virtual wzWidget *getWidget() { return NULL; }
	virtual wzSize measure() { wzSize s; s.w = s.h = 0; return s; }
	virtual void draw(wzRect clip) {}
	virtual void handleEvent(wzEvent *e) {}

	void add(Widget *widget);
	void remove(Widget *widget);
	
	wzRenderer *renderer;
	std::vector<Widget *> children;
	std::vector<IEventHandler *> eventHandlers;
	std::string fontFace;
	float fontSize;
};

struct ButtonPrivate : public WidgetPrivate
{
	ButtonPrivate(wzRenderer *renderer);
	~ButtonPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	enum DrawStyle
	{
		Normal,
		Tab
	};

	wzButton *button;
	DrawStyle drawStyle;
	wzBorder padding;
	std::string icon;
	std::string label;
};

struct CheckboxPrivate : public WidgetPrivate
{
	CheckboxPrivate(wzRenderer *renderer);
	~CheckboxPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzButton *button;
	std::string label;
};

struct ComboPrivate : public WidgetPrivate
{
	ComboPrivate(wzRenderer *renderer);
	~ComboPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)combo; }
	virtual wzWidget *getWidget() { return (wzWidget *)combo; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzCombo *combo;
	uint8_t *itemData;
	size_t itemStride;
	std::auto_ptr<List> list;
};

struct FramePrivate : public WidgetPrivate
{
	FramePrivate(wzRenderer *renderer);
	~FramePrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)frame; }
	virtual wzWidget *getWidget() { return (wzWidget *)frame; }

	wzFrame *frame;
};

struct GroupBoxPrivate : public WidgetPrivate
{
	GroupBoxPrivate(wzRenderer *renderer);
	~GroupBoxPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)frame; }
	virtual wzWidget *getWidget() { return (wzWidget *)frame; }
	virtual void draw(wzRect clip);

	void refreshMargin();

	wzFrame *frame;
	std::string label;
};

struct LabelPrivate : public WidgetPrivate
{
	LabelPrivate(wzRenderer *renderer);
	~LabelPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)label; }
	virtual wzWidget *getWidget() { return (wzWidget *)label; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzLabel *label;
	bool multiline;
	std::string text;
	wzColor color;
};

struct ListPrivate : public WidgetPrivate
{
	ListPrivate(wzRenderer *renderer);
	~ListPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)list; }
	virtual wzWidget *getWidget() { return (wzWidget *)list; }
	virtual void draw(wzRect clip);

	// Called on init and when the font face or size changed.
	void refreshItemHeight();

	void setItems(uint8_t *itemData, size_t itemStride, int nItems);

	wzList *list;
	uint8_t *itemData;
	size_t itemStride;
	std::auto_ptr<Scroller> scroller;
	wzDrawListItemCallback drawItemCallback;
};

struct MainWindowPrivate : public WidgetPrivate
{
	MainWindowPrivate(wzRenderer *renderer);
	~MainWindowPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)mainWindow; }
	virtual wzWidget *getWidget() { return (wzWidget *)mainWindow; }
	void draw(wzRect clip);
	void drawDockIcon(wzRect rect);
	void drawDockPreview(wzRect rect);

	wzMainWindow *mainWindow;
	wzRenderer *renderer;
	DockTabBar *dockTabBars[WZ_NUM_DOCK_POSITIONS];
	bool showCursor;
};

struct RadioButtonPrivate : public WidgetPrivate
{
	RadioButtonPrivate(wzRenderer *renderer);
	~RadioButtonPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)button; }
	virtual wzWidget *getWidget() { return (wzWidget *)button; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzButton *button;
	std::string label;
	RadioButtonGroup *group;
};

struct ScrollerPrivate : public WidgetPrivate
{
	ScrollerPrivate(wzRenderer *renderer);
	~ScrollerPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)scroller; }
	virtual wzWidget *getWidget() { return (wzWidget *)scroller; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzScroller *scroller;
};

struct SpinnerPrivate : public WidgetPrivate
{
	SpinnerPrivate(wzRenderer *renderer);
	~SpinnerPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)spinner; }
	virtual wzWidget *getWidget() { return (wzWidget *)spinner; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzSpinner *spinner;
	std::auto_ptr<TextEdit> textEdit;
	std::auto_ptr<Button> decrementButton;
	std::auto_ptr<Button> incrementButton;
};

struct StackLayoutPrivate : public WidgetPrivate
{
	StackLayoutPrivate();
	~StackLayoutPrivate();
	virtual const wzWidget *getWidget() const { return (wzWidget *)layout; }
	virtual wzWidget *getWidget() { return (wzWidget *)layout; }
	
	wzStackLayout *layout;
};

class TabBar : public WidgetPrivate
{
public:
	TabBar(wzRenderer *renderer);
	~TabBar();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabBar_; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabBar_; }
	virtual void draw(wzRect clip);
	Button *createTab();

protected:
	wzTabBar *tabBar_;
	std::auto_ptr<Button> decrementButton_;
	std::auto_ptr<Button> incrementButton_;
};

class DockTabBar : public TabBar
{
public:
	DockTabBar(wzRenderer *renderer);
	virtual void handleEvent(wzEvent *e);
};

class TabPage : public WidgetPrivate
{
public:
	TabPage(wzRenderer *renderer);
	virtual const wzWidget *getWidget() const { return widget_; }
	virtual wzWidget *getWidget() { return widget_; }
	virtual void draw(wzRect clip);

private:
	wzWidget *widget_;
};

// Wraps tab button and page.
struct TabPrivate
{
	TabPrivate();
	~TabPrivate();

	Button *button;
	TabPage *page;
	std::vector<Widget *> children;
};

struct TabbedPrivate : public WidgetPrivate
{
	TabbedPrivate(wzRenderer *renderer);
	~TabbedPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)tabbed; }
	virtual wzWidget *getWidget() { return (wzWidget *)tabbed; }
	virtual void draw(wzRect clip);

	wzTabbed *tabbed;
	std::auto_ptr<TabBar> tabBar;
	std::vector<TabPage *> tabPages;
};

struct TextEditPrivate : public WidgetPrivate
{
	TextEditPrivate(wzRenderer *renderer, bool multiline);
	~TextEditPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)textEdit; }
	virtual wzWidget *getWidget() { return (wzWidget *)textEdit; }
	virtual wzSize measure();
	virtual void draw(wzRect clip);

	wzTextEdit *textEdit;
	std::auto_ptr<Scroller> scroller;
};

struct WindowPrivate : public WidgetPrivate
{
	WindowPrivate(wzRenderer *renderer);
	virtual ~WindowPrivate();
	virtual const wzWidget *getWidget() const { return (const wzWidget *)window; }
	virtual wzWidget *getWidget() { return (wzWidget *)window; }
	virtual void draw(wzRect clip);

	void refreshHeaderHeight();

	wzWindow *window;
	std::string title;
};

} // namespace wz
