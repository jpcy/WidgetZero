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
	void (*begin_frame)(struct wzRenderer *renderer, int windowWidth, int windowHeight);
	void (*end_frame)(struct wzRenderer *renderer);

	// width or height can be NULL.
	void (*measure_text)(struct wzRenderer *renderer, const char *text, int n, int *width, int *height);

	// Return the pixel delta between the index and index + 1 characters.
	int (*text_get_pixel_delta)(struct wzRenderer *renderer, const char *text, int index);

	void (*draw_dock_icon)(struct wzRenderer *renderer, wzRect rect);
	void (*draw_dock_preview)(struct wzRenderer *renderer, wzRect rect);

	int (*measure_window_header_height)(struct wzRenderer *renderer, const char *title);
	void (*draw_window)(struct wzRenderer *renderer, wzRect clip, struct wzWindow *window, const char *title);

	wzSize (*measure_button)(struct wzRenderer *renderer, const char *label);
	void (*draw_button)(struct wzRenderer *renderer, wzRect clip, struct wzButton *button, const char *label);

	wzSize (*measure_checkbox)(struct wzRenderer *renderer, const char *label);
	void (*draw_checkbox)(struct wzRenderer *renderer, wzRect clip, struct wzButton *checkbox, const char *label);

	wzSize (*measure_combo)(struct wzRenderer *renderer, const char **items, int nItems);
	void (*draw_combo)(struct wzRenderer *renderer, wzRect clip, struct wzCombo *combo, const char *item);

	wzBorder (*measure_group_box_margin)(struct wzRenderer *renderer, const char *label);
	void (*draw_group_box)(struct wzRenderer *renderer, wzRect clip, struct wzFrame *frame, const char *label);

	wzSize (*measure_radio_button)(struct wzRenderer *renderer, const char *label);
	void (*draw_radio_button)(struct wzRenderer *renderer, wzRect clip, struct wzButton *button, const char *label);

	void (*draw_scroller)(struct wzRenderer *renderer, wzRect clip, struct wzScroller *scroller);

	wzSize (*measure_label)(struct wzRenderer *renderer, const char *text);
	void (*draw_label)(struct wzRenderer *renderer, wzRect clip, struct wzLabel *label, const char *text, uint8_t r, uint8_t g, uint8_t b);

	void (*draw_list)(struct wzRenderer *renderer, wzRect clip, struct wzList *list, const char **items);
	void (*draw_tab_button)(struct wzRenderer *renderer, wzRect clip, struct wzButton *tabButton, const char *label);
	void (*draw_tab_page)(struct wzRenderer *renderer, wzRect clip, struct wzWidget *tabPage);

	wzSize (*measure_text_edit)(struct wzRenderer *renderer, wzBorder border, const char *text);
	void (*draw_text_edit)(struct wzRenderer *renderer, wzRect clip, const struct wzTextEdit *textEdit, bool showCursor);

	void (*debug_draw_text)(struct wzRenderer *renderer, const char *text, int x, int y);

	void *data;
};

#endif // _WIDGET_ZERO_RENDERER_H_
