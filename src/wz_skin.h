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

#include <wz.h>

#define WZ_SKIN_TEXT_COLOR nvgRGBf(0.8f, 0.8f, 0.8f)
#define WZ_SKIN_DARK_BORDER_COLOR nvgRGB(25, 25, 25)
#define WZ_SKIN_LIGHT_BORDER_COLOR nvgRGBf(0.8f, 0.8f, 0.8f)
#define WZ_SKIN_HOVER_COLOR nvgRGBf(0.5f, 0.76f, 0.9f)
#define WZ_SKIN_CORNER_RADIUS 4.0f

#define WZ_SKIN_BUTTON_TEXT_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_BUTTON_BORDER_COLOR WZ_SKIN_DARK_BORDER_COLOR
#define WZ_SKIN_BUTTON_BORDER_HOVER_COLOR WZ_SKIN_HOVER_COLOR
#define WZ_SKIN_BUTTON_BG_COLOR1 nvgRGB(80, 80, 80)
#define WZ_SKIN_BUTTON_BG_COLOR2 nvgRGB(70, 70, 70)
#define WZ_SKIN_BUTTON_BG_PRESSED_COLOR1 nvgRGB(60, 60, 60)
#define WZ_SKIN_BUTTON_BG_PRESSED_COLOR2 nvgRGB(50, 50, 50)
#define WZ_SKIN_BUTTON_BG_SET_COLOR1 nvgRGB(150, 150, 150)
#define WZ_SKIN_BUTTON_BG_SET_COLOR2 nvgRGB(140, 140, 140)
#define WZ_SKIN_BUTTON_ICON_SPACING 6
#define WZ_SKIN_BUTTON_CORNER_RADIUS WZ_SKIN_CORNER_RADIUS

#define WZ_SKIN_CHECK_BOX_TEXT_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_CHECK_BOX_CHECK_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_CHECK_BOX_BORDER_COLOR WZ_SKIN_LIGHT_BORDER_COLOR
#define WZ_SKIN_CHECK_BOX_BORDER_HOVER_COLOR WZ_SKIN_HOVER_COLOR
#define WZ_SKIN_CHECK_BOX_BOX_SIZE 16
#define WZ_SKIN_CHECK_BOX_BOX_RIGHT_MARGIN 8
#define WZ_SKIN_CHECK_BOX_BOX_INTERNAL_MARGIN 4
#define WZ_SKIN_CHECK_BOX_CHECK_THICKNESS 2.5f

#define WZ_SKIN_COMBO_TEXT_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_COMBO_ICON_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_COMBO_BORDER_COLOR WZ_SKIN_DARK_BORDER_COLOR
#define WZ_SKIN_COMBO_BORDER_HOVER_COLOR WZ_SKIN_HOVER_COLOR
#define WZ_SKIN_COMBO_BG_COLOR1 nvgRGB(80, 80, 80)
#define WZ_SKIN_COMBO_BG_COLOR2 nvgRGB(70, 70, 70)
#define WZ_SKIN_COMBO_PADDING_X 20
#define WZ_SKIN_COMBO_PADDING_Y 8
#define WZ_SKIN_COMBO_BUTTON_WIDTH 24
#define WZ_SKIN_COMBO_ICON_WIDTH 8
#define WZ_SKIN_COMBO_ICON_HEIGHT 4
#define WZ_SKIN_COMBO_CORNER_RADIUS WZ_SKIN_CORNER_RADIUS

#define WZ_SKIN_GROUP_BOX_TEXT_COLOR nvgRGBf(1, 1, 1)
#define WZ_SKIN_GROUP_BOX_BORDER_COLOR WZ_SKIN_DARK_BORDER_COLOR
#define WZ_SKIN_GROUP_BOX_MARGIN 8
#define WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN 20
#define WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING 5
#define WZ_SKIN_GROUP_BOX_CORNER_RADIUS WZ_SKIN_CORNER_RADIUS

#define WZ_SKIN_LABEL_TEXT_COLOR WZ_SKIN_TEXT_COLOR

#define WZ_SKIN_LIST_TEXT_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_LIST_BORDER_COLOR WZ_SKIN_DARK_BORDER_COLOR
#define WZ_SKIN_LIST_BG_COLOR1 nvgRGB(40, 40, 40)
#define WZ_SKIN_LIST_BG_COLOR2 nvgRGB(50, 50, 50)
#define WZ_SKIN_LIST_SET_COLOR nvgRGB(40, 140, 190)
#define WZ_SKIN_LIST_HOVER_COLOR nvgRGB(60, 60, 80)
#define WZ_SKIN_LIST_ITEM_LEFT_PADDING 4

#define WZ_SKIN_MAIN_WINDOW_DOCK_PREVIEW_COLOR nvgRGBAf(0, 0, 1, 0.25f)

