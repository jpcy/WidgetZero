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
#include <wz.h>
#include "wz_renderer.h"

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
	wzNanoVgGlDestroy destroy;
	struct NVGcontext *vg;
	bool showTextCursor;
	wzImage images[WZ_NANOVG_MAX_IMAGES];
	int nImages;
	char fontDirectory[WZ_NANOVG_MAX_PATH];
	float defaultFontSize;
}
wzRendererData;

static char errorMessage[WZ_NANOVG_MAX_ERROR_MESSAGE];

const NVGcolor color_hover = { 0.2510f, 0.2510f, 0.3176f, 1 };
const NVGcolor color_set = { 0.1529f, 0.5569f, 0.7412f, 1 };
const NVGcolor color_pressed = { 0.1765f, 0.1765f, 0.2157f, 1 };
const NVGcolor color_border = { 0.4000f, 0.4000f, 0.4000f, 1 };
const NVGcolor color_borderHover = { 0.5f, 0.76f, 0.9f, 1 };
const NVGcolor color_borderSet = { 0.17f, 0.38f, 0.55f, 1 };
const NVGcolor color_background = { 0.2000f, 0.2000f, 0.2000f, 1 };
const NVGcolor color_foreground = { 0.2510f, 0.2510f, 0.2510f, 1 };
const NVGcolor color_text = { 0.8f, 0.8f, 0.8f, 1 };
const NVGcolor color_textSelection = { 0.1529f, 0.5569f, 0.7412f, 0.5f };
const NVGcolor color_textCursor = { 1, 1, 1, 1 };
const NVGcolor color_dockPreview = { 0, 0, 1, 0.25f };
const NVGcolor color_windowHeaderBackground = { 0.2039f, 0.2863f, 0.3686f, 1 };
const NVGcolor color_windowBorder = { 0.2784f, 0.4000f, 0.4902f, 1 };

/*
================================================================================

MISC. UTILITY FUNCTIONS

================================================================================
*/

int wz_renderer_create_image(struct wzRenderer *renderer, const char *filename, int *width, int *height)
{
	wzRendererData *rendererData;
	int handle, i;

	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;
	handle = -1;

	// Check cache.
	for (i = 0; i < rendererData->nImages; i++)
	{
		if (strcmp(rendererData->images[i].filename, filename) == 0)
		{
			handle = rendererData->images[i].handle;
			break;
		}
	}

	if (handle == -1)
	{
		// Not found, create and cache it.
		rendererData->images[rendererData->nImages].handle = nvgCreateImage(rendererData->vg, filename, 0);
		strcpy(rendererData->images[rendererData->nImages].filename, filename);
		handle = rendererData->images[rendererData->nImages].handle;
		rendererData->nImages++;
	}

	nvgImageSize(rendererData->vg, handle, width, height);
	return handle;
}

