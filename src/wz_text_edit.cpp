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
	type_ = WidgetType::TextEdit;
	validateText_ = NULL;
	pressed_ = false;
	cursorIndex_ = scrollValue_ = 0;
	selectionStartIndex_ = selectionEndIndex_ = 0;

	if (multiline)
	{
		scroller_ = new Scroller(ScrollerDirection::Vertical, 0, 1, 0);
		addChildWidget(scroller_);
		updateScroller();
		scroller_->addEventHandler(EventType::ScrollerValueChanged, this, &TextEdit::onScrollerValueChanged);
	}

	multiline_ = multiline;
	text_ = text;
}

void TextEdit::setValidateTextCallback(TextEditValidateTextCallback callback)
{
	validateText_ = callback;
}

bool TextEdit::isMultiline() const
{
	return multiline_;
}

Border TextEdit::getBorder() const
{
	return border_;
}

void TextEdit::setBorder(Border border)
{
	border_ = border;
}

Rect TextEdit::getTextRect() const
{
	Rect textRect;
	textRect = getAbsoluteRect();
	textRect.x += border_.left;
	textRect.y += border_.top;
	textRect.w -= border_.left + border_.right;
	textRect.h -= border_.top + border_.bottom;

	if (multiline_ && scroller_->isVisible())
	{
		textRect.w -= scroller_->getWidth();
	}

	return textRect;
}

const char *TextEdit::getText() const
{
	return text_.c_str();
}

void TextEdit::setText(const char *text)
{
	text_ = text;
	setMeasureDirty();
}

int TextEdit::getScrollValue() const
{
	return scrollValue_;
}

const char *TextEdit::getVisibleText() const
{
	if (multiline_)
	{
		LineBreakResult line;
		int lineIndex = 0;

		line.next = text_.c_str();

		for (;;)
		{
			line = lineBreakText(line.next, 0, getTextRect().w);

			if (lineIndex == scrollValue_)
				return line.start;

			if (!line.next || !line.next[0])
				break;

			lineIndex++;
		}

		return text_.c_str();
	}
	else
	{
		return &text_[scrollValue_];
	}
}

Position TextEdit::getCursorPosition() const
{
	return positionFromIndex(cursorIndex_);
}

bool TextEdit::hasSelection() const
{
	return selectionStartIndex_ != selectionEndIndex_;
}

int TextEdit::getSelectionStartIndex() const
{
	return WZ_MIN(selectionStartIndex_, selectionEndIndex_);
}

Position TextEdit::getSelectionStartPosition() const
{
	return positionFromIndex(WZ_MIN(selectionStartIndex_, selectionEndIndex_));
}

int TextEdit::getSelectionEndIndex() const
{
	return WZ_MAX(selectionStartIndex_, selectionEndIndex_);
}

Position TextEdit::getSelectionEndPosition() const
{
	return positionFromIndex(WZ_MAX(selectionStartIndex_, selectionEndIndex_));
}

Position TextEdit::positionFromIndex(int index) const
{
	// Get the line height.
	const int lineHeight = getLineHeight();
	Position position;

	if (text_.length() == 0)
	{
		// Text is empty.
		position.x = 0;
		position.y = lineHeight / 2;
	}
	else if (multiline_)
	{
		LineBreakResult line;
		int lineNo = 0;

		// Iterate through lines.
		line.next = text_.c_str();

		for (;;)
		{
			int lineStartIndex;

			line = lineBreakText(line.next, 0, getTextRect().w);
			WZ_ASSERT(line.start);
			WZ_ASSERT(line.next);
			lineStartIndex = line.start - text_.c_str();

			// Is the index on this line?
			if ((index >= lineStartIndex && index <= lineStartIndex + (int)line.length) || !line.next || !line.next[0])
			{
				int width = 0;

				if (index - lineStartIndex > 0)
				{
					measureText(line.start, index - lineStartIndex, &width, NULL);
				}

				position.x = width;
				position.y = (lineNo - scrollValue_) * lineHeight + lineHeight / 2;
				break;
			}

			lineNo++;
		}
	}
	else
	{
		int width = 0;
		int delta = index - scrollValue_;

		if (delta > 0)
		{
			// Text width from the scroll index to the requested index.
			measureText(&text_[scrollValue_], delta, &width, NULL);
		}
		else if (delta < 0)
		{
			// Text width from the requested index to the scroll index.
			measureText(&text_[cursorIndex_], -delta, &width, NULL);
			width = -width;
		}
	
		position.x = width;
		position.y = lineHeight / 2;
	}

	return position;
}

