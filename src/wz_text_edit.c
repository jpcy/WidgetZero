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

struct wzTextEdit
{
	struct wzWidget base;
	bool multiline;
	int maximumTextLength;
	wzBorder border;
	bool pressed;
	int cursorIndex;
	int scrollIndex;
	int selectionStartIndex;
	int selectionEndIndex;

	// Must be last.
	char text;
};

/*
================================================================================

PRIVATE FUNCTIONS

================================================================================
*/

static void wz_text_edit_insert_text(struct wzTextEdit *textEdit, int index, const char *text, int n)
{
	int length;

	WZ_ASSERT(textEdit);
	WZ_ASSERT(text);
	length = (int)strlen(&textEdit->text);

	// Move the displaced characters to the end.
	if (index + n < length)
	{
		memmove(&(&textEdit->text)[index + n], &(&textEdit->text)[index], length - index - (n - 1));
	}

	// Copy in the inserted text.
	memcpy(&(&textEdit->text)[index], text, n);

	// Null terminate.
	(&textEdit->text)[length + n] = 0;
}

static void wz_text_edit_delete_text(struct wzTextEdit *textEdit, int index, int n)
{
	int length;

	WZ_ASSERT(textEdit);
	length = (int)strlen(&textEdit->text);

	if (index < 0 || index >= length)
		return;

	memmove(&(&textEdit->text)[index], &(&textEdit->text)[index + n], length - (index + n));
	(&textEdit->text)[length - n] = 0;
}

static int wz_text_edit_index_from_relative_position(const struct wzTextEdit *textEdit, wzPosition pos)
{
	wzRect rect;
	int i, previousWidth, result;

	WZ_ASSERT(textEdit);

	// Calculate relative position.
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)textEdit);

	// Outside widget.
	if (pos.x < 0 || pos.x > rect.w || pos.y < 0 || pos.y > rect.h)
		return textEdit->scrollIndex;

	if (textEdit->multiline)
	{
		int lineHeight;
		wzLineBreakResult line;
		int lineY = 0;

		// Get line height.
		lineHeight = wz_main_window_get_line_height(textEdit->base.mainWindow, (struct wzWidget *)textEdit);

		// Iterate through lines.
		result = 0;
		line.next = &textEdit->text;

		for (;;)
		{
			line = wz_main_window_line_break_text(textEdit->base.mainWindow, (struct wzWidget *)textEdit, line.next, 0, rect.w - (textEdit->border.left + textEdit->border.right));

			result = line.start - &textEdit->text;

			if (pos.y >= lineY && pos.y < lineY + lineHeight)
				break; // On this line.

			if (!line.next || !line.next[0])
				break;

			lineY += lineHeight;
		}

		// Walk through the text until we find two glyphs that the x coordinate straddles.
		previousWidth = 0;

		for (i = 1; i <= (int)line.length; i++)
		{
			int width, deltaWidth;

			// Calculate the width of the text up to the current character.
			wz_main_window_measure_text(textEdit->base.mainWindow, (struct wzWidget *)textEdit, line.start, i, &width, NULL);

			// Calculate the change in text width since the last iteration.
			deltaWidth = width - previousWidth;

			// Check the intersection between the position and the change in text width since the last iteration.
			if (pos.x >= previousWidth && pos.x <= previousWidth + deltaWidth / 2)
			{
				// Left side of glyph.
				result += i - 1;
				break;
			}
			else if (pos.x >= previousWidth + deltaWidth / 2 && pos.x <= width)
			{
				// Right side of glyph.
				result += i;
				break;
			}

			// Made it to the end of text string.
			if (i == line.length)
			{
				result += i;
				break;
			}

			// Store this text width for the next iteration.
			previousWidth = width;
		}
	}
	else
	{
		// Walk through the text until we find two glyphs that the x coordinate straddles.
		previousWidth = 0;
		result = textEdit->scrollIndex;

		for (i = 1; i <= (int)strlen(&textEdit->text); i++)
		{
			int width, deltaWidth;

			// Calculate the width of the text up to the current character.
			wz_main_window_measure_text(textEdit->base.mainWindow, (struct wzWidget *)textEdit, &(&textEdit->text)[textEdit->scrollIndex], i, &width, NULL);

			// Check if we've gone beyond the width of the widget.
			if (width > rect.w - (textEdit->border.left + textEdit->border.right))
			{
				result = textEdit->scrollIndex + i - 1;
				break;
			}

			// Calculate the change in text width since the last iteration.
			deltaWidth = width - previousWidth;

			// Check the intersection between the position and the change in text width since the last iteration.
			if (pos.x >= previousWidth && pos.x <= previousWidth + deltaWidth / 2)
			{
				// Left side of glyph.
				result = textEdit->scrollIndex + i - 1;
				break;
			}
			else if (pos.x >= previousWidth + deltaWidth / 2 && pos.x <= width)
			{
				// Right side of glyph.
				result = textEdit->scrollIndex + i;
				break;
			}

			// Made it to the end of text string.
			if (i == (int)strlen(&textEdit->text))
			{
				result = i;
				break;
			}

			// Store this text width for the next iteration.
			previousWidth = width;
		}
	}

	return WZ_CLAMPED(0, result, (int)strlen(&textEdit->text));
}

