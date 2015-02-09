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

typedef struct NVGcontext *(*wzNanoVgGlCreate)(int flags);
typedef void (*wzNanoVgGlDestroy)(struct NVGcontext* ctx);

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
	virtual void drawButton(ButtonImpl *button, Rect clip);
	virtual Size measureButton(ButtonImpl *button);
	virtual void drawCheckBox(CheckBoxImpl *checkBox, Rect clip);
	virtual Size measureCheckBox(CheckBoxImpl *checkBox);
	virtual void drawCombo(ComboImpl *combo, Rect clip);
	virtual Size measureCombo(ComboImpl *combo);
	virtual void drawGroupBox(GroupBoxImpl *groupBox, Rect clip);
	virtual Size measureGroupBox(GroupBoxImpl *groupBox);
	virtual void drawLabel(LabelImpl *label, Rect clip);
	virtual Size measureLabel(LabelImpl *label);
	virtual void drawList(ListImpl *list, Rect clip);
	virtual Size measureList(ListImpl *list);

	struct NVGcontext *getContext();
	float getDefaultFontSize() const;
	int createImage(const char *filename, int *width, int *height);
	void setFontFace(const char *face);
	void printBox(Rect rect, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength);
	void print(int x, int y, int align, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength);
	void clipToRect(Rect rect);
	bool clipToRectIntersection(Rect rect1, Rect rect2);
	void drawFilledRect(Rect rect, struct NVGcolor color);
	void drawRect(Rect rect, struct NVGcolor color);
	void drawLine(int x1, int y1, int x2, int y2, struct NVGcolor color);
	void drawImage(Rect rect, int image);
	void createRectPath(Rect rect, float r, int sides, int roundedCorners);

private:
	std::auto_ptr<NVGRendererImpl> impl;
};

} // namespace wz
