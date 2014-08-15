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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <wz.h>
#include <wz_nanovg.h>
#include "nanovg.h"

#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"

#define WZ_NANOVG_MAX_PATH 256
#define WZ_NANOVG_MAX_IMAGES 1024
#define WZ_NANOVG_MAX_ERROR_MESSAGE 1024

typedef struct
{
	int handle;
	char filename[WZ_NANOVG_MAX_PATH];
}
wzImage;

typedef struct
{
	struct NVGcontext *vg;
	wzImage images[WZ_NANOVG_MAX_IMAGES];
	int nImages;
	char fontDirectory[WZ_NANOVG_MAX_PATH];
	float defaultFontSize;
}
wzRendererData;

static char errorMessage[WZ_NANOVG_MAX_ERROR_MESSAGE];

static const int buttonIconSpacing = 6;

static const int checkBoxBoxSize = 16;
static const int checkBoxBoxRightMargin = 8;

static const int radioButtonOuterRadius = 8;
static const int radioButtonInnerRadius = 4;
static const int radioButtonSpacing = 8;

/*
================================================================================

MISC. UTILITY FUNCTIONS

================================================================================
*/

static int wz_nanovg_create_image(wzRendererData *rendererData, const char *filename)
{
	int i;

	// Check cache.
	for (i = 0; i < rendererData->nImages; i++)
	{
		if (strcmp(rendererData->images[i].filename, filename) == 0)
			return rendererData->images[i].handle;
	}

	// Not found, create and cache it.
	rendererData->images[rendererData->nImages].handle = nvgCreateImage(rendererData->vg, filename, 0);
	strcpy(rendererData->images[rendererData->nImages].filename, filename);
	rendererData->nImages++;

	return rendererData->images[rendererData->nImages - 1].handle;
}

static int wz_nanovg_create_font(wzRendererData *rendererData, const char *face)
{
	char fontPath[WZ_NANOVG_MAX_PATH];
	int id;

	// Empty face: return the first font.
	if (!face || !face[0])
		return 0;

	// Try to find it.
	id = nvgFindFont(rendererData->vg, face);

	if (id != -1)
		return id;

	// Not found, create it.
	strcpy(fontPath, rendererData->fontDirectory);
	strcat(fontPath, "/");
	strcat(fontPath, face);
	strcat(fontPath, ".ttf");
	id = nvgCreateFont(rendererData->vg, face, fontPath);

	if (id != -1)
		return id;

	// Failed to create it, return the first font.
	return 0;
}

static void wz_nanovg_set_font_face(wzRendererData *rendererData, const char *face)
{
	int id = wz_nanovg_create_font(rendererData, face);

	// Use the first font if creating failed.
	if (id == -1)
		id = 0;

	nvgFontFaceId(rendererData->vg, id);
}

