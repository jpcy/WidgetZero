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

#define WZ_NANOVG_MAX_PATH 256
#define WZ_NANOVG_MAX_IMAGES 1024
#define WZ_NANOVG_MAX_ERROR_MESSAGE 1024

namespace wz {

struct Image
{
	Image() : handle(0) {}

	int handle;
	char filename[WZ_NANOVG_MAX_PATH];
};

struct NVGRendererImpl
{
	NVGRendererImpl() : destroy(NULL), vg(NULL), nImages(0), defaultFontSize(0)
	{
		errorMessage[0] = 0;
	}

	char errorMessage[WZ_NANOVG_MAX_ERROR_MESSAGE];
	wzNanoVgGlDestroy destroy;
	NVGcontext *vg;
	Image images[WZ_NANOVG_MAX_IMAGES];
	int nImages;
	char fontDirectory[WZ_NANOVG_MAX_PATH];
	float defaultFontSize;
};

static NVGcolor ConvertColor(Color c)
{
	return nvgRGBAf(c.r, c.g, c.b, c.a);
}

NVGRenderer::NVGRenderer(wzNanoVgGlCreate create, wzNanoVgGlDestroy destroy, int flags, const char *fontDirectory, const char *defaultFontFace, float defaultFontSize)
{
	WZ_ASSERT(create);
	WZ_ASSERT(destroy);

	impl.reset(new NVGRendererImpl);
	impl->destroy = destroy;
	impl->defaultFontSize = defaultFontSize;

	// Init nanovg.
	impl->vg = create(flags);

	if (!impl->vg)
	{
		strcpy(impl->errorMessage, "Error initializing NanoVG");
		return;
	}

	// Load the default font.
	strncpy(impl->fontDirectory, fontDirectory, WZ_NANOVG_MAX_PATH);

	if (createFont(defaultFontFace) == -1)
	{
		sprintf(impl->errorMessage, "Error loading font %s", defaultFontFace);
		return;
	}
}

NVGRenderer::~NVGRenderer()
{
	if (impl->vg)
	{
		impl->destroy(impl->vg);
	}
}

Color NVGRenderer::getClearColor()
{
	return Color(WZ_SKIN_CLEAR.rgba);
}

void NVGRenderer::beginFrame(int windowWidth, int windowHeight)
{
	nvgBeginFrame(impl->vg, windowWidth, windowHeight, 1);
}

void NVGRenderer::endFrame()
{
	nvgEndFrame(impl->vg);
}

void NVGRenderer::drawButton(Button *button, Rect clip)
{
	NVGcontext *vg = impl->vg;
	nvgSave(vg);
	const Rect rect = button->getAbsoluteRect();

	if (!clipToRectIntersection(clip, rect))
		return;

	// Background color.
	NVGcolor bgColor;

	if (button->isPressed() && button->getHover())
	{
		bgColor = WZ_SKIN_BUTTON_BG_PRESSED;
	}
	else if (button->isSet())
	{
		bgColor = WZ_SKIN_BUTTON_BG_SET;
	}
	else
	{
		bgColor = WZ_SKIN_BUTTON_BG;
	}

	drawFilledRect(rect, bgColor);
	drawRect(rect, button->getHover() ? WZ_SKIN_BUTTON_BORDER_HOVER : WZ_SKIN_BUTTON_BORDER);
	drawCenteredIconAndLabel(rect - button->getPadding(), button->getLabel(), WZ_SKIN_BUTTON_TEXT, button->getFontFace(), button->getFontSize(), button->getIcon(), WZ_SKIN_BUTTON_ICON_SPACING);

	nvgRestore(vg);
}

Size NVGRenderer::measureButton(Button *button)
{
	Size size;
	button->measureText(button->getLabel(), 0, &size.w, &size.h);

	if (button->getIcon() && button->getIcon()[0])
	{
		int w, h;
		int handle = createImage(button->getIcon(), &w, &h);

		if (handle)
		{
			size.w += w + WZ_SKIN_BUTTON_ICON_SPACING;
			size.h = WZ_MAX(size.h, h);
		}
	}

	const Border padding = button->getPadding();
	size.w += padding.left + padding.right;
	size.h += padding.top + padding.bottom;
	return size;
}

void NVGRenderer::drawCheckBox(CheckBox *checkBox, Rect clip)
{
	NVGcontext *vg = impl->vg;

	nvgSave(vg);
	clipToRect(clip);

	// Calculate box rect.
	const Rect rect = checkBox->getAbsoluteRect();
	Rect boxRect;
	boxRect.x = rect.x;
	boxRect.y = (int)(rect.y + rect.h / 2.0f - WZ_SKIN_CHECK_BOX_BOX_SIZE / 2.0f);
	boxRect.w = boxRect.h = WZ_SKIN_CHECK_BOX_BOX_SIZE;

	// Box border.
	drawRect(boxRect, checkBox->getHover() ? WZ_SKIN_CHECK_BOX_BORDER_HOVER : WZ_SKIN_CHECK_BOX_BORDER);

	// Box checkmark.
	if (checkBox->isChecked())
	{
		const float left = (float)boxRect.x + WZ_SKIN_CHECK_BOX_BOX_INTERNAL_MARGIN;
		const float right = (float)boxRect.x + boxRect.w - WZ_SKIN_CHECK_BOX_BOX_INTERNAL_MARGIN;
		const float top = (float)boxRect.y + WZ_SKIN_CHECK_BOX_BOX_INTERNAL_MARGIN;
		const float bottom = (float)boxRect.y + boxRect.h - WZ_SKIN_CHECK_BOX_BOX_INTERNAL_MARGIN;

		nvgBeginPath(vg);
		nvgMoveTo(vg, left, top);
		nvgLineTo(vg, right, bottom);
		nvgStrokeColor(vg, WZ_SKIN_CHECK_BOX_CHECK);
		nvgStrokeWidth(vg, WZ_SKIN_CHECK_BOX_CHECK_THICKNESS);
		nvgStroke(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, left, bottom);
		nvgLineTo(vg, right, top);
		nvgStroke(vg);
	}

	// Label.
	print(rect.x + WZ_SKIN_CHECK_BOX_BOX_SIZE + WZ_SKIN_CHECK_BOX_BOX_RIGHT_MARGIN, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, checkBox->getFontFace(), checkBox->getFontSize(), WZ_SKIN_CHECK_BOX_TEXT, checkBox->getLabel(), 0);

	nvgRestore(vg);
}

Size NVGRenderer::measureCheckBox(CheckBox *checkBox)
{
	Size size;
	checkBox->measureText(checkBox->getLabel(), 0, &size.w, &size.h);
	size.w += WZ_SKIN_CHECK_BOX_BOX_SIZE + WZ_SKIN_CHECK_BOX_BOX_RIGHT_MARGIN;
	return size;
}

void NVGRenderer::drawCombo(Combo *combo, Rect clip)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = combo->getAbsoluteRect();
	const uint8_t *itemData = combo->getList()->getItemData();
	const int itemStride = combo->getList()->getItemStride();
	const int selectedItemIndex = combo->getList()->getSelectedItem();