static int wz_nanovg_create_font(struct wzRenderer *renderer, const char *face)
{
	wzRendererData *rendererData;
	char fontPath[WZ_NANOVG_MAX_PATH];
	int id;

	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;

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

static void wz_nanovg_set_font_face(struct wzRenderer *renderer, const char *face)
{
	wzRendererData *rendererData;
	int id;
	
	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;
	id = wz_nanovg_create_font(renderer, face);

	// Use the first font if creating failed.
	if (id == -1)
		id = 0;

	nvgFontFaceId(rendererData->vg, id);
}

void wz_renderer_print_box(struct wzRenderer *renderer, wzRect rect, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength)
{
	wzRendererData *rendererData;

	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;
	nvgFontSize(rendererData->vg, fontSize == 0 ? rendererData->defaultFontSize : fontSize);
	wz_nanovg_set_font_face(renderer, fontFace);
	nvgTextAlign(rendererData->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	nvgTextLineHeight(rendererData->vg, 1.0f);
	nvgFontBlur(rendererData->vg, 0);
	nvgFillColor(rendererData->vg, color);
	nvgTextBox(rendererData->vg, (float)rect.x, (float)rect.y, (float)rect.w, text, textLength == 0 ? NULL : &text[textLength]);
}

void wz_renderer_print(struct wzRenderer *renderer, int x, int y, int align, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength)
{
	wzRendererData *rendererData;

	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;
	nvgFontSize(rendererData->vg, fontSize == 0 ? rendererData->defaultFontSize : fontSize);
	wz_nanovg_set_font_face(renderer, fontFace);
	nvgTextAlign(rendererData->vg, align);
	nvgFontBlur(rendererData->vg, 0);
	nvgFillColor(rendererData->vg, color);
	nvgText(rendererData->vg, (float)x, (float)y, text, textLength == 0 ? NULL : &text[textLength]);
}

void wz_renderer_clip_to_rect(struct NVGcontext *vg, wzRect rect)
{
	WZ_ASSERT(vg);
	nvgScissor(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
}

bool wz_renderer_clip_to_rect_intersection(struct NVGcontext *vg, wzRect rect1, wzRect rect2)
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

void wz_renderer_draw_filled_rect(struct NVGcontext *vg, wzRect rect, struct NVGcolor color)
{
	nvgBeginPath(vg);
	nvgRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgFillColor(vg, color);
	nvgFill(vg);
}

void wz_renderer_draw_rect(struct NVGcontext *vg, wzRect rect, struct NVGcolor color)
{
	nvgBeginPath(vg);
	nvgRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 0.5f, rect.h - 0.5f);
	nvgStrokeColor(vg, color);
	nvgStroke(vg);
}

void wz_renderer_draw_line(struct NVGcontext *vg, int x1, int y1, int x2, int y2, struct NVGcolor color)
{
	nvgBeginPath(vg);
	nvgMoveTo(vg, (float)x1, (float)y1);
	nvgLineTo(vg, (float)x2, (float)y2);
	nvgStrokeColor(vg, color);
	nvgStroke(vg);
}

void wz_renderer_draw_image(struct NVGcontext *vg, wzRect rect, int image)
{
	int w, h;
	struct NVGpaint paint;

	nvgImageSize(vg, image, &w, &h);
	paint = nvgImagePattern(vg, (float)rect.x, (float)rect.y, (float)w, (float)h, 0, image, NVG_NOREPEAT, 1);
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

void wz_renderer_begin_frame(struct wzRenderer *renderer, const struct wzMainWindow *mainWindow)
{
	wzSize windowSize;

	WZ_ASSERT(renderer);
	windowSize = wz_widget_get_size((const struct wzWidget *)mainWindow);
	nvgBeginFrame(((wzRendererData *)renderer->data)->vg, windowSize.w, windowSize.h, 1);
}

void wz_renderer_end_frame(struct wzRenderer *renderer)
{
	nvgEndFrame(((wzRendererData *)renderer->data)->vg);
}

void wz_renderer_toggle_text_cursor(struct wzRenderer *renderer)
{
	wzRendererData *rendererData;

	WZ_ASSERT(renderer);
	rendererData = (wzRendererData *)renderer->data;
	rendererData->showTextCursor = !rendererData->showTextCursor;
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
	wz_nanovg_set_font_face(renderer, fontFace);
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
		wz_nanovg_set_font_face(renderer, fontFace);
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

wzColor wz_nanovg_get_default_text_color(struct wzRenderer *renderer)
{
	wzColor color;
	color.r = color_text.r;
	color.g = color_text.g;
	color.b = color_text.b;
	color.a = color_text.a;
	return color;
}

static wzSize wz_nanovg_get_dock_icon_size(struct wzRenderer *renderer)
{
	wzSize size;
	size.w = size.h = 48;
	return size;
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
	wz_renderer_draw_filled_rect(vg, rect, color_dockPreview);
	nvgRestore(vg);
}

static wzSize wz_nanovg_measure_combo(struct wzRenderer *renderer, const struct wzCombo *combo)
{
	const char *fontFace;
	float fontSize;
	struct wzList *list;
	uint8_t *itemData;
	int itemStride, nItems;
	wzSize size;
	int i;

	WZ_ASSERT(renderer);
	WZ_ASSERT(combo);

	fontFace = wz_widget_get_font_face((const struct wzWidget *)combo);
	fontSize = wz_widget_get_font_size((const struct wzWidget *)combo);
	list = wz_combo_get_list(combo);
	itemData = wz_list_get_item_data(list);
	itemStride = wz_list_get_item_stride(list);
	nItems = wz_list_get_num_items(list);

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

static void wz_nanovg_draw_combo(struct wzRenderer *renderer, wzRect clip, const struct wzCombo *combo)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	bool hover;
	const char *fontFace;
	float fontSize;
	struct wzList *list;
	uint8_t *itemData;
	int itemStride, nItems, selectedItemIndex;

	WZ_ASSERT(renderer);
	WZ_ASSERT(combo);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)combo);
	hover = wz_widget_get_hover((const struct wzWidget *)combo);

	fontFace = wz_widget_get_font_face((const struct wzWidget *)combo);
	fontSize = wz_widget_get_font_size((const struct wzWidget *)combo);
	list = wz_combo_get_list(combo);
	itemData = wz_list_get_item_data(list);
	itemStride = wz_list_get_item_stride(list);
	nItems = wz_list_get_num_items(list);
	selectedItemIndex = wz_list_get_selected_item(list);

	// Background.
	if (hover)
	{
		wz_renderer_draw_filled_rect(vg, rect, color_hover);
	}
	else
	{
		wz_renderer_draw_filled_rect(vg, rect, color_foreground);
	}

	// Border.
	if (hover)
	{
		wz_renderer_draw_rect(vg, rect, color_borderSet);
	}
	else if (hover)
	{
		wz_renderer_draw_rect(vg, rect, color_borderHover);
	}
	else
	{
		wz_renderer_draw_rect(vg, rect, color_border);
	}

	// Selected item.
	if (selectedItemIndex >= 0)
		wz_renderer_print(renderer, rect.x + 10, rect.y + rect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, color_text, *((const char **)&itemData[selectedItemIndex * itemStride]), 0);

	nvgRestore(vg);
}

static wzBorder wz_nanovg_measure_group_box_margin(struct wzRenderer *renderer, const struct wzGroupBox *groupBox)
{
	wzBorder margin;
	const char *label;

	WZ_ASSERT(renderer);
	WZ_ASSERT(groupBox);
	label = wz_group_box_get_label(groupBox);
	margin.top = (!label || !label[0]) ? 8 : wz_nanovg_get_line_height(renderer, wz_widget_get_font_face((const struct wzWidget *)groupBox), wz_widget_get_font_size((const struct wzWidget *)groupBox)) + 8;
	margin.bottom = 8;
	margin.left = 8;
	margin.right = 8;

	return margin;
}

static void wz_nanovg_draw_group_box(struct wzRenderer *renderer, wzRect clip, const struct wzGroupBox *groupBox)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	const char *label;

	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)groupBox);
	
	// Background.
	wz_renderer_draw_filled_rect(vg, rect, color_background);

	// Border.
	label = wz_group_box_get_label(groupBox);

	if (!label || !label[0])
	{
		wz_renderer_draw_rect(vg, rect, color_border);
	}
	else
	{
		const char *fontFace;
		float fontSize;
		int textLeftMargin, textBorderSpacing;
		int textWidth, textHeight;

		fontFace = wz_widget_get_font_face((const struct wzWidget *)groupBox);
		fontSize = wz_widget_get_font_size((const struct wzWidget *)groupBox);
		textLeftMargin = 20;
		textBorderSpacing = 5;
		wz_nanovg_measure_text(renderer, fontFace, fontSize, label, 0, &textWidth, &textHeight);

		// Left, right, bottom, top left, top right.
		wz_renderer_draw_line(vg, rect.x, rect.y + textHeight / 2, rect.x, rect.y + rect.h, color_border);
		wz_renderer_draw_line(vg, rect.x + rect.w, rect.y + textHeight / 2, rect.x + rect.w, rect.y + rect.h, color_border);
		wz_renderer_draw_line(vg, rect.x, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h, color_border);
		wz_renderer_draw_line(vg, rect.x, rect.y + textHeight / 2, rect.x + textLeftMargin - textBorderSpacing, rect.y + textHeight / 2, color_border);
		wz_renderer_draw_line(vg, rect.x + textLeftMargin + textWidth + textBorderSpacing * 2, rect.y + textHeight / 2, rect.x + rect.w, rect.y + textHeight / 2, color_border);

		// Label.
		wz_renderer_print(renderer, rect.x + textLeftMargin, rect.y, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, fontFace, fontSize, color_text, label, 0);
	}

	nvgRestore(vg);
}

