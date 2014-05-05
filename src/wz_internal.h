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

struct wzContext
{
	int dummy;
};

typedef struct
{
	void (*destroy)(struct wzWidget *widget);

	void (*draw)(struct wzWidget *widget);

	// If NULL, wzWidget.rect will be set to rect, otherwise this function is called.
	void (*set_rect)(struct wzWidget *widget, wzRect rect);

	void (*mouse_button_down)(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY);
	void (*mouse_button_up)(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY);
	void (*mouse_move)(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY);
	void (*mouse_hover_on)(struct wzWidget *widget);
	void (*mouse_hover_off)(struct wzWidget *widget);
}
wzWidgetVtable;

struct wzWidget
{
	struct wzContext *context;
	wzWidgetType type;
	wzRect rect;
	void *metadata;
	bool hover;
	wzWidgetVtable vtable;

	struct wzDesktop *desktop;

	// The closest ancestor window. NULL if the widget is the descendant of a desktop. Set in wz_widget_add_child_widget.
	struct wzWindow *window;

	struct wzWidget *parent;
	struct wzWidget *firstChild;
	struct wzWidget *prev;
	struct wzWidget *next;
};

// Lock input to this widget.
void wz_desktop_lock_input(struct wzDesktop *desktop, struct wzWidget *widget);

void wz_desktop_unlock_input(struct wzDesktop *desktop);

struct wzWidget *wz_widget_find_closest_ancestor(struct wzWidget *widget, wzWidgetType type);

#endif
