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
#ifndef _WIDGET_ZERO_H_
#define _WIDGET_ZERO_H_

#ifndef __cplusplus
#ifdef _MSC_VER
typedef int bool;
#define false 0
#define true 1
#else
#include <stdbool.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct wzContext;
struct wzWindow;
struct wzWidget;
struct wzButton;
struct wzGroupBox;

typedef enum
{
	WZ_TYPE_UNKNOWN,
	WZ_TYPE_BUTTON,
	WZ_TYPE_GROUPBOX,
	WZ_MAX_WIDGET_TYPES = 64
}
wzWidgetType;

typedef struct
{
	int x, y;
}
wzPosition;

typedef struct
{
	int w, h;
}
wzSize;

typedef struct
{
	int x, y, w, h;
}
wzRect;

struct wzContext *wz_context_create(void);
void wz_context_destroy(struct wzContext *context);

struct wzWindow *wz_window_create(struct wzContext *context);
void wz_window_destroy(struct wzWindow *window);
void wz_window_mouse_button_down(struct wzWindow *window, int mouseButton, int mouseX, int mouseY);
void wz_window_mouse_button_up(struct wzWindow *window, int mouseButton, int mouseX, int mouseY);
void wz_window_mouse_move(struct wzWindow *window, int mouseX, int mouseY);
void wz_window_draw(struct wzWindow *window);

wzWidgetType wz_widget_get_type(const struct wzWidget *widget);
void wz_widget_set_position(struct wzWidget *widget, int x, int y);
wzPosition wz_widget_get_position(const struct wzWidget *widget);
void wz_widget_set_size(struct wzWidget *widget, wzSize size);
wzSize wz_widget_get_size(const struct wzWidget *widget);
void wz_widget_set_rect(struct wzWidget *widget, wzRect rect);
wzRect wz_widget_get_rect(const struct wzWidget *widget);
bool wz_widget_get_hover(const struct wzWidget *widget);
void wz_widget_set_metadata(struct wzWidget *widget, void *metadata);
void *wz_widget_get_metadata(struct wzWidget *widget);
void wz_widget_set_draw_function(struct wzWidget *widget, void (*draw)(struct wzWidget *));
bool wz_widget_can_have_child_widgets(const struct wzWidget *widget);
void wz_widget_add_child_widget(struct wzWidget *widget, struct wzWidget *child);

struct wzButton *wz_button_create(struct wzWindow *window);
void wz_button_set_toggle_behavior(struct wzButton *button, bool enabled);
bool wz_button_is_pressed(const struct wzButton *button);
bool wz_button_is_set(const struct wzButton *button);

struct wzGroupBox *wz_groupbox_create(struct wzWindow *window);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