static wzSize wz_nanovg_measure_label(struct wzRenderer *renderer, const struct wzLabel *label)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	const char *fontFace;
	float fontSize;
	const char *text;
	wzSize size;

	WZ_ASSERT(renderer);
	WZ_ASSERT(label);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;
	fontFace = wz_widget_get_font_face((const struct wzWidget *)label);
	fontSize = wz_widget_get_font_size((const struct wzWidget *)label);
	text = wz_label_get_text(label);

	if (wz_label_get_multiline(label))
	{
		float bounds[4];

		nvgFontSize(vg, fontSize == 0 ? rendererData->defaultFontSize : fontSize);
		wz_nanovg_set_font_face(renderer, fontFace);
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

static void wz_nanovg_draw_label(struct wzRenderer *renderer, wzRect clip, const struct wzLabel *label)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	const char *fontFace;
	float fontSize;
	const char *text;
	wzColor color;

	WZ_ASSERT(renderer);
	WZ_ASSERT(label);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)label);
	fontFace = wz_widget_get_font_face((const struct wzWidget *)label);
	fontSize = wz_widget_get_font_size((const struct wzWidget *)label);
	text = wz_label_get_text(label);
	color = wz_label_get_text_color(label);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);

	if (wz_label_get_multiline(label))
	{
		wz_renderer_print_box(renderer, rect, fontFace, fontSize, nvgRGBAf(color.r, color.g, color.b, color.a), text, 0);
	}
	else
	{
		wz_renderer_print(renderer, rect.x, (int)(rect.y + rect.h * 0.5f), NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, nvgRGBAf(color.r, color.g, color.b, color.a), text, 0);
	}

	nvgRestore(vg);
}

