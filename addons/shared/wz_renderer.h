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
#ifndef _WZ_RENDERER_H_
#define _WZ_RENDERER_H_

#include <wz.h>

struct wzRenderer
{
	void (*begin_frame)(struct wzRenderer *renderer, const struct wzMainWindow *mainWindow);
	void (*end_frame)(struct wzRenderer *renderer);

	int (*get_line_height)(struct wzRenderer *renderer, const char *fontFace, float fontSize);

	// width or height can be NULL.
	void (*measure_text)(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int *width, int *height);

	wzLineBreakResult (*line_break_text)(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int lineWidth);

	void (*debug_draw_text)(struct wzRenderer *renderer, const char *text, int x, int y);

	void (*draw_dock_icon)(struct wzRenderer *renderer, wzRect rect);
	void (*draw_dock_preview)(struct wzRenderer *renderer, wzRect rect);

	wzSize (*measure_button)(struct wzRenderer *renderer, wzBorder padding, const char *icon, const char *fontFace, float fontSize, const char *label);
	void (*draw_button)(struct wzRenderer *renderer, wzRect clip, struct wzButton *button, wzBorder padding, const char *icon, const char *fontFace, float fontSize, const char *label);

	wzSize (*measure_checkbox)(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *label);
	void (*draw_checkbox)(struct wzRenderer *renderer, wzRect clip, struct wzButton *checkbox, const char *fontFace, float fontSize, const char *label);

	wzSize (*measure_combo)(struct wzRenderer *renderer, const char *fontFace, float fontSize, uint8_t *itemData, size_t itemStride, int nItems);
	void (*draw_combo)(struct wzRenderer *renderer, wzRect clip, struct wzCombo *combo, const char *fontFace, float fontSize, const char *item);

	wzBorder (*measure_group_box_margin)(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *label);
	void (*draw_group_box)(struct wzRenderer *renderer, wzRect clip, struct wzFrame *frame, const char *fontFace, float fontSize, const char *label);

	wzSize (*measure_label)(struct wzRenderer *renderer, const struct wzLabel *label, const char *fontFace, float fontSize, bool multiline, const char *text);
	void (*draw_label)(struct wzRenderer *renderer, wzRect clip, struct wzLabel *label, const char *fontFace, float fontSize, bool multiline, const char *text, uint8_t r, uint8_t g, uint8_t b);

	wzBorder (*get_list_items_border)(struct wzRenderer *renderer, struct wzList *list);
	int (*measure_list_item_height)(struct wzRenderer *renderer, struct wzList *list, const char *fontFace, float fontSize);
	void (*draw_list)(struct wzRenderer *renderer, wzRect clip, struct wzList *list, const char *fontFace, float fontSize, uint8_t *itemData, size_t itemStride);

	wzSize (*measure_radio_button)(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *label);
	void (*draw_radio_button)(struct wzRenderer *renderer, wzRect clip, struct wzButton *button, const char *fontFace, float fontSize, const char *label);

	wzSize (*measure_scroller)(struct wzRenderer *renderer, wzScrollerType scrollerType);
	void (*draw_scroller)(struct wzRenderer *renderer, wzRect clip, struct wzScroller *scroller);

	void (*draw_tab_button)(struct wzRenderer *renderer, wzRect clip, struct wzButton *tabButton, wzBorder padding, const char *fontFace, float fontSize, const char *label);

	void (*draw_tab_page)(struct wzRenderer *renderer, wzRect clip, struct wzWidget *tabPage);

	wzBorder (*get_text_edit_border)(struct wzRenderer *renderer, const struct wzTextEdit *textEdit);
	wzSize (*measure_text_edit)(struct wzRenderer *renderer, const struct wzTextEdit *textEdit, const char *fontFace, float fontSize, const char *text);
	void (*draw_text_edit)(struct wzRenderer *renderer, wzRect clip, const struct wzTextEdit *textEdit, const char *fontFace, float fontSize, bool showCursor);

	int (*measure_window_header_height)(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *title);
	void (*draw_window)(struct wzRenderer *renderer, wzRect clip, struct wzWindow *window, const char *fontFace, float fontSize, const char *title);

	void *data;
};

#endif // _WIDGET_ZERO_RENDERER_H_