static void wz_nanovg_print_box(wzRendererData *rendererData, wzRect rect, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength)
{
	WZ_ASSERT(rendererData);
	nvgFontSize(rendererData->vg, fontSize == 0 ? rendererData->defaultFontSize : fontSize);
	wz_nanovg_set_font_face(rendererData, fontFace);
	nvgTextAlign(rendererData->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	nvgTextLineHeight(rendererData->vg, 1.0f);
	nvgFontBlur(rendererData->vg, 0);
	nvgFillColor(rendererData->vg, color);
	nvgTextBox(rendererData->vg, (float)rect.x, (float)rect.y, (float)rect.w, text, textLength == 0 ? NULL : &text[textLength]);
}

static void wz_nanovg_print(wzRendererData *rendererData, int x, int y, int align, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength)
{
	WZ_ASSERT(rendererData);
	nvgFontSize(rendererData->vg, fontSize == 0 ? rendererData->defaultFontSize : fontSize);
	wz_nanovg_set_font_face(rendererData, fontFace);
	nvgTextAlign(rendererData->vg, align);
	nvgFontBlur(rendererData->vg, 0);
	nvgFillColor(rendererData->vg, color);
	nvgText(rendererData->vg, (float)x, (float)y, text, textLength == 0 ? NULL : &text[textLength]);
}

static void wz_nanovg_clip_to_rect(struct NVGcontext *vg, wzRect rect)
{
	WZ_ASSERT(vg);
	nvgScissor(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
}

static bool wz_nanovg_clip_to_rect_intersection(struct NVGcontext *vg, wzRect rect1, wzRect rect2)
{
	wzRect intersection;

	WZ_ASSERT(vg);

	if (!wz_intersect_rects(rect1, rect2, &intersection))
	{
		return false;
	}

	nvgScissor(vg, (float)intersection.x, (float)intersection.y, (float)intersection.w, (float)intersection.h);
	return true;
}

static void wz_nanovg_draw_filled_rect(struct NVGcontext *vg, wzRect rect, struct NVGcolor color)
{
	nvgBeginPath(vg);
	nvgRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgFillColor(vg, color);
	nvgFill(vg);
}

static void wz_nanovg_draw_rect(struct NVGcontext *vg, wzRect rect, struct NVGcolor color)
{
	nvgBeginPath(vg);
	nvgRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 0.5f, rect.h - 0.5f);
	nvgStrokeColor(vg, color);
	nvgStroke(vg);
}

static void wz_nanovg_draw_line(struct NVGcontext *vg, int x1, int y1, int x2, int y2, struct NVGcolor color)
{
	nvgBeginPath(vg);
	nvgMoveTo(vg, (float)x1, (float)y1);
	nvgLineTo(vg, (float)x2, (float)y2);
	nvgStrokeColor(vg, color);
	nvgStroke(vg);
}

static void wz_nanovg_draw_icon(struct NVGcontext *vg, wzRect rect, int icon)
{
	int w, h;
	struct NVGpaint paint;

	nvgImageSize(vg, icon, &w, &h);
	paint = nvgImagePattern(vg, (float)rect.x, (float)rect.y, (float)w, (float)h, 0, icon, NVG_NOREPEAT, 1);
	nvgBeginPath(vg);
	nvgRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgFillPaint(vg, paint);
	nvgFill(vg);
}

/*
================================================================================

RENDERER

================================================================================
*/

static void wz_nanovg_begin_frame(struct wzRenderer *renderer, const struct wzMainWindow *mainWindow)
{
	wzSize windowSize;

	WZ_ASSERT(renderer);
	windowSize = wz_widget_get_size((const struct wzWidget *)mainWindow);
	nvgBeginFrame(((wzRendererData *)renderer->data)->vg, windowSize.w, windowSize.h, 1);
}

static void wz_nanovg_end_frame(struct wzRenderer *renderer)
{
	nvgEndFrame(((wzRendererData *)renderer->data)->vg);
}

static int wz_nanovg_get_line_height(struct wzRenderer *renderer, const char *fontFace, float fontSize)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	float lineHeight;

	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgFontSize(vg, fontSize == 0 ? rendererData->defaultFontSize : fontSize);
	wz_nanovg_set_font_face(rendererData, fontFace);
	nvgTextMetrics(vg, NULL, NULL, &lineHeight);
	return (int)lineHeight;
}

static void wz_nanovg_measure_text(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int *width, int *height)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;

	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	if (width)
	{
		nvgFontSize(vg, fontSize == 0 ? rendererData->defaultFontSize : fontSize);
		wz_nanovg_set_font_face(rendererData, fontFace);
		*width = (int)nvgTextBounds(vg, 0, 0, text, n == 0 ? NULL : &text[n], NULL);
	}

	if (height)
	{
		*height = (int)(fontSize == 0 ? rendererData->defaultFontSize : fontSize);
	}
}

static wzLineBreakResult wz_nanovg_line_break_text(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int lineWidth)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	struct NVGtextRow row;
	wzLineBreakResult result;

	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	if (text && nvgTextBreakLines(vg, text, n == 0 ? NULL : &text[n], (float)lineWidth, &row, 1) > 0)
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

static void wz_nanovg_debug_draw_text(struct wzRenderer *renderer, const char *text, int x, int y)
{
	WZ_ASSERT(renderer);
	wz_nanovg_print((wzRendererData *)renderer->data, x, y, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, NULL, 0, nvgRGB(0, 0, 0), text, 0);
}

static void wz_nanovg_draw_dock_icon(struct wzRenderer *renderer, wzRect rect)
{
	struct NVGcontext *vg;

	WZ_ASSERT(renderer);
	vg = ((wzRendererData *)renderer->data)->vg;

	nvgSave(vg);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h, 3);
	nvgFillColor(vg, nvgRGBA(64, 64, 64, 128));
	nvgFill(vg);
	nvgRestore(vg);
}

static void wz_nanovg_draw_dock_preview(struct wzRenderer *renderer, wzRect rect)
{
	struct NVGcontext *vg;

	WZ_ASSERT(renderer);
	vg = ((wzRendererData *)renderer->data)->vg;

	nvgSave(vg);
	wz_nanovg_draw_filled_rect(vg, rect, nvgRGBA(0, 0, 128, 64));
	nvgRestore(vg);
}

static wzSize wz_nanovg_measure_button(struct wzRenderer *renderer, wzBorder padding, const char *icon, const char *fontFace, float fontSize, const char *label)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzSize size;

	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	wz_nanovg_measure_text(renderer, fontFace, fontSize, label, 0, &size.w, &size.h);

	if (icon && icon[0])
	{
		const int handle = wz_nanovg_create_image(rendererData, icon);

		if (handle)
		{
			int w, h;
			nvgImageSize(vg, handle, &w, &h);
			size.w += w + buttonIconSpacing;
			size.h = WZ_MAX(size.h, h);
		}
	}

	size.w += padding.left + padding.right;
	size.h += padding.top + padding.bottom;
	return size;
}

