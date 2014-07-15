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
#include <stdlib.h>
#include <string.h>
#include "wz_main_window.h"
#include "wz_widget.h"

#define STB_TEXTEDIT_CHARTYPE char
#include "stb_textedit.h"

struct wzTextEdit
{
	struct wzWidget base;
	int maximumTextLength;
	wzBorder border;
	bool pressed;
	int scroll;
	STB_TexteditState state;

	// Must be last.
	char text;
};

/*
================================================================================

STB TEXTEDIT

================================================================================
*/

#define STB_TEXTEDIT_STRING struct wzTextEdit

static int wz_stb_textedit_stringlen(STB_TEXTEDIT_STRING *textEdit)
{
	return (int)strlen(&textEdit->text);
}

static void wz_stb_textedit_layoutrow(StbTexteditRow *row, STB_TEXTEDIT_STRING *textEdit, int i)
{
	int width;

	row->num_chars = (int)strlen(&textEdit->text) - i;

	if (textEdit->scroll > 0)
	{
		wz_main_window_measure_text(textEdit->base.mainWindow, &textEdit->text, textEdit->scroll, &width, NULL);
		row->x0 = (float)-width;
	}
	else
	{
		row->x0 = 0;
	}

	wz_main_window_measure_text(textEdit->base.mainWindow, &textEdit->text, 0, &width, NULL);
	row->x1 = (float)width;

	row->baseline_y_delta = 1;
	row->ymin = 0;
	row->ymax = (float)textEdit->base.rect.h;
}

static float wz_stb_textedit_getwidth(STB_TEXTEDIT_STRING *textEdit, int n, int i)
{
	return (float)wz_main_window_text_get_pixel_delta(textEdit->base.mainWindow, &textEdit->text, i);
}

static int wz_stb_textedit_keytotext(int key)
{
	if (key < WZ_NUM_KEYS)
		return -1;

	return key;
}

static char wz_stb_textedit_getchar(STB_TEXTEDIT_STRING *textEdit, int i)
{
	return (&textEdit->text)[i];
}

static void wz_stb_textedit_deletechars(STB_TEXTEDIT_STRING *textEdit, int i, int n)
{
	int length;

	WZ_ASSERT(textEdit);
	length = (int)strlen(&textEdit->text);
	memmove(&(&textEdit->text)[i], &(&textEdit->text)[i + n], length - (i + n));
	(&textEdit->text)[length - n] = 0;
}

static int wz_stb_textedit_insertchars(STB_TEXTEDIT_STRING *textEdit, int i, STB_TEXTEDIT_CHARTYPE *chars, int n)
{
	int length;

	WZ_ASSERT(textEdit);
	WZ_ASSERT(chars);
	length = (int)strlen(&textEdit->text);

	// Move the displaced characters to the end.
	if (i + n < length)
	{
		memmove(&(&textEdit->text)[i + n], &(&textEdit->text)[i], length - i - (n - 1));
	}

	// Copy in the inserted characters.
	memcpy(&(&textEdit->text)[i], chars, n);

	// Null terminate.
	(&textEdit->text)[length + n] = 0;

	return 1;
}

#define STB_TEXTEDIT_STRINGLEN(obj) wz_stb_textedit_stringlen(obj)
#define STB_TEXTEDIT_LAYOUTROW(r, obj, n) wz_stb_textedit_layoutrow(r, obj, n)
#define STB_TEXTEDIT_GETWIDTH(obj, n, i) wz_stb_textedit_getwidth(obj, n, i)
#define STB_TEXTEDIT_KEYTOTEXT(k) wz_stb_textedit_keytotext(k)
#define STB_TEXTEDIT_GETCHAR(obj, i) wz_stb_textedit_getchar(obj, i)
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_DELETECHARS(obj, i, n) wz_stb_textedit_deletechars(obj, i, n)
#define STB_TEXTEDIT_INSERTCHARS(obj, i, c, n) wz_stb_textedit_insertchars(obj, i, c, n)

