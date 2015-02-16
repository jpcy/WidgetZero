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
#include "wz.h"
#pragma hdrstop

namespace wz {

TextEdit::TextEdit(bool multiline, const std::string &text)
{
	type = WZ_TYPE_TEXT_EDIT;
	validate_text = NULL;
	pressed = false;
	cursorIndex = scrollValue = 0;
	selectionStartIndex = selectionEndIndex = 0;

	if (multiline)
	{
		scroller = new Scroller(WZ_SCROLLER_VERTICAL, 0, 1, 0);
		addChildWidget(scroller);
		updateScroller();
		scroller->addEventHandler(WZ_EVENT_SCROLLER_VALUE_CHANGED, this, &TextEdit::onScrollerValueChanged);
	}

	this->multiline = multiline;
	this->text = text;
}

void TextEdit::onRendererChanged()
{
	border.left = border.top = border.right = border.bottom = 4;
}

void TextEdit::onRectChanged()
{
	updateScroller();
}

void TextEdit::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
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
		cursorIndex = indexFromPosition(mouseX, mouseY);

		if (cursorIndex == -1)
		{
			// Couldn't get a valid index.
			cursorIndex = oldCursorIndex;
			return;
		}

		updateScrollIndex();
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

void TextEdit::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		mainWindow->popLockInputWidget(this);
		pressed = false;
	}
}

void TextEdit::onMouseMove(int mouseX, int mouseY, int mouseDeltaX, int mouseDeltaY)
{
	if (!(hover && WZ_POINT_IN_RECT(mouseX, mouseY, getTextRect())))
		return;
	
	mainWindow->setCursor(WZ_CURSOR_IBEAM);

	if (pressed)
	{
		// Move the cursor to the mouse position.
		int index = indexFromPosition(mouseX, mouseY);

		if (index == -1)
			return;

		cursorIndex = index;
		updateScrollIndex();

		// Set the selection end to the new cursor index.
		selectionEndIndex = cursorIndex;
	}
}

void TextEdit::onMouseWheelMove(int /*x*/, int y)
{
	if (multiline && scroller->getVisible())
	{
		scroller->setValue(scroller->getValue() - y);
	}
}

void TextEdit::onKeyDown(Key key)
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
		moveCursorAndSelection(cursorIndex - 1);
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
		moveCursorAndSelection(cursorIndex + 1);
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
		newCursorIndex = indexFromRelativePosition(cursorPosition);

		if (newCursorIndex == -1)
			return; // Couldn't move cursor.

		// Apply the new cursor index.
		if ((key & WZ_KEY_SHIFT_BIT) != 0)
		{
			moveCursorAndSelection(newCursorIndex);
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
			moveCursorAndSelection(newCursorIndex);
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
		enterText("\r");
	}
	else if (key == WZ_KEY_DELETE)
	{
		if (selectionStartIndex != selectionEndIndex)
		{
			deleteSelectedText();
		}
		else
		{
			deleteText(cursorIndex, 1);
		}
	}
	else if (key == WZ_KEY_BACKSPACE && cursorIndex > 0)
	{
		if (selectionStartIndex != selectionEndIndex)
		{
			deleteSelectedText();
		}
		else
		{
			deleteText(cursorIndex - 1, 1);
			cursorIndex--;
		}
	}
	else
	{
		return;
	}

	updateScrollIndex();
}

void TextEdit::onTextInput(const char *text)
{
	if (validate_text && !validate_text(text))
		return;

	enterText(text);
}

void TextEdit::draw(Rect clip)
{
	renderer->drawTextEdit(this, clip);
}

Size TextEdit::measure()
{
	return renderer->measureTextEdit(this);
}

void TextEdit::setValidateTextCallback(TextEditValidateTextCallback callback)
{
	validate_text = callback;
}

bool TextEdit::isMultiline() const
{
	return multiline;
}

Border TextEdit::getBorder() const
{
	return border;
}

void TextEdit::setBorder(Border border)
{
	this->border = border;
}

Rect TextEdit::getTextRect() const
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

