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

static void wz_spinner_draw_button(struct WidgetImpl *widget, Rect clip, bool decrement)
{
	const struct ButtonImpl *button = (struct ButtonImpl *)widget;
	NVGRenderer *r = (NVGRenderer *)widget->renderer;
	struct NVGcontext *vg = r->getContext();
	const Rect rect = widget->getAbsoluteRect();
	const int buttonX = rect.x + rect.w - WZ_SKIN_SPINNER_BUTTON_WIDTH;
	const float buttonCenterX = buttonX + WZ_SKIN_SPINNER_BUTTON_WIDTH * 0.5f;
	const float buttonCenterY = rect.y + rect.h * 0.5f;

	nvgSave(vg);
	r->clipToRect(clip);
	nvgBeginPath(vg);

	if (decrement)
	{
		nvgMoveTo(vg, buttonCenterX, buttonCenterY + WZ_SKIN_SPINNER_ICON_HEIGHT * 0.5f); // bottom
		nvgLineTo(vg, buttonCenterX + WZ_SKIN_SPINNER_ICON_WIDTH * 0.5f, buttonCenterY - WZ_SKIN_SPINNER_ICON_HEIGHT * 0.5f); // right
		nvgLineTo(vg, buttonCenterX - WZ_SKIN_SPINNER_ICON_WIDTH * 0.5f, buttonCenterY - WZ_SKIN_SPINNER_ICON_HEIGHT * 0.5f); // left
	}
	else
	{
		nvgMoveTo(vg, buttonCenterX, buttonCenterY - WZ_SKIN_SPINNER_ICON_HEIGHT * 0.5f); // top
		nvgLineTo(vg, buttonCenterX - WZ_SKIN_SPINNER_ICON_WIDTH * 0.5f, buttonCenterY + WZ_SKIN_SPINNER_ICON_HEIGHT * 0.5f); // left
		nvgLineTo(vg, buttonCenterX + WZ_SKIN_SPINNER_ICON_WIDTH * 0.5f, buttonCenterY + WZ_SKIN_SPINNER_ICON_HEIGHT * 0.5f); // right
	}

	nvgFillColor(vg, widget->hover ? WZ_SKIN_SPINNER_ICON_HOVER_COLOR : WZ_SKIN_SPINNER_ICON_COLOR);
	nvgFill(vg);
	nvgRestore(vg);
}

static void wz_spinner_decrement_button_draw(struct WidgetImpl *widget, Rect clip)
{
	WZ_ASSERT(widget);
	wz_spinner_draw_button(widget, clip, true);
}

static void wz_spinner_decrement_button_clicked(Event *e)
{
	struct SpinnerImpl *spinner;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	spinner = (struct SpinnerImpl *)e->base.widget->parent;
	spinner->setValue(spinner->getValue() - 1);
}

static void wz_spinner_increment_button_draw(struct WidgetImpl *widget, Rect clip)
{
	WZ_ASSERT(widget);
	wz_spinner_draw_button(widget, clip, false);
}

static void wz_spinner_increment_button_clicked(Event *e)
{
	struct SpinnerImpl *spinner;

	WZ_ASSERT(e);
	WZ_ASSERT(e->base.widget);
	WZ_ASSERT(e->base.widget->parent);
	spinner = (struct SpinnerImpl *)e->base.widget->parent;
	spinner->setValue(spinner->getValue() + 1);
}

static Size wz_spinner_measure(struct WidgetImpl *widget)
{
	Border border;
	Size size;
	struct SpinnerImpl *spinner = (struct SpinnerImpl *)widget;

	border = spinner->textEdit->getBorder();
	size.w = 100;
	size.h = widget->getLineHeight() + border.top + border.bottom;
	return size;
}

static void wz_spinner_renderer_changed(struct WidgetImpl *widget)
{
	struct SpinnerImpl *spinner;
	Border textEditBorder;

	WZ_ASSERT(widget);
	spinner = (struct SpinnerImpl *)widget;

	// Shrink the text edit border to exclude the increment and decrement buttons.
	textEditBorder = spinner->textEdit->getBorder();
	textEditBorder.right += 16;
	wz_text_edit_set_border(spinner->textEdit, textEditBorder);
}

// Set the text edit font to match.
static void wz_spinner_font_changed(struct WidgetImpl *widget, const char *fontFace, float fontSize)
{
	struct SpinnerImpl *spinner = (struct SpinnerImpl *)widget;
	WZ_ASSERT(spinner);
	spinner->textEdit->setFont(fontFace, fontSize);
}

SpinnerImpl::SpinnerImpl()
{
	type = WZ_TYPE_SPINNER;

	vtable.measure = wz_spinner_measure;
	vtable.renderer_changed = wz_spinner_renderer_changed;
	vtable.font_changed = wz_spinner_font_changed;

	textEdit = new TextEditImpl(false, 256);
	textEdit->setStretch(WZ_STRETCH);
	textEdit->setValidateTextCallback(wz_spinner_validate_text);
	addChildWidget(textEdit);

	decrementButton = new ButtonImpl();
	decrementButton->setWidth(WZ_SKIN_SPINNER_BUTTON_WIDTH);
	decrementButton->setStretch(WZ_STRETCH_HEIGHT);
	decrementButton->setStretchScale(1, 0.5f);
	decrementButton->setAlign(WZ_ALIGN_RIGHT | WZ_ALIGN_BOTTOM);
	decrementButton->setDrawCallback(wz_spinner_decrement_button_draw);
	decrementButton->addCallbackClicked(wz_spinner_decrement_button_clicked);
	decrementButton->setOverlap(true);
	addChildWidget(decrementButton);

	incrementButton = new ButtonImpl();
	incrementButton->setWidth(WZ_SKIN_SPINNER_BUTTON_WIDTH);
	incrementButton->setStretch(WZ_STRETCH_HEIGHT);
	incrementButton->setStretchScale(1, 0.5f);
	incrementButton->setAlign(WZ_ALIGN_RIGHT | WZ_ALIGN_TOP);
	incrementButton->setDrawCallback(wz_spinner_increment_button_draw);
	incrementButton->addCallbackClicked(wz_spinner_increment_button_clicked);
	incrementButton->setOverlap(true);
	addChildWidget(incrementButton);
}

int SpinnerImpl::getValue() const
{
	int value = 0;
	sscanf(textEdit->getText(), "%d", &value);
	return value;
}

void SpinnerImpl::setValue(int value)
{
	char buffer[32];
	sprintf(buffer, "%d", value);
	textEdit->setText(buffer);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

Spinner::Spinner()
{
	impl.reset(new SpinnerImpl);
}

Spinner::~Spinner()
{
}

Spinner *Spinner::setValue(int value)
{
	((SpinnerImpl *)impl.get())->setValue(value);
	return this;
}

int Spinner::getValue() const
{
	return ((SpinnerImpl *)impl.get())->getValue();
}

} // namespace wz