static void wz_nanovg_draw_button(struct wzRenderer *renderer, wzRect clip, struct wzButton *button, wzBorder padding, const char *icon, const char *fontFace, float fontSize, const char *label)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	bool hover, pressed, set;
	wzRect paddedRect;
	wzSize iconSize;
	int iconHandle, labelWidth, iconX, labelX;

	WZ_ASSERT(renderer);
	WZ_ASSERT(button);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)button);

	if (!wz_nanovg_clip_to_rect_intersection(vg, clip, rect))
		return;

	hover = wz_widget_get_hover((struct wzWidget *)button);
	pressed = wz_button_is_pressed(button);
	set = wz_button_is_set(button); // For toggle buttons.

	// Background.
	if (set || (pressed && hover))
	{
		wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(127, 194, 229));
	}
	else if (hover)
	{
		wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(188, 229, 252));
	}
	else
	{
		wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(218, 218, 218));
	}

	// Border.
	if (pressed && hover)
	{
		wz_nanovg_draw_rect(vg, rect, nvgRGB(44, 98, 139));
	}
	else
	{
		wz_nanovg_draw_rect(vg, rect, nvgRGB(112, 112, 112));
	}

	// Calculate padded rect.
	paddedRect.x = rect.x + padding.left;
	paddedRect.y = rect.y + padding.top;
	paddedRect.w = rect.w - (padding.left + padding.right);
	paddedRect.h = rect.h - (padding.top + padding.bottom);

	// Calculate icon and label sizes.
	iconSize.w = iconSize.h = 0;

	if (icon && icon[0])
	{
		iconHandle = wz_nanovg_create_image(rendererData, icon);

		if (iconHandle)
			nvgImageSize(vg, iconHandle, &iconSize.w, &iconSize.h);
	}

	wz_nanovg_measure_text(renderer, fontFace, fontSize, label, 0, &labelWidth, NULL);

	// Position the icon and label centered.
	if (icon && icon[0] && iconHandle && label && label[0])
	{
		iconX = paddedRect.x + (int)(paddedRect.w / 2.0f - (iconSize.w + buttonIconSpacing + labelWidth) / 2.0f);
		labelX = iconX + iconSize.w + buttonIconSpacing;
	}
	else if (icon && icon[0] && iconHandle)
	{
		iconX = paddedRect.x + (int)(paddedRect.w / 2.0f - iconSize.w / 2.0f);
	}
	else if (label && label[0])
	{
		labelX = paddedRect.x + (int)(paddedRect.w / 2.0f - labelWidth / 2.0f);
	}

	// Draw the icon.
	if (icon && icon[0] && iconHandle)
	{
		wzRect iconRect;
		iconRect.x = iconX;
		iconRect.y = paddedRect.y + (int)(paddedRect.h / 2.0f - iconSize.h / 2.0f);
		iconRect.w = iconSize.w;
		iconRect.h = iconSize.h;
		wz_nanovg_draw_icon(vg, iconRect, iconHandle);
	}

	// Draw the label.
	if (label && label[0])
	{
		wz_nanovg_print(rendererData, labelX, paddedRect.y + paddedRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGB(0, 0, 0), label, 0);
	}

	nvgRestore(vg);
}

static wzSize wz_nanovg_measure_checkbox(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *label)
{
	wzSize size;

	WZ_ASSERT(renderer);
	wz_nanovg_measure_text(renderer, fontFace, fontSize, label, 0, &size.w, &size.h);
	size.w += checkBoxBoxSize + checkBoxBoxRightMargin;
	return size;
}

static void wz_nanovg_draw_checkbox(struct wzRenderer *renderer, wzRect clip, struct wzButton *checkbox, const char *fontFace, float fontSize, const char *label)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	bool hover;
	wzRect boxRect;

	WZ_ASSERT(renderer);
	WZ_ASSERT(checkbox);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_nanovg_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)checkbox);
	hover = wz_widget_get_hover((struct wzWidget *)checkbox);

	// Box.
	boxRect.x = rect.x;
	boxRect.y = (int)(rect.y + rect.h / 2.0f - checkBoxBoxSize / 2.0f);
	boxRect.w = checkBoxBoxSize;
	boxRect.h = checkBoxBoxSize;

	// Box background.
	if (wz_button_is_pressed(checkbox) && hover)
	{
		wz_nanovg_draw_filled_rect(vg, boxRect, nvgRGB(127, 194, 229));
	}
	else if (hover)
	{
		wz_nanovg_draw_filled_rect(vg, boxRect, nvgRGB(188, 229, 252));
	}

	// Box border.
	wz_nanovg_draw_rect(vg, boxRect, nvgRGB(0, 0, 0));

	// Box checkmark.
	if (wz_button_is_set(checkbox))
	{
		boxRect.x = rect.x + 4;
		boxRect.y = (int)(rect.y + rect.h / 2.0f - checkBoxBoxSize / 2.0f) + 4;
		boxRect.w = checkBoxBoxSize / 2;
		boxRect.h = checkBoxBoxSize / 2;
		wz_nanovg_draw_filled_rect(vg, boxRect, nvgRGB(0, 0, 0));
	}

	// Label.
	wz_nanovg_print(rendererData, rect.x + checkBoxBoxSize + checkBoxBoxRightMargin, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGB(0, 0, 0), label, 0);

	nvgRestore(vg);
}

