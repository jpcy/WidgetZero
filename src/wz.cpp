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
#include "wz.h"
#pragma hdrstop

#define WZ_NOT_IMPLEMENTED { WZ_ASSERT("not implemented" && false); }
#define WZ_NOT_IMPLEMENTED_RETURN(type) { WZ_ASSERT("not implemented" && false); return type(); }

namespace wz {

IRenderer::~IRenderer() {}
Color IRenderer::getClearColor() { WZ_NOT_IMPLEMENTED_RETURN(Color) }
void IRenderer::beginFrame(int, int) {}
void IRenderer::endFrame() {}
void IRenderer::drawButton(Button *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureButton(Button *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawCheckBox(CheckBox *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureCheckBox(CheckBox *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawCombo(Combo *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureCombo(Combo *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawDockIcon(DockIcon *, Rect) { WZ_NOT_IMPLEMENTED }
void IRenderer::drawDockPreview(DockPreview *, Rect) { WZ_NOT_IMPLEMENTED }
Border IRenderer::getGroupBoxMargin(GroupBox *) { WZ_NOT_IMPLEMENTED_RETURN(Border) }
void IRenderer::drawGroupBox(GroupBox *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureGroupBox(GroupBox *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
Color IRenderer::getLabelTextColor(Label *) { WZ_NOT_IMPLEMENTED_RETURN(Color) }
void IRenderer::drawLabel(Label *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureLabel(Label *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawList(List *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureList(List *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawMenuBarButton(MenuBarButton *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureMenuBarButton(MenuBarButton *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
int IRenderer::getMenuBarPadding(MenuBar *) { WZ_NOT_IMPLEMENTED_RETURN(int) }
void IRenderer::drawMenuBar(MenuBar *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureMenuBar(MenuBar *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawRadioButton(RadioButton *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureRadioButton(RadioButton *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawScrollerDecrementButton(Button *, Rect) { WZ_NOT_IMPLEMENTED }
void IRenderer::drawScrollerIncrementButton(Button *, Rect) { WZ_NOT_IMPLEMENTED }
void IRenderer::drawScroller(Scroller *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureScroller(Scroller *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
int IRenderer::getSpinnerButtonWidth(Spinner *) { WZ_NOT_IMPLEMENTED_RETURN(int) }
void IRenderer::drawSpinnerDecrementButton(Button *, Rect) { WZ_NOT_IMPLEMENTED }
void IRenderer::drawSpinnerIncrementButton(Button *, Rect) { WZ_NOT_IMPLEMENTED }
void IRenderer::drawSpinner(Spinner *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureSpinner(Spinner *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawTabButton(TabButton *, Rect) { WZ_NOT_IMPLEMENTED }
int IRenderer::getTabBarScrollButtonWidth(TabBar *) { WZ_NOT_IMPLEMENTED_RETURN(int) }
void IRenderer::drawTabBarDecrementButton(Button *, Rect) { WZ_NOT_IMPLEMENTED }
void IRenderer::drawTabBarIncrementButton(Button *, Rect) { WZ_NOT_IMPLEMENTED }
void IRenderer::drawTabBar(TabBar *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureTabBar(TabBar *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawTabbed(Tabbed *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureTabbed(Tabbed *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawTextEdit(TextEdit *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureTextEdit(TextEdit *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
void IRenderer::drawWindow(Window *, Rect) { WZ_NOT_IMPLEMENTED }
Size IRenderer::measureWindow(Window *) { WZ_NOT_IMPLEMENTED_RETURN(Size) }
int IRenderer::getLineHeight(const char *, float) { WZ_NOT_IMPLEMENTED_RETURN(int) }
void IRenderer::measureText(const char *, float, const char *, int, int *, int *) { WZ_NOT_IMPLEMENTED }
LineBreakResult IRenderer::lineBreakText(const char *, float, const char *, int, int) { WZ_NOT_IMPLEMENTED_RETURN(LineBreakResult) }

/*
SDL_IntersectRect

  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
bool Rect::intersect(const Rect A, const Rect B, Rect *result)
{
    int Amin, Amax, Bmin, Bmax;

	WZ_ASSERT(result);

    /* Special cases for empty rects */
    if (A.isEmpty() || B.isEmpty())
	{
		result->x = result->y = result->w = result->h = 0;
        return false;
    }

    /* Horizontal intersection */
    Amin = A.x;
    Amax = Amin + A.w;
    Bmin = B.x;
    Bmax = Bmin + B.w;
    if (Bmin > Amin)
        Amin = Bmin;
    result->x = Amin;
    if (Bmax < Amax)
        Amax = Bmax;
    result->w = Amax - Amin;

    /* Vertical intersection */
    Amin = A.y;
    Amax = Amin + A.h;
    Bmin = B.y;
    Bmax = Bmin + B.h;
    if (Bmin > Amin)
        Amin = Bmin;
    result->y = Amin;
    if (Bmax < Amax)
        Amax = Bmax;
    result->h = Amax - Amin;

    return !result->isEmpty();
}

} // namespace wz
