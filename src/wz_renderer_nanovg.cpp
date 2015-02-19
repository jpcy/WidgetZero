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
	NVGcolor bgColor1, bgColor2;

	if (button->isPressed() && button->getHover())
	{
		bgColor1 = WZ_SKIN_BUTTON_BG_PRESSED_COLOR1;
		bgColor2 = WZ_SKIN_BUTTON_BG_PRESSED_COLOR2;
	}
	else if (button->isSet())
	{
		bgColor1 = WZ_SKIN_BUTTON_BG_SET_COLOR1;
		bgColor2 = WZ_SKIN_BUTTON_BG_SET_COLOR2;
	}
	else
	{
		bgColor1 = WZ_SKIN_BUTTON_BG_COLOR1;
		bgColor2 = WZ_SKIN_BUTTON_BG_COLOR2;
	}

	nvgBeginPath(vg);
	nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, WZ_SKIN_BUTTON_CORNER_RADIUS);

	// Background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, bgColor1, bgColor2));
	nvgFill(vg);

	// Border.
	nvgStrokeColor(vg, button->getHover() ? WZ_SKIN_BUTTON_BORDER_HOVER_COLOR : WZ_SKIN_BUTTON_BORDER_COLOR);
	nvgStroke(vg);

	// Calculate padded rect.
	Rect paddedRect;
	const Border padding = button->getPadding();
	paddedRect.x = rect.x + padding.left;
	paddedRect.y = rect.y + padding.top;
	paddedRect.w = rect.w - (padding.left + padding.right);
	paddedRect.h = rect.h - (padding.top + padding.bottom);

	// Calculate icon and label sizes.
	Size iconSize;
	int iconHandle;

	if (button->getIcon() && button->getIcon()[0])
	{
		iconHandle = createImage(button->getIcon(), &iconSize.w, &iconSize.h);
	}

	int labelWidth;
	button->measureText(button->getLabel(), 0, &labelWidth, NULL);

	// Position the icon and label centered.
	int iconX, labelX;

	if (button->getIcon() && button->getIcon()[0] && iconHandle && button->getLabel() && button->getLabel()[0])
	{
		iconX = paddedRect.x + (int)(paddedRect.w / 2.0f - (iconSize.w + WZ_SKIN_BUTTON_ICON_SPACING + labelWidth) / 2.0f);
		labelX = iconX + iconSize.w + WZ_SKIN_BUTTON_ICON_SPACING;
	}
	else if (button->getIcon() && button->getIcon()[0] && iconHandle)
	{
		iconX = paddedRect.x + (int)(paddedRect.w / 2.0f - iconSize.w / 2.0f);
	}
	else if (button->getLabel() && button->getLabel()[0])
	{
		labelX = paddedRect.x + (int)(paddedRect.w / 2.0f - labelWidth / 2.0f);
	}

	// Draw the icon.
	if (button->getIcon() && button->getIcon()[0] && iconHandle)
	{
		Rect iconRect;
		iconRect.x = iconX;
		iconRect.y = paddedRect.y + (int)(paddedRect.h / 2.0f - iconSize.h / 2.0f);
		iconRect.w = iconSize.w;
		iconRect.h = iconSize.h;
		drawImage(iconRect, iconHandle);
	}

	// Draw the label.
	if (button->getLabel() && button->getLabel()[0])
	{
		print(labelX, paddedRect.y + paddedRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, button->getFontFace(), button->getFontSize(), WZ_SKIN_BUTTON_TEXT_COLOR, button->getLabel(), 0);
	}

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
	drawRect(boxRect, checkBox->getHover() ? WZ_SKIN_CHECK_BOX_BORDER_HOVER_COLOR : WZ_SKIN_CHECK_BOX_BORDER_COLOR);

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
		nvgStrokeColor(vg, WZ_SKIN_CHECK_BOX_CHECK_COLOR);
		nvgStrokeWidth(vg, WZ_SKIN_CHECK_BOX_CHECK_THICKNESS);
		nvgStroke(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, left, bottom);
		nvgLineTo(vg, right, top);
		nvgStroke(vg);
	}

	// Label.
	print(rect.x + WZ_SKIN_CHECK_BOX_BOX_SIZE + WZ_SKIN_CHECK_BOX_BOX_RIGHT_MARGIN, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, checkBox->getFontFace(), checkBox->getFontSize(), WZ_SKIN_CHECK_BOX_TEXT_COLOR, checkBox->getLabel(), 0);

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
	const int nItems = combo->getList()->getNumItems();
	const int selectedItemIndex = combo->getList()->getSelectedItem();

	nvgSave(vg);
	clipToRect(clip);
	nvgBeginPath(vg);

	// Don't round the bottom corners if the combo is open.
	if (combo->isOpen())
	{
		createRectPath(rect, WZ_SKIN_COMBO_CORNER_RADIUS, WZ_SIDE_ALL, WZ_CORNER_TL | WZ_CORNER_TR);
	}
	else
	{
		nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, WZ_SKIN_COMBO_CORNER_RADIUS);
	}

	// Background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, WZ_SKIN_COMBO_BG_COLOR1, WZ_SKIN_COMBO_BG_COLOR2));
	nvgFill(vg);

	// Border.
	nvgStrokeColor(vg, combo->getHover() ? WZ_SKIN_COMBO_BORDER_HOVER_COLOR : WZ_SKIN_COMBO_BORDER_COLOR);
	nvgStroke(vg);

	// Internal border.
	int buttonX = rect.x + rect.w - WZ_SKIN_COMBO_BUTTON_WIDTH;
	drawLine(buttonX, rect.y + 1, buttonX, rect.y + rect.h - 1, combo->getHover() ? WZ_SKIN_COMBO_BORDER_HOVER_COLOR : WZ_SKIN_COMBO_BORDER_COLOR);

	// Icon.
	{
		const float buttonCenterX = buttonX + WZ_SKIN_COMBO_BUTTON_WIDTH * 0.5f;
		const float buttonCenterY = rect.y + rect.h * 0.5f;

		nvgBeginPath(vg);
		nvgMoveTo(vg, buttonCenterX, buttonCenterY + WZ_SKIN_COMBO_ICON_HEIGHT * 0.5f); // bottom
		nvgLineTo(vg, buttonCenterX + WZ_SKIN_COMBO_ICON_WIDTH * 0.5f, buttonCenterY - WZ_SKIN_COMBO_ICON_HEIGHT * 0.5f); // right
		nvgLineTo(vg, buttonCenterX - WZ_SKIN_COMBO_ICON_WIDTH * 0.5f, buttonCenterY - WZ_SKIN_COMBO_ICON_HEIGHT * 0.5f); // left
		nvgFillColor(vg, WZ_SKIN_COMBO_ICON_COLOR);
		nvgFill(vg);
	}

	// Selected item.
	if (selectedItemIndex >= 0)
	{
		print(rect.x + WZ_SKIN_COMBO_PADDING_X / 2, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, combo->getFontFace(), combo->getFontSize(), WZ_SKIN_COMBO_TEXT_COLOR, *((const char **)&itemData[selectedItemIndex * itemStride]), 0);
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
	drawFilledRect(dockPreview->getAbsoluteRect(), WZ_SKIN_MAIN_WINDOW_DOCK_PREVIEW_COLOR);
	nvgRestore(impl->vg);
}