#define STB_TEXTEDIT_K_SHIFT WZ_KEY_SHIFT
#define STB_TEXTEDIT_K_LEFT WZ_KEY_LEFT
#define STB_TEXTEDIT_K_RIGHT WZ_KEY_RIGHT
#define STB_TEXTEDIT_K_UP WZ_KEY_UP
#define STB_TEXTEDIT_K_DOWN WZ_KEY_DOWN
#define STB_TEXTEDIT_K_LINESTART WZ_KEY_HOME
#define STB_TEXTEDIT_K_LINEEND WZ_KEY_END
#define STB_TEXTEDIT_K_TEXTSTART (WZ_KEY_CONTROL|WZ_KEY_HOME)
#define STB_TEXTEDIT_K_TEXTEND (WZ_KEY_CONTROL|WZ_KEY_END)
#define STB_TEXTEDIT_K_DELETE WZ_KEY_DELETE
#define STB_TEXTEDIT_K_BACKSPACE WZ_KEY_BACKSPACE
#define STB_TEXTEDIT_K_UNDO (WZ_KEY_CONTROL|'z')
#define STB_TEXTEDIT_K_REDO (WZ_KEY_CONTROL|'y')

#define STB_TEXTEDIT_IMPLEMENTATION
#include "stb_textedit.h"

/*
================================================================================

TEXT EDIT WIDGET

================================================================================
*/

// stb coordinates are relative to the top left of the text.
static wzPosition wz_text_edit_calculate_relative_mouse_position(const struct wzTextEdit *textEdit, int mouseX, int mouseY)
{
	wzPosition pos;

	WZ_ASSERT(textEdit);
	pos = wz_widget_get_absolute_position((const struct wzWidget *)textEdit);
	pos.x = mouseX - (pos.x + textEdit->border.left);
	pos.y = mouseY - (pos.y + textEdit->border.top);
	return pos;
}

// Relative to text rect.
static int wz_text_edit_calculate_cursor_x(const struct wzTextEdit *textEdit)
{
	int width, delta;

	WZ_ASSERT(textEdit);
	width = 0;
	delta = textEdit->state.cursor - textEdit->scroll;

	if (delta > 0)
	{
		// Text width from the scroll position to the cursor.
		wz_main_window_measure_text(textEdit->base.mainWindow, &(&textEdit->text)[textEdit->scroll], delta, &width, NULL);
	}
	else if (delta < 0)
	{
		// Text width from the cursor to the scroll position.
		wz_main_window_measure_text(textEdit->base.mainWindow, &(&textEdit->text)[textEdit->state.cursor], -delta, &width, NULL);
		width = -width;
	}
	
	return width;
}

static void wz_text_edit_update_scroll_value(struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);

	for (;;)
	{
		int cursorX = wz_text_edit_calculate_cursor_x(textEdit);

		if (cursorX > textEdit->base.rect.w - (textEdit->border.left + textEdit->border.right))
		{
			textEdit->scroll++;
		}
		else if (cursorX < 0)
		{
			textEdit->scroll--;
		}
		else
		{
			break;
		}
	}
}

static void wz_text_edit_mouse_button_down(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzTextEdit *textEdit;
	
	WZ_ASSERT(widget);
	textEdit = (struct wzTextEdit *)widget;

	if (mouseButton == 1)
	{
		const wzPosition pos = wz_text_edit_calculate_relative_mouse_position(textEdit, mouseX, mouseY);
		stb_textedit_click(textEdit, &textEdit->state, (float)pos.x, (float)pos.y);
		wz_text_edit_update_scroll_value(textEdit);
		textEdit->pressed = true;
	}
}