static wzSize wz_nanovg_measure_combo(struct wzRenderer *renderer, const char *fontFace, float fontSize, uint8_t *itemData, size_t itemStride, int nItems)
{
	wzSize size;
	int i;

	WZ_ASSERT(renderer);
	WZ_ASSERT(itemData);

	// Use the widest item text.
	size.w = 0;

	for (i = 0; i < nItems; i++)
	{
		int w;
		wz_nanovg_measure_text(renderer, fontFace, fontSize, *((const char **)&itemData[i * itemStride]), 0, &w, NULL);
		size.w = WZ_MAX(size.w, w);
	}

	// Use line height.
	size.h = wz_nanovg_get_line_height(renderer, fontFace, fontSize);

	// Add vertical scroller width.
	size.w += renderer->measure_scroller(renderer, WZ_SCROLLER_VERTICAL).w;

	// Padding.
	size.w += 20;
	size.h += 4;
	return size;
}

static void wz_nanovg_draw_combo(struct wzRenderer *renderer, wzRect clip, struct wzCombo *combo, const char *fontFace, float fontSize, const char *item)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	bool hover;

	WZ_ASSERT(renderer);
	WZ_ASSERT(combo);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_nanovg_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)combo);
	hover = wz_widget_get_hover((struct wzWidget *)combo);

	// Background.
	if (hover)
	{
		wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(188, 229, 252));
	}
	else
	{
		wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(218, 218, 218));
	}

	// Border.
	if (hover)
	{
		wz_nanovg_draw_rect(vg, rect, nvgRGB(44, 98, 139));
	}
	else
	{
		wz_nanovg_draw_rect(vg, rect, nvgRGB(112, 112, 112));
	}

	// Selected item.
	if (item)
		wz_nanovg_print(rendererData, rect.x + 10, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGB(0, 0, 0), item, 0);

	nvgRestore(vg);
}

static wzBorder wz_nanovg_measure_group_box_margin(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *label)
{
	wzBorder margin;

	WZ_ASSERT(renderer);

	margin.top = (!label || !label[0]) ? 8 : wz_nanovg_get_line_height(renderer, fontFace, fontSize) + 8;
	margin.bottom = 8;
	margin.left = 8;
	margin.right = 8;

	return margin;
}

static void wz_nanovg_draw_group_box(struct wzRenderer *renderer, wzRect clip, struct wzFrame *frame, const char *fontFace, float fontSize, const char *label)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;

	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_nanovg_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)frame);
	
	// Background.
	wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(255, 255, 255));

	// Border.
	if (!label || !label[0])
	{
		wz_nanovg_draw_rect(vg, rect, nvgRGB(98, 135, 157));
	}
	else
	{
		int textLeftMargin, textBorderSpacing;
		int textWidth, textHeight;

		textLeftMargin = 20;
		textBorderSpacing = 5;
		wz_nanovg_measure_text(renderer, fontFace, fontSize, label, 0, &textWidth, &textHeight);

		// Left, right, bottom, top left, top right.
		wz_nanovg_draw_line(vg, rect.x, rect.y + textHeight / 2, rect.x, rect.y + rect.h, nvgRGB(98, 135, 157));
		wz_nanovg_draw_line(vg, rect.x + rect.w, rect.y + textHeight / 2, rect.x + rect.w, rect.y + rect.h, nvgRGB(98, 135, 157));
		wz_nanovg_draw_line(vg, rect.x, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h, nvgRGB(98, 135, 157));
		wz_nanovg_draw_line(vg, rect.x, rect.y + textHeight / 2, rect.x + textLeftMargin - textBorderSpacing, rect.y + textHeight / 2, nvgRGB(98, 135, 157));
		wz_nanovg_draw_line(vg, rect.x + textLeftMargin + textWidth + textBorderSpacing * 2, rect.y + textHeight / 2, rect.x + rect.w, rect.y + textHeight / 2, nvgRGB(98, 135, 157));

		// Label.
		wz_nanovg_print(rendererData, rect.x + textLeftMargin, rect.y, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, fontFace, fontSize, nvgRGB(0, 0, 0), label, 0);
	}

	nvgRestore(vg);
}