Border NVGRenderer::getGroupBoxMargin(GroupBox *groupBox)
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
		nvgBeginPath(vg);
		nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, WZ_SKIN_GROUP_BOX_CORNER_RADIUS);
		nvgStrokeColor(vg, WZ_SKIN_GROUP_BOX_BORDER_COLOR);
		nvgStroke(vg);
	}
	else
	{
		int textWidth, textHeight;
		Rect borderRect;

		groupBox->measureText(groupBox->getLabel(), 0, &textWidth, &textHeight);
		borderRect = rect;
		borderRect.y += textHeight / 2;
		borderRect.h -= textHeight / 2;

		// Border top, left of text.
		drawLine((int)(borderRect.x + WZ_SKIN_GROUP_BOX_CORNER_RADIUS), borderRect.y, borderRect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN - WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING, borderRect.y, WZ_SKIN_GROUP_BOX_BORDER_COLOR);

		// Border top, right of text.
		drawLine(borderRect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN + textWidth + WZ_SKIN_GROUP_BOX_TEXT_BORDER_SPACING * 2, borderRect.y, (int)(borderRect.x + borderRect.w - WZ_SKIN_GROUP_BOX_CORNER_RADIUS), borderRect.y, WZ_SKIN_GROUP_BOX_BORDER_COLOR);

		// The rest of the border.
		createRectPath(borderRect, WZ_SKIN_GROUP_BOX_CORNER_RADIUS, WZ_SIDE_LEFT | WZ_SIDE_RIGHT | WZ_SIDE_BOTTOM, WZ_CORNER_ALL);
		nvgStrokeColor(vg, WZ_SKIN_GROUP_BOX_BORDER_COLOR);
		nvgStroke(vg);

		// Label.
		print(rect.x + WZ_SKIN_GROUP_BOX_TEXT_LEFT_MARGIN, rect.y, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, groupBox->getFontFace(), groupBox->getFontSize(), WZ_SKIN_GROUP_BOX_TEXT_COLOR, groupBox->getLabel(), 0);
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureGroupBox(GroupBox *groupBox)
{
	return Size();
}

Color NVGRenderer::getLabelTextColor(Label *label)
{
	NVGcolor c = WZ_SKIN_LABEL_TEXT_COLOR;
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
	const Rect rect = label->getAbsoluteRect();
	Size size;

	if (label->getMultiline())
	{
		nvgFontSize(vg, label->getFontSize() == 0 ? getDefaultFontSize() : label->getFontSize());
		setFontFace(label->getFontFace());
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgTextLineHeight(vg, 1.2f);
		float bounds[4];
		nvgTextBoxBounds(vg, 0, 0, (float)rect.w, label->getText(), NULL, bounds);
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
	nvgBeginPath(vg);
	nvgRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, WZ_SKIN_LIST_BG_COLOR1, WZ_SKIN_LIST_BG_COLOR2));
	nvgFill(vg);

	// Border.
	drawRect(rect, WZ_SKIN_LIST_BORDER_COLOR);

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
			drawFilledRect(itemRect, WZ_SKIN_LIST_SET_COLOR);
		}
		else if (i == list->getPressedItem() || i == list->getHoveredItem())
		{
			drawFilledRect(itemRect, WZ_SKIN_LIST_HOVER_COLOR);
		}

		if (list->getDrawItemCallback())
		{
			list->getDrawItemCallback()(this, itemRect, list, list->getFontFace(), list->getFontSize(), i, itemData);
		}
		else
		{
			print(itemsRect.x + WZ_SKIN_LIST_ITEM_LEFT_PADDING, y + list->getItemHeight() / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, list->getFontFace(), list->getFontSize(), WZ_SKIN_LIST_TEXT_COLOR, (const char *)itemData, 0);
		}

		y += list->getItemHeight();
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureList(List *list)
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
		drawFilledRect(rect, WZ_SKIN_MENU_BAR_SET_COLOR);
	}

	if (button->getHover())
	{
		drawRect(rect, WZ_SKIN_MENU_BAR_BORDER_HOVER_COLOR);
	}

	print(rect.x + rect.w / 2, rect.y + rect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, button->getFontFace(), button->getFontSize(), WZ_SKIN_MENU_BAR_TEXT_COLOR, button->getLabel(), 0);

	nvgRestore(vg);
}

