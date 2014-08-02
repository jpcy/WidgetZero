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
#ifndef _WZ_MAIN_WINDOW_H_
#define _WZ_MAIN_WINDOW_H_

#include <wz.h>

void wz_main_window_set_cursor(struct wzMainWindow *mainWindow, wzCursor cursor);

// Set keyboard focus to this widget.
void wz_main_window_set_keyboard_focus_widget(struct wzMainWindow *mainWindow, struct wzWidget *widget);

bool wz_main_window_is_shift_key_down(const struct wzMainWindow *mainWindow);
bool wz_main_window_is_control_key_down(const struct wzMainWindow *mainWindow);

// Lock input to this widget.
void wz_main_window_push_lock_input_widget(struct wzMainWindow *mainWindow, struct wzWidget *widget);

// Stop locking input to this widget.
void wz_main_window_pop_lock_input_widget(struct wzMainWindow *mainWindow, struct wzWidget *widget);

void wz_main_window_set_moving_window(struct wzMainWindow *mainWindow, struct wzWindow *window);

void wz_main_window_update_content_rect(struct wzMainWindow *mainWindow);

void wz_main_window_update_docked_window_rect(struct wzMainWindow *mainWindow, struct wzWindow *window);

wzDockPosition wz_main_window_get_window_dock_position(const struct wzMainWindow *mainWindow, const struct wzWindow *window);

void wz_main_window_undock_window(struct wzMainWindow *mainWindow, struct wzWindow *window);

void wz_invoke_event(wzEvent *e, wzEventCallback *callbacks);

void wz_main_window_measure_text(struct wzMainWindow *mainWindow, struct wzWidget *widget, const char *text, int n, int *width, int *height);

#endif // _WZ_MAIN_WINDOW_H_