#define WZ_SKIN_MENU_BAR_TEXT_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_MENU_BAR_SET_COLOR nvgRGB(40, 140, 190)
#define WZ_SKIN_MENU_BAR_BORDER_HOVER_COLOR WZ_SKIN_HOVER_COLOR
#define WZ_SKIN_MENU_BAR_BG_COLOR nvgRGB(80, 80, 80)
#define WZ_SKIN_MENU_BAR_PADDING 6

#define WZ_SKIN_RADIO_BUTTON_TEXT_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_RADIO_BUTTON_SET_COLOR nvgRGBf(0.1529f, 0.5569f, 0.7412f)
#define WZ_SKIN_RADIO_BUTTON_BORDER_COLOR WZ_SKIN_LIGHT_BORDER_COLOR
#define WZ_SKIN_RADIO_BUTTON_BORDER_HOVER_COLOR WZ_SKIN_HOVER_COLOR
#define WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS 8
#define WZ_SKIN_RADIO_BUTTON_INNER_RADIUS 4
#define WZ_SKIN_RADIO_BUTTON_SPACING 8

#define WZ_SKIN_SCROLLER_ICON_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_SCROLLER_ICON_HOVER_COLOR WZ_SKIN_HOVER_COLOR
#define WZ_SKIN_SCROLLER_BORDER_COLOR WZ_SKIN_DARK_BORDER_COLOR
#define WZ_SKIN_SCROLLER_BORDER_HOVER_COLOR WZ_SKIN_HOVER_COLOR
#define WZ_SKIN_SCROLLER_BG_COLOR1 nvgRGB(80, 80, 80)
#define WZ_SKIN_SCROLLER_BG_COLOR2 nvgRGB(70, 70, 70)
#define WZ_SKIN_SCROLLER_BG_PRESSED_COLOR1 nvgRGB(60, 60, 60)
#define WZ_SKIN_SCROLLER_BG_PRESSED_COLOR2 nvgRGB(50, 50, 50)
#define WZ_SKIN_SCROLLER_NUB_ICON_MARGIN 4
#define WZ_SKIN_SCROLLER_NUB_ICON_SPACING 4
#define WZ_SKIN_SCROLLER_CORNER_RADIUS WZ_SKIN_CORNER_RADIUS
#define WZ_SKIN_SCROLLER_DEFAULT_SIZE 16 // Width for vertical, height for horizontal.
#define WZ_SKIN_SCROLLER_BUTTON_SIZE 16 // Height for vertical, width for horizontal.

#define WZ_SKIN_SPINNER_ICON_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_SPINNER_ICON_HOVER_COLOR WZ_SKIN_HOVER_COLOR
#define WZ_SKIN_SPINNER_BUTTON_WIDTH 16
#define WZ_SKIN_SPINNER_ICON_WIDTH 10
#define WZ_SKIN_SPINNER_ICON_HEIGHT 6

#define WZ_SKIN_TAB_BUTTON_TEXT_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_TAB_BUTTON_TEXT_HOVER_COLOR WZ_SKIN_HOVER_COLOR

#define WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH 14

#define WZ_SKIN_TABBED_BORDER_COLOR WZ_SKIN_DARK_BORDER_COLOR

#define WZ_SKIN_TEXT_EDIT_TEXT_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_TEXT_EDIT_BORDER_COLOR WZ_SKIN_DARK_BORDER_COLOR
#define WZ_SKIN_TEXT_EDIT_BORDER_HOVER_COLOR WZ_SKIN_HOVER_COLOR
#define WZ_SKIN_TEXT_EDIT_BG_COLOR1 nvgRGB(80, 80, 80)
#define WZ_SKIN_TEXT_EDIT_BG_COLOR2 nvgRGB(70, 70, 70)
#define WZ_SKIN_TEXT_EDIT_SELECTION_COLOR nvgRGBAf(0.1529f, 0.5569f, 0.7412f, 0.5f)
#define WZ_SKIN_TEXT_EDIT_CURSOR_COLOR nvgRGBf(1, 1, 1)
#define WZ_SKIN_TEXT_EDIT_CORNER_RADIUS WZ_SKIN_CORNER_RADIUS

#define WZ_SKIN_WINDOW_TEXT_COLOR WZ_SKIN_TEXT_COLOR
#define WZ_SKIN_WINDOW_BORDER_COLOR WZ_SKIN_DARK_BORDER_COLOR
#define WZ_SKIN_WINDOW_INNER_BORDER_COLOR nvgRGB(50, 50, 50)
#define WZ_SKIN_WINDOW_BG_COLOR1 nvgRGB(70, 70, 70)
#define WZ_SKIN_WINDOW_BG_COLOR2 nvgRGB(60, 60, 60)
#define WZ_SKIN_WINDOW_BORDER_BG_COLOR1 nvgRGB(52, 73, 94)
#define WZ_SKIN_WINDOW_BORDER_BG_COLOR2 nvgRGB(70, 98, 125)
#define WZ_SKIN_WINDOW_CORNER_RADIUS WZ_SKIN_CORNER_RADIUS
