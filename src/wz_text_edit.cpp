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
#include "wz_pch.h"
#pragma hdrstop

namespace wz {

struct TextEditImpl : public WidgetImpl
{
	TextEditImpl();

	struct ScrollerImpl *scroller;
	bool multiline;
	int maximumTextLength;
	TextEditValidateTextCallback validate_text;
	Border border;
	bool pressed;
	int cursorIndex;
	int scrollValue;
	int selectionStartIndex;
	int selectionEndIndex;
	std::string text;
};

static void wz_text_edit_update_scroll_index(struct TextEditImpl *textEdit);
static void wz_text_edit_delete_selection(struct TextEditImpl *textEdit);

static int wz_calculate_num_lines(struct TextEditImpl *textEdit, int lineWidth)
{
	LineBreakResult line;
	int nLines = 0;

	WZ_ASSERT(textEdit);

	if (!textEdit->multiline)
		return 0;

	line.next = textEdit->text.c_str();

	for (;;)
	{
		line = wz_renderer_line_break_text(textEdit->renderer, wz_widget_get_font_face(textEdit), wz_widget_get_font_size(textEdit), line.next, 0, lineWidth);

		if (!line.next || !line.next[0])
			break;

		nLines++;
	}

	return nLines;
}

/*
================================================================================

SCROLLER

================================================================================
*/

static void wz_text_edit_update_scroller(struct TextEditImpl *textEdit)
{
	Rect textEditRect, rect;
	int lineHeight, nLines, max, maxHeight;

	WZ_ASSERT(textEdit);

	if (!textEdit->multiline)
		return;

	if (!textEdit->mainWindow)
		return; // Not added yet

	// Hide/show scroller depending on if it's needed.
	lineHeight = wz_widget_get_line_height(textEdit);
	nLines = wz_calculate_num_lines(textEdit, textEdit->rect.w - (textEdit->border.left + textEdit->border.right));

	if (lineHeight * nLines > textEdit->rect.h - (textEdit->border.top + textEdit->border.bottom))
	{
		wz_widget_set_visible((struct WidgetImpl *)textEdit->scroller, true);
	}
	else
	{
		wz_widget_set_visible((struct WidgetImpl *)textEdit->scroller, false);
		return;
	}

	// Update max value.
	nLines = wz_calculate_num_lines(textEdit, wz_text_edit_get_text_rect(textEdit).w);
	max = nLines - (wz_text_edit_get_text_rect(textEdit).h / lineHeight);
	wz_scroller_set_max_value(textEdit->scroller, max);

	// Fit to the right of the rect. Width doesn't change.
	textEditRect = wz_widget_get_rect((struct WidgetImpl *)textEdit);
	rect.w = ((struct WidgetImpl *)textEdit->scroller)->rect.w;
	rect.x = textEditRect.w - textEdit->border.right - rect.w;
	rect.y = textEdit->border.top;
	rect.h = textEditRect.h - (textEdit->border.top + textEdit->border.bottom);
	wz_widget_set_rect_internal((struct WidgetImpl *)textEdit->scroller, rect);

	// Now that the height has been calculated, update the nub scale.
	maxHeight = nLines * lineHeight;
	wz_scroller_set_nub_scale(textEdit->scroller, 1.0f - ((maxHeight - rect.h) / (float)maxHeight));
}

static void wz_text_edit_scroller_value_changed(Event *e)
{
	struct TextEditImpl *textEdit;
	
	WZ_ASSERT(e);
	textEdit = (struct TextEditImpl *)e->base.widget->parent;
	WZ_ASSERT(textEdit->multiline);
	textEdit->scrollValue = e->scroller.value;
}

/*
================================================================================

INSERTING / DELETING TEXT

================================================================================
*/

static void wz_text_edit_insert_text(struct TextEditImpl *textEdit, int index, const char *text, int n)
{
	WZ_ASSERT(textEdit);
	WZ_ASSERT(text);
	textEdit->text.insert(index, text, n);

	// Update the scroller.
	wz_text_edit_update_scroller(textEdit);
}

static void wz_text_edit_enter_text(struct TextEditImpl *textEdit, const char *text)
{
	int n;

	WZ_ASSERT(textEdit);
	WZ_ASSERT(text);
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

static void wz_text_edit_delete_text(struct TextEditImpl *textEdit, int index, int n)
{
	WZ_ASSERT(textEdit);

	if (index < 0 || index >= (int)textEdit->text.length())
		return;

	textEdit->text.erase(index, n);

	// Update the scroller.
	wz_text_edit_update_scroller(textEdit);
}

static void wz_text_edit_delete_selection(struct TextEditImpl *textEdit)
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

/*
================================================================================

PRIVATE UTILITY FUNCTIONS

================================================================================
*/

// Returns -1 if an index could not be calculated. e.g. if the position is outside the widget.
static int wz_text_edit_index_from_relative_position(const struct TextEditImpl *textEdit, Position pos)
{
	Rect rect;
	int i, previousWidth, result;

	WZ_ASSERT(textEdit);

	// Calculate relative position.
	rect = wz_widget_get_absolute_rect((const struct WidgetImpl *)textEdit);

	if (textEdit->multiline)
	{
		int lineHeight, lineY;
		LineBreakResult line;

		// Get line height.
		lineHeight = wz_widget_get_line_height(textEdit);

		// Set line starting position. May be outside the widget.
		lineY = lineHeight * -textEdit->scrollValue;

		// Iterate through lines.
		result = 0;
		line.next = textEdit->text.c_str();

		for (;;)
		{
			line = wz_renderer_line_break_text(textEdit->renderer, wz_widget_get_font_face(textEdit), wz_widget_get_font_size(textEdit), line.next, 0, wz_text_edit_get_text_rect(textEdit).w);

			result = line.start - textEdit->text.c_str();

			if (pos.y >= lineY && pos.y < lineY + lineHeight)
				break; // On this line.

			if (!line.next || !line.next[0])
				return -1;

			lineY += lineHeight;
		}

		// Walk through the text until we find two glyphs that the x coordinate straddles.
		previousWidth = 0;

		for (i = 1; i <= (int)line.length; i++)
		{
			int width, deltaWidth;

			// Calculate the width of the text up to the current character.
			wz_widget_measure_text(textEdit, line.start, i, &width, NULL);

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
		// Outside widget.
		if (pos.x < 0 || pos.x > rect.w || pos.y < 0 || pos.y > rect.h)
			return textEdit->scrollValue;

		// Walk through the text until we find two glyphs that the x coordinate straddles.
		previousWidth = 0;
		result = textEdit->scrollValue;

		for (i = 1; i <= (int)textEdit->text.length(); i++)
		{
			int width, deltaWidth;

			// Calculate the width of the text up to the current character.
			wz_widget_measure_text(textEdit, &textEdit->text[textEdit->scrollValue], i, &width, NULL);

			// Check if we've gone beyond the width of the widget.
			if (width > wz_text_edit_get_text_rect(textEdit).w)
			{
				result = textEdit->scrollValue + i - 1;
				break;
			}

			// Calculate the change in text width since the last iteration.
			deltaWidth = width - previousWidth;

			// Check the intersection between the position and the change in text width since the last iteration.
			if (pos.x >= previousWidth && pos.x <= previousWidth + deltaWidth / 2)
			{
				// Left side of glyph.
				result = textEdit->scrollValue + i - 1;
				break;
			}
			else if (pos.x >= previousWidth + deltaWidth / 2 && pos.x <= width)
			{
				// Right side of glyph.
				result = textEdit->scrollValue + i;
				break;
			}

			// Made it to the end of text string.
			if (i == textEdit->text.length())
			{
				result = i;
				break;
			}

			// Store this text width for the next iteration.
			previousWidth = width;
		}
	}

	return WZ_CLAMPED(0, result, (int)textEdit->text.length());
}

// Calculate the text index at the given absolute position.
static int wz_text_edit_index_from_position(const struct TextEditImpl *textEdit, int x, int y)
{
	Rect rect;
	Position pos;

	// Make position relative.
	rect = wz_widget_get_absolute_rect((const struct WidgetImpl *)textEdit);
	pos.x = x - rect.x;
	pos.y = y - rect.y;

	return wz_text_edit_index_from_relative_position(textEdit, pos);
}

// Update the scroll value so the cursor is visible.
static void wz_text_edit_update_scroll_index(struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);

	if (textEdit->multiline)
	{
		for (;;)
		{
			int cursorY = wz_text_edit_position_from_index(textEdit, textEdit->cursorIndex).y;

			if (cursorY > textEdit->rect.h - (textEdit->border.top + textEdit->border.bottom))
			{
				textEdit->scrollValue++;
				wz_scroller_set_value(textEdit->scroller, textEdit->scrollValue);
			}
			else if (cursorY < 0)
			{
				textEdit->scrollValue--;
				wz_scroller_set_value(textEdit->scroller, textEdit->scrollValue);
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		for (;;)
		{
			int cursorX = wz_text_edit_position_from_index(textEdit, textEdit->cursorIndex).x;

			if (cursorX > wz_text_edit_get_text_rect(textEdit).w)
			{
				textEdit->scrollValue++;
			}
			else if (cursorX < 0)
			{
				textEdit->scrollValue--;
			}
			else
			{
				break;
			}
		}
	}
}

/*
================================================================================

EVENT HANDLING

================================================================================
*/

static Size wz_text_edit_measure(struct WidgetImpl *widget)
{
	Size size;
	const struct TextEditImpl *textEdit = (struct TextEditImpl *)widget;

	if (textEdit->multiline)
	{
		size.w = 100;
		size.h = 100;
	}
	else
	{
		size.w = 100;
		size.h = wz_widget_get_line_height(widget) + textEdit->border.top + textEdit->border.bottom;
	}

	return size;
}

static void wz_text_edit_draw(struct WidgetImpl *widget, Rect clip)
{
	struct NVGcontext *vg = widget->renderer->vg;
	const struct TextEditImpl *textEdit = (struct TextEditImpl *)widget;
	const Rect rect = wz_widget_get_absolute_rect(widget);
	const Rect textRect = wz_text_edit_get_text_rect(textEdit);
	const int lineHeight = wz_widget_get_line_height(widget);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	
	nvgBeginPath(vg);
	nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, WZ_SKIN_TEXT_EDIT_CORNER_RADIUS);

	// Background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, WZ_SKIN_TEXT_EDIT_BG_COLOR1, WZ_SKIN_TEXT_EDIT_BG_COLOR2));
	nvgFill(vg);

	// Border.
	nvgStrokeColor(vg, widget->hover ? WZ_SKIN_TEXT_EDIT_BORDER_HOVER_COLOR : WZ_SKIN_TEXT_EDIT_BORDER_COLOR);
	nvgStroke(vg);

	// Clip to the text rect.
	if (!wz_renderer_clip_to_rect_intersection(vg, clip, textRect))
		return;

	// Text.
	if (textEdit->multiline)
	{
		LineBreakResult line;
		int selectionStartIndex, selectionEndIndex, lineY = 0;

		selectionStartIndex = wz_text_edit_get_selection_start_index(textEdit);
		selectionEndIndex = wz_text_edit_get_selection_end_index(textEdit);
		line.next = wz_text_edit_get_visible_text(textEdit);

		for (;;)
		{
			line = wz_renderer_line_break_text(widget->renderer, widget->fontFace, widget->fontSize, line.next, 0, textRect.w);

			if (line.length > 0)
			{
				// Draw this line.
				wz_renderer_print(widget->renderer, textRect.x, textRect.y + lineY, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, widget->fontFace, widget->fontSize, WZ_SKIN_TEXT_EDIT_TEXT_COLOR, line.start, line.length);

				// Selection.
				if (wz_text_edit_has_selection(textEdit))
				{
					int lineStartIndex;
					bool startOnThisLine, endOnThisLine, straddleThisLine;
					Position start, end;

					lineStartIndex = line.start - wz_text_edit_get_text(textEdit);
					startOnThisLine = selectionStartIndex >= lineStartIndex && selectionStartIndex <= lineStartIndex + (int)line.length;
					endOnThisLine = selectionEndIndex >= lineStartIndex && selectionEndIndex <= lineStartIndex + (int)line.length;
					straddleThisLine = selectionStartIndex < lineStartIndex && selectionEndIndex > lineStartIndex + (int)line.length;

					if (startOnThisLine)
					{
						start = wz_text_edit_position_from_index(textEdit, selectionStartIndex);
					}
					else
					{
						start = wz_text_edit_position_from_index(textEdit, lineStartIndex);
					}

					if (endOnThisLine)
					{
						end = wz_text_edit_position_from_index(textEdit, selectionEndIndex);
					}
					else
					{
						end = wz_text_edit_position_from_index(textEdit, lineStartIndex + (int)line.length);
					}

					if (startOnThisLine || straddleThisLine || endOnThisLine)
					{
						Rect selectionRect;
						selectionRect.x = textRect.x + start.x;
						selectionRect.y = textRect.y + start.y - lineHeight / 2;
						selectionRect.w = end.x - start.x;
						selectionRect.h = lineHeight;
						wz_renderer_draw_filled_rect(vg, selectionRect, WZ_SKIN_TEXT_EDIT_SELECTION_COLOR);
					}
				}
			}

			if (!line.next || !line.next[0])
				break;

			lineY += lineHeight;
		}
	}
	else
	{
		wz_renderer_print(widget->renderer, textRect.x, textRect.y + textRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_TEXT_EDIT_TEXT_COLOR, wz_text_edit_get_visible_text(textEdit), 0);

		// Selection.
		if (wz_text_edit_has_selection(textEdit))
		{
			Position position1, position2;
			Rect selectionRect;

			position1 = wz_text_edit_get_selection_start_position(textEdit);
			position2 = wz_text_edit_get_selection_end_position(textEdit);
			selectionRect.x = textRect.x + position1.x;
			selectionRect.y = textRect.y + position1.y - lineHeight / 2;
			selectionRect.w = position2.x - position1.x;
			selectionRect.h = lineHeight;
			wz_renderer_draw_filled_rect(vg, selectionRect, WZ_SKIN_TEXT_EDIT_SELECTION_COLOR);
		}
	}

	// Cursor.
	if (wz_main_window_text_cursor_is_visible(widget->mainWindow) && wz_widget_has_keyboard_focus((const struct WidgetImpl *)textEdit))
	{
		Position position;
		
		position = wz_text_edit_get_cursor_position(textEdit);
		position.x += textRect.x;
		position.y += textRect.y;

		wz_renderer_clip_to_rect(vg, rect);
		nvgBeginPath(vg);
		nvgMoveTo(vg, (float)position.x, position.y - lineHeight / 2.0f);
		nvgLineTo(vg, (float)position.x, position.y + lineHeight / 2.0f);
		nvgStrokeColor(vg, WZ_SKIN_TEXT_EDIT_CURSOR_COLOR);
		nvgStroke(vg);
	}

	nvgRestore(vg);
}

static void wz_text_edit_renderer_changed(struct WidgetImpl *widget)
{
	struct TextEditImpl *textEdit = (struct TextEditImpl *)widget;
	WZ_ASSERT(textEdit);
	textEdit->border.left = textEdit->border.top = textEdit->border.right = textEdit->border.bottom = 4;
}

static void wz_text_edit_set_rect(struct WidgetImpl *widget, Rect rect)
{
	struct TextEditImpl *textEdit;

	WZ_ASSERT(widget);
	widget->rect = rect;
	textEdit = (struct TextEditImpl *)widget;
	wz_text_edit_update_scroller(textEdit);
}

static void wz_text_edit_mouse_button_down(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct TextEditImpl *textEdit;
	
	WZ_ASSERT(widget);
	textEdit = (struct TextEditImpl *)widget;

	if (mouseButton == 1)
	{
		// Set keyboard focus to this widget.
		wz_main_window_set_keyboard_focus_widget(widget->mainWindow, widget);
	}

	if (mouseButton == 1 && WZ_POINT_IN_RECT(mouseX, mouseY, wz_text_edit_get_text_rect(textEdit)))
	{
		int oldCursorIndex;

		// Lock input to this widget.
		wz_main_window_push_lock_input_widget(widget->mainWindow, widget);

		// Move the cursor to the mouse position.
		oldCursorIndex = textEdit->cursorIndex;
		textEdit->cursorIndex = wz_text_edit_index_from_position(textEdit, mouseX, mouseY);

		if (textEdit->cursorIndex == -1)
		{
			// Couldn't get a valid index.
			textEdit->cursorIndex = oldCursorIndex;
			return;
		}

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

static void wz_text_edit_mouse_button_up(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct TextEditImpl *textEdit;
	
	WZ_ASSERT(widget);
	textEdit = (struct TextEditImpl *)widget;

	if (mouseButton == 1)
	{
		wz_main_window_pop_lock_input_widget(widget->mainWindow, widget);
		textEdit->pressed = false;
	}
}

static void wz_text_edit_mouse_move(struct WidgetImpl *widget, int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	struct TextEditImpl *textEdit;

	WZ_ASSERT(widget);
	textEdit = (struct TextEditImpl *)widget;

	if (!(widget->hover && WZ_POINT_IN_RECT(mouseX, mouseY, wz_text_edit_get_text_rect(textEdit))))
		return;
	
	wz_main_window_set_cursor(widget->mainWindow, WZ_CURSOR_IBEAM);

	if (textEdit->pressed)
	{
		// Move the cursor to the mouse position.
		int index = wz_text_edit_index_from_position(textEdit, mouseX, mouseY);

		if (index == -1)
			return;

		textEdit->cursorIndex = index;
		wz_text_edit_update_scroll_index(textEdit);

		// Set the selection end to the new cursor index.
		textEdit->selectionEndIndex = textEdit->cursorIndex;
	}
}

static void wz_text_edit_mouse_wheel_move(struct WidgetImpl *widget, int x, int y)
{
	struct TextEditImpl *textEdit;

	WZ_ASSERT(widget);
	textEdit = (struct TextEditImpl *)widget;

	if (textEdit->multiline && wz_widget_get_visible((struct WidgetImpl *)textEdit->scroller))
	{
		wz_scroller_set_value(textEdit->scroller, wz_scroller_get_value(textEdit->scroller) - y);
	}
}

// Helper function for moving the cursor while the selection (shift) key is held.
static void wz_text_edit_move_cursor_and_selection(struct TextEditImpl *textEdit, int newCursorIndex)
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

static void wz_text_edit_key_down(struct WidgetImpl *widget, Key key)
{
	struct TextEditImpl *textEdit;

	WZ_ASSERT(widget);
	textEdit = (struct TextEditImpl *)widget;

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
		else if (textEdit->cursorIndex < (int)textEdit->text.length())
		{
			textEdit->cursorIndex++;
		}
	}
	else if (key == (WZ_KEY_RIGHT | WZ_KEY_SHIFT_BIT) && textEdit->cursorIndex < (int)textEdit->text.length())
	{
		wz_text_edit_move_cursor_and_selection(textEdit, textEdit->cursorIndex + 1);
	}
	else if (WZ_KEY_MOD_OFF(key) == WZ_KEY_UP || WZ_KEY_MOD_OFF(key) == WZ_KEY_DOWN)
	{
		Position cursorPosition;
		int lineHeight, newCursorIndex;

		// Get the cursor position.
		cursorPosition = wz_text_edit_position_from_index(textEdit, textEdit->cursorIndex);

		// Get line height.
		lineHeight = wz_widget_get_line_height(textEdit);

		// Move the cursor up/down.
		cursorPosition.y += (WZ_KEY_MOD_OFF(key) == WZ_KEY_UP ? -lineHeight : lineHeight);
		newCursorIndex = wz_text_edit_index_from_relative_position(textEdit, cursorPosition);

		if (newCursorIndex == -1)
			return; // Couldn't move cursor.

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
	else if (WZ_KEY_MOD_OFF(key) == WZ_KEY_HOME || WZ_KEY_MOD_OFF(key) == WZ_KEY_END)
	{
		int newCursorIndex;

		// Go to text start/end.
		if (!textEdit->multiline || (textEdit->multiline && (key & WZ_KEY_CONTROL_BIT)))
		{
			if (WZ_KEY_MOD_OFF(key) == WZ_KEY_HOME)
			{
				newCursorIndex = 0;
			}
			else
			{
				newCursorIndex = textEdit->text.length();
			}
		}
		// Go to line start/end.
		else
		{
			// Find the line we're on.
			LineBreakResult line;
			int lineStartIndex, lineEndIndex;

			line.next = textEdit->text.c_str();

			for (;;)
			{
				line = wz_renderer_line_break_text(textEdit->renderer, wz_widget_get_font_face(textEdit), wz_widget_get_font_size(textEdit), line.next, 0, wz_text_edit_get_text_rect(textEdit).w);
				WZ_ASSERT(line.start);
				WZ_ASSERT(line.next);

				lineStartIndex = line.start - textEdit->text.c_str();
				lineEndIndex = lineStartIndex + line.length;

				// Is the cursor index on this line?
				if (textEdit->cursorIndex >= lineStartIndex && textEdit->cursorIndex <= lineEndIndex)
					break;

				if (!line.next || !line.next[0])
					return;
			}

			if (WZ_KEY_MOD_OFF(key) == WZ_KEY_HOME)
			{
				newCursorIndex = lineStartIndex;
			}
			else
			{
				newCursorIndex = lineEndIndex;
			}
		}

		if (key & WZ_KEY_SHIFT_BIT)
		{
			wz_text_edit_move_cursor_and_selection(textEdit, newCursorIndex);
		}
		else
		{
			textEdit->cursorIndex = newCursorIndex;

			// Clear the selection.
			textEdit->selectionStartIndex = textEdit->selectionEndIndex = 0;
		}
	}
	else if (textEdit->multiline && key == WZ_KEY_ENTER)
	{
		wz_text_edit_enter_text(textEdit, "\r");
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

static void wz_text_edit_text_input(struct WidgetImpl *widget, const char *text)
{
	struct TextEditImpl *textEdit;

	WZ_ASSERT(widget);
	WZ_ASSERT(text);
	textEdit = (struct TextEditImpl *)widget;

	if (textEdit->validate_text && !textEdit->validate_text(text))
		return;

	wz_text_edit_enter_text(textEdit, text);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

TextEditImpl::TextEditImpl()
{
	type = WZ_TYPE_TEXT_EDIT;
	validate_text = NULL;
	pressed = false;
	cursorIndex = scrollValue = 0;
	selectionStartIndex = selectionEndIndex = 0;
}

TextEdit::TextEdit(bool multiline)
{
	impl = wz_text_edit_create(multiline, 256);
}

TextEdit::TextEdit(const std::string &text, bool multiline)
{
	impl = wz_text_edit_create(multiline, 256);
	setText(text);
}

TextEdit::~TextEdit()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

TextEdit *TextEdit::setText(const std::string &text)
{
	wz_text_edit_set_text((TextEditImpl *)impl, text.c_str());
	return this;
}

struct TextEditImpl *wz_text_edit_create(bool multiline, int maximumTextLength)
{
	struct TextEditImpl *textEdit = new struct TextEditImpl;
	textEdit->vtable.measure = wz_text_edit_measure;
	textEdit->vtable.draw = wz_text_edit_draw;
	textEdit->vtable.renderer_changed = wz_text_edit_renderer_changed;
	textEdit->vtable.set_rect = wz_text_edit_set_rect;
	textEdit->vtable.mouse_button_down = wz_text_edit_mouse_button_down;
	textEdit->vtable.mouse_button_up = wz_text_edit_mouse_button_up;
	textEdit->vtable.mouse_move = wz_text_edit_mouse_move;
	textEdit->vtable.mouse_wheel_move = wz_text_edit_mouse_wheel_move;
	textEdit->vtable.key_down = wz_text_edit_key_down;
	textEdit->vtable.text_input = wz_text_edit_text_input;

	if (multiline)
	{
		textEdit->scroller = wz_scroller_create(WZ_SCROLLER_VERTICAL, 0, 1, 0);
		wz_widget_add_child_widget((struct WidgetImpl *)textEdit, (struct WidgetImpl *)textEdit->scroller);
		wz_text_edit_update_scroller(textEdit);
		wz_scroller_add_callback_value_changed(textEdit->scroller, wz_text_edit_scroller_value_changed);
	}

	textEdit->multiline = multiline;
	textEdit->maximumTextLength = maximumTextLength;
	textEdit->text = std::string();

	return textEdit;
}

void wz_text_edit_set_validate_text_callback(struct TextEditImpl *textEdit, TextEditValidateTextCallback callback)
{
	WZ_ASSERT(textEdit);
	textEdit->validate_text = callback;
}

bool wz_text_edit_is_multiline(const struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->multiline;
}

Border wz_text_edit_get_border(const struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->border;
}

Rect wz_text_edit_get_text_rect(const struct TextEditImpl *textEdit)
{
	Rect textRect;

	WZ_ASSERT(textEdit);
	textRect = wz_widget_get_absolute_rect((struct WidgetImpl *)textEdit);
	textRect.x += textEdit->border.left;
	textRect.y += textEdit->border.top;
	textRect.w -= textEdit->border.left + textEdit->border.right;
	textRect.h -= textEdit->border.top + textEdit->border.bottom;

	if (textEdit->multiline && wz_widget_get_visible((const struct WidgetImpl *)textEdit->scroller))
	{
		textRect.w -= wz_widget_get_width((const struct WidgetImpl *)textEdit->scroller);
	}

	return textRect;
}

const char *wz_text_edit_get_text(const struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->text.c_str();
}

void wz_text_edit_set_text(struct TextEditImpl *textEdit, const char *text)
{
	WZ_ASSERT(textEdit);
	textEdit->text = std::string(text);
	wz_widget_resize_to_measured(textEdit);
}

int wz_text_edit_get_scroll_value(const struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->scrollValue;
}

const char *wz_text_edit_get_visible_text(const struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);

	if (textEdit->multiline)
	{
		LineBreakResult line;
		int lineIndex = 0;

		line.next = textEdit->text.c_str();

		for (;;)
		{
			line = wz_renderer_line_break_text(textEdit->renderer, wz_widget_get_font_face(textEdit), wz_widget_get_font_size(textEdit), line.next, 0, wz_text_edit_get_text_rect(textEdit).w);

			if (lineIndex == textEdit->scrollValue)
				return line.start;

			if (!line.next || !line.next[0])
				break;

			lineIndex++;
		}

		return textEdit->text.c_str();
	}
	else
	{
		return &textEdit->text[textEdit->scrollValue];
	}
}

Position wz_text_edit_get_cursor_position(const struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);
	return wz_text_edit_position_from_index(textEdit, textEdit->cursorIndex);
}

bool wz_text_edit_has_selection(const struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);
	return textEdit->selectionStartIndex != textEdit->selectionEndIndex;
}

int wz_text_edit_get_selection_start_index(const struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);
	return WZ_MIN(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
}

Position wz_text_edit_get_selection_start_position(const struct TextEditImpl *textEdit)
{
	int index;

	WZ_ASSERT(textEdit);
	index = WZ_MIN(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
	return wz_text_edit_position_from_index(textEdit, index);
}

int wz_text_edit_get_selection_end_index(const struct TextEditImpl *textEdit)
{
	WZ_ASSERT(textEdit);
	return WZ_MAX(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
}

Position wz_text_edit_get_selection_end_position(const struct TextEditImpl *textEdit)
{
	int index;

	WZ_ASSERT(textEdit);
	index = WZ_MAX(textEdit->selectionStartIndex, textEdit->selectionEndIndex);
	return wz_text_edit_position_from_index(textEdit, index);
}

// Calculate the position of the index - relative to text rect - based on the cursor index and scroll index. 
Position wz_text_edit_position_from_index(const struct TextEditImpl *textEdit, int index)
{
	int lineHeight;
	Position position;

	WZ_ASSERT(textEdit);

	// Get the line height.
	lineHeight = wz_widget_get_line_height(textEdit);

	if (textEdit->text.length() == 0)
	{
		// Text is empty.
		position.x = 0;
		position.y = lineHeight / 2;
	}
	else if (textEdit->multiline)
	{
		LineBreakResult line;
		int lineNo = 0;

		// Iterate through lines.
		line.next = textEdit->text.c_str();

		for (;;)
		{
			int lineStartIndex;

			line = wz_renderer_line_break_text(textEdit->renderer, wz_widget_get_font_face(textEdit), wz_widget_get_font_size(textEdit), line.next, 0, wz_text_edit_get_text_rect(textEdit).w);
			WZ_ASSERT(line.start);
			WZ_ASSERT(line.next);
			lineStartIndex = line.start - textEdit->text.c_str();

			// Is the index on this line?
			if ((index >= lineStartIndex && index <= lineStartIndex + (int)line.length) || !line.next || !line.next[0])
			{
				int width = 0;

				if (index - lineStartIndex > 0)
				{
					wz_widget_measure_text(textEdit, line.start, index - lineStartIndex, &width, NULL);
				}

				position.x = width;
				position.y = (lineNo - textEdit->scrollValue) * lineHeight + lineHeight / 2;
				break;
			}

			lineNo++;
		}
	}
	else
	{
		int width = 0;
		int delta = index - textEdit->scrollValue;

		if (delta > 0)
		{
			// Text width from the scroll index to the requested index.
			wz_widget_measure_text(textEdit, &textEdit->text[textEdit->scrollValue], delta, &width, NULL);
		}
		else if (delta < 0)
		{
			// Text width from the requested index to the scroll index.
			wz_widget_measure_text(textEdit, &textEdit->text[textEdit->cursorIndex], -delta, &width, NULL);
			width = -width;
		}
	
		position.x = width;
		position.y = lineHeight / 2;
	}

	return position;
}

/*
================================================================================

PRIVATE INTERFACE

================================================================================
*/

void wz_text_edit_set_border(struct TextEditImpl *textEdit, Border border)
{
	WZ_ASSERT(textEdit);
	textEdit->border = border;
}

} // namespace wz