static wzBorder wz_nanovg_get_list_items_border(struct wzRenderer *renderer, const struct wzList *list)
{
	wzBorder b;
	b.left = b.top = b.right = b.bottom = 2;
	return b;
}

static int wz_nanovg_measure_list_item_height(struct wzRenderer *renderer, const struct wzList *list)
{
	WZ_ASSERT(renderer);
	WZ_ASSERT(list);

	// Add a little padding.
	return wz_nanovg_get_line_height(renderer, wz_widget_get_font_face((const struct wzWidget *)list), wz_widget_get_font_size((const struct wzWidget *)list)) + 2;
}

static void wz_nanovg_draw_list(struct wzRenderer *renderer, wzRect clip, const struct wzList *list)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect, itemsRect;
	const char *fontFace;
	float fontSize;
	int itemHeight, itemLeftPadding;
	uint8_t *data;
	int itemStride, nItems, scrollerValue, y, i;
	wzDrawListItemCallback draw_item;

	WZ_ASSERT(renderer);
	WZ_ASSERT(list);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)list);
	fontFace = wz_widget_get_font_face((const struct wzWidget *)list);
	fontSize = wz_widget_get_font_size((const struct wzWidget *)list);
	
	// Background.
	wz_renderer_draw_filled_rect(vg, rect, color_background);

	// Border.
	wz_renderer_draw_rect(vg, rect, color_border);

	// Items.
	itemHeight = wz_list_get_item_height(list);
	itemLeftPadding = 4;
	itemsRect = wz_list_get_absolute_items_rect(list);

	if (!wz_renderer_clip_to_rect_intersection(vg, clip, itemsRect))
		return;

	data = wz_list_get_item_data(list);
	itemStride = wz_list_get_item_stride(list);
	nItems = wz_list_get_num_items(list);
	scrollerValue = wz_list_get_scroll_value(list);
	y = itemsRect.y - (scrollerValue % itemHeight);
	draw_item = wz_list_get_draw_item_callback(list);

	for (i = wz_list_get_first_item(list); i < nItems; i++)
	{
		wzRect itemRect;
		const uint8_t *itemData;

		// Outside widget?
		if (y > itemsRect.y + itemsRect.h)
			break;

		itemRect.x = itemsRect.x;
		itemRect.y = y;
		itemRect.w = itemsRect.w;
		itemRect.h = itemHeight;
		itemData = *((uint8_t **)&data[i * itemStride]);

		if (i == wz_list_get_selected_item(list))
		{
			wz_renderer_draw_filled_rect(vg, itemRect, color_set);
		}
		else if (i == wz_list_get_pressed_item(list) || i == wz_list_get_hovered_item(list))
		{
			wz_renderer_draw_filled_rect(vg, itemRect, color_hover);
		}

		if (draw_item)
		{
			draw_item(renderer, itemRect, list, fontFace, fontSize, i, itemData);
		}
		else
		{
			wz_renderer_print(renderer, itemsRect.x + itemLeftPadding, y + itemHeight / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, color_text, (const char *)itemData, 0);
		}

		y += itemHeight;
	}

	nvgRestore(vg);
}

static void wz_nanovg_draw_main_window(struct wzRenderer *renderer, const struct wzMainWindow *mainWindow)
{
	WZ_ASSERT(renderer);
	WZ_ASSERT(mainWindow);
	wz_renderer_draw_filled_rect(((wzRendererData *)renderer->data)->vg, wz_widget_get_absolute_rect((const struct wzWidget *)mainWindow), color_background);
}

static int wz_nanovg_calculate_menu_bar_height(struct wzRenderer *renderer, const struct wzMenuBar *menuBar)
{
	return wz_nanovg_get_line_height(renderer, wz_widget_get_font_face((const struct wzWidget *)menuBar), wz_widget_get_font_size((const struct wzWidget *)menuBar)) + 6;
}