static wzSize wz_nanovg_measure_label(struct wzRenderer *renderer, const struct wzLabel *label, const char *fontFace, float fontSize, bool multiline, const char *text)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzSize size;

	WZ_ASSERT(renderer);
	WZ_ASSERT(label);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	if (multiline)
	{
		float bounds[4];

		nvgFontSize(vg, fontSize == 0 ? rendererData->defaultFontSize : fontSize);
		wz_nanovg_set_font_face(rendererData, fontFace);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgTextLineHeight(vg, 1.2f);
		nvgTextBoxBounds(vg, 0, 0, (float)wz_widget_get_width((const struct wzWidget *)label), text, NULL, bounds);
		size.w = (int)bounds[2];
		size.h = (int)bounds[3];
	}
	else
	{
		wz_nanovg_measure_text(renderer, fontFace, fontSize, text, 0, &size.w, &size.h);
	}

	return size;
}

static void wz_nanovg_draw_label(struct wzRenderer *renderer, wzRect clip, struct wzLabel *label, const char *fontFace, float fontSize, bool multiline, const char *text, uint8_t r, uint8_t g, uint8_t b)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;

	WZ_ASSERT(renderer);
	WZ_ASSERT(label);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;
	rect = wz_widget_get_absolute_rect((struct wzWidget *)label);

	nvgSave(vg);
	wz_nanovg_clip_to_rect(vg, clip);

	if (multiline)
	{
		wz_nanovg_print_box(rendererData, rect, fontFace, fontSize, nvgRGB(r, g, b), text, 0);
	}
	else
	{
		wz_nanovg_print(rendererData, rect.x, (int)(rect.y + rect.h * 0.5f), NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGB(r, g, b), text, 0);
	}

	nvgRestore(vg);
}

static wzBorder wz_nanovg_get_list_items_border(struct wzRenderer *renderer, struct wzList *list)
{
	wzBorder b;
	b.left = b.top = b.right = b.bottom = 2;
	return b;
}

static int wz_nanovg_measure_list_item_height(struct wzRenderer *renderer, struct wzList *list, const char *fontFace, float fontSize)
{
	WZ_ASSERT(renderer);
	return wz_nanovg_get_line_height(renderer, fontFace, fontSize) + 2; // Add a little padding.
}

static void wz_nanovg_draw_list(struct wzRenderer *renderer, wzRect clip, struct wzList *list, const char *fontFace, float fontSize, uint8_t *itemData, size_t itemStride)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect, itemsRect;
	int itemHeight, itemLeftPadding;
	int nItems, scrollerValue, y, i;

	WZ_ASSERT(renderer);
	WZ_ASSERT(list);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_nanovg_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)list);
	
	// Background.
	wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(255, 255, 255));

	// Border.
	wz_nanovg_draw_rect(vg, rect, nvgRGB(130, 135, 144));

	// Items.
	itemHeight = wz_list_get_item_height(list);
	itemLeftPadding = 4;
	itemsRect = wz_list_get_absolute_items_rect(list);

	if (!wz_nanovg_clip_to_rect_intersection(vg, clip, itemsRect))
		return;

	nItems = wz_list_get_num_items(list);
	scrollerValue = wz_scroller_get_value(wz_list_get_scroller(list));
	y = itemsRect.y - (scrollerValue % itemHeight);

	for (i = wz_list_get_first_item(list); i < nItems; i++)
	{
		wzRect itemRect;

		// Outside widget?
		if (y > itemsRect.y + itemsRect.h)
			break;

		itemRect.x = itemsRect.x;
		itemRect.y = y;
		itemRect.w = itemsRect.w;
		itemRect.h = itemHeight;

		if (i == wz_list_get_selected_item(list))
		{
			wz_nanovg_draw_filled_rect(vg, itemRect, nvgRGB(127, 194, 229));
		}
		else if (i == wz_list_get_pressed_item(list) || i == wz_list_get_hovered_item(list))
		{
			wz_nanovg_draw_filled_rect(vg, itemRect, nvgRGB(188, 229, 252));
		}

		wz_nanovg_print(rendererData, itemsRect.x + itemLeftPadding, y + itemHeight / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGB(0, 0, 0), *((const char **)&itemData[i * itemStride]), 0);
		y += itemHeight;
	}

	nvgRestore(vg);
}

static wzSize wz_nanovg_measure_radio_button(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *label)
{
	wzSize size;

	WZ_ASSERT(renderer);
	wz_nanovg_measure_text(renderer, fontFace, fontSize, label, 0, &size.w, &size.h);
	size.w += radioButtonOuterRadius * 2 + radioButtonSpacing;
	size.h = WZ_MAX(size.h, radioButtonOuterRadius);
	return size;
}