Size NVGRenderer::measureMenuBarButton(MenuBarButton *button)
{
	Size size;
	button->measureText(button->getLabel(), 0, &size.w, &size.h);
	size.w += 12;
	return size;
}

int NVGRenderer::getMenuBarPadding(MenuBar *menuBar)
{
	return WZ_SKIN_MENU_BAR_PADDING;
}

void NVGRenderer::drawMenuBar(MenuBar *menuBar, Rect clip)
{
	nvgSave(impl->vg);
	clipToRect(clip);
	drawFilledRect(menuBar->getAbsoluteRect(), WZ_SKIN_MENU_BAR_BG_COLOR);
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
		nvgFillColor(vg, WZ_SKIN_RADIO_BUTTON_SET_COLOR);
		nvgFill(vg);
	}

	// Outer circle.
	nvgBeginPath(vg);
	nvgCircle(vg, (float)(rect.x + WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS), rect.y + rect.h / 2.0f, (float)WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS - 0.5f);
	nvgStrokeColor(vg, button->getHover() ? WZ_SKIN_RADIO_BUTTON_BORDER_HOVER_COLOR : WZ_SKIN_RADIO_BUTTON_BORDER_COLOR);
	nvgStroke(vg);

	// Label.
	print(rect.x + WZ_SKIN_RADIO_BUTTON_OUTER_RADIUS * 2 + WZ_SKIN_RADIO_BUTTON_SPACING, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, button->getFontFace(), button->getFontSize(), WZ_SKIN_RADIO_BUTTON_TEXT_COLOR, button->getLabel(), 0);

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
	
	nvgSave(vg);
	clipToRect(clip);

	// Background color.
	NVGcolor bgColor1, bgColor2;

	if (button->isPressed() && button->getHover())
	{
		bgColor1 = WZ_SKIN_SCROLLER_BG_PRESSED_COLOR1;
		bgColor2 = WZ_SKIN_SCROLLER_BG_PRESSED_COLOR2;
	}
	else
	{
		bgColor1 = WZ_SKIN_SCROLLER_BG_COLOR1;
		bgColor2 = WZ_SKIN_SCROLLER_BG_COLOR2;
	}

	nvgBeginPath(vg);
	Scroller *scroller = (Scroller *)button->getParent();
	int sides, roundedCorners;

	if (scroller->getDirection() == ScrollerDirection::Vertical)
	{
		if (decrement)
		{
			sides = WZ_SIDE_LEFT | WZ_SIDE_TOP | WZ_SIDE_RIGHT;
			roundedCorners = WZ_CORNER_TL | WZ_CORNER_TR;
		}
		else
		{
			sides = WZ_SIDE_LEFT | WZ_SIDE_BOTTOM | WZ_SIDE_RIGHT;
			roundedCorners = WZ_CORNER_BL | WZ_CORNER_BR;
		}
	}
	else
	{
		if (decrement)
		{
			sides = WZ_SIDE_TOP | WZ_SIDE_LEFT | WZ_SIDE_BOTTOM;
			roundedCorners = WZ_CORNER_TL | WZ_CORNER_BL;
		}
		else
		{
			sides = WZ_SIDE_TOP | WZ_SIDE_RIGHT | WZ_SIDE_BOTTOM;
			roundedCorners = WZ_CORNER_TR | WZ_CORNER_BR;
		}
	}

	// Background.
	const Rect rect = button->getAbsoluteRect();
	createRectPath(rect, WZ_SKIN_SCROLLER_CORNER_RADIUS, WZ_SIDE_ALL, roundedCorners);
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, bgColor1, bgColor2));
	nvgFill(vg);

	// Border.
	createRectPath(rect, WZ_SKIN_SCROLLER_CORNER_RADIUS, sides, roundedCorners);
	nvgStrokeColor(vg, button->getHover() ? WZ_SKIN_SCROLLER_BORDER_HOVER_COLOR : WZ_SKIN_SCROLLER_BORDER_COLOR);
	nvgStroke(vg);

	// Icon.
	nvgBeginPath(vg);

	if (scroller->getDirection() == ScrollerDirection::Vertical)
	{
		if (decrement)
		{
			nvgMoveTo(vg, rect.x + rect.w * 0.5f, rect.y + rect.h * 0.25f); // top
			nvgLineTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.75f); // left
			nvgLineTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.75f); // right
		}
		else
		{
			nvgMoveTo(vg, rect.x + rect.w * 0.5f, rect.y + rect.h * 0.75f); // bottom
			nvgLineTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.25f); // right
			nvgLineTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.25f); // left
		}
	}
	else
	{
		if (decrement)
		{
			nvgMoveTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.5f); // left
			nvgLineTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.75f); // bottom
			nvgLineTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.25f); // top
		}
		else
		{
			nvgMoveTo(vg, rect.x + rect.w * 0.75f, rect.y + rect.h * 0.5f); // right
			nvgLineTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.25f); // top
			nvgLineTo(vg, rect.x + rect.w * 0.25f, rect.y + rect.h * 0.75f); // bottom
		}
	}

	nvgFillColor(vg, button->getHover() ? WZ_SKIN_SCROLLER_ICON_HOVER_COLOR : WZ_SKIN_SCROLLER_ICON_COLOR);
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
	
	nvgSave(vg);
	clipToRect(clip);

	Rect nubContainerRect, nubRect;
	bool hover, pressed;
	scroller->getNubState(&nubContainerRect, &nubRect, &hover, &pressed);

	// Nub container.
	drawFilledRect(nubContainerRect, WZ_SKIN_SCROLLER_BG_COLOR1);

	// Nub.
	{
		const Rect r = nubRect;
		NVGcolor bgColor1, bgColor2;
		int i;

		// Background color.
		if (pressed)
		{
			bgColor1 = WZ_SKIN_SCROLLER_BG_PRESSED_COLOR1;
			bgColor2 = WZ_SKIN_SCROLLER_BG_PRESSED_COLOR2;
		}
		else
		{
			bgColor1 = WZ_SKIN_SCROLLER_BG_COLOR1;
			bgColor2 = WZ_SKIN_SCROLLER_BG_COLOR2;
		}

		nvgBeginPath(vg);
		nvgRect(vg, r.x + 0.5f, r.y + 0.5f, r.w - 1.0f, r.h - 1.0f);

		// Background.
		nvgFillPaint(vg, nvgLinearGradient(vg, (float)r.x, (float)r.y, (float)r.x, (float)r.y + r.h, bgColor1, bgColor2));
		nvgFill(vg);

		// Border.
		nvgStrokeColor(vg, hover || pressed ? WZ_SKIN_SCROLLER_BORDER_HOVER_COLOR : WZ_SKIN_SCROLLER_BORDER_COLOR);
		nvgStroke(vg);

		// Icon.
		for (i = 0; i < 3; i++)
		{
			nvgBeginPath(vg);

			if (scroller->getDirection() == StackLayoutDirection::Vertical)
			{
				const float y = (float)((int)(r.y + r.h * 0.5f) + WZ_SKIN_SCROLLER_NUB_ICON_SPACING * (i - 1));
				nvgMoveTo(vg, (float)r.x + WZ_SKIN_SCROLLER_NUB_ICON_MARGIN, y);
				nvgLineTo(vg, (float)r.x + r.w - WZ_SKIN_SCROLLER_NUB_ICON_MARGIN, y);
			}
			else
			{
				const float x = (float)((int)(r.x + r.w * 0.5f) + WZ_SKIN_SCROLLER_NUB_ICON_SPACING * (i - 1));
				nvgMoveTo(vg, x, (float)r.y + WZ_SKIN_SCROLLER_NUB_ICON_MARGIN);
				nvgLineTo(vg, x, (float)r.y + r.h - WZ_SKIN_SCROLLER_NUB_ICON_MARGIN);
			}

			nvgStrokeColor(vg, hover || pressed ? WZ_SKIN_SCROLLER_ICON_HOVER_COLOR : WZ_SKIN_SCROLLER_ICON_COLOR);
			nvgStrokeWidth(vg, 2);
			nvgLineCap(vg, NVG_ROUND);
			nvgStroke(vg);
		}
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureScroller(Scroller *scroller)
{
	return scroller->getDirection() == ScrollerDirection::Vertical ? Size(WZ_SKIN_SCROLLER_THICKNESS, 0) : Size(0, WZ_SKIN_SCROLLER_THICKNESS);
}

void NVGRenderer::drawSpinnerButton(Button *button, Rect clip, bool decrement)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = button->getAbsoluteRect();
	const int buttonX = rect.x + rect.w - WZ_SKIN_SPINNER_BUTTON_WIDTH;
	const float buttonCenterX = buttonX + WZ_SKIN_SPINNER_BUTTON_WIDTH * 0.5f;
	const float buttonCenterY = rect.y + rect.h * 0.5f;

	nvgSave(vg);
	clipToRect(clip);
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

	nvgFillColor(vg, button->getHover() ? WZ_SKIN_SPINNER_ICON_HOVER_COLOR : WZ_SKIN_SPINNER_ICON_COLOR);
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

void NVGRenderer::drawSpinner(Spinner *spinner, Rect clip)
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
	clipToRect(clip);

	// Label.
	const Rect rect = button->getAbsoluteRect();
	const Border padding = button->getPadding();
	Rect labelRect;

	labelRect.x = rect.x + padding.left;
	labelRect.y = rect.y + padding.top;
	labelRect.w = rect.w - (padding.left + padding.right);
	labelRect.h = rect.h - (padding.top + padding.bottom);
	print(labelRect.x + labelRect.w / 2, labelRect.y + labelRect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, button->getFontFace(), button->getFontSize(), button->getHover() ? WZ_SKIN_TAB_BUTTON_TEXT_HOVER_COLOR : WZ_SKIN_TAB_BUTTON_TEXT_COLOR, button->getLabel(), 0);

	nvgRestore(vg);
}

