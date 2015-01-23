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

#include <vector>
#include <wz.h>

namespace wz {

void wz_main_window_set_cursor(struct MainWindowImpl *mainWindow, Cursor cursor);

// Set keyboard focus to this widget.
void wz_main_window_set_keyboard_focus_widget(struct MainWindowImpl *mainWindow, struct WidgetImpl *widget);

bool wz_main_window_is_shift_key_down(const struct MainWindowImpl *mainWindow);
bool wz_main_window_is_control_key_down(const struct MainWindowImpl *mainWindow);

// Lock input to this widget.
void wz_main_window_push_lock_input_widget(struct MainWindowImpl *mainWindow, struct WidgetImpl *widget);

// Stop locking input to this widget.
void wz_main_window_pop_lock_input_widget(struct MainWindowImpl *mainWindow, struct WidgetImpl *widget);

void wz_main_window_set_moving_window(struct MainWindowImpl *mainWindow, struct WindowImpl *window);

void wz_main_window_update_content_rect(struct MainWindowImpl *mainWindow);

void wz_main_window_update_docked_window_rect(struct MainWindowImpl *mainWindow, struct WindowImpl *window);

DockPosition wz_main_window_get_window_dock_position(const struct MainWindowImpl *mainWindow, const struct WindowImpl *window);

void wz_main_window_undock_window(struct MainWindowImpl *mainWindow, struct WindowImpl *window);

void wz_invoke_event(Event *e);
void wz_invoke_event(Event *e, const std::vector<EventCallback> &callbacks);

bool wz_main_window_text_cursor_is_visible(const struct MainWindowImpl *mainWindow);

} // namespace wz