	nvgSave(vg);
	clipToRect(clip);
	
	drawFilledRect(rect, WZ_SKIN_COMBO_BG);
	drawRect(rect, combo->getHover() ? WZ_SKIN_COMBO_BORDER_HOVER : WZ_SKIN_COMBO_BORDER);

	// Internal border.
	int buttonX = rect.x + rect.w - WZ_SKIN_COMBO_BUTTON_WIDTH;
	drawLine(buttonX, rect.y + 1, buttonX, rect.y + rect.h - 1, combo->getHover() ? WZ_SKIN_COMBO_BORDER_HOVER : WZ_SKIN_COMBO_BORDER);

	// Icon.
	{
		const float buttonCenterX = buttonX + WZ_SKIN_COMBO_BUTTON_WIDTH * 0.5f;
		const float buttonCenterY = rect.y + rect.h * 0.5f;

		nvgBeginPath(vg);
		nvgMoveTo(vg, buttonCenterX, buttonCenterY + WZ_SKIN_COMBO_ICON_HEIGHT * 0.5f); // bottom
		nvgLineTo(vg, buttonCenterX + WZ_SKIN_COMBO_ICON_WIDTH * 0.5f, buttonCenterY - WZ_SKIN_COMBO_ICON_HEIGHT * 0.5f); // right
		nvgLineTo(vg, buttonCenterX - WZ_SKIN_COMBO_ICON_WIDTH * 0.5f, buttonCenterY - WZ_SKIN_COMBO_ICON_HEIGHT * 0.5f); // left
		nvgFillColor(vg, WZ_SKIN_COMBO_ICON);
		nvgFill(vg);
	}

	// Selected item.
	if (selectedItemIndex >= 0)
	{
		print(rect.x + WZ_SKIN_COMBO_PADDING_X / 2, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, combo->getFontFace(), combo->getFontSize(), WZ_SKIN_COMBO_TEXT, *((const char **)&itemData[selectedItemIndex * itemStride]), 0);
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureCombo(Combo *combo)
{
	const uint8_t *itemData = combo->getList()->getItemData();
	const int itemStride = combo->getList()->getItemStride();

	// Use the widest item text.
	Size size;
	size.w = 0;

	for (int i = 0; i < combo->getList()->getNumItems(); i++)
	{
		int w;
		combo->measureText(*((const char **)&itemData[i * itemStride]), 0, &w, NULL);
		size.w = WZ_MAX(size.w, w);
	}

	// Use line height.
	size.h = combo->getLineHeight();

	// Add scroller width or button width, whichever is largest.
	Size scrollerSize = measureScroller(combo->getList()->getScroller());
	size.w += WZ_MAX(scrollerSize.w, WZ_SKIN_COMBO_BUTTON_WIDTH);

	// Padding.
	size.w += WZ_SKIN_COMBO_PADDING_X;
	size.h += WZ_SKIN_COMBO_PADDING_Y;
	return size;
}

void NVGRenderer::drawDockIcon(DockIcon *dockIcon, Rect /*clip*/)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = dockIcon->getAbsoluteRect();

	// Never clipped.
	nvgSave(vg);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h, 3);
	nvgFillColor(vg, nvgRGBA(64, 64, 64, 128));
	nvgFill(vg);
	nvgRestore(vg);
}

void NVGRenderer::drawDockPreview(DockPreview *dockPreview, Rect /*clip*/)
{
	// Never clipped.
	nvgSave(impl->vg);
	drawFilledRect(dockPreview->getAbsoluteRect(), WZ_SKIN_MAIN_WINDOW_DOCK_PREVIEW);
	nvgRestore(impl->vg);
}

