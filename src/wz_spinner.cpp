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

class SpinnerDecrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer_->drawSpinnerDecrementButton(this, clip);
	}
};

class SpinnerIncrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer_->drawSpinnerIncrementButton(this, clip);
	}
};

static bool wz_spinner_validate_text(const char *text)
{
	size_t i;

	for (i = 0; i < strlen(text); i++)
	{
		if (!(isdigit(text[i]) || text[i] == '.'|| text[i] == '-'))
			return false;
	}

	return true;
}

Spinner::Spinner()
{
	type_ = WidgetType::Spinner;

	textEdit_ = new TextEdit(false);
	textEdit_->setStretch(Stretch::All);
	textEdit_->setValidateTextCallback(wz_spinner_validate_text);
	addChildWidget(textEdit_);

	decrementButton_ = new SpinnerDecrementButton();
	decrementButton_->setStretch(Stretch::Height);
	decrementButton_->setStretchScale(1, 0.5f);
	decrementButton_->setAlign(Align::Right | Align::Bottom);
	decrementButton_->addEventHandler(EventType::ButtonClicked, this, &Spinner::onDecrementButtonClicked);
	decrementButton_->setOverlap(true);
	addChildWidget(decrementButton_);

	incrementButton_ = new SpinnerIncrementButton();
	incrementButton_->setStretch(Stretch::Height);
	incrementButton_->setStretchScale(1, 0.5f);
	incrementButton_->setAlign(Align::Right | Align::Top);
	incrementButton_->addEventHandler(EventType::ButtonClicked, this, &Spinner::onIncrementButtonClicked);
	incrementButton_->setOverlap(true);
	addChildWidget(incrementButton_);
}

int Spinner::getValue() const
{
	int value = 0;
	sscanf(textEdit_->getText(), "%d", &value);
	return value;
}

void Spinner::setValue(int value)
{
	char buffer[32];
	sprintf(buffer, "%d", value);
	textEdit_->setText(buffer);
}

TextEdit *Spinner::getTextEdit()
{
	return textEdit_;
}

const TextEdit *Spinner::getTextEdit() const
{
	return textEdit_;
}

void Spinner::onRendererChanged()
{
	decrementButton_->setWidth(renderer_->getSpinnerButtonWidth(this));
	incrementButton_->setWidth(renderer_->getSpinnerButtonWidth(this));

	// Shrink the text edit border to exclude the increment and decrement buttons.
	Border textEditBorder = textEdit_->getBorder();
	textEditBorder.right += renderer_->getSpinnerButtonWidth(this);
	textEdit_->setBorder(textEditBorder);
}

void Spinner::onFontChanged(const char *fontFace, float fontSize)
{
	// Set the text edit font to match.
	textEdit_->setFont(fontFace, fontSize);
}

void Spinner::draw(Rect clip)
{
	renderer_->drawSpinner(this, clip);
}

Size Spinner::measure()
{
	return renderer_->measureSpinner(this);
}

void Spinner::onDecrementButtonClicked(Event)
{
	setValue(getValue() - 1);
}

void Spinner::onIncrementButtonClicked(Event)
{
	setValue(getValue() + 1);
}

} // namespace wz
