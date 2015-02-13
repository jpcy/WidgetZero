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

#include "wz.h"
#include "wz_skin.h"

namespace wz {

enum
{
	WZ_SIDE_TOP = 1<<0,
	WZ_SIDE_RIGHT = 1<<1,
	WZ_SIDE_BOTTOM = 1<<2,
	WZ_SIDE_LEFT = 1<<3,
	WZ_SIDE_ALL = WZ_SIDE_TOP | WZ_SIDE_RIGHT | WZ_SIDE_BOTTOM | WZ_SIDE_LEFT,

	WZ_CORNER_TL = 1<<0,
	WZ_CORNER_TR = 1<<1,
	WZ_CORNER_BR = 1<<2,
	WZ_CORNER_BL = 1<<3,
	WZ_CORNER_ALL = WZ_CORNER_TL | WZ_CORNER_TR | WZ_CORNER_BR | WZ_CORNER_BL
};

typedef NVGcontext *(*wzNanoVgGlCreate)(int flags);
typedef void (*wzNanoVgGlDestroy)(NVGcontext* ctx);

struct NVGRendererImpl;

class NVGRenderer : public IRenderer
{
public:
	NVGRenderer(wzNanoVgGlCreate create, wzNanoVgGlDestroy destroy, int flags, const char *fontDirectory, const char *defaultFontFace, float defaultFontSize);
	~NVGRenderer();
	virtual const char *getError();
	virtual int getLineHeight(const char *fontFace, float fontSize);

	// width or height can be NULL.
	virtual void measureText(const char *fontFace, float fontSize, const char *text, int n, int *width, int *height);

	virtual LineBreakResult lineBreakText(const char *fontFace, float fontSize, const char *text, int n, int lineWidth);
	virtual void drawButton(Button *button, Rect clip);
	virtual Size measureButton(Button *button);
	virtual void drawCheckBox(CheckBox *checkBox, Rect clip);
	virtual Size measureCheckBox(CheckBox *checkBox);
	virtual void drawCombo(Combo *combo, Rect clip);
	virtual Size measureCombo(Combo *combo);
	virtual void drawGroupBox(GroupBox *groupBox, Rect clip);
	virtual Size measureGroupBox(GroupBox *groupBox);
	virtual void drawLabel(Label *label, Rect clip);
	virtual Size measureLabel(Label *label);
	virtual void drawList(List *list, Rect clip);
	virtual Size measureList(List *list);
	virtual void drawMenuBarButton(MenuBarButton *button, Rect clip);
	virtual Size measureMenuBarButton(MenuBarButton *button);
	virtual void drawMenuBar(MenuBar *menuBar, Rect clip);
	virtual Size measureMenuBar(MenuBar *menuBar);
	virtual void drawRadioButton(RadioButton *button, Rect clip);
	virtual Size measureRadioButton(RadioButton *button);
	virtual void drawScroller(Scroller *scroller, Rect clip);
	virtual Size measureScroller(Scroller *scroller);
	virtual void drawSpinner(Spinner *spinner, Rect clip);
	virtual Size measureSpinner(Spinner *spinner);
	virtual void drawTabButton(TabButton *button, Rect clip);
	virtual void drawTabBar(TabBar *tabBar, Rect clip);
	virtual Size measureTabBar(TabBar *tabBar);
	virtual void drawTabbed(Tabbed *tabbed, Rect clip);
	virtual Size measureTabbed(Tabbed *tabbed);
	virtual void drawTextEdit(TextEdit *textEdit, Rect clip);
	virtual Size measureTextEdit(TextEdit *textEdit);
	virtual void drawWindow(Window *window, Rect clip);
	virtual Size measureWindow(Window *window);

	NVGcontext *getContext();
	float getDefaultFontSize() const;
	int createImage(const char *filename, int *width, int *height);
	void setFontFace(const char *face);
	void printBox(Rect rect, const char *fontFace, float fontSize, NVGcolor color, const char *text, size_t textLength);
	void print(int x, int y, int align, const char *fontFace, float fontSize, NVGcolor color, const char *text, size_t textLength);
	void clipToRect(Rect rect);
	bool clipToRectIntersection(Rect rect1, Rect rect2);
	void drawFilledRect(Rect rect, NVGcolor color);
	void drawRect(Rect rect, NVGcolor color);
	void drawLine(int x1, int y1, int x2, int y2, NVGcolor color);
	void drawImage(Rect rect, int image);
	void createRectPath(Rect rect, float r, int sides, int roundedCorners);

private:
	std::auto_ptr<NVGRendererImpl> impl;
};

} // namespace wz
