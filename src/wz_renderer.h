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
#ifndef WZ_RENDERER_H
#define WZ_RENDERER_H

#include <wz.h>

struct wzRenderer
{
	int (*get_line_height)(struct wzRenderer *renderer, const char *fontFace, float fontSize);

	// width or height can be NULL.
	void (*measure_text)(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int *width, int *height);

	wzLineBreakResult (*line_break_text)(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int lineWidth);

	wzColor (*get_default_text_color)(struct wzRenderer *renderer);

	wzSize (*get_dock_icon_size)(struct wzRenderer *renderer);
	void (*draw_dock_icon)(struct wzRenderer *renderer, wzRect rect);
	void (*draw_dock_preview)(struct wzRenderer *renderer, wzRect rect);

	wzSize (*measure_combo)(struct wzRenderer *renderer, const struct wzCombo *combo);
	void (*draw_combo)(struct wzRenderer *renderer, wzRect clip, const struct wzCombo *combo);

	wzBorder (*measure_group_box_margin)(struct wzRenderer *renderer, const struct wzGroupBox *groupBox);
	void (*draw_group_box)(struct wzRenderer *renderer, wzRect clip, const struct wzGroupBox *groupBox);

	wzSize (*measure_label)(struct wzRenderer *renderer, const struct wzLabel *label);
	void (*draw_label)(struct wzRenderer *renderer, wzRect clip, const struct wzLabel *label);

	wzBorder (*get_list_items_border)(struct wzRenderer *renderer, const struct wzList *list);
	int (*measure_list_item_height)(struct wzRenderer *renderer, const struct wzList *list);
	void (*draw_list)(struct wzRenderer *renderer, wzRect clip, const struct wzList *list);

	void (*draw_main_window)(struct wzRenderer *renderer, const struct wzMainWindow *mainWindow);

	int (*calculate_menu_bar_height)(struct wzRenderer *renderer, const struct wzMenuBar *menuBar);
	void (*draw_menu_bar)(struct wzRenderer *renderer, wzRect clip, const struct wzMenuBar *menuBar);

	wzSize (*measure_menu_bar_button)(struct wzRenderer *renderer, const struct wzMenuBarButton *button);
	void (*draw_menu_bar_button)(struct wzRenderer *renderer, wzRect clip, const struct wzMenuBarButton *button);

	wzSize (*measure_scroller)(struct wzRenderer *renderer, wzScrollerType scrollerType);
	void (*draw_scroller)(struct wzRenderer *renderer, wzRect clip, const struct wzScroller *scroller);
	void (*draw_scroller_button)(struct wzRenderer *renderer, wzRect clip, const struct wzScroller *scroller, const struct wzButton *button, bool decrement);

	int (*get_spinner_button_width)(struct wzRenderer *renderer);
	wzSize (*measure_spinner)(struct wzRenderer *renderer, const struct wzSpinner *spinner, const struct wzTextEdit *textEdit);
	void (*draw_spinner)(struct wzRenderer *renderer, wzRect clip, const struct wzSpinner *spinner);
	void (*draw_spinner_button)(struct wzRenderer *renderer, wzRect clip, const struct wzSpinner *spinner, const struct wzButton *button, bool decrement);

	int (*get_tab_bar_height)(struct wzRenderer *renderer, const struct wzTabBar *tabBar);

	void (*draw_tab_page)(struct wzRenderer *renderer, wzRect clip, const struct wzWidget *tabPage);

	wzBorder (*get_text_edit_border)(struct wzRenderer *renderer, const struct wzTextEdit *textEdit);
	wzSize (*measure_text_edit)(struct wzRenderer *renderer, const struct wzTextEdit *textEdit);
	void (*draw_text_edit)(struct wzRenderer *renderer, wzRect clip, const struct wzTextEdit *textEdit);

	int (*get_window_border_size)(struct wzRenderer *renderer, const struct wzWindow *window);
	int (*measure_window_header_height)(struct wzRenderer *renderer, const struct wzWindow *window);
	void (*draw_window)(struct wzRenderer *renderer, wzRect clip, const struct wzWindow *window);

	void *data;
	struct NVGcontext *vg;
	wzRendererStyle style;
};

#endif // WZ_RENDERER_H
