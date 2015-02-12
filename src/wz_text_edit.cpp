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
#include "wz_internal.h"
#pragma hdrstop

namespace wz {

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
		line = textEdit->lineBreakText(line.next, 0, lineWidth);

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
	lineHeight = textEdit->getLineHeight();
	nLines = wz_calculate_num_lines(textEdit, textEdit->rect.w - (textEdit->border.left + textEdit->border.right));

	if (lineHeight * nLines > textEdit->rect.h - (textEdit->border.top + textEdit->border.bottom))
	{
		textEdit->scroller->setVisible(true);
	}
	else
	{
		textEdit->scroller->setVisible(false);
		return;
	}

	// Update max value.
	nLines = wz_calculate_num_lines(textEdit, textEdit->getTextRect().w);
	max = nLines - (textEdit->getTextRect().h / lineHeight);
	textEdit->scroller->setMaxValue(max);

	// Fit to the right of the rect. Width doesn't change.
	textEditRect = textEdit->getRect();
	rect.w = (textEdit->scroller)->rect.w;
	rect.x = textEditRect.w - textEdit->border.right - rect.w;
	rect.y = textEdit->border.top;
	rect.h = textEditRect.h - (textEdit->border.top + textEdit->border.bottom);
	textEdit->scroller->setRectInternal(rect);

	// Now that the height has been calculated, update the nub scale.
	maxHeight = nLines * lineHeight;
	textEdit->scroller->setNubScale(1.0f - ((maxHeight - rect.h) / (float)maxHeight));
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
	rect = textEdit->getAbsoluteRect();

	if (textEdit->multiline)
	{
		int lineHeight, lineY;
		LineBreakResult line;

		// Get line height.
		lineHeight = textEdit->getLineHeight();

		// Set line starting position. May be outside the widget.
		lineY = lineHeight * -textEdit->scrollValue;

		// Iterate through lines.
		result = 0;
		line.next = textEdit->text.c_str();

		for (;;)
		{
			line = textEdit->lineBreakText(line.next, 0, textEdit->getTextRect().w);

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
			textEdit->measureText(line.start, i, &width, NULL);

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
			textEdit->measureText(&textEdit->text[textEdit->scrollValue], i, &width, NULL);

			// Check if we've gone beyond the width of the widget.
			if (width > textEdit->getTextRect().w)
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
	rect = textEdit->getAbsoluteRect();
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
			int cursorY = textEdit->positionFromIndex(textEdit->cursorIndex).y;

			if (cursorY > textEdit->rect.h - (textEdit->border.top + textEdit->border.bottom))
			{
				textEdit->scrollValue++;
				textEdit->scroller->setValue(textEdit->scrollValue);
			}
			else if (cursorY < 0)
			{
				textEdit->scrollValue--;
				textEdit->scroller->setValue(textEdit->scrollValue);
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
			int cursorX = textEdit->positionFromIndex(textEdit->cursorIndex).x;

			if (cursorX > textEdit->getTextRect().w)
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

TextEditImpl::TextEditImpl(bool multiline, int maximumTextLength)
{
	type = WZ_TYPE_TEXT_EDIT;
	validate_text = NULL;
	pressed = false;
	cursorIndex = scrollValue = 0;
	selectionStartIndex = selectionEndIndex = 0;

	if (multiline)
	{
		scroller = new ScrollerImpl(WZ_SCROLLER_VERTICAL, 0, 1, 0);
		addChildWidget(scroller);
		wz_text_edit_update_scroller(this);
		scroller->addCallbackValueChanged(wz_text_edit_scroller_value_changed);
	}

	this->multiline = multiline;
	this->maximumTextLength = maximumTextLength;
}

void TextEditImpl::onRendererChanged()
{
	border.left = border.top = border.right = border.bottom = 4;
}

void TextEditImpl::onRectChanged()
{
	wz_text_edit_update_scroller(this);
}

void TextEditImpl::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		// Set keyboard focus to this widget.
		mainWindow->setKeyboardFocusWidget(this);
	}

	if (mouseButton == 1 && WZ_POINT_IN_RECT(mouseX, mouseY, getTextRect()))
	{
		// Lock input to this widget.
		mainWindow->pushLockInputWidget(this);

		// Move the cursor to the mouse position.
		int oldCursorIndex = cursorIndex;
		cursorIndex = wz_text_edit_index_from_position(this, mouseX, mouseY);

		if (cursorIndex == -1)
		{
			// Couldn't get a valid index.
			cursorIndex = oldCursorIndex;
			return;
		}

		wz_text_edit_update_scroll_index(this);
		pressed = true;

		// Handle selecting.
		if (mainWindow->isShiftKeyDown())
		{
			// Start a new selection if there isn't one.
			if (selectionStartIndex == selectionEndIndex)
			{
				// Use the old cursor index as the selection start.
				selectionStartIndex = oldCursorIndex;
			}

			selectionEndIndex = cursorIndex;
		}
		else
		{
			selectionStartIndex = cursorIndex;
			selectionEndIndex = cursorIndex;
		}
	}
}

void TextEditImpl::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		mainWindow->popLockInputWidget(this);
		pressed = false;
	}
}