Border NVGRenderer::getGroupBoxMargin(GroupBox * /*groupBox*/)
{
	return Border(WZ_SKIN_GROUP_BOX_MARGIN);
}

void NVGRenderer::drawGroupBox(GroupBox *groupBox, Rect clip)
{
	NVGcontext *vg = impl->vg;

	nvgSave(vg);
	clipToRect(clip);
	const Rect rect = groupBox->getAbsoluteRect();
	
	if (!groupBox->getLabel() || !groupBox->getLabel()[0])
	{
		drawRect(rect, WZ_SKIN_GROUP_BOX_BORDER);
	}
	else
	{
		// Border
		int textWidth, textHeight;
		groupBox->measureText(groupBox->getLabel(), 0, &textWidth, &textHeight);

		Rect borderRect = rect;
		borderRect.y += textHeight / 2;
		borderRect.h -= textHeight / 2;

		const float left = float(borderRect.x);
		const float right = float(borderRect.x + borderRect.w - 1);
		const float top = float(borderRect.y);
		const float bottom = float(borderRect.y + borderRect.h - 1);

		nvgBeginPath(vg);
		nvgMoveTo(vg, left + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN - WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING, top);
		nvgLineTo(vg, left, top);
		nvgLineTo(vg, left, bottom);
		nvgLineTo(vg, right, bottom);
		nvgLineTo(vg, right, top);
		nvgLineTo(vg, left + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN + textWidth + WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING * 2, top);
		nvgStrokeColor(vg, WZ_SKIN_GROUP_BOX_BORDER);
		nvgStroke(vg);

		// Label.
		print(rect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN, rect.y, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, groupBox->getFontFace(), groupBox->getFontSize(), WZ_SKIN_GROUP_BOX_TEXT, groupBox->getLabel(), 0);
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureGroupBox(GroupBox *groupBox)
{
	Size s;

	if (groupBox->getLabel() && groupBox->getLabel()[0])
	{
		int textWidth;
		groupBox->measureText(groupBox->getLabel(), 0, &textWidth, NULL);

		// Give as much margin on the right as the left.
		s.w = WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN * 2 + WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING + textWidth;
	}

	return s;
}

Color NVGRenderer::getLabelTextColor(Label * /*label*/)
{
	NVGcolor c = WZ_SKIN_TEXT;
	return Color(c.rgba);
}

void NVGRenderer::drawLabel(Label *label, Rect clip)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = label->getAbsoluteRect();

	nvgSave(vg);
	clipToRect(clip);

	if (label->getMultiline())
	{
		printBox(rect, label->getFontFace(), label->getFontSize(), ConvertColor(label->getTextColor()), label->getText(), 0);
	}
	else
	{
		print(rect.x, (int)(rect.y + rect.h * 0.5f), NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, label->getFontFace(), label->getFontSize(), ConvertColor(label->getTextColor()), label->getText(), 0);
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureLabel(Label *label)
{
	NVGcontext *vg = impl->vg;
	Size size;

	if (label->getMultiline())
	{
		nvgFontSize(vg, label->getFontSize() == 0 ? getDefaultFontSize() : label->getFontSize());
		setFontFace(label->getFontFace());
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgTextLineHeight(vg, 1.0f);
		float bounds[4];
		nvgTextBoxBounds(vg, 0, 0, (float)label->getUserOrMeasuredSize().w, label->getText(), NULL, bounds);
		size.w = (int)bounds[2];
		size.h = (int)bounds[3];
	}
	else
	{
		label->measureText(label->getText(), 0, &size.w, &size.h);
	}

	return size;
}

void NVGRenderer::drawList(List *list, Rect clip)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = list->getAbsoluteRect();
	const Rect itemsRect = list->getAbsoluteItemsRect();

	nvgSave(vg);
	clipToRect(clip);
	
	// Background.
	drawFilledRect(rect, WZ_SKIN_LIST_BG);

	// Border.
	drawRect(rect, WZ_SKIN_LIST_BORDER);

	// Items.
	if (!clipToRectIntersection(clip, itemsRect))
		return;

	int y = itemsRect.y - (list->getScroller()->getValue() % list->getItemHeight());

	for (int i = list->getFirstItem(); i < list->getNumItems(); i++)
	{
		Rect itemRect;
		const uint8_t *itemData;

		// Outside widget?
		if (y > itemsRect.y + itemsRect.h)
			break;

		itemRect.x = itemsRect.x;
		itemRect.y = y;
		itemRect.w = itemsRect.w;
		itemRect.h = list->getItemHeight();
		itemData = *((uint8_t **)&list->getItemData()[i * list->getItemStride()]);

		if (i == list->getSelectedItem())
		{
			drawFilledRect(itemRect, WZ_SKIN_LIST_SET);
		}
		else if (i == list->getPressedItem() || i == list->getHoveredItem())
		{
			drawFilledRect(itemRect, WZ_SKIN_LIST_HOVER);
		}

		if (list->getDrawItemCallback())
		{
			list->getDrawItemCallback()(this, itemRect, list, list->getFontFace(), list->getFontSize(), i, itemData);
		}
		else
		{
			print(itemsRect.x + WZ_SKIN_LIST_ITEM_LEFT_PADDING, y + list->getItemHeight() / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, list->getFontFace(), list->getFontSize(), WZ_SKIN_LIST_TEXT, (const char *)itemData, 0);
		}

		y += list->getItemHeight();
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureList(List * /*list*/)
{
	return Size();
}

void NVGRenderer::drawMenuBarButton(MenuBarButton *button, Rect clip)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = button->getAbsoluteRect();

	nvgSave(vg);
	clipToRect(clip);

	if (button->isPressed())
	{
		drawFilledRect(rect, WZ_SKIN_MENU_BAR_SET);
	}

	if (button->getHover())
	{
		drawRect(rect, WZ_SKIN_MENU_BAR_BORDER_HOVER);
	}

	print(rect.x + rect.w / 2, rect.y + rect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, button->getFontFace(), button->getFontSize(), WZ_SKIN_MENU_BAR_TEXT, button->getLabel(), 0);

	nvgRestore(vg);
}

Size NVGRenderer::measureMenuBarButton(MenuBarButton *button)
{
	Size size;
	button->measureText(button->getLabel(), 0, &size.w, &size.h);
	size.w += 12;
	return size;
}

int NVGRenderer::getMenuBarPadding(MenuBar * /*menuBar*/)
{
	return WZ_SKIN_MENU_BAR_PADDING;
}

void NVGRenderer::drawMenuBar(MenuBar *menuBar, Rect clip)
{
	nvgSave(impl->vg);
	clipToRect(clip);
	drawFilledRect(menuBar->getAbsoluteRect(), WZ_SKIN_MENU_BAR_BG);
	nvgRestore(impl->vg);
}

Size NVGRenderer::measureMenuBar(MenuBar * /*menuBar*/)
{
	return Size();
}

void NVGRenderer::drawRadioButton(RadioButton *button, Rect clip)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = button->getAbsoluteRect();

	nvgSave(vg);

	if (!clipToRectIntersection(clip, rect))
		return;

	// Inner circle.
	if (button->isSet())
	{
		nvgBeginPath(vg);
		nvgCircle(vg, (float)(rect.x + WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS), rect.y + rect.h / 2.0f, (float)WZ_SKIN_RADIO_BUTTON_INNER_RADIUS);
		nvgFillColor(vg, WZ_SKIN_RADIO_BUTTON_SET);
		nvgFill(vg);
	}

	// Outer circle.
	nvgBeginPath(vg);
	nvgCircle(vg, (float)(rect.x + WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS), rect.y + rect.h / 2.0f, (float)WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS - 0.5f);
	nvgStrokeColor(vg, button->getHover() ? WZ_SKIN_RADIO_BUTTON_BORDER_HOVER : WZ_SKIN_RADIO_BUTTON_BORDER);
	nvgStroke(vg);

	// Label.
	print(rect.x + WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS * 2 + WZ_SKIN_RADIO_BUTTON_SPACING, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, button->getFontFace(), button->getFontSize(), WZ_SKIN_RADIO_BUTTON_TEXT, button->getLabel(), 0);

	nvgRestore(vg);
}

Size NVGRenderer::measureRadioButton(RadioButton *button)
{
	Size size;
	button->measureText(button->getLabel(), 0, &size.w, &size.h);
	size.w += WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS * 2 + WZ_SKIN_RADIO_BUTTON_SPACING;
	size.h = WZ_MAX(size.h, WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS);
	return size;
}

void NVGRenderer::drawScrollerButton(Button *button, Rect clip, bool decrement)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = button->getAbsoluteRect();

	nvgSave(vg);
	clipToRect(clip);

	// Icon.
	nvgBeginPath(vg);
	nvgTranslate(vg, rect.x + rect.w * 0.5f, rect.y + rect.h * 0.5f); // center

	if (((Scroller *)button->getParent())->getDirection() == ScrollerDirection::Vertical)
	{
		nvgRotate(vg, decrement ? 0 : NVG_PI);
	}
	else
	{
		nvgRotate(vg, decrement ? NVG_PI * -0.5f : NVG_PI * 0.5f);
	}

	nvgMoveTo(vg, 0, rect.h * -0.25f); // top
	nvgLineTo(vg, rect.w * -0.25f, rect.h * 0.25f); // left
	nvgLineTo(vg, rect.w * 0.25f, rect.h * 0.25f); // right
	nvgFillColor(vg, button->getHover() ? WZ_SKIN_SCROLLER_ICON_HOVER : WZ_SKIN_SCROLLER_ICON);
	nvgFill(vg);
	nvgRestore(vg);
}

void NVGRenderer::drawScrollerDecrementButton(Button *button, Rect clip)
{
	drawScrollerButton(button, clip, true);
}

void NVGRenderer::drawScrollerIncrementButton(Button *button, Rect clip)
{
	drawScrollerButton(button, clip, false);
}

void NVGRenderer::drawScroller(Scroller *scroller, Rect clip)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = scroller->getAbsoluteRect();

	nvgSave(vg);
	clipToRect(clip);

	Rect nubContainerRect, nubRect;
	bool hover, pressed;
	scroller->getNubState(&nubContainerRect, &nubRect, &hover, &pressed);

	// Nub container.
	drawFilledRect(rect, WZ_SKIN_SCROLLER_BG);

	// Nub.
	{
		// Draw as half thickness.
		Rect r = nubRect;

		if (scroller->getDirection() == ScrollerDirection::Vertical)
		{
			r.x = int(r.x + r.w * 0.25f);
			r.w = int(r.w * 0.5f);
		}
		else
		{
			r.y = int(r.y + r.h * 0.25f);
			r.h = int(r.h * 0.5f);
		}

		drawFilledRect(r, hover ? WZ_SKIN_SCROLLER_NUB_HOVER : WZ_SKIN_SCROLLER_NUB);
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureScroller(Scroller *scroller)
{
	return scroller->getDirection() == ScrollerDirection::Vertical ? Size(WZ_SKIN_SCROLLER_THICKNESS, 0) : Size(0, WZ_SKIN_SCROLLER_THICKNESS);
}

int NVGRenderer::getSpinnerButtonWidth(Spinner * /*spinner*/)
{
	return WZ_SKIN_SPINNER_BUTTON_WIDTH;
}

void NVGRenderer::drawSpinnerButton(Button *button, Rect clip, bool decrement)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = button->getAbsoluteRect();
	const int buttonX = rect.x + rect.w - WZ_SKIN_SPINNER_BUTTON_WIDTH;

	nvgSave(vg);
	clipToRect(clip);
	nvgBeginPath(vg);
	nvgTranslate(vg, buttonX + WZ_SKIN_SPINNER_BUTTON_WIDTH * 0.5f, rect.y + rect.h * 0.5f); // center
	nvgRotate(vg, decrement ? NVG_PI : 0);
	nvgMoveTo(vg, 0, WZ_SKIN_SPINNER_ICON_HEIGHT * -0.5f); // top
	nvgLineTo(vg, WZ_SKIN_SPINNER_ICON_WIDTH * -0.5f, WZ_SKIN_SPINNER_ICON_HEIGHT * 0.5f); // left
	nvgLineTo(vg, WZ_SKIN_SPINNER_ICON_WIDTH * 0.5f, WZ_SKIN_SPINNER_ICON_HEIGHT * 0.5f); // right
	nvgFillColor(vg, button->getHover() ? WZ_SKIN_SPINNER_ICON_HOVER : WZ_SKIN_SPINNER_ICON);
	nvgFill(vg);
	nvgRestore(vg);
}

void NVGRenderer::drawSpinnerDecrementButton(Button *button, Rect clip)
{
	drawSpinnerButton(button, clip, true);
}

void NVGRenderer::drawSpinnerIncrementButton(Button *button, Rect clip)
{
	drawSpinnerButton(button, clip, false);
}

void NVGRenderer::drawSpinner(Spinner * /*spinner*/, Rect /*clip*/)
{
}

Size NVGRenderer::measureSpinner(Spinner *spinner)
{
	const Border border = spinner->getTextEdit()->getBorder();
	Size size;
	size.w = 100;
	size.h = spinner->getLineHeight() + border.top + border.bottom;
	return size;
}

void NVGRenderer::drawTabButton(TabButton *button, Rect clip)
{
	NVGcontext *vg = impl->vg;

	nvgSave(vg);
	const Rect rect = button->getAbsoluteRect();
	clipToRect(clip);

	if (button->isSet())
	{
		drawFilledRect(rect, WZ_SKIN_TAB_BUTTON_BG_SET);

		nvgBeginPath(vg);

		const float x = (float)rect.x;
		const float y = (float)rect.y;
		nvgMoveTo(vg, x, y + rect.h); // bl
		nvgLineTo(vg, x, y); // tl
		nvgLineTo(vg, x + rect.w, y); // tr
		nvgLineTo(vg, x + rect.w, y + rect.h); // br

		nvgStrokeColor(vg, WZ_SKIN_TABBED_BORDER);
		nvgStroke(vg);
	}

	drawCenteredIconAndLabel(rect - button->getPadding(), button->getLabel(), button->getHover() ? WZ_SKIN_TAB_BUTTON_TEXT_HOVER : WZ_SKIN_TAB_BUTTON_TEXT, button->getFontFace(), button->getFontSize(), button->getIcon(), WZ_SKIN_BUTTON_ICON_SPACING);
	nvgRestore(vg);
}

int NVGRenderer::getTabBarScrollButtonWidth(TabBar * /*tabBar*/)
{
	return WZ_SKIN_TAB_BAR_SCROLL_BUTTON_WIDTH;
}

void NVGRenderer::drawTabBarScrollButton(Button *button, Rect clip, bool decrement)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = button->getAbsoluteRect();

	nvgSave(vg);
	clipToRect(clip);

	// Background.
	drawFilledRect(rect, WZ_SKIN_BUTTON_BG);

	// Icon.
	nvgBeginPath(vg);
	nvgTranslate(vg, rect.x + rect.w * 0.5f, rect.y + rect.h * 0.5f); // center
	nvgRotate(vg, decrement ? 0 : NVG_PI);
	const float hs = WZ_SKIN_TAB_BAR_SCROLL_ICON_SIZE / 2.0f;
	nvgMoveTo(vg, -hs, 0); // left
	nvgLineTo(vg, hs, -hs); // top
	nvgLineTo(vg, hs, hs); // bottom
	nvgFillColor(vg, button->getHover() ? WZ_SKIN_TAB_BAR_SCROLL_ICON_HOVER : WZ_SKIN_TAB_BAR_SCROLL_ICON);
	nvgFill(vg);

	nvgRestore(vg);
}