static void wz_nanovg_draw_radio_button(struct wzRenderer *renderer, wzRect clip, struct wzButton *button, const char *fontFace, float fontSize, const char *label)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	bool hover;

	WZ_ASSERT(renderer);
	WZ_ASSERT(button);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)button);

	if (!wz_nanovg_clip_to_rect_intersection(vg, clip, rect))
		return;

	hover = wz_widget_get_hover((struct wzWidget *)button);

	// Inner circle.
	if (wz_button_is_set(button))
	{
		nvgBeginPath(vg);
		nvgCircle(vg, (float)(rect.x + radioButtonOuterRadius), rect.y + rect.h / 2.0f, (float)radioButtonInnerRadius);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFill(vg);
	}

	// Outer circle.
	nvgBeginPath(vg);
	nvgCircle(vg, (float)(rect.x + radioButtonOuterRadius), rect.y + rect.h / 2.0f, (float)radioButtonOuterRadius);
	nvgStrokeColor(vg, hover ? nvgRGB(44, 98, 139) : nvgRGB(0, 0, 0));
	nvgStroke(vg);

	// Label.
	wz_nanovg_print(rendererData, rect.x + radioButtonOuterRadius * 2 + radioButtonSpacing, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGB(0, 0, 0), label, 0);

	nvgRestore(vg);
}

static wzSize wz_nanovg_measure_scroller(struct wzRenderer *renderer, wzScrollerType scrollerType)
{
	wzSize size;

	WZ_ASSERT(renderer);

	if (scrollerType == WZ_SCROLLER_VERTICAL)
	{
		size.w = 16;
		size.h = 0;
	}
	else
	{
		size.w = 0;
		size.h = 16;
	}

	return size;
}

static void wz_nanovg_draw_scroller(struct wzRenderer *renderer, wzRect clip, struct wzScroller *scroller)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect, nubRect;

	WZ_ASSERT(renderer);
	WZ_ASSERT(scroller);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_nanovg_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)scroller);
	
	// Background.
	wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(192, 192, 192));

	// Nub.
	nubRect = wz_scroller_get_nub_rect(scroller);
	wz_nanovg_draw_filled_rect(vg, nubRect, nvgRGB(218, 218, 218));
	wz_nanovg_draw_rect(vg, nubRect, nvgRGB(112, 112, 112));

	nvgRestore(vg);
}

static void wz_nanovg_draw_tab_button(struct wzRenderer *renderer, wzRect clip, struct wzButton *tabButton, wzBorder padding, const char *fontFace, float fontSize, const char *label)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	bool hover, set;
	wzRect labelRect;

	WZ_ASSERT(renderer);
	WZ_ASSERT(tabButton);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_nanovg_clip_to_rect(vg, clip);

	rect = wz_widget_get_absolute_rect((struct wzWidget *)tabButton);
	hover = wz_widget_get_hover((struct wzWidget *)tabButton);
	set = wz_button_is_set(tabButton);

	// Background.
	if (set)
	{
		wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(127, 194, 229));
	}
	else if (hover)
	{
		wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(188, 229, 252));
	}
	else
	{
		wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(218, 218, 218));
	}


	// Border.
	if (set)
	{
		wz_nanovg_draw_rect(vg, rect, nvgRGB(44, 98, 139));
	}
	else
	{
		wz_nanovg_draw_rect(vg, rect, nvgRGB(112, 112, 112));
	}

	// Label.
	labelRect.x = rect.x + padding.left;
	labelRect.y = rect.y + padding.top;
	labelRect.w = rect.w - (padding.left + padding.right);
	labelRect.h = rect.h - (padding.top + padding.bottom);
	wz_nanovg_print(rendererData, labelRect.x + labelRect.w / 2, labelRect.y + labelRect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGB(0, 0, 0), label, 0);

	nvgRestore(vg);
}

static void wz_nanovg_draw_tab_page(struct wzRenderer *renderer, wzRect clip, struct wzWidget *tabPage)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;

	WZ_ASSERT(renderer);
	WZ_ASSERT(tabPage);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_nanovg_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)tabPage);
	wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(224, 224, 224));
	wz_nanovg_draw_rect(vg, rect, nvgRGB(112, 112, 112));
	nvgRestore(vg);
}

static wzBorder wz_nanovg_get_text_edit_border(struct wzRenderer *renderer, const struct wzTextEdit *textEdit)
{
	wzBorder b;
	b.left = b.top = b.right = b.bottom = 4;
	return b;
}

static wzSize wz_nanovg_measure_text_edit(struct wzRenderer *renderer, const struct wzTextEdit *textEdit, const char *fontFace, float fontSize, const char *text)
{
	wzBorder border;
	wzSize size;

	WZ_ASSERT(renderer);
	WZ_ASSERT(textEdit);
	border = wz_text_edit_get_border(textEdit);

	if (wz_text_edit_is_multiline(textEdit))
	{
		size.w = 100;
		size.h = 100;
	}
	else
	{
		size.w = 100;
		size.h = wz_nanovg_get_line_height(renderer, fontFace, fontSize) + border.top + border.bottom;
	}

	return size;
}

