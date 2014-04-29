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
#ifndef _WZ_INTERNAL_H_
#define _WZ_INTERNAL_H_

#include <widgetzero/wz.h>

#define WZ_MAX_WINDOW_WIDGETS 1024

struct wzContext
{
	int dummy;
};

typedef struct
{
	wzSize (*autosize)(struct wzWidget *);
	void (*draw)(struct wzWidget *);
	void (*mouse_button_down)(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY);
	void (*mouse_button_up)(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY);
}
wzWidgetVtable;

struct wzWidget
{
	wzWidgetType type;
	wzRect rect;
	void *metadata;
	struct wzWindow *window;
	bool hover;
	wzWidgetVtable vtable;
};

struct wzWindow
{
	struct wzContext *context;
	struct wzWidget *widgets[WZ_MAX_WINDOW_WIDGETS];
	size_t nWidgets;
};

void wz_window_add_widget(struct wzWindow *window, struct wzWidget *widget);

#endif