void NVGRenderer::drawTabBar(TabBar *tabBar, Rect clip)
{
}

Size NVGRenderer::measureTabBar(TabBar *tabBar)
{
	// Padding.
	return Size(0, tabBar->getLineHeight() + 8);
}

void NVGRenderer::drawTabbed(Tabbed *tabbed, Rect clip)
{
	NVGcontext *vg = impl->vg;
	const Button *selectedTab = tabbed->tabBar->getSelectedTab();

	// Use the page rect.
	const int tabBarHeight = tabbed->tabBar->getHeight();
	Rect rect = tabbed->getAbsoluteRect();
	rect.y += tabBarHeight;
	rect.h -= tabBarHeight;

	nvgSave(vg);
	clipToRectIntersection(clip, tabbed->getAbsoluteRect());

	// Draw an outline around the selected tab and the tab page.
	nvgBeginPath(vg);

	if (selectedTab->getVisible())
	{
		// Selected tab.
		const Rect tr = selectedTab->getAbsoluteRect();
		nvgMoveTo(vg, tr.x + 0.5f, tr.y + tr.h + 0.5f); // bl
		nvgLineTo(vg, tr.x + 0.5f, tr.y + 0.5f); // tl
		nvgLineTo(vg, tr.x + tr.w - 0.5f, tr.y + 0.5f); // tr
		nvgLineTo(vg, tr.x + tr.w - 0.5f, tr.y + tr.h + 0.5f); // br

		// The tab page.
		nvgLineTo(vg, rect.x + rect.w - 0.5f, rect.y + 0.5f); // tr
		nvgLineTo(vg, rect.x + rect.w - 0.5f, rect.y + rect.h - 0.5f); // br
		nvgLineTo(vg, rect.x + 0.5f, rect.y + rect.h - 0.5f); // bl
		nvgLineTo(vg, rect.x + 0.5f, rect.y + 0.5f); // tl
		nvgClosePath(vg);
	}
	else
	{
		nvgRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f);
	}
	
	nvgStrokeColor(vg, WZ_SKIN_TABBED_BORDER_COLOR);
	nvgStroke(vg);

	// Get the selected tab index.
	int selectedTabIndex = -1;

	for (size_t i = 0; i < tabbed->pages.size(); i++)
	{
		if (tabbed->pages[i].tab == selectedTab)
		{
			selectedTabIndex = (int)i;
			break;
		}
	}

	// Draw an outline around the non-selected tabs.
	for (size_t i = 0; i < tabbed->pages.size(); i++)
	{
		const Button *tab = tabbed->pages[i].tab;

		if (tab == selectedTab || !tab->getVisible())
			continue;

		const Rect tr = tab->getAbsoluteRect();
		nvgBeginPath(vg);

		// Only draw the left side if this is the leftmost tab.
		if (i == tabbed->tabBar->getScrollValue())
		{
			nvgMoveTo(vg, tr.x + 0.5f, tr.y + tr.h - 0.5f); // bl
			nvgLineTo(vg, tr.x + 0.5f, tr.y + 0.5f); // tl
		}
		else
		{
			nvgMoveTo(vg, tr.x + 0.5f, tr.y + 0.5f); // tl
		}

		nvgLineTo(vg, tr.x + tr.w - 0.5f, tr.y + 0.5f); // tr

		// If the selected tab is next to this tab, on the right, don't draw the right side.
		if (selectedTabIndex != i + 1)
		{
			nvgLineTo(vg, tr.x + tr.w - 0.5f, tr.y + tr.h - 0.5f); // br
		}

		nvgStrokeColor(vg, WZ_SKIN_TABBED_BORDER_COLOR);
		nvgStroke(vg);
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureTabbed(Tabbed *tabbed)
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
	
	nvgBeginPath(vg);
	nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, WZ_SKIN_TEXT_EDIT_CORNER_RADIUS);

	// Background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, WZ_SKIN_TEXT_EDIT_BG_COLOR1, WZ_SKIN_TEXT_EDIT_BG_COLOR2));
	nvgFill(vg);

	// Border.
	nvgStrokeColor(vg, textEdit->getHover() ? WZ_SKIN_TEXT_EDIT_BORDER_HOVER_COLOR : WZ_SKIN_TEXT_EDIT_BORDER_COLOR);
	nvgStroke(vg);

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
				print(textRect.x, textRect.y + lineY, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, textEdit->getFontFace(), textEdit->getFontSize(), WZ_SKIN_TEXT_EDIT_TEXT_COLOR, line.start, line.length);

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
						drawFilledRect(selectionRect, WZ_SKIN_TEXT_EDIT_SELECTION_COLOR);
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
		print(textRect.x, textRect.y + textRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, textEdit->getFontFace(), textEdit->getFontSize(), WZ_SKIN_TEXT_EDIT_TEXT_COLOR, textEdit->getVisibleText(), 0);

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
			drawFilledRect(selectionRect, WZ_SKIN_TEXT_EDIT_SELECTION_COLOR);
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
		nvgStrokeColor(vg, WZ_SKIN_TEXT_EDIT_CURSOR_COLOR);
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