static void wz_nanovg_draw_text_edit(struct wzRenderer *renderer, wzRect clip, const struct wzTextEdit *textEdit, const char *fontFace, float fontSize, bool showCursor)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	bool hover;
	wzBorder border;
	wzRect textRect;
	int lineHeight;

	WZ_ASSERT(renderer);
	WZ_ASSERT(textEdit);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_nanovg_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)textEdit);
	hover = wz_widget_get_hover((struct wzWidget *)textEdit);
	border = wz_text_edit_get_border(textEdit);

	// Background.
	wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(255, 255, 255));

	// Border.
	if (hover)
	{
		wz_nanovg_draw_rect(vg, rect, nvgRGB(44, 98, 139));
	}
	else
	{
		wz_nanovg_draw_rect(vg, rect, nvgRGB(0, 0, 0));
	}

	// Clip to the text rect.
	textRect = wz_text_edit_get_text_rect(textEdit);

	if (!wz_nanovg_clip_to_rect_intersection(vg, clip, textRect))
		return;

	// Get line height.
	lineHeight = wz_nanovg_get_line_height(renderer, fontFace, fontSize);

	// Text.
	if (wz_text_edit_is_multiline(textEdit))
	{
		wzLineBreakResult line;
		int selectionStartIndex, selectionEndIndex, lineY = 0;

		selectionStartIndex = wz_text_edit_get_selection_start_index(textEdit);
		selectionEndIndex = wz_text_edit_get_selection_end_index(textEdit);
		line.next = wz_text_edit_get_visible_text(textEdit);

		for (;;)
		{
			line = wz_nanovg_line_break_text(renderer, fontFace, fontSize, line.next, 0, textRect.w);

			if (line.length > 0)
			{
				// Draw this line.
				wz_nanovg_print(rendererData, textRect.x, textRect.y + lineY, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, fontFace, fontSize, nvgRGB(0, 0, 0), line.start, line.length);

				// Selection.
				if (wz_text_edit_has_selection(textEdit))
				{
					int lineStartIndex;
					bool startOnThisLine, endOnThisLine, straddleThisLine;
					wzPosition start, end;

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
						wzRect selectionRect;
						selectionRect.x = textRect.x + start.x;
						selectionRect.y = textRect.y + start.y - lineHeight / 2;
						selectionRect.w = end.x - start.x;
						selectionRect.h = lineHeight;
						wz_nanovg_draw_filled_rect(vg, selectionRect, nvgRGBA(0, 0, 255, 65));
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
		wz_nanovg_print(rendererData, textRect.x, textRect.y + textRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGB(0, 0, 0), wz_text_edit_get_visible_text(textEdit), 0);

		// Selection.
		if (wz_text_edit_has_selection(textEdit))
		{
			wzPosition position1, position2;
			wzRect selectionRect;

			position1 = wz_text_edit_get_selection_start_position(textEdit);
			position2 = wz_text_edit_get_selection_end_position(textEdit);
			selectionRect.x = textRect.x + position1.x;
			selectionRect.y = textRect.y + position1.y - lineHeight / 2;
			selectionRect.w = position2.x - position1.x;
			selectionRect.h = lineHeight;
			wz_nanovg_draw_filled_rect(vg, selectionRect, nvgRGBA(0, 0, 255, 65));
		}
	}

	// Cursor.
	if (showCursor && wz_widget_has_keyboard_focus((const struct wzWidget *)textEdit))
	{
		wzPosition position;
		
		position = wz_text_edit_get_cursor_position(textEdit);
		position.x += textRect.x;
		position.y += textRect.y;

		wz_nanovg_clip_to_rect(vg, clip); // Don't clip.
		nvgBeginPath(vg);
		nvgMoveTo(vg, (float)position.x, position.y - lineHeight / 2.0f);
		nvgLineTo(vg, (float)position.x, position.y + lineHeight / 2.0f);
		nvgStrokeColor(vg, nvgRGB(0, 0, 0));
		nvgStroke(vg);
	}

	nvgRestore(vg);
}

static int wz_nanovg_measure_window_header_height(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *title)
{
	WZ_ASSERT(renderer);
	return wz_nanovg_get_line_height(renderer, fontFace, fontSize) + 6; // Padding.
}

static void wz_nanovg_draw_window(struct wzRenderer *renderer, wzRect clip, struct wzWindow *window, const char *fontFace, float fontSize, const char *title)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	int borderSize;
	wzRect borderRect, headerRect;

	WZ_ASSERT(renderer);
	WZ_ASSERT(window);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)window);
	borderSize = wz_window_get_border_size(window);
	
	// Background.
	wz_nanovg_draw_filled_rect(vg, rect, nvgRGB(255, 255, 255));

	// Border top.
	borderRect = rect;
	borderRect.h = borderSize;
	wz_nanovg_draw_filled_rect(vg, borderRect, nvgRGB(128, 128, 128));

	// Border bottom.
	borderRect.y = rect.y + rect.h - borderSize;
	wz_nanovg_draw_filled_rect(vg, borderRect, nvgRGB(128, 128, 128));

	// Border left.
	borderRect = rect;
	borderRect.w = borderSize;
	wz_nanovg_draw_filled_rect(vg, borderRect, nvgRGB(128, 128, 128));

	// Border right.
	borderRect.x = rect.x + rect.w - borderSize;
	wz_nanovg_draw_filled_rect(vg, borderRect, nvgRGB(128, 128, 128));

	// Header.
	headerRect = wz_window_get_header_rect(window);

	if (headerRect.w > 0 && headerRect.h > 0)
	{
		wz_nanovg_clip_to_rect(vg, headerRect);
		wz_nanovg_draw_filled_rect(vg, headerRect, nvgRGB(255, 232, 166));
		wz_nanovg_print(rendererData, headerRect.x + 10, headerRect.y + headerRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGB(0, 0, 0), title, 0);
	}

	nvgRestore(vg);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