static void wz_nanovg_draw_menu_bar(struct wzRenderer *renderer, wzRect clip, const struct wzMenuBar *menuBar)
{
	struct NVGcontext *vg;
	wzRect rect;

	WZ_ASSERT(renderer);
	WZ_ASSERT(menuBar);
	vg = ((wzRendererData *)renderer->data)->vg;
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)menuBar);

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	wz_renderer_draw_filled_rect(vg, rect, color_foreground);
	nvgRestore(vg);
}

static wzSize wz_nanovg_measure_menu_bar_button(struct wzRenderer *renderer, const struct wzMenuBarButton *button)
{
	wzSize size;

	WZ_ASSERT(renderer);
	WZ_ASSERT(button);
	wz_nanovg_measure_text(renderer, wz_widget_get_font_face((const struct wzWidget *)button), wz_widget_get_font_size((const struct wzWidget *)button), wz_menu_bar_button_get_label(button), 0, &size.w, &size.h);
	size.w += 12;
	return size;
}

static void wz_nanovg_draw_menu_bar_button(struct wzRenderer *renderer, wzRect clip, const struct wzMenuBarButton *button)
{
	struct NVGcontext *vg;
	wzRect rect;

	WZ_ASSERT(renderer);
	WZ_ASSERT(button);
	vg = ((wzRendererData *)renderer->data)->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)button);

	if (wz_menu_bar_button_is_pressed(button))
	{
		wz_renderer_draw_filled_rect(vg, rect, color_set);
	}

	if (wz_widget_get_hover((const struct wzWidget *)button))
	{
		wz_renderer_draw_rect(vg, rect, color_borderHover);
	}

	wz_renderer_print(renderer, rect.x + rect.w / 2, rect.y + rect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, wz_widget_get_font_face((const struct wzWidget *)button), wz_widget_get_font_size((const struct wzWidget *)button), color_text, wz_menu_bar_button_get_label(button), 0);

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

static void wz_nanovg_draw_scroller(struct wzRenderer *renderer, wzRect clip, const struct wzScroller *scroller)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	wzScrollerType type;

	WZ_ASSERT(renderer);
	WZ_ASSERT(scroller);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)scroller);
	type = wz_scroller_get_type(scroller);
	
	wz_renderer_draw_filled_rect(vg, rect, color_foreground);

	// Nub.
	{
		wzRect r;
		bool hover, pressed;
		NVGcolor color, borderColor;

		wz_scroller_get_nub_state(scroller, &r, &hover, &pressed);

		if (pressed)
		{
			color = color_pressed;
			borderColor = color_borderSet;
		}
		else if (hover)
		{
			color = color_hover;
			borderColor = color_borderHover;
		}
		else
		{
			color = color_foreground;
			borderColor = color_border;
		}

		wz_renderer_draw_filled_rect(vg, r, color);
		wz_renderer_draw_rect(vg, r, borderColor);
	}

	nvgRestore(vg);
}

static void wz_nanovg_draw_scroller_button(struct wzRenderer *renderer, wzRect clip, const struct wzScroller *scroller, const struct wzButton *button, bool decrement)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect r;
	wzScrollerType type;
	bool hover, pressed;
	NVGcolor color, borderColor;

	WZ_ASSERT(renderer);
	WZ_ASSERT(scroller);
	WZ_ASSERT(button);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	r = wz_widget_get_absolute_rect((const struct wzWidget *)button);
	type = wz_scroller_get_type(scroller);
	hover = wz_widget_get_hover((const struct wzWidget *)button);
	pressed = wz_button_is_pressed(button);

	if (pressed && hover)
	{
		color = color_pressed;
		borderColor = color_borderSet;
	}
	else if (hover)
	{
		color = color_hover;
		borderColor = color_borderHover;
	}
	else
	{
		color = color_foreground;
		borderColor = color_border;
	}

	wz_renderer_draw_filled_rect(vg, r, color);
	wz_renderer_draw_rect(vg, r, borderColor);
	nvgBeginPath(vg);

	if (type == WZ_SCROLLER_VERTICAL)
	{
		if (decrement)
		{
			nvgMoveTo(vg, r.x + r.w * 0.5f, r.y + r.h * 0.25f); // top
			nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.75f); // left
			nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.75f); // right
		}
		else
		{
			nvgMoveTo(vg, r.x + r.w * 0.5f, r.y + r.h * 0.75f); // bottom
			nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.25f); // right
			nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.25f); // left
		}
	}
	else
	{
		if (decrement)
		{
			nvgMoveTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.5f); // left
			nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.75f); // bottom
			nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.25f); // top
		}
		else
		{
			nvgMoveTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.5f); // right
			nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.25f); // top
			nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.75f); // bottom
		}
	}

	nvgFillColor(vg, borderColor);
	nvgFill(vg);
	nvgRestore(vg);
}