void NVGRenderer::drawWindow(Window *window, Rect clip)
{
	NVGcontext *vg = impl->vg;
	const Rect rect = window->getAbsoluteRect();
	const Rect contentRect = window->getContentWidget()->getAbsoluteRect();
	const Rect headerRect = window->getHeaderRect();

	nvgSave(vg);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h, WZ_SKIN_WINDOW_CORNER_RADIUS);

	// Border/header background.
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)rect.x, (float)rect.y, (float)rect.x + rect.w, (float)rect.y, WZ_SKIN_WINDOW_BORDER_BG_COLOR1, WZ_SKIN_WINDOW_BORDER_BG_COLOR2));
	nvgFill(vg);

	// Outer border.
	nvgStrokeColor(vg, WZ_SKIN_WINDOW_BORDER_COLOR);
	nvgStroke(vg);

	// Inner border.
	nvgBeginPath(vg);
	nvgRect(vg, contentRect.x - 1.0f, contentRect.y - 1.0f, contentRect.w + 2.0f, contentRect.h + 2.0f);
	nvgStrokeColor(vg, WZ_SKIN_WINDOW_INNER_BORDER_COLOR);
	nvgStroke(vg);

	// Background/content.
	nvgBeginPath(vg);
	nvgRect(vg, (float)contentRect.x, (float)contentRect.y, (float)contentRect.w, (float)contentRect.h);
	nvgFillPaint(vg, nvgLinearGradient(vg, (float)contentRect.x, (float)contentRect.y, (float)contentRect.x, (float)contentRect.y + contentRect.h, WZ_SKIN_WINDOW_BG_COLOR1, WZ_SKIN_WINDOW_BG_COLOR2));
	nvgFill(vg);

	// Header.
	if (headerRect.w > 0 && headerRect.h > 0)
	{
		clipToRect(headerRect);
		print(headerRect.x + 10, headerRect.y + headerRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, window->getFontFace(), window->getFontSize(), WZ_SKIN_WINDOW_TEXT_COLOR, window->getTitle(), 0);
	}

	nvgRestore(vg);
}