static void wz_text_edit_mouse_button_up(struct wzWidget *widget, int mouseButton, int mouseX, int mouseY)
{
	struct wzTextEdit *textEdit;
	
	WZ_ASSERT(widget);
	textEdit = (struct wzTextEdit *)widget;

	if (mouseButton == 1)
	{
		textEdit->pressed = false;
	}
}

static void wz_text_edit_mouse_move(struct wzWidget *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct wzTextEdit *textEdit;

	WZ_ASSERT(widget);
	textEdit = (struct wzTextEdit *)widget;

	if (widget->hover)
		wz_main_window_set_cursor(widget->mainWindow, WZ_CURSOR_IBEAM);

	if (textEdit->pressed)
	{
		if (textEdit->pressed && WZ_POINT_IN_RECT(mouseX, mouseY, wz_widget_get_absolute_rect(widget)))
		{
			const wzPosition pos = wz_text_edit_calculate_relative_mouse_position(textEdit, mouseX, mouseY);
			stb_textedit_drag(textEdit, &textEdit->state, (float)pos.x, (float)pos.y);
			wz_text_edit_update_scroll_value(textEdit);
		}
	}
}

static void wz_text_edit_key_down(struct wzWidget *widget, wzKey key)
{
	struct wzTextEdit *textEdit;

	WZ_ASSERT(widget);
	textEdit = (struct wzTextEdit *)widget;
	stb_textedit_key(textEdit, &textEdit->state, (int)key);
	wz_text_edit_update_scroll_value(textEdit);
}

static void wz_text_edit_text_input(struct wzWidget *widget, const char *text)
{
	struct wzTextEdit *textEdit;

	WZ_ASSERT(widget);
	WZ_ASSERT(text);
	textEdit = (struct wzTextEdit *)widget;
	stb_textedit_key(textEdit, &textEdit->state, (int)text[0]);
	wz_text_edit_update_scroll_value(textEdit);
}

struct wzTextEdit *wz_text_edit_create(int maximumTextLength)
{
	struct wzTextEdit *textEdit;

	textEdit = (struct wzTextEdit *)malloc(sizeof(struct wzTextEdit) + maximumTextLength);
	memset(textEdit, 0, sizeof(struct wzTextEdit) + maximumTextLength);
	textEdit->base.type = WZ_TYPE_TEXT_EDIT;
	textEdit->base.vtable.mouse_button_down = wz_text_edit_mouse_button_down;
	textEdit->base.vtable.mouse_button_up = wz_text_edit_mouse_button_up;
	textEdit->base.vtable.mouse_move = wz_text_edit_mouse_move;
	textEdit->base.vtable.key_down = wz_text_edit_key_down;
	textEdit->base.vtable.text_input = wz_text_edit_text_input;
	textEdit->maximumTextLength = maximumTextLength;

	stb_textedit_initialize_state(&textEdit->state, 1);

	return textEdit;
}

wzBorder wz_text_edit_get_border(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->border;
}

void wz_text_edit_set_border(struct wzTextEdit *textEdit, wzBorder border)
{
	WZ_ASSERT(textEdit);
	textEdit->border = border;
}

void wz_text_edit_set_border_args(struct wzTextEdit *textEdit, int top, int right, int bottom, int left)
{
	WZ_ASSERT(textEdit);
	textEdit->border.top = top;
	textEdit->border.right = right;
	textEdit->border.bottom = bottom;
	textEdit->border.left = left;
}

const char *wz_text_edit_get_text(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return &textEdit->text;
}

void wz_text_edit_set_text(struct wzTextEdit *textEdit, const char *text)
{
	WZ_ASSERT(textEdit);
	strncpy(&textEdit->text, text, WZ_MIN((int)strlen(text), textEdit->maximumTextLength));
}

int wz_text_edit_get_scroll_value(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->scroll;
}

int wz_text_edit_get_cursor_index(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->state.cursor;
}

int wz_text_edit_get_selection_start_index(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->state.select_start;
}

int wz_text_edit_get_selection_end_index(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->state.select_end;
}