const char *TextEdit::getText() const
{
	return text.c_str();
}

void TextEdit::setText(const char *text)
{
	this->text = text;
	resizeToMeasured();
}

int TextEdit::getScrollValue() const
{
	return scrollValue;
}

const char *TextEdit::getVisibleText() const
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

Position TextEdit::getCursorPosition() const
{
	return positionFromIndex(cursorIndex);
}

bool TextEdit::hasSelection() const
{
	return selectionStartIndex != selectionEndIndex;
}

int TextEdit::getSelectionStartIndex() const
{
	return WZ_MIN(selectionStartIndex, selectionEndIndex);
}

Position TextEdit::getSelectionStartPosition() const
{
	return positionFromIndex(WZ_MIN(selectionStartIndex, selectionEndIndex));
}

int TextEdit::getSelectionEndIndex() const
{
	return WZ_MAX(selectionStartIndex, selectionEndIndex);
}

Position TextEdit::getSelectionEndPosition() const
{
	return positionFromIndex(WZ_MAX(selectionStartIndex, selectionEndIndex));
}

Position TextEdit::positionFromIndex(int index) const
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

void TextEdit::onScrollerValueChanged(Event e)
{
	scrollValue = e.scroller.value;
}

int TextEdit::calculateNumLines(int lineWidth)
{
	if (!multiline)
		return 0;

	LineBreakResult line;
	line.next = text.c_str();
	int nLines = 0;

	for (;;)
	{
		line = lineBreakText(line.next, 0, lineWidth);

		if (!line.next || !line.next[0])
			break;

		nLines++;
	}

	return nLines;
}

void TextEdit::updateScroller()
{
	if (!multiline)
		return;

	if (!mainWindow)
		return; // Not added yet

	// Hide/show scroller depending on if it's needed.
	const int lineHeight = getLineHeight();
	int nLines = calculateNumLines(rect.w - (border.left + border.right));

	if (lineHeight * nLines > rect.h - (border.top + border.bottom))
	{
		scroller->setVisible(true);
	}
	else
	{
		scroller->setVisible(false);
		return;
	}

	// Update max value.
	nLines = calculateNumLines(getTextRect().w);
	int max = nLines - (getTextRect().h / lineHeight);
	scroller->setMaxValue(max);

	// Fit to the right of the rect. Width doesn't change.
	Rect scrollerRect;
	scrollerRect.w = scroller->rect.w;
	scrollerRect.x = rect.w - border.right - scrollerRect.w;
	scrollerRect.y = border.top;
	scrollerRect.h = rect.h - (border.top + border.bottom);
	scroller->setRectInternal(scrollerRect);

	// Now that the height has been calculated, update the nub scale.
	const int maxHeight = nLines * lineHeight;
	scroller->setNubScale(1.0f - ((maxHeight - scrollerRect.h) / (float)maxHeight));
}

void TextEdit::insertText(int index, const char *text, int n)
{
	WZ_ASSERT(text);
	this->text.insert(index, text, n);

	// Update the scroller.
	updateScroller();
}

void TextEdit::enterText(const char *text)
{
	WZ_ASSERT(text);
	const int n = (int)strlen(text);

	if (n == 0)
		return;

	// The text replaces the selection.
	if (selectionStartIndex != selectionEndIndex)
	{
		deleteSelectedText();
	}

	insertText(cursorIndex, text, n);
	cursorIndex += n;
	updateScrollIndex();
}

void TextEdit::deleteText(int index, int n)
{
	if (index < 0 || index >= (int)text.length())
		return;

	text.erase(index, n);

	// Update the scroller.
	updateScroller();
}

void TextEdit::deleteSelectedText()
{
	// No selection.
	if (selectionStartIndex == selectionEndIndex)
		return;

	const int start = WZ_MIN(selectionStartIndex, selectionEndIndex);
	const int end = WZ_MAX(selectionStartIndex, selectionEndIndex);
	deleteText(start, end - start);

	// Move the cursor to the start (smallest of start and end, not the real selection start).
	cursorIndex = start;

	// Clear the selection.
	selectionStartIndex = selectionEndIndex = 0;
}