static int wz_nanovg_get_spinner_button_width(struct wzRenderer *renderer)
{
	return 16;
}

static wzSize wz_nanovg_measure_spinner(struct wzRenderer *renderer, const struct wzSpinner *spinner, const struct wzTextEdit *textEdit)
{
	wzBorder border;
	const char *fontFace;
	float fontSize;
	wzSize size;

	WZ_ASSERT(renderer);
	WZ_ASSERT(spinner);
	WZ_ASSERT(textEdit);
	border = wz_text_edit_get_border(textEdit);
	fontFace = wz_widget_get_font_face((const struct wzWidget *)spinner);
	fontSize = wz_widget_get_font_size((const struct wzWidget *)spinner);
	size.w = 100;
	size.h = wz_nanovg_get_line_height(renderer, fontFace, fontSize) + border.top + border.bottom;
	return size;
}

static void wz_nanovg_draw_spinner(struct wzRenderer *renderer, wzRect clip, const struct wzSpinner *spinner)
{
}

static void wz_nanovg_draw_spinner_button(struct wzRenderer *renderer, wzRect clip, const struct wzSpinner *spinner, const struct wzButton *button, bool decrement)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect r;
	bool hover, pressed;
	NVGcolor color, borderColor;

	WZ_ASSERT(renderer);
	WZ_ASSERT(spinner);
	WZ_ASSERT(button);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	r = wz_widget_get_absolute_rect((const struct wzWidget *)button);
	hover = wz_widget_get_hover((const struct wzWidget *)button);
	pressed = wz_button_is_pressed(button);

	if (pressed && hover)
	{
		color = color_pressed;
		borderColor = color_borderSet;
	}
	else if (hover)
	{
		color = color_hover;
		borderColor = color_borderHover;
	}
	else
	{
		color = color_foreground;
		borderColor = color_border;
	}

	wz_renderer_draw_filled_rect(vg, r, color);
	wz_renderer_draw_rect(vg, r, borderColor);
	nvgBeginPath(vg);

	if (decrement)
	{
		nvgMoveTo(vg, r.x + r.w * 0.5f, r.y + r.h * 0.25f); // top
		nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.75f); // left
		nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.75f); // right
	}
	else
	{
		nvgMoveTo(vg, r.x + r.w * 0.5f, r.y + r.h * 0.75f); // bottom
		nvgLineTo(vg, r.x + r.w * 0.75f, r.y + r.h * 0.25f); // right
		nvgLineTo(vg, r.x + r.w * 0.25f, r.y + r.h * 0.25f); // left
	}

	nvgFillColor(vg, borderColor);
	nvgFill(vg);
	nvgRestore(vg);
}

int wz_nanovg_get_tab_bar_height(struct wzRenderer *renderer, const struct wzTabBar *tabBar)
{
	return 20;
}

static void wz_nanovg_draw_tab_page(struct wzRenderer *renderer, wzRect clip, const struct wzWidget *tabPage)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;

	WZ_ASSERT(renderer);
	WZ_ASSERT(tabPage);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)tabPage);
	wz_renderer_draw_filled_rect(vg, rect, color_foreground);
	wz_renderer_draw_rect(vg, rect, color_border);
	nvgRestore(vg);
}

static wzBorder wz_nanovg_get_text_edit_border(struct wzRenderer *renderer, const struct wzTextEdit *textEdit)
{
	wzBorder b;
	b.left = b.top = b.right = b.bottom = 4;
	return b;
}