void TextEdit::onRendererChanged()
{
	border_.left = border_.top = border_.right = border_.bottom = 4;
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
		mainWindow_->setKeyboardFocusWidget(this);
	}

	if (mouseButton == 1 && WZ_POINT_IN_RECT(mouseX, mouseY, getTextRect()))
	{
		// Lock input to this widget.
		mainWindow_->pushLockInputWidget(this);

		// Move the cursor to the mouse position.
		int oldCursorIndex = cursorIndex_;
		cursorIndex_ = indexFromPosition(mouseX, mouseY);

		if (cursorIndex_ == -1)
		{
			// Couldn't get a valid index.
			cursorIndex_ = oldCursorIndex;
			return;
		}

		updateScrollIndex();
		pressed_ = true;

		// Handle selecting.
		if (mainWindow_->isShiftKeyDown())
		{
			// Start a new selection if there isn't one.
			if (selectionStartIndex_ == selectionEndIndex_)
			{
				// Use the old cursor index as the selection start.
				selectionStartIndex_ = oldCursorIndex;
			}

			selectionEndIndex_ = cursorIndex_;
		}
		else
		{
			selectionStartIndex_ = cursorIndex_;
			selectionEndIndex_ = cursorIndex_;
		}
	}
}

void TextEdit::onMouseButtonUp(int mouseButton, int /*mouseX*/, int /*mouseY*/)
{
	if (mouseButton == 1)
	{
		mainWindow_->popLockInputWidget(this);
		pressed_ = false;
	}
}

void TextEdit::onMouseMove(int mouseX, int mouseY, int /*mouseDeltaX*/, int /*mouseDeltaY*/)
{
	if (!(hover_ && WZ_POINT_IN_RECT(mouseX, mouseY, getTextRect())))
		return;

	mainWindow_->setCursor(Cursor::Ibeam);

	if (pressed_)
	{
		// Move the cursor to the mouse position.
		int index = indexFromPosition(mouseX, mouseY);

		if (index == -1)
			return;

		cursorIndex_ = index;
		updateScrollIndex();

		// Set the selection end to the new cursor index.
		selectionEndIndex_ = cursorIndex_;
	}
}

void TextEdit::onMouseWheelMove(int /*x*/, int y)
{
	if (multiline_ && scroller_->isVisible())
	{
		scroller_->setValue(scroller_->getValue() - y);
	}
}

