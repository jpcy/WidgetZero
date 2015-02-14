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
#include "wz_renderer_nanovg.h"

namespace wz {

class SpinnerDecrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer->drawSpinnerDecrementButton(this, clip);
	}
};

class SpinnerIncrementButton : public Button
{
public:
	virtual void draw(Rect clip)
	{
		renderer->drawSpinnerIncrementButton(this, clip);
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

static void wz_spinner_decrement_button_clicked(Event *e)
{
	Spinner *spinner;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	spinner = (Spinner *)e->base.widget->parent;
	spinner->setValue(spinner->getValue() - 1);
}

static void wz_spinner_increment_button_clicked(Event *e)
{
	Spinner *spinner;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	spinner = (Spinner *)e->base.widget->parent;
	spinner->setValue(spinner->getValue() + 1);
}

Spinner::Spinner()
{
	type = WZ_TYPE_SPINNER;

	textEdit = new TextEdit(false);
	textEdit->setStretch(WZ_STRETCH);
	textEdit->setValidateTextCallback(wz_spinner_validate_text);
	addChildWidget(textEdit);

	decrementButton = new SpinnerDecrementButton();
	decrementButton->setWidth(WZ_SKIN_SPINNER_BUTTON_WIDTH);
	decrementButton->setStretch(WZ_STRETCH_HEIGHT);
	decrementButton->setStretchScale(1, 0.5f);
	decrementButton->setAlign(WZ_ALIGN_RIGHT | WZ_ALIGN_BOTTOM);
	decrementButton->addCallbackClicked(wz_spinner_decrement_button_clicked);
	decrementButton->setOverlap(true);
	addChildWidget(decrementButton);

	incrementButton = new SpinnerIncrementButton();
	incrementButton->setWidth(WZ_SKIN_SPINNER_BUTTON_WIDTH);
	incrementButton->setStretch(WZ_STRETCH_HEIGHT);
	incrementButton->setStretchScale(1, 0.5f);
	incrementButton->setAlign(WZ_ALIGN_RIGHT | WZ_ALIGN_TOP);
	incrementButton->addCallbackClicked(wz_spinner_increment_button_clicked);
	incrementButton->setOverlap(true);
	addChildWidget(incrementButton);
}

void Spinner::onRendererChanged()
{
	// Shrink the text edit border to exclude the increment and decrement buttons.
	Border textEditBorder = textEdit->getBorder();
	textEditBorder.right += 16;
	textEdit->setBorder(textEditBorder);
}

void Spinner::onFontChanged(const char *fontFace, float fontSize)
{
	// Set the text edit font to match.
	textEdit->setFont(fontFace, fontSize);
}

void Spinner::draw(Rect clip)
{
	renderer->drawSpinner(this, clip);
}

Size Spinner::measure()
{
	return renderer->measureSpinner(this);
}

int Spinner::getValue() const
{
	int value = 0;
	sscanf(textEdit->getText(), "%d", &value);
	return value;
}

void Spinner::setValue(int value)
{
	char buffer[32];
	sprintf(buffer, "%d", value);
	textEdit->setText(buffer);
}

} // namespace wz