static wzSize wz_nanovg_measure_text_edit(struct wzRenderer *renderer, const struct wzTextEdit *textEdit)
{
	wzBorder border;
	const char *fontFace;
	float fontSize;
	wzSize size;

	WZ_ASSERT(renderer);
	WZ_ASSERT(textEdit);
	border = wz_text_edit_get_border(textEdit);
	fontFace = wz_widget_get_font_face((const struct wzWidget *)textEdit);
	fontSize = wz_widget_get_font_size((const struct wzWidget *)textEdit);

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

static void wz_nanovg_draw_text_edit(struct wzRenderer *renderer, wzRect clip, const struct wzTextEdit *textEdit)
{
	wzRendererData *rendererData;
	struct NVGcontext *vg;
	wzRect rect;
	bool hover;
	const char *fontFace;
	float fontSize;
	wzRect textRect;
	int lineHeight;

	WZ_ASSERT(renderer);
	WZ_ASSERT(textEdit);
	rendererData = (wzRendererData *)renderer->data;
	vg = rendererData->vg;

	nvgSave(vg);
	wz_renderer_clip_to_rect(vg, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)textEdit);
	hover = wz_widget_get_hover((struct wzWidget *)textEdit);
	fontFace = wz_widget_get_font_face((const struct wzWidget *)textEdit);
	fontSize = wz_widget_get_font_size((const struct wzWidget *)textEdit);

	// Background.
	wz_renderer_draw_filled_rect(vg, rect, color_background);

	// Border.
	if (hover)
	{
		wz_renderer_draw_rect(vg, rect, color_borderHover);
	}
	else
	{
		wz_renderer_draw_rect(vg, rect, color_border);
	}

	// Clip to the text rect.
	textRect = wz_text_edit_get_text_rect(textEdit);

	if (!wz_renderer_clip_to_rect_intersection(vg, clip, textRect))
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
				wz_renderer_print(renderer, textRect.x, textRect.y + lineY, NVG_ALIGN_LEFT | NVG_ALIGN_TOP, fontFace, fontSize, color_text, line.start, line.length);

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
						wz_renderer_draw_filled_rect(vg, selectionRect, color_textSelection);
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
		wz_renderer_print(renderer, textRect.x, textRect.y + textRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, fontFace, fontSize, color_text, wz_text_edit_get_visible_text(textEdit), 0);

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
			wz_renderer_draw_filled_rect(vg, selectionRect, color_textSelection);
		}
	}

	// Cursor.
	if (rendererData->showTextCursor && wz_widget_has_keyboard_focus((const struct wzWidget *)textEdit))
	{
		wzPosition position;
		
		position = wz_text_edit_get_cursor_position(textEdit);
		position.x += textRect.x;
		position.y += textRect.y;

		wz_renderer_clip_to_rect(vg, rect);
		nvgBeginPath(vg);
		nvgMoveTo(vg, (float)position.x, position.y - lineHeight / 2.0f);
		nvgLineTo(vg, (float)position.x, position.y + lineHeight / 2.0f);
		nvgStrokeColor(vg, color_textCursor);
		nvgStroke(vg);
	}

	nvgRestore(vg);
}

static int wz_nanovg_get_window_border_size(struct wzRenderer *renderer, const struct wzWindow *window)
{
	WZ_ASSERT(renderer);
	WZ_ASSERT(window);
	return 4;
}

static int wz_nanovg_measure_window_header_height(struct wzRenderer *renderer, const struct wzWindow *window)
{
	WZ_ASSERT(renderer);
	WZ_ASSERT(window);
	return wz_nanovg_get_line_height(renderer, wz_widget_get_font_face((const struct wzWidget *)window), wz_widget_get_font_size((const struct wzWidget *)window)) + 6; // Padding.
}

static void wz_nanovg_draw_window(struct wzRenderer *renderer, wzRect clip, const struct wzWindow *window)
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
	rect = wz_widget_get_absolute_rect((const struct wzWidget *)window);
	borderSize = wz_window_get_border_size(window);
	
	// Background.
	wz_renderer_draw_filled_rect(vg, rect, color_background);

	// Border top.
	borderRect = rect;
	borderRect.h = borderSize;
	wz_renderer_draw_filled_rect(vg, borderRect, color_windowBorder);

	// Border bottom.
	borderRect.y = rect.y + rect.h - borderSize;
	wz_renderer_draw_filled_rect(vg, borderRect, color_windowBorder);

	// Border left.
	borderRect = rect;
	borderRect.w = borderSize;
	wz_renderer_draw_filled_rect(vg, borderRect, color_windowBorder);

	// Border right.
	borderRect.x = rect.x + rect.w - borderSize;
	wz_renderer_draw_filled_rect(vg, borderRect, color_windowBorder);

	// Header.
	headerRect = wz_window_get_header_rect(window);

	if (headerRect.w > 0 && headerRect.h > 0)
	{
		wz_renderer_clip_to_rect(vg, headerRect);
		wz_renderer_draw_filled_rect(vg, headerRect, color_windowHeaderBackground);
		wz_renderer_print(renderer, headerRect.x + 10, headerRect.y + headerRect.h / 2, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, wz_widget_get_font_face((const struct wzWidget *)window), wz_widget_get_font_size((const struct wzWidget *)window), color_text, wz_window_get_title(window), 0);
	}

	nvgRestore(vg);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