// Calculate the text index at the given absolute position.
static int wz_text_edit_index_from_position(const struct wzTextEdit *textEdit, int x, int y)
{
	wzRect rect;
	wzPosition pos;

	// Make position relative.
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)textEdit);
	pos.x = x - rect.x;
	pos.y = y - rect.y;

	return wz_text_edit_index_from_relative_position(textEdit, pos);
}

// Update the scroll index so the cursor is visible.
static void wz_text_edit_update_scroll_index(struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);

	if (textEdit->multiline) // TODO
		return;

	for (;;)
	{
		int cursorX = wz_text_edit_position_from_index(textEdit, textEdit->cursorIndex).x;

		if (cursorX > textEdit->base.rect.w - (textEdit->border.left + textEdit->border.right))
		{
			textEdit->scrollIndex++;
		}
		else if (cursorX < 0)
		{
			textEdit->scrollIndex--;
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
		int oldCursorIndex;

		// Set keyboard focus to this widget.
		wz_main_window_set_keyboard_focus_widget(widget->mainWindow, widget);

		// Move the cursor to the mouse position.
		oldCursorIndex = textEdit->cursorIndex;
		textEdit->cursorIndex = wz_text_edit_index_from_position(textEdit, mouseX, mouseY);
		wz_text_edit_update_scroll_index(textEdit);
		textEdit->pressed = true;

		// Handle selecting.
		if (wz_main_window_is_shift_key_down(widget->mainWindow))
		{
			// Start a new selection if there isn't one.
			if (textEdit->selectionStartIndex == textEdit->selectionEndIndex)
			{
				// Use the old cursor index as the selection start.
				textEdit->selectionStartIndex = oldCursorIndex;
			}

			textEdit->selectionEndIndex = textEdit->cursorIndex;
		}
		else
		{
			textEdit->selectionStartIndex = textEdit->cursorIndex;
			textEdit->selectionEndIndex = textEdit->cursorIndex;
		}
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

	if (textEdit->pressed && WZ_POINT_IN_RECT(mouseX, mouseY, wz_widget_get_absolute_rect(widget)))
	{
		// Move the cursor to the mouse position.
		textEdit->cursorIndex = wz_text_edit_index_from_position(textEdit, mouseX, mouseY);
		wz_text_edit_update_scroll_index(textEdit);

		// Set the selection end to the new cursor index.
		textEdit->selectionEndIndex = textEdit->cursorIndex;
	}
}

static void wz_text_edit_delete_selection(struct wzTextEdit *textEdit)
{
	int start, end;

	WZ_ASSERT(textEdit);

	// No selection.
	if (textEdit->selectionStartIndex == textEdit->selectionEndIndex)
		return;

	start = WZ_MIN(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
	end = WZ_MAX(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
	wz_text_edit_delete_text(textEdit, start, end - start);

	// Move the cursor to the start (smallest of start and end, not the real selection start).
	textEdit->cursorIndex = start;

	// Clear the selection.
	textEdit->selectionStartIndex = textEdit->selectionEndIndex = 0;
}

// Helper function for moving the cursor while the selection (shift) key is held.
static void wz_text_edit_move_cursor_and_selection(struct wzTextEdit *textEdit, int newCursorIndex)
{
	WZ_ASSERT(textEdit);

	if (textEdit->selectionStartIndex != textEdit->selectionEndIndex)
	{
		// If there's already a selection, move the cursor, and the selection end to match.
		textEdit->cursorIndex = newCursorIndex;
		textEdit->selectionEndIndex = textEdit->cursorIndex;
	}
	else
	{
		// No selection, start a new one and move the cursor.
		textEdit->selectionStartIndex = textEdit->cursorIndex;
		textEdit->cursorIndex = newCursorIndex;
		textEdit->selectionEndIndex = textEdit->cursorIndex;
	}
}

static void wz_text_edit_key_down(struct wzWidget *widget, wzKey key)
{
	struct wzTextEdit *textEdit;

	WZ_ASSERT(widget);
	textEdit = (struct wzTextEdit *)widget;

	if (key == WZ_KEY_LEFT)
	{
		if (textEdit->selectionStartIndex != textEdit->selectionEndIndex)
		{
			// If the cursor is to the right of the selection start, move the cursor to the start of the selection.
			if (textEdit->cursorIndex > textEdit->selectionStartIndex)
			{
				textEdit->cursorIndex = textEdit->selectionStartIndex;
			}

			// Clear the selection.
			textEdit->selectionStartIndex = textEdit->selectionEndIndex = 0;
		}
		// Move the cursor to the left, if there's room.
		else if (textEdit->cursorIndex > 0)
		{
			textEdit->cursorIndex--;
		}
	}
	else if (key == (WZ_KEY_LEFT | WZ_KEY_SHIFT_BIT) && textEdit->cursorIndex > 0)
	{
		wz_text_edit_move_cursor_and_selection(textEdit, textEdit->cursorIndex - 1);
	}
	else if (key == WZ_KEY_RIGHT)
	{
		if (textEdit->selectionStartIndex != textEdit->selectionEndIndex)
		{
			// If the cursor is to the left of the selection start, move the cursor to the start of the selection.
			if (textEdit->cursorIndex < textEdit->selectionStartIndex)
			{
				textEdit->cursorIndex = textEdit->selectionStartIndex;
			}

			// Clear the selection.
			textEdit->selectionStartIndex = textEdit->selectionEndIndex = 0;
		}
		// Move the cursor to the right, if there's room.
		else if (textEdit->cursorIndex < (int)strlen(&textEdit->text))
		{
			textEdit->cursorIndex++;
		}
	}
	else if (key == (WZ_KEY_RIGHT | WZ_KEY_SHIFT_BIT) && textEdit->cursorIndex < (int)strlen(&textEdit->text))
	{
		wz_text_edit_move_cursor_and_selection(textEdit, textEdit->cursorIndex + 1);
	}
	else if (WZ_KEY_MOD_OFF(key) == WZ_KEY_UP || WZ_KEY_MOD_OFF(key) == WZ_KEY_DOWN)
	{
		wzPosition cursorPosition;
		int lineHeight, newCursorIndex;

		// Get the cursor position.
		cursorPosition = wz_text_edit_position_from_index(textEdit, textEdit->cursorIndex);

		// Get line height.
		lineHeight = wz_main_window_get_line_height(textEdit->base.mainWindow, (struct wzWidget *)textEdit);

		// Move the cursor up/down.
		cursorPosition.y += (WZ_KEY_MOD_OFF(key) == WZ_KEY_UP ? -lineHeight : lineHeight);
		newCursorIndex = wz_text_edit_index_from_relative_position(textEdit, cursorPosition);

		// Apply the new cursor index.
		if ((key & WZ_KEY_SHIFT_BIT) != 0)
		{
			wz_text_edit_move_cursor_and_selection(textEdit, newCursorIndex);
		}
		else if (key == WZ_KEY_UP || key == WZ_KEY_DOWN)
		{
			textEdit->cursorIndex = newCursorIndex;

			// Clear the selection.
			textEdit->selectionStartIndex = textEdit->selectionEndIndex = 0;
		}
	}
	else if (key == WZ_KEY_HOME)
	{
		textEdit->cursorIndex = 0;

		// Clear the selection.
		textEdit->selectionStartIndex = textEdit->selectionEndIndex = 0;
	}
	else if (key == (WZ_KEY_HOME | WZ_KEY_SHIFT_BIT))
	{
		if (textEdit->selectionStartIndex != textEdit->selectionEndIndex)
		{
			// If there's already a selection, move the cursor, and the selection end to match.
			textEdit->cursorIndex = 0;
			textEdit->selectionEndIndex = textEdit->cursorIndex;
		}
		else
		{
			// No selection, start a new one and move the cursor.
			textEdit->selectionStartIndex = textEdit->cursorIndex;
			textEdit->cursorIndex = 0;
			textEdit->selectionEndIndex = textEdit->cursorIndex;
		}
	}
	else if (key == WZ_KEY_END)
	{
		textEdit->cursorIndex = (int)strlen(&textEdit->text);

		// Clear the selection.
		textEdit->selectionStartIndex = textEdit->selectionEndIndex = 0;
	}
	else if (key == (WZ_KEY_END | WZ_KEY_SHIFT_BIT))
	{
		if (textEdit->selectionStartIndex != textEdit->selectionEndIndex)
		{
			// If there's already a selection, move the cursor, and the selection end to match.
			textEdit->cursorIndex = (int)strlen(&textEdit->text);
			textEdit->selectionEndIndex = textEdit->cursorIndex;
		}
		else
		{
			// No selection, start a new one and move the cursor.
			textEdit->selectionStartIndex = textEdit->cursorIndex;
			textEdit->cursorIndex = (int)strlen(&textEdit->text);
			textEdit->selectionEndIndex = textEdit->cursorIndex;
		}
	}
	else if (key == WZ_KEY_DELETE)
	{
		if (textEdit->selectionStartIndex != textEdit->selectionEndIndex)
		{
			wz_text_edit_delete_selection(textEdit);
		}
		else
		{
			wz_text_edit_delete_text(textEdit, textEdit->cursorIndex, 1);
		}
	}
	else if (key == WZ_KEY_BACKSPACE && textEdit->cursorIndex > 0)
	{
		if (textEdit->selectionStartIndex != textEdit->selectionEndIndex)
		{
			wz_text_edit_delete_selection(textEdit);
		}
		else
		{
			wz_text_edit_delete_text(textEdit, textEdit->cursorIndex - 1, 1);
			textEdit->cursorIndex--;
		}
	}
	else
	{
		return;
	}

	wz_text_edit_update_scroll_index(textEdit);
}

static void wz_text_edit_text_input(struct wzWidget *widget, const char *text)
{
	struct wzTextEdit *textEdit;
	int n;

	WZ_ASSERT(widget);
	WZ_ASSERT(text);
	textEdit = (struct wzTextEdit *)widget;
	n = (int)strlen(text);

	if (n == 0)
		return;

	// The text replaces the selection.
	if (textEdit->selectionStartIndex != textEdit->selectionEndIndex)
	{
		wz_text_edit_delete_selection(textEdit);
	}

	wz_text_edit_insert_text(textEdit, textEdit->cursorIndex, text, n);
	textEdit->cursorIndex += n;
	wz_text_edit_update_scroll_index(textEdit);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

struct wzTextEdit *wz_text_edit_create(bool multiline, int maximumTextLength)
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
	textEdit->multiline = multiline;
	textEdit->maximumTextLength = maximumTextLength;

	return textEdit;
}

bool wz_text_edit_is_multiline(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->multiline;
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
	return textEdit->scrollIndex;
}

const char *wz_text_edit_get_visible_text(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);

	if (textEdit->multiline)
	{
		return &textEdit->text;
	}
	else
	{
		return &((&textEdit->text)[textEdit->scrollIndex]);
	}
}

wzPosition wz_text_edit_get_cursor_position(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return wz_text_edit_position_from_index(textEdit, textEdit->cursorIndex);
}

bool wz_text_edit_has_selection(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->selectionStartIndex != textEdit->selectionEndIndex;
}

int wz_text_edit_get_selection_start_index(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return WZ_MIN(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
}

wzPosition wz_text_edit_get_selection_start_position(const struct wzTextEdit *textEdit)
{
	int index;

	WZ_ASSERT(textEdit);
	index = WZ_MIN(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
	return wz_text_edit_position_from_index(textEdit, index);
}

int wz_text_edit_get_selection_end_index(const struct wzTextEdit *textEdit)
{
	WZ_ASSERT(textEdit);
	return WZ_MAX(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
}

wzPosition wz_text_edit_get_selection_end_position(const struct wzTextEdit *textEdit)
{
	int index;

	WZ_ASSERT(textEdit);
	index = WZ_MAX(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
	return wz_text_edit_position_from_index(textEdit, index);
}

// Calculate the position of the index - relative to text rect - based on the cursor index and scroll index. 
wzPosition wz_text_edit_position_from_index(const struct wzTextEdit *textEdit, int index)
{
	int lineHeight;
	wzPosition position;

	WZ_ASSERT(textEdit);

	// Get the line height.
	lineHeight = wz_main_window_get_line_height(textEdit->base.mainWindow, (struct wzWidget *)textEdit);

	if (textEdit->multiline)
	{
		wzLineBreakResult line;
		int lineNo = 0;

		// Iterate through lines.
		line.next = &textEdit->text;

		for (;;)
		{
			int lineStartIndex;

			line = wz_main_window_line_break_text(textEdit->base.mainWindow, (struct wzWidget *)textEdit, line.next, 0, textEdit->base.rect.w - (textEdit->border.left + textEdit->border.right));
			WZ_ASSERT(line.start);
			WZ_ASSERT(line.next);
			lineStartIndex = line.start - &textEdit->text;

			// Is the index on this line?
			if (index >= lineStartIndex && index <= lineStartIndex + (int)line.length)
			{
				int width = 0;

				if (index - lineStartIndex > 0)
				{
					wz_main_window_measure_text(textEdit->base.mainWindow, (struct wzWidget *)textEdit, line.start, index - lineStartIndex, &width, NULL);
				}

				position.x = width;
				position.y = lineNo * lineHeight + lineHeight / 2;
				break;
			}

			lineNo++;
		}
	}
	else
	{
		int width = 0;
		int delta = index - textEdit->scrollIndex;

		if (delta > 0)
		{
			// Text width from the scroll index to the requested index.
			wz_main_window_measure_text(textEdit->base.mainWindow, (struct wzWidget *)textEdit, &(&textEdit->text)[textEdit->scrollIndex], delta, &width, NULL);
		}
		else if (delta < 0)
		{
			// Text width from the requested index to the scroll index.
			wz_main_window_measure_text(textEdit->base.mainWindow, (struct wzWidget *)textEdit, &(&textEdit->text)[textEdit->cursorIndex], -delta, &width, NULL);
			width = -width;
		}
	
		position.x = width;
		position.y = lineHeight / 2;
	}

	return position;
}