void NVGRenderer::drawTabBarDecrementButton(Button *button, Rect clip)
{
	drawTabBarScrollButton(button, clip, true);
}

void NVGRenderer::drawTabBarIncrementButton(Button *button, Rect clip)
{
	drawTabBarScrollButton(button, clip, false);
}

void NVGRenderer::drawTabBar(TabBar *tabBar, Rect /*clip*/)
{
	nvgSave(impl->vg);
	drawFilledRect(tabBar->getAbsoluteRect(), WZ_SKIN_TAB_BAR_BG);
	nvgRestore(impl->vg);
}

Size NVGRenderer::measureTabBar(TabBar *tabBar)
{
	// Padding.
	return Size(0, tabBar->getLineHeight() + 8);
}

void NVGRenderer::drawTabbed(Tabbed *tabbed, Rect clip)
{
	NVGcontext *vg = impl->vg;

	nvgSave(vg);
	clipToRectIntersection(clip, tabbed->getAbsoluteRect());

	// Page background.
	const Tab *selectedTab = tabbed->getSelectedTab();
	const Rect pr = selectedTab->getPage()->getAbsoluteRect();
	drawFilledRect(pr, WZ_SKIN_TABBED_BG);

	// Draw an outline around the selected tab button and page.
	nvgBeginPath(vg);

	if (selectedTab->getButton()->isVisible())
	{
		// The tab button.
		const Rect tr = selectedTab->getButton()->getAbsoluteRect();
		nvgMoveTo(vg, float(tr.x + tr.w), float(tr.y + tr.h)); // br

		// The tab page.
		nvgLineTo(vg, float(pr.x + pr.w), float(pr.y)); // tr
		nvgLineTo(vg, float(pr.x + pr.w), float(pr.y + pr.h)); // br
		nvgLineTo(vg, float(pr.x), float(pr.y + pr.h)); // bl
		nvgLineTo(vg, float(pr.x), float(pr.y)); // tl

		// The tab button.
		nvgLineTo(vg, float(tr.x), float(tr.y + tr.h)); // bl
	}
	else
	{
		// Selected tab is scrolled out of view, just draw an outline around the page.
		nvgRect(vg, (float)pr.x, (float)pr.y, (float)pr.w, (float)pr.h);
	}
	
	nvgStrokeColor(vg, WZ_SKIN_TABBED_BORDER);
	nvgStroke(vg);
	nvgRestore(vg);
}