struct wzRenderer *wz_renderer_create(wzNanoVgGlCreate create, wzNanoVgGlDestroy destroy, const char *fontDirectory, const char *defaultFontFace, float defaultFontSize)
{
	struct wzRenderer *renderer;
	wzRendererData *rendererData;

	WZ_ASSERT(create);
	WZ_ASSERT(destroy);

	// Alloc renderer.
	renderer = malloc(sizeof(struct wzRenderer));
	rendererData = renderer->data = malloc(sizeof(wzRendererData));
	memset(rendererData, 0, sizeof(wzRendererData));
	rendererData->destroy = destroy;
	rendererData->showTextCursor = true;

	// Init nanovg.
	renderer->vg = create(0);
	rendererData->vg = renderer->vg;

	if (!rendererData->vg)
	{
		strcpy(errorMessage, "Error initializing NanoVG");
		wz_renderer_destroy(renderer);
		return NULL;
	}

	// Load the default font.
	strncpy(rendererData->fontDirectory, fontDirectory, WZ_NANOVG_MAX_PATH);
	rendererData->defaultFontSize = defaultFontSize;

	if (wz_nanovg_create_font(renderer, defaultFontFace) == -1)
	{
		sprintf(errorMessage, "Error loading font %s", defaultFontFace);
		wz_renderer_destroy(renderer);
		return NULL;
	}

	// Set renderer function pointers.
	renderer->get_line_height = wz_nanovg_get_line_height;
	renderer->measure_text = wz_nanovg_measure_text;
	renderer->line_break_text = wz_nanovg_line_break_text;
	renderer->get_default_text_color = wz_nanovg_get_default_text_color;
	renderer->get_dock_icon_size = wz_nanovg_get_dock_icon_size;
	renderer->draw_dock_icon = wz_nanovg_draw_dock_icon;
	renderer->draw_dock_preview = wz_nanovg_draw_dock_preview;
	renderer->measure_combo = wz_nanovg_measure_combo;
	renderer->draw_combo = wz_nanovg_draw_combo;
	renderer->measure_group_box_margin = wz_nanovg_measure_group_box_margin;
	renderer->draw_group_box = wz_nanovg_draw_group_box;
	renderer->measure_label = wz_nanovg_measure_label;
	renderer->draw_label = wz_nanovg_draw_label;
	renderer->get_list_items_border = wz_nanovg_get_list_items_border;
	renderer->measure_list_item_height = wz_nanovg_measure_list_item_height;
	renderer->draw_list = wz_nanovg_draw_list;
	renderer->draw_main_window = wz_nanovg_draw_main_window;
	renderer->calculate_menu_bar_height = wz_nanovg_calculate_menu_bar_height;
	renderer->draw_menu_bar = wz_nanovg_draw_menu_bar;
	renderer->measure_menu_bar_button = wz_nanovg_measure_menu_bar_button;
	renderer->draw_menu_bar_button = wz_nanovg_draw_menu_bar_button;
	renderer->measure_scroller = wz_nanovg_measure_scroller;
	renderer->draw_scroller = wz_nanovg_draw_scroller;
	renderer->draw_scroller_button = wz_nanovg_draw_scroller_button;
	renderer->get_spinner_button_width = wz_nanovg_get_spinner_button_width;
	renderer->measure_spinner = wz_nanovg_measure_spinner;
	renderer->draw_spinner = wz_nanovg_draw_spinner;
	renderer->draw_spinner_button = wz_nanovg_draw_spinner_button;
	renderer->get_tab_bar_height = wz_nanovg_get_tab_bar_height;
	renderer->draw_tab_page = wz_nanovg_draw_tab_page;
	renderer->get_text_edit_border = wz_nanovg_get_text_edit_border;
	renderer->measure_text_edit = wz_nanovg_measure_text_edit;
	renderer->draw_text_edit = wz_nanovg_draw_text_edit;
	renderer->get_window_border_size = wz_nanovg_get_window_border_size;
	renderer->measure_window_header_height = wz_nanovg_measure_window_header_height;
	renderer->draw_window = wz_nanovg_draw_window;

	return renderer;
}

void wz_renderer_destroy(struct wzRenderer *renderer)
{
	if (renderer)
	{
		wzRendererData *rendererData;

		rendererData = (wzRendererData *)renderer->data;

		if (rendererData->vg)
		{
			rendererData->destroy(rendererData->vg);
		}

		free(renderer->data);
		free(renderer);
	}
}

const char *wz_renderer_get_error()
{
	return errorMessage;
}

struct NVGcontext *wz_renderer_get_context(struct wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	return ((wzRendererData *)renderer->data)->vg;
}