void TextEditImpl::onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	if (!(hover && WZ_POINT_IN_RECT(mouseX, mouseY, getTextRect())))
		return;
	
	mainWindow->setCursor(WZ_CURSOR_IBEAM);

	if (pressed)
	{
		// Move the cursor to the mouse position.
		int index = wz_text_edit_index_from_position(this, mouseX, mouseY);

		if (index == -1)
			return;

		cursorIndex = index;
		wz_text_edit_update_scroll_index(this);

		// Set the selection end to the new cursor index.
		selectionEndIndex = cursorIndex;
	}
}

void TextEditImpl::onMouseWheelMove(int /*x*/, int y)
{
	if (multiline && scroller->getVisible())
	{
		scroller->setValue(scroller->getValue() - y);
	}
}

void TextEditImpl::onKeyDown(Key key)
{
	if (key == WZ_KEY_LEFT)
	{
		if (selectionStartIndex != selectionEndIndex)
		{
			// If the cursor is to the right of the selection start, move the cursor to the start of the selection.
			if (cursorIndex > selectionStartIndex)
			{
				cursorIndex = selectionStartIndex;
			}

			// Clear the selection.
			selectionStartIndex = selectionEndIndex = 0;
		}
		// Move the cursor to the left, if there's room.
		else if (cursorIndex > 0)
		{
			cursorIndex--;
		}
	}
	else if (key == (WZ_KEY_LEFT | WZ_KEY_SHIFT_BIT) && cursorIndex > 0)
	{
		wz_text_edit_move_cursor_and_selection(this, cursorIndex - 1);
	}
	else if (key == WZ_KEY_RIGHT)
	{
		if (selectionStartIndex != selectionEndIndex)
		{
			// If the cursor is to the left of the selection start, move the cursor to the start of the selection.
			if (cursorIndex < selectionStartIndex)
			{
				cursorIndex = selectionStartIndex;
			}

			// Clear the selection.
			selectionStartIndex = selectionEndIndex = 0;
		}
		// Move the cursor to the right, if there's room.
		else if (cursorIndex < (int)text.length())
		{
			cursorIndex++;
		}
	}
	else if (key == (WZ_KEY_RIGHT | WZ_KEY_SHIFT_BIT) && cursorIndex < (int)text.length())
	{
		wz_text_edit_move_cursor_and_selection(this, cursorIndex + 1);
	}
	else if (WZ_KEY_MOD_OFF(key) == WZ_KEY_UP || WZ_KEY_MOD_OFF(key) == WZ_KEY_DOWN)
	{
		Position cursorPosition;
		int lineHeight, newCursorIndex;

		// Get the cursor position.
		cursorPosition = positionFromIndex(cursorIndex);

		// Get line height.
		lineHeight = getLineHeight();

		// Move the cursor up/down.
		cursorPosition.y += (WZ_KEY_MOD_OFF(key) == WZ_KEY_UP ? -lineHeight : lineHeight);
		newCursorIndex = wz_text_edit_index_from_relative_position(this, cursorPosition);

		if (newCursorIndex == -1)
			return; // Couldn't move cursor.

		// Apply the new cursor index.
		if ((key & WZ_KEY_SHIFT_BIT) != 0)
		{
			wz_text_edit_move_cursor_and_selection(this, newCursorIndex);
		}
		else if (key == WZ_KEY_UP || key == WZ_KEY_DOWN)
		{
			cursorIndex = newCursorIndex;

			// Clear the selection.
			selectionStartIndex = selectionEndIndex = 0;
		}
	}
	else if (WZ_KEY_MOD_OFF(key) == WZ_KEY_HOME || WZ_KEY_MOD_OFF(key) == WZ_KEY_END)
	{
		int newCursorIndex;

		// Go to text start/end.
		if (!multiline || (multiline && (key & WZ_KEY_CONTROL_BIT)))
		{
			if (WZ_KEY_MOD_OFF(key) == WZ_KEY_HOME)
			{
				newCursorIndex = 0;
			}
			else
			{
				newCursorIndex = text.length();
			}
		}
		// Go to line start/end.
		else
		{
			// Find the line we're on.
			LineBreakResult line;
			int lineStartIndex, lineEndIndex;

			line.next = text.c_str();

			for (;;)
			{
				line = lineBreakText(line.next, 0, getTextRect().w);
				WZ_ASSERT(line.start);
				WZ_ASSERT(line.next);

				lineStartIndex = line.start - text.c_str();
				lineEndIndex = lineStartIndex + line.length;

				// Is the cursor index on this line?
				if (cursorIndex >= lineStartIndex && cursorIndex <= lineEndIndex)
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
			wz_text_edit_move_cursor_and_selection(this, newCursorIndex);
		}
		else
		{
			cursorIndex = newCursorIndex;

			// Clear the selection.
			selectionStartIndex = selectionEndIndex = 0;
		}
	}
	else if (multiline && key == WZ_KEY_ENTER)
	{
		wz_text_edit_enter_text(this, "\r");
	}
	else if (key == WZ_KEY_DELETE)
	{
		if (selectionStartIndex != selectionEndIndex)
		{
			wz_text_edit_delete_selection(this);
		}
		else
		{
			wz_text_edit_delete_text(this, cursorIndex, 1);
		}
	}
	else if (key == WZ_KEY_BACKSPACE && cursorIndex > 0)
	{
		if (selectionStartIndex != selectionEndIndex)
		{
			wz_text_edit_delete_selection(this);
		}
		else
		{
			wz_text_edit_delete_text(this, cursorIndex - 1, 1);
			cursorIndex--;
		}
	}
	else
	{
		return;
	}

	wz_text_edit_update_scroll_index(this);
}

void TextEditImpl::onTextInput(const char *text)
{
	if (validate_text && !validate_text(text))
		return;

	wz_text_edit_enter_text(this, text);
}

void TextEditImpl::draw(Rect clip)
{
	renderer->drawTextEdit(this, clip);
}

Size TextEditImpl::measure()
{
	return renderer->measureTextEdit(this);
}

void TextEditImpl::setValidateTextCallback(TextEditValidateTextCallback callback)
{
	validate_text = callback;
}

bool TextEditImpl::isMultiline() const
{
	return multiline;
}

Border TextEditImpl::getBorder() const
{
	return border;
}

void TextEditImpl::setBorder(Border border)
{
	this->border = border;
}

Rect TextEditImpl::getTextRect() const
{
	Rect textRect;
	textRect = getAbsoluteRect();
	textRect.x += border.left;
	textRect.y += border.top;
	textRect.w -= border.left + border.right;
	textRect.h -= border.top + border.bottom;

	if (multiline && scroller->getVisible())
	{
		textRect.w -= scroller->getWidth();
	}

	return textRect;
}

const char *TextEditImpl::getText() const
{
	return text.c_str();
}

void TextEditImpl::setText(const char *text)
{
	this->text = text;
	resizeToMeasured();
}

int TextEditImpl::getScrollValue() const
{
	return scrollValue;
}

const char *TextEditImpl::getVisibleText() const
{
	if (multiline)
	{
		LineBreakResult line;
		int lineIndex = 0;

		line.next = text.c_str();

		for (;;)
		{
			line = lineBreakText(line.next, 0, getTextRect().w);

			if (lineIndex == scrollValue)
				return line.start;

			if (!line.next || !line.next[0])
				break;

			lineIndex++;
		}

		return text.c_str();
	}
	else
	{
		return &text[scrollValue];
	}
}

Position TextEditImpl::getCursorPosition() const
{
	return positionFromIndex(cursorIndex);
}

bool TextEditImpl::hasSelection() const
{
	return selectionStartIndex != selectionEndIndex;
}

int TextEditImpl::getSelectionStartIndex() const
{
	return WZ_MIN(selectionStartIndex, selectionEndIndex);
}

Position TextEditImpl::getSelectionStartPosition() const
{
	return positionFromIndex(WZ_MIN(selectionStartIndex, selectionEndIndex));
}

int TextEditImpl::getSelectionEndIndex() const
{
	return WZ_MAX(selectionStartIndex, selectionEndIndex);
}

Position TextEditImpl::getSelectionEndPosition() const
{
	return positionFromIndex(WZ_MAX(selectionStartIndex, selectionEndIndex));
}

Position TextEditImpl::positionFromIndex(int index) const
{
	// Get the line height.
	const int lineHeight = getLineHeight();
	Position position;

	if (text.length() == 0)
	{
		// Text is empty.
		position.x = 0;
		position.y = lineHeight / 2;
	}
	else if (multiline)
	{
		LineBreakResult line;
		int lineNo = 0;

		// Iterate through lines.
		line.next = text.c_str();

		for (;;)
		{
			int lineStartIndex;

			line = lineBreakText(line.next, 0, getTextRect().w);
			WZ_ASSERT(line.start);
			WZ_ASSERT(line.next);
			lineStartIndex = line.start - text.c_str();

			// Is the index on this line?
			if ((index >= lineStartIndex && index <= lineStartIndex + (int)line.length) || !line.next || !line.next[0])
			{
				int width = 0;

				if (index - lineStartIndex > 0)
				{
					measureText(line.start, index - lineStartIndex, &width, NULL);
				}

				position.x = width;
				position.y = (lineNo - scrollValue) * lineHeight + lineHeight / 2;
				break;
			}

			lineNo++;
		}
	}
	else
	{
		int width = 0;
		int delta = index - scrollValue;

		if (delta > 0)
		{
			// Text width from the scroll index to the requested index.
			measureText(&text[scrollValue], delta, &width, NULL);
		}
		else if (delta < 0)
		{
			// Text width from the requested index to the scroll index.
			measureText(&text[cursorIndex], -delta, &width, NULL);
			width = -width;
		}
	
		position.x = width;
		position.y = lineHeight / 2;
	}

	return position;
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

TextEdit::TextEdit(bool multiline)
{
	impl.reset(new TextEditImpl(multiline, 256));
}

TextEdit::TextEdit(const std::string &text, bool multiline)
{
	impl.reset(new TextEditImpl(multiline, 256));
	setText(text);
}

TextEdit::~TextEdit()
{
}

TextEdit *TextEdit::setText(const std::string &text)
{
	getImpl()->setText(text.c_str());
	return this;
}

TextEditImpl *TextEdit::getImpl()
{
	return (TextEditImpl *)impl.get();
}

const TextEditImpl *TextEdit::getImpl() const
{
	return (const TextEditImpl *)impl.get();
}

} // namespace wz