int TextEdit::indexFromRelativePosition(Position pos) const
{
	int result = 0;

	// Calculate relative position.
	const Rect absRect = getAbsoluteRect();

	if (multiline)
	{
		// Get line height.
		int lineHeight = getLineHeight();

		// Set line starting position. May be outside the widget.
		int lineY = lineHeight * -scrollValue;

		// Iterate through lines.
		LineBreakResult line;
		line.next = text.c_str();

		for (;;)
		{
			line = lineBreakText(line.next, 0, getTextRect().w);
			result = line.start - text.c_str();

			if (pos.y >= lineY && pos.y < lineY + lineHeight)
				break; // On this line.

			if (!line.next || !line.next[0])
				return -1;

			lineY += lineHeight;
		}

		// Walk through the text until we find two glyphs that the x coordinate straddles.
		int previousWidth = 0;

		for (int i = 1; i <= (int)line.length; i++)
		{
			int width, deltaWidth;

			// Calculate the width of the text up to the current character.
			measureText(line.start, i, &width, NULL);

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
		if (pos.x < 0 || pos.x > absRect.w || pos.y < 0 || pos.y > absRect.h)
			return scrollValue;

		// Walk through the text until we find two glyphs that the x coordinate straddles.
		int previousWidth = 0;
		result = scrollValue;

		for (int i = 1; i <= (int)text.length(); i++)
		{
			// Calculate the width of the text up to the current character.
			int width;
			measureText(&text[scrollValue], i, &width, NULL);

			// Check if we've gone beyond the width of the widget.
			if (width > getTextRect().w)
			{
				result = scrollValue + i - 1;
				break;
			}

			// Calculate the change in text width since the last iteration.
			const int deltaWidth = width - previousWidth;

			// Check the intersection between the position and the change in text width since the last iteration.
			if (pos.x >= previousWidth && pos.x <= previousWidth + deltaWidth / 2)
			{
				// Left side of glyph.
				result = scrollValue + i - 1;
				break;
			}
			else if (pos.x >= previousWidth + deltaWidth / 2 && pos.x <= width)
			{
				// Right side of glyph.
				result = scrollValue + i;
				break;
			}

			// Made it to the end of text string.
			if (i == text.length())
			{
				result = i;
				break;
			}

			// Store this text width for the next iteration.
			previousWidth = width;
		}
	}

	return WZ_CLAMPED(0, result, (int)text.length());
}

int TextEdit::indexFromPosition(int x, int y)
{
	// Make position relative.f
	const Rect absRect = getAbsoluteRect();
	const Position pos(x - absRect.x, y - absRect.y);

	return indexFromRelativePosition(pos);
}

// Update the scroll value so the cursor is visible.
void TextEdit::updateScrollIndex()
{
	if (multiline)
	{
		for (;;)
		{
			const int cursorY = positionFromIndex(cursorIndex).y;

			if (cursorY > rect.h - (border.top + border.bottom))
			{
				scrollValue++;
				scroller->setValue(scrollValue);
			}
			else if (cursorY < 0)
			{
				scrollValue--;
				scroller->setValue(scrollValue);
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
			const int cursorX = positionFromIndex(cursorIndex).x;

			if (cursorX > getTextRect().w)
			{
				scrollValue++;
			}
			else if (cursorX < 0)
			{
				scrollValue--;
			}
			else
			{
				break;
			}
		}
	}
}

void TextEdit::moveCursorAndSelection(int newCursorIndex)
{
	if (selectionStartIndex != selectionEndIndex)
	{
		// If there's already a selection, move the cursor, and the selection end to match.
		cursorIndex = newCursorIndex;
		selectionEndIndex = cursorIndex;
	}
	else
	{
		// No selection, start a new one and move the cursor.
		selectionStartIndex = cursorIndex;
		cursorIndex = newCursorIndex;
		selectionEndIndex = cursorIndex;
	}
}

} // namespace wz
