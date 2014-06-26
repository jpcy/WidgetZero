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
#ifndef _WZ_DESKTOP_H_
#define _WZ_DESKTOP_H_

#include <wz.h>

void wz_desktop_set_cursor(struct wzDesktop *desktop, wzCursor cursor);

// Lock input to this widget.
void wz_desktop_push_lock_input_widget(struct wzDesktop *desktop, struct wzWidget *widget);

// Stop locking input to this widget.
void wz_desktop_pop_lock_input_widget(struct wzDesktop *desktop, struct wzWidget *widget);

void wz_desktop_set_moving_window(struct wzDesktop *desktop, struct wzWindow *window);

void wz_desktop_update_content_rect(struct wzDesktop *desktop);

void wz_desktop_update_docked_window_rect(struct wzDesktop *desktop, struct wzWindow *window);

wzDockPosition wz_desktop_get_window_dock_position(const struct wzDesktop *desktop, const struct wzWindow *window);

void wz_desktop_undock_window(struct wzDesktop *desktop, struct wzWindow *window);

void wz_invoke_event(wzEvent e, wzEventCallback *callbacks);

void wz_desktop_measure_text(struct wzDesktop *desktop, const char *text, int n, int *width, int *height);
int wz_desktop_text_get_pixel_delta(struct wzDesktop *desktop, const char *text, int index);

#endif // _WZ_DESKTOP_H_
