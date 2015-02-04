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
	struct NVGcontext *vg;
	Image images[WZ_NANOVG_MAX_IMAGES];
	int nImages;
	char fontDirectory[WZ_NANOVG_MAX_PATH];
	float defaultFontSize;
};

static int wz_nanovg_create_font(struct NVGRendererImpl *renderer, const char *face)
{
	char fontPath[WZ_NANOVG_MAX_PATH];
	int id;

	// Empty face: return the first font.
	if (!face || !face[0])
		return 0;

	// Try to find it.
	id = nvgFindFont(renderer->vg, face);

	if (id != -1)
		return id;

	// Not found, create it.
	strcpy(fontPath, renderer->fontDirectory);
	strcat(fontPath, "/");
	strcat(fontPath, face);
	strcat(fontPath, ".ttf");
	id = nvgCreateFont(renderer->vg, face, fontPath);

	if (id != -1)
		return id;

	// Failed to create it, return the first font.
	return 0;
}

NVGRenderer::NVGRenderer(wzNanoVgGlCreate create, wzNanoVgGlDestroy destroy, int flags, const char *fontDirectory, const char *defaultFontFace, float defaultFontSize)
{
	WZ_ASSERT(create);
	WZ_ASSERT(destroy);

	impl = new NVGRendererImpl;
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

	if (wz_nanovg_create_font(impl, defaultFontFace) == -1)
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

	delete impl;
}

const char *NVGRenderer::getError()
{
	return impl->errorMessage[0] == 0 ? NULL : impl->errorMessage;
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
	struct NVGtextRow row;
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

void NVGRenderer::drawButton(const ButtonImpl *button, Rect clip)
{
	struct NVGcontext *vg = impl->vg;
	nvgSave(impl->vg);
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

	nvgBeginPath(impl->vg);
	nvgRoundedRect(impl->vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f, WZ_SKIN_BUTTON_CORNER_RADIUS);

	// Background.
	nvgFillPaint(impl->vg, nvgLinearGradient(impl->vg, (float)rect.x, (float)rect.y, (float)rect.x, (float)rect.y + rect.h, bgColor1, bgColor2));
	nvgFill(impl->vg);

	// Border.
	nvgStrokeColor(impl->vg, button->getHover() ? WZ_SKIN_BUTTON_BORDER_HOVER_COLOR : WZ_SKIN_BUTTON_BORDER_COLOR);
	nvgStroke(impl->vg);

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

	nvgRestore(impl->vg);
}

Size NVGRenderer::measureButton(const ButtonImpl *button)
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

struct NVGcontext *NVGRenderer::getContext()
{
	return impl->vg;
}

float NVGRenderer::getDefaultFontSize() const
{
	return impl->defaultFontSize;
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
	int id;
	
	id = wz_nanovg_create_font(impl, face);

	// Use the first font if creating failed.
	if (id == -1)
		id = 0;

	nvgFontFaceId(impl->vg, id);
}

void NVGRenderer::printBox(Rect rect, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength)
{
	nvgFontSize(impl->vg, fontSize == 0 ? impl->defaultFontSize : fontSize);
	setFontFace(fontFace);
	nvgTextAlign(impl->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	nvgTextLineHeight(impl->vg, 1.0f);
	nvgFontBlur(impl->vg, 0);
	nvgFillColor(impl->vg, color);
	nvgTextBox(impl->vg, (float)rect.x, (float)rect.y, (float)rect.w, text, textLength == 0 ? NULL : &text[textLength]);
}

void NVGRenderer::print(int x, int y, int align, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength)
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

void NVGRenderer::drawFilledRect(Rect rect, struct NVGcolor color)
{
	nvgBeginPath(impl->vg);
	nvgRect(impl->vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f);
	nvgFillColor(impl->vg, color);
	nvgFill(impl->vg);
}

void NVGRenderer::drawRect(Rect rect, struct NVGcolor color)
{
	nvgBeginPath(impl->vg);
	nvgRect(impl->vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f);
	nvgStrokeColor(impl->vg, color);
	nvgStroke(impl->vg);
}

void NVGRenderer::drawLine(int x1, int y1, int x2, int y2, struct NVGcolor color)
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
	struct NVGpaint paint;

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