Size NVGRenderer::measureTabbed(Tabbed * /*tabbed*/)
{
	return Size();
}

void NVGRenderer::drawTextEdit(TextEdit *textEdit, Rect clip)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = textEdit->getAbsoluteRect();
	const Rect textRect = textEdit->getTextRect();
	const int lineHeight = textEdit->getLineHeight();

	nvgSave(vg);
	clipToRect(clip);
	
	// Background.
	drawFilledRect(rect, WZ_SKIN_TEXT_EDIT_BG);

	// Border.
	drawRect(rect, textEdit->getHover() ? WZ_SKIN_TEXT_EDIT_BORDER_HOVER : WZ_SKIN_TEXT_EDIT_BORDER);

	// Clip to the text rect.
	if (!clipToRectIntersection(clip, textRect))
		return;

	// Text.
	if (textEdit->isMultiline())
	{
		int lineY = 0;
		int selectionStartIndex = textEdit->getSelectionStartIndex();
		int selectionEndIndex = textEdit->getSelectionEndIndex();
		LineBreakResult line;
		line.next = textEdit->getVisibleText();

		for (;;)
		{
			line = textEdit->lineBreakText(line.next, 0, textRect.w);

			if (line.length > 0)
			{
				// Draw this line.
				print(textRect.x, textRect.y + lineY, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, textEdit->getFontFace(), textEdit->getFontSize(), WZ_SKIN_TEXT_EDIT_TEXT, line.start, line.length);

				// Selection.
				if (textEdit->hasSelection())
				{
					int lineStartIndex;
					bool startOnThisLine, endOnThisLine, straddleThisLine;
					Position start, end;

					lineStartIndex = line.start - textEdit->getText();
					startOnThisLine = selectionStartIndex >= lineStartIndex && selectionStartIndex <= lineStartIndex + (int)line.length;
					endOnThisLine = selectionEndIndex >= lineStartIndex && selectionEndIndex <= lineStartIndex + (int)line.length;
					straddleThisLine = selectionStartIndex < lineStartIndex && selectionEndIndex > lineStartIndex + (int)line.length;

					if (startOnThisLine)
					{
						start = textEdit->positionFromIndex(selectionStartIndex);
					}
					else
					{
						start = textEdit->positionFromIndex(lineStartIndex);
					}

					if (endOnThisLine)
					{
						end = textEdit->positionFromIndex(selectionEndIndex);
					}
					else
					{
						end = textEdit->positionFromIndex(lineStartIndex + (int)line.length);
					}

					if (startOnThisLine || straddleThisLine || endOnThisLine)
					{
						Rect selectionRect;
						selectionRect.x = textRect.x + start.x;
						selectionRect.y = textRect.y + start.y - lineHeight / 2;
						selectionRect.w = end.x - start.x;
						selectionRect.h = lineHeight;
						drawFilledRect(selectionRect, WZ_SKIN_TEXT_EDIT_SELECTION);
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
		print(textRect.x, textRect.y + textRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, textEdit->getFontFace(), textEdit->getFontSize(), WZ_SKIN_TEXT_EDIT_TEXT, textEdit->getVisibleText(), 0);

		// Selection.
		if (textEdit->hasSelection())
		{
			Position position1, position2;
			Rect selectionRect;

			position1 = textEdit->getSelectionStartPosition();
			position2 = textEdit->getSelectionEndPosition();
			selectionRect.x = textRect.x + position1.x;
			selectionRect.y = textRect.y + position1.y - lineHeight / 2;
			selectionRect.w = position2.x - position1.x;
			selectionRect.h = lineHeight;
			drawFilledRect(selectionRect, WZ_SKIN_TEXT_EDIT_SELECTION);
		}
	}

	// Cursor.
	if (textEdit->getMainWindow()->isTextCursorVisible() && textEdit->hasKeyboardFocus())
	{
		Position position = textEdit->getCursorPosition();
		position.x += textRect.x;
		position.y += textRect.y;

		clipToRect(rect);
		nvgBeginPath(vg);
		nvgMoveTo(vg, (float)position.x, position.y - lineHeight / 2.0f);
		nvgLineTo(vg, (float)position.x, position.y + lineHeight / 2.0f);
		nvgStrokeColor(vg, WZ_SKIN_TEXT_EDIT_CURSOR);
		nvgStroke(vg);
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureTextEdit(TextEdit *textEdit)
{
	if (textEdit->isMultiline())
	{
		return Size(100, 100);
	}
	else
	{
		return Size(100, textEdit->getLineHeight() + textEdit->getBorder().top + textEdit->getBorder().bottom);
	}
}

void NVGRenderer::drawWindow(Window *window, Rect /*clip*/)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = window->getAbsoluteRect();

	nvgSave(vg);
	drawFilledRect(rect, WZ_SKIN_WINDOW_BG);

	// Header.
	const Rect headerRect = window->getHeaderRect();

	if (headerRect.w > 0 && headerRect.h > 0)
	{
		// Draw the header bg a little larger than the header rect, since we're only drawing the window border as 1 pixel thick.
		Rect r = rect;
		r.h = (headerRect.y + headerRect.h - 1) - rect.y;
		drawFilledRect(r, WZ_SKIN_WINDOW_HEADER_BG);

		nvgSave(vg);
		clipToRect(headerRect);
		print(headerRect.x + 10, headerRect.y + headerRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, window->getFontFace(), window->getFontSize(), WZ_SKIN_WINDOW_TEXT, window->getTitle(), 0);
		nvgRestore(vg);
	}

	drawRect(rect, WZ_SKIN_WINDOW_BORDER);
	nvgRestore(vg);
}

Size NVGRenderer::measureWindow(Window * /*window*/)
{
	return Size();
}

int NVGRenderer::getLineHeight(const char *fontFace, float fontSize)
{
	nvgFontSize(impl->vg, fontSize == 0 ? impl->defaultFontSize : fontSize);
	setFontFace(fontFace);
	float lineHeight;
	nvgTextMetrics(impl->vg, NULL, NULL, &lineHeight);
	return (int)lineHeight;
}

void NVGRenderer::measureText(const char *fontFace, float fontSize, const char *text, int n, int *width, int *height)
{
	if (width)
	{
		nvgFontSize(impl->vg, fontSize == 0 ? impl->defaultFontSize : fontSize);
		setFontFace(fontFace);
		*width = (int)nvgTextBounds(impl->vg, 0, 0, text, n == 0 ? NULL : &text[n], NULL);
	}

	if (height)
	{
		*height = (int)(fontSize == 0 ? impl->defaultFontSize : fontSize);
	}
}

LineBreakResult NVGRenderer::lineBreakText(const char * /*fontFace*/, float /*fontSize*/, const char *text, int n, int lineWidth)
{
	NVGtextRow row;
	LineBreakResult result;

	if (text && nvgTextBreakLines(impl->vg, text, n == 0 ? NULL : &text[n], (float)lineWidth, &row, 1) > 0)
	{
		result.start = row.start;
		result.length = row.end - row.start;
		result.next = row.next;
	}
	else
	{
		result.start = NULL;
		result.length = 0;
		result.next = NULL;
	}

	return result;
}

const char *NVGRenderer::getError()
{
	return impl->errorMessage[0] == 0 ? NULL : impl->errorMessage;
}

NVGcontext *NVGRenderer::getContext()
{
	return impl->vg;
}

float NVGRenderer::getDefaultFontSize() const
{
	return impl->defaultFontSize;
}

int NVGRenderer::createFont(const char *face)
{
	// Empty face: return the first font.
	if (!face || !face[0])
		return 0;

	// Try to find it.
	int id = nvgFindFont(impl->vg, face);

	if (id != -1)
		return id;

	// Not found, create it.
	char fontPath[WZ_NANOVG_MAX_PATH];
	strcpy(fontPath, impl->fontDirectory);
	strcat(fontPath, "/");
	strcat(fontPath, face);
	strcat(fontPath, ".ttf");
	id = nvgCreateFont(impl->vg, face, fontPath);

	if (id != -1)
		return id;

	// Failed to create it, return the first font.
	return 0;
}

int NVGRenderer::createImage(const char *filename, int *width, int *height)
{
	int handle, i;

	handle = -1;

	// Check cache.
	for (i = 0; i < impl->nImages; i++)
	{
		if (strcmp(impl->images[i].filename, filename) == 0)
		{
			handle = impl->images[i].handle;
			break;
		}
	}

	if (handle == -1)
	{
		// Not found, create and cache it.
		impl->images[impl->nImages].handle = nvgCreateImage(impl->vg, filename, 0);
		strcpy(impl->images[impl->nImages].filename, filename);
		handle = impl->images[impl->nImages].handle;
		impl->nImages++;
	}

	nvgImageSize(impl->vg, handle, width, height);
	return handle;
}

void NVGRenderer::setFontFace(const char *face)
{
	int id = createFont(face);

	// Use the first font if creating failed.
	if (id == -1)
		id = 0;

	nvgFontFaceId(impl->vg, id);
}

void NVGRenderer::printBox(Rect rect, const char *fontFace, float fontSize, NVGcolor color, const char *text, size_t textLength)
{
	nvgFontSize(impl->vg, fontSize == 0 ? impl->defaultFontSize : fontSize);
	setFontFace(fontFace);
	nvgTextAlign(impl->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	nvgTextLineHeight(impl->vg, 1.0f);
	nvgFontBlur(impl->vg, 0);
	nvgFillColor(impl->vg, color);
	nvgTextBox(impl->vg, (float)rect.x, (float)rect.y, (float)rect.w, text, textLength == 0 ? NULL : &text[textLength]);
}

void NVGRenderer::print(int x, int y, int align, const char *fontFace, float fontSize, NVGcolor color, const char *text, size_t textLength)
{
	nvgFontSize(impl->vg, fontSize == 0 ? impl->defaultFontSize : fontSize);
	setFontFace(fontFace);
	nvgTextAlign(impl->vg, align);
	nvgFontBlur(impl->vg, 0);
	nvgFillColor(impl->vg, color);
	nvgText(impl->vg, (float)x, (float)y, text, textLength == 0 ? NULL : &text[textLength]);
}

void NVGRenderer::clipToRect(Rect rect)
{
	nvgScissor(impl->vg, (float)rect.x - 1.0f, (float)rect.y - 1.0f, (float)rect.w + 2.0f, (float)rect.h + 2.0f);
}

bool NVGRenderer::clipToRectIntersection(Rect rect1, Rect rect2)
{
	Rect intersection;

	if (!Rect::intersect(rect1, rect2, &intersection))
	{
		return false;
	}

	nvgScissor(impl->vg, (float)intersection.x - 1.0f, (float)intersection.y - 1.0f, (float)intersection.w + 2.0f, (float)intersection.h + 2.0f);
	return true;
}

void NVGRenderer::drawFilledRect(Rect rect, NVGcolor color)
{
	nvgBeginPath(impl->vg);
	nvgRect(impl->vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgFillColor(impl->vg, color);
	nvgFill(impl->vg);
}

void NVGRenderer::drawRect(Rect rect, NVGcolor color)
{
	nvgBeginPath(impl->vg);
	nvgRect(impl->vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgStrokeColor(impl->vg, color);
	nvgStroke(impl->vg);
}

void NVGRenderer::drawLine(int x1, int y1, int x2, int y2, NVGcolor color)
{
	nvgBeginPath(impl->vg);
	nvgMoveTo(impl->vg, (float)x1, (float)y1);
	nvgLineTo(impl->vg, (float)x2, (float)y2);
	nvgStrokeColor(impl->vg, color);
	nvgStroke(impl->vg);
}

void NVGRenderer::drawImage(Rect rect, int image)
{
	int w, h;
	NVGpaint paint;

	nvgImageSize(impl->vg, image, &w, &h);
	paint = nvgImagePattern(impl->vg, (float)rect.x, (float)rect.y, (float)w, (float)h, 0, image, 1);
	nvgBeginPath(impl->vg);
	nvgRect(impl->vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgFillPaint(impl->vg, paint);
	nvgFill(impl->vg);
}

void NVGRenderer::drawCenteredIconAndLabel(Rect rect, const char *label, NVGcolor labelColor, const char *fontFace, float fontSize, const char *icon, int iconSpacing)
{
	// Calculate icon and label sizes.
	Size iconSize;
	int iconHandle = 0;

	if (icon && icon[0])
	{
		iconHandle = createImage(icon, &iconSize.w, &iconSize.h);
	}

	int labelWidth;
	measureText(fontFace, fontSize, label, 0, &labelWidth, NULL);

	// Position the icon and label centered.
	int iconX = 0, labelX = 0;

	if (icon && icon[0] && iconHandle && label && label[0])
	{
		iconX = rect.x + (int)(rect.w / 2.0f - (iconSize.w + iconSpacing + labelWidth) / 2.0f);
		labelX = iconX + iconSize.w + iconSpacing;
	}
	else if (icon && icon[0] && iconHandle)
	{
		iconX = rect.x + (int)(rect.w / 2.0f - iconSize.w / 2.0f);
	}
	else if (label && label[0])
	{
		labelX = rect.x + (int)(rect.w / 2.0f - labelWidth / 2.0f);
	}

	// Draw the icon.
	if (icon && icon[0] && iconHandle)
	{
		Rect iconRect;
		iconRect.x = iconX;
		iconRect.y = rect.y + (int)(rect.h / 2.0f - iconSize.h / 2.0f);
		iconRect.w = iconSize.w;
		iconRect.h = iconSize.h;
		drawImage(iconRect, iconHandle);
	}

	// Draw the label.
	if (label && label[0])
	{
		print(labelX, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, labelColor, label, 0);
	}
}

} // namespace wz