struct wzRenderer *wz_nanovg_create_renderer(const char *fontDirectory, const char *defaultFontFace, float defaultFontSize)
{
	struct wzRenderer *renderer;
	wzRendererData *rendererData;

	// Alloc renderer.
	renderer = malloc(sizeof(struct wzRenderer));
	rendererData = renderer->data = malloc(sizeof(wzRendererData));
	memset(rendererData, 0, sizeof(wzRendererData));

	// Init nanovg.
	rendererData->vg = nvgCreateGL2(0);

	if (!rendererData->vg)
	{
		strcpy(errorMessage, "Error initializing NanoVG");
		wz_nanovg_destroy_renderer(renderer);
		return NULL;
	}

	// Load the default font.
	strncpy(rendererData->fontDirectory, fontDirectory, WZ_NANOVG_MAX_PATH);
	rendererData->defaultFontSize = defaultFontSize;

	if (wz_nanovg_create_font(rendererData, defaultFontFace) == -1)
	{
		sprintf(errorMessage, "Error loading font %s", defaultFontFace);
		wz_nanovg_destroy_renderer(renderer);
		return NULL;
	}

	// Set renderer function pointers.
	renderer->begin_frame = wz_nanovg_begin_frame;
	renderer->end_frame = wz_nanovg_end_frame;
	renderer->get_line_height = wz_nanovg_get_line_height;
	renderer->measure_text = wz_nanovg_measure_text;
	renderer->line_break_text = wz_nanovg_line_break_text;
	renderer->debug_draw_text = wz_nanovg_debug_draw_text;
	renderer->draw_dock_icon = wz_nanovg_draw_dock_icon;
	renderer->draw_dock_preview = wz_nanovg_draw_dock_preview;
	renderer->measure_button = wz_nanovg_measure_button;
	renderer->draw_button = wz_nanovg_draw_button;
	renderer->measure_checkbox = wz_nanovg_measure_checkbox;
	renderer->draw_checkbox = wz_nanovg_draw_checkbox;
	renderer->measure_combo = wz_nanovg_measure_combo;
	renderer->draw_combo = wz_nanovg_draw_combo;
	renderer->measure_group_box_margin = wz_nanovg_measure_group_box_margin;
	renderer->draw_group_box = wz_nanovg_draw_group_box;
	renderer->measure_label = wz_nanovg_measure_label;
	renderer->draw_label = wz_nanovg_draw_label;
	renderer->get_list_items_border = wz_nanovg_get_list_items_border;
	renderer->measure_list_item_height = wz_nanovg_measure_list_item_height;
	renderer->draw_list = wz_nanovg_draw_list;
	renderer->measure_radio_button = wz_nanovg_measure_radio_button;
	renderer->draw_radio_button = wz_nanovg_draw_radio_button;
	renderer->measure_scroller = wz_nanovg_measure_scroller;
	renderer->draw_scroller = wz_nanovg_draw_scroller;
	renderer->draw_tab_button = wz_nanovg_draw_tab_button;
	renderer->draw_tab_page = wz_nanovg_draw_tab_page;
	renderer->get_text_edit_border = wz_nanovg_get_text_edit_border;
	renderer->measure_text_edit = wz_nanovg_measure_text_edit;
	renderer->draw_text_edit = wz_nanovg_draw_text_edit;
	renderer->measure_window_header_height = wz_nanovg_measure_window_header_height;
	renderer->draw_window = wz_nanovg_draw_window;

	return renderer;
}

const char *wz_nanovg_get_error()
{
	return errorMessage;
}

void wz_nanovg_destroy_renderer(struct wzRenderer *renderer)
{
	if (renderer)
	{
		wzRendererData *rendererData;

		rendererData = (wzRendererData *)renderer->data;

		if (rendererData->vg)
		{
			nvgDeleteGL2(rendererData->vg);
		}

		free(renderer->data);
		free(renderer);
	}
}