Size NVGRenderer::measureWindow(Window *window)
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

LineBreakResult NVGRenderer::lineBreakText(const char *fontFace, float fontSize, const char *text, int n, int lineWidth)
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
	nvgScissor(impl->vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
}

bool NVGRenderer::clipToRectIntersection(Rect rect1, Rect rect2)
{
	Rect intersection;

	if (!Rect::intersect(rect1, rect2, &intersection))
	{
		return false;
	}

	nvgScissor(impl->vg, (float)intersection.x, (float)intersection.y, (float)intersection.w, (float)intersection.h);
	return true;
}

void NVGRenderer::drawFilledRect(Rect rect, NVGcolor color)
{
	nvgBeginPath(impl->vg);
	nvgRect(impl->vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f);
	nvgFillColor(impl->vg, color);
	nvgFill(impl->vg);
}

void NVGRenderer::drawRect(Rect rect, NVGcolor color)
{
	nvgBeginPath(impl->vg);
	nvgRect(impl->vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f);
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

void NVGRenderer::createRectPath(Rect rect, float r, int sides, int roundedCorners)
{
	const float x = rect.x + 0.5f;
	const float y = rect.y + 0.5f;
	const float w = rect.w - 1.0f;
	const float h = rect.h - 1.0f;
	const float rx = WZ_MIN(r, WZ_ABS(w) * 0.5f) * WZ_SIGN(w);
	const float ry = WZ_MIN(r, WZ_ABS(h) * 0.5f) * WZ_SIGN(h);
	const float NVG_KAPPA90 = 0.5522847493f;

	nvgBeginPath(impl->vg);
	
	if (roundedCorners & WZ_CORNER_TL)
	{
		nvgMoveTo(impl->vg, x, y + ry);
	}
	else
	{
		nvgMoveTo(impl->vg, x, y);
	}

	if (roundedCorners & WZ_CORNER_BL)
	{
		// left straight
		if (sides & WZ_SIDE_LEFT)
		{
			nvgLineTo(impl->vg, x, y + h - ry);
		}
		else
		{
			nvgMoveTo(impl->vg, x, y + h - ry);
		}

		nvgBezierTo(impl->vg, x, y+h-ry*(1-NVG_KAPPA90), x+rx*(1-NVG_KAPPA90), y+h, x+rx, y+h); // bottom left arc
	}
	else
	{
		if (sides & WZ_SIDE_LEFT)
		{
			nvgLineTo(impl->vg, x, y + h);
		}
		else
		{
			nvgMoveTo(impl->vg, x, y + h);
		}
	}

	if (roundedCorners & WZ_CORNER_BR)
	{
		// bottom straight
		if (sides & WZ_SIDE_BOTTOM)
		{
			nvgLineTo(impl->vg, x+w-rx, y+h);
		}
		else
		{
			nvgMoveTo(impl->vg, x+w-rx, y+h);
		}

		nvgBezierTo(impl->vg, x+w-rx*(1-NVG_KAPPA90), y+h, x+w, y+h-ry*(1-NVG_KAPPA90), x+w, y+h-ry); // bottom right arc
	}
	else
	{
		if (sides & WZ_SIDE_BOTTOM)
		{
			nvgLineTo(impl->vg, x + w, y + h);
		}
		else
		{
			nvgMoveTo(impl->vg, x + w, y + h);
		}
	}

	if (roundedCorners & WZ_CORNER_TR)
	{
		// right straight
		if (sides & WZ_SIDE_RIGHT)
		{
			nvgLineTo(impl->vg, x+w, y+ry);
		}
		else
		{
			nvgMoveTo(impl->vg, x+w, y+ry);
		}

		nvgBezierTo(impl->vg, x+w, y+ry*(1-NVG_KAPPA90), x+w-rx*(1-NVG_KAPPA90), y, x+w-rx, y); // top right arc
	}
	else
	{
		if (sides & WZ_SIDE_RIGHT)
		{
			nvgLineTo(impl->vg, x + w, y);
		}
		else
		{
			nvgMoveTo(impl->vg, x + w, y);
		}
	}

	if (roundedCorners & WZ_CORNER_TL)
	{
		// top straight
		if (sides & WZ_SIDE_TOP)
		{
			nvgLineTo(impl->vg, x+rx, y);
		}
		else
		{
			nvgMoveTo(impl->vg, x+rx, y);
		}

		nvgBezierTo(impl->vg, x+rx*(1-NVG_KAPPA90), y, x, y+ry*(1-NVG_KAPPA90), x, y+ry); // top left arc
	}
	else if (sides & WZ_SIDE_TOP)
	{
		nvgLineTo(impl->vg, x, y);
	}
}

} // namespace wz