void TextEdit::onKeyDown(Key::Enum key)
{
	if (key == Key::LeftArrow)
	{
		if (selectionStartIndex_ != selectionEndIndex_)
		{
			// If the cursor is to the right of the selection start, move the cursor to the start of the selection.
			if (cursorIndex_ > selectionStartIndex_)
			{
				cursorIndex_ = selectionStartIndex_;
			}

			// Clear the selection.
			selectionStartIndex_ = selectionEndIndex_ = 0;
		}
		// Move the cursor to the left, if there's room.
		else if (cursorIndex_ > 0)
		{
			cursorIndex_--;
		}
	}
	else if (key == (Key::LeftArrow | Key::ShiftBit) && cursorIndex_ > 0)
	{
		moveCursorAndSelection(cursorIndex_ - 1);
	}
	else if (key == Key::RightArrow)
	{
		if (selectionStartIndex_ != selectionEndIndex_)
		{
			// If the cursor is to the left of the selection start, move the cursor to the start of the selection.
			if (cursorIndex_ < selectionStartIndex_)
			{
				cursorIndex_ = selectionStartIndex_;
			}

			// Clear the selection.
			selectionStartIndex_ = selectionEndIndex_ = 0;
		}
		// Move the cursor to the right, if there's room.
		else if (cursorIndex_ < (int)text_.length())
		{
			cursorIndex_++;
		}
	}
	else if (key == (Key::RightArrow | Key::ShiftBit) && cursorIndex_ < (int)text_.length())
	{
		moveCursorAndSelection(cursorIndex_ + 1);
	}
	else if (WZ_KEY_MOD_OFF(key) == Key::UpArrow || WZ_KEY_MOD_OFF(key) == Key::DownArrow)
	{
		Position cursorPosition;
		int lineHeight, newCursorIndex;

		// Get the cursor position.
		cursorPosition = positionFromIndex(cursorIndex_);

		// Get line height.
		lineHeight = getLineHeight();

		// Move the cursor up/down.
		cursorPosition.y += (WZ_KEY_MOD_OFF(key) == Key::UpArrow ? -lineHeight : lineHeight);
		newCursorIndex = indexFromRelativePosition(cursorPosition);

		if (newCursorIndex == -1)
			return; // Couldn't move cursor.

		// Apply the new cursor index.
		if ((key & Key::ShiftBit) != 0)
		{
			moveCursorAndSelection(newCursorIndex);
		}
		else if (key == Key::UpArrow || key == Key::DownArrow)
		{
			cursorIndex_ = newCursorIndex;

			// Clear the selection.
			selectionStartIndex_ = selectionEndIndex_ = 0;
		}
	}
	else if (WZ_KEY_MOD_OFF(key) == Key::Home || WZ_KEY_MOD_OFF(key) == Key::End)
	{
		int newCursorIndex;

		// Go to text start/end.
		if (!multiline_ || (multiline_ && (key & Key::ControlBit)))
		{
			if (WZ_KEY_MOD_OFF(key) == Key::Home)
			{
				newCursorIndex = 0;
			}
			else
			{
				newCursorIndex = text_.length();
			}
		}
		// Go to line start/end.
		else
		{
			// Find the line we're on.
			LineBreakResult line;
			int lineStartIndex, lineEndIndex;

			line.next = text_.c_str();

			for (;;)
			{
				line = lineBreakText(line.next, 0, getTextRect().w);
				WZ_ASSERT(line.start);
				WZ_ASSERT(line.next);

				lineStartIndex = line.start - text_.c_str();
				lineEndIndex = lineStartIndex + line.length;

				// Is the cursor index on this line?
				if (cursorIndex_ >= lineStartIndex && cursorIndex_ <= lineEndIndex)
					break;

				if (!line.next || !line.next[0])
					return;
			}

			if (WZ_KEY_MOD_OFF(key) == Key::Home)
			{
				newCursorIndex = lineStartIndex;
			}
			else
			{
				newCursorIndex = lineEndIndex;
			}
		}

		if (key & Key::ShiftBit)
		{
			moveCursorAndSelection(newCursorIndex);
		}
		else
		{
			cursorIndex_ = newCursorIndex;

			// Clear the selection.
			selectionStartIndex_ = selectionEndIndex_ = 0;
		}
	}
	else if (multiline_ && key == Key::Enter)
	{
		enterText("\r");
	}
	else if (key == Key::Delete)
	{
		if (selectionStartIndex_ != selectionEndIndex_)
		{
			deleteSelectedText();
		}
		else
		{
			deleteText(cursorIndex_, 1);
		}
	}
	else if (key == Key::Backspace && cursorIndex_ > 0)
	{
		if (selectionStartIndex_ != selectionEndIndex_)
		{
			deleteSelectedText();
		}
		else
		{
			deleteText(cursorIndex_ - 1, 1);
			cursorIndex_--;
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
	if (validateText_ && !validateText_(text))
		return;

	enterText(text);
}

void TextEdit::draw(Rect clip)
{
	renderer_->drawTextEdit(this, clip);
}

Size TextEdit::measure()
{
	return renderer_->measureTextEdit(this);
}

void TextEdit::onScrollerValueChanged(Event e)
{
	scrollValue_ = e.scroller.value;
}

int TextEdit::calculateNumLines(int lineWidth)
{
	if (!multiline_)
		return 0;

	LineBreakResult line;
	line.next = text_.c_str();
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
	if (!multiline_)
		return;

	if (!mainWindow_)
		return; // Not added yet

	// Hide/show scroller depending on if it's needed.
	const int lineHeight = getLineHeight();
	int nLines = calculateNumLines(rect_.w - (border_.left + border_.right));

	if (lineHeight * nLines > rect_.h - (border_.top + border_.bottom))
	{
		scroller_->setVisible(true);
	}
	else
	{
		scroller_->setVisible(false);
		return;
	}

	// Update max value.
	nLines = calculateNumLines(getTextRect().w);
	int max = nLines - (getTextRect().h / lineHeight);
	scroller_->setMaxValue(max);

	// Fit to the right of the rect. Width doesn't change.
	Rect scrollerRect;
	scrollerRect.w = scroller_->getMeasuredSize().w;
	scrollerRect.x = rect_.w - border_.right - scrollerRect.w;
	scrollerRect.y = border_.top;
	scrollerRect.h = rect_.h - (border_.top + border_.bottom);
	scroller_->setRect(scrollerRect);

	// Now that the height has been calculated, update the nub scale.
	const int maxHeight = nLines * lineHeight;
	scroller_->setNubScale(1.0f - ((maxHeight - scrollerRect.h) / (float)maxHeight));
}

void TextEdit::insertText(int index, const char *text, int n)
{
	WZ_ASSERT(text);
	text_.insert(index, text, n);

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
	if (selectionStartIndex_ != selectionEndIndex_)
	{
		deleteSelectedText();
	}

	insertText(cursorIndex_, text, n);
	cursorIndex_ += n;
	updateScrollIndex();
}

void TextEdit::deleteText(int index, int n)
{
	if (index < 0 || index >= (int)text_.length())
		return;

	text_.erase(index, n);

	// Update the scroller.
	updateScroller();
}

void TextEdit::deleteSelectedText()
{
	// No selection.
	if (selectionStartIndex_ == selectionEndIndex_)
		return;

	const int start = WZ_MIN(selectionStartIndex_, selectionEndIndex_);
	const int end = WZ_MAX(selectionStartIndex_, selectionEndIndex_);
	deleteText(start, end - start);

	// Move the cursor to the start (smallest of start and end, not the real selection start).
	cursorIndex_ = start;

	// Clear the selection.
	selectionStartIndex_ = selectionEndIndex_ = 0;
}

int TextEdit::indexFromRelativePosition(Position pos) const
{
	int result = 0;

	// Calculate relative position.
	const Rect absRect = getAbsoluteRect();

	if (multiline_)
	{
		// Get line height.
		int lineHeight = getLineHeight();

		// Set line starting position. May be outside the widget.
		int lineY = lineHeight * -scrollValue_;

		// Iterate through lines.
		LineBreakResult line;
		line.next = text_.c_str();

		for (;;)
		{
			line = lineBreakText(line.next, 0, getTextRect().w);
			result = line.start - text_.c_str();

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
			if (i == (int)line.length)
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
			return scrollValue_;

		// Walk through the text until we find two glyphs that the x coordinate straddles.
		int previousWidth = 0;
		result = scrollValue_;

		for (int i = 1; i <= (int)text_.length(); i++)
		{
			// Calculate the width of the text up to the current character.
			int width;
			measureText(&text_[scrollValue_], i, &width, NULL);

			// Check if we've gone beyond the width of the widget.
			if (width > getTextRect().w)
			{
				result = scrollValue_ + i - 1;
				break;
			}

			// Calculate the change in text width since the last iteration.
			const int deltaWidth = width - previousWidth;

			// Check the intersection between the position and the change in text width since the last iteration.
			if (pos.x >= previousWidth && pos.x <= previousWidth + deltaWidth / 2)
			{
				// Left side of glyph.
				result = scrollValue_ + i - 1;
				break;
			}
			else if (pos.x >= previousWidth + deltaWidth / 2 && pos.x <= width)
			{
				// Right side of glyph.
				result = scrollValue_ + i;
				break;
			}

			// Made it to the end of text string.
			if (i == (int)text_.length())
			{
				result = i;
				break;
			}

			// Store this text width for the next iteration.
			previousWidth = width;
		}
	}

	return WZ_CLAMPED(0, result, (int)text_.length());
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
	if (multiline_)
	{
		for (;;)
		{
			const int cursorY = positionFromIndex(cursorIndex_).y;

			if (cursorY > rect_.h - (border_.top + border_.bottom))
			{
				scrollValue_++;
				scroller_->setValue(scrollValue_);
			}
			else if (cursorY < 0)
			{
				scrollValue_--;
				scroller_->setValue(scrollValue_);
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
			const int cursorX = positionFromIndex(cursorIndex_).x;

			if (cursorX > getTextRect().w)
			{
				scrollValue_++;
			}
			else if (cursorX < 0)
			{
				scrollValue_--;
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
	if (selectionStartIndex_ != selectionEndIndex_)
	{
		// If there's already a selection, move the cursor, and the selection end to match.
		cursorIndex_ = newCursorIndex;
		selectionEndIndex_ = cursorIndex_;
	}
	else
	{
		// No selection, start a new one and move the cursor.
		selectionStartIndex_ = cursorIndex_;
		cursorIndex_ = newCursorIndex;
		selectionEndIndex_ = cursorIndex_;
	}
}

} // namespace wz
