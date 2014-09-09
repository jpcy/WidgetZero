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

static char errorMessage[WZ_NANOVG_MAX_ERROR_MESSAGE];

static int wz_nanovg_create_font(struct wzRenderer *renderer, const char *face)
{
	char fontPath[WZ_NANOVG_MAX_PATH];
	int id;

	WZ_ASSERT(renderer);

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

struct wzRenderer *wz_renderer_create(wzNanoVgGlCreate create, wzNanoVgGlDestroy destroy, int flags, const char *fontDirectory, const char *defaultFontFace, float defaultFontSize)
{
	struct wzRenderer *renderer;

	WZ_ASSERT(create);
	WZ_ASSERT(destroy);

	// Alloc renderer.
	renderer = malloc(sizeof(struct wzRenderer));
	memset(renderer, 0, sizeof(struct wzRenderer));
	renderer->destroy = destroy;
	renderer->showTextCursor = true;
	renderer->defaultFontSize = defaultFontSize;

	// Init nanovg.
	renderer->vg = create(flags);

	if (!renderer->vg)
	{
		strcpy(errorMessage, "Error initializing NanoVG");
		wz_renderer_destroy(renderer);
		return NULL;
	}

	// Load the default font.
	strncpy(renderer->fontDirectory, fontDirectory, WZ_NANOVG_MAX_PATH);

	if (wz_nanovg_create_font(renderer, defaultFontFace) == -1)
	{
		sprintf(errorMessage, "Error loading font %s", defaultFontFace);
		wz_renderer_destroy(renderer);
		return NULL;
	}

	// Setup default style.
	renderer->style.hoverColor = nvgRGBf(0.2510f, 0.2510f, 0.3176f);
	renderer->style.setColor = nvgRGBf(0.1529f, 0.5569f, 0.7412f);
	renderer->style.pressedColor = nvgRGBf(0.1765f, 0.1765f, 0.2157f);
	renderer->style.borderColor = nvgRGBf(0.4000f, 0.4000f, 0.4000f);
	renderer->style.borderHoverColor = nvgRGBf(0.5f, 0.76f, 0.9f);
	renderer->style.borderSetColor = nvgRGBf(0.17f, 0.38f, 0.55f);
	renderer->style.backgroundColor = nvgRGBf(0.2000f, 0.2000f, 0.2000f);
	renderer->style.foregroundColor = nvgRGBf(0.2510f, 0.2510f, 0.2510f);
	renderer->style.textColor = nvgRGBf(0.8f, 0.8f, 0.8f);
	renderer->style.textSelectionColor = nvgRGBAf(0.1529f, 0.5569f, 0.7412f, 0.5f);
	renderer->style.textCursorColor = nvgRGBf(1, 1, 1);
	renderer->style.dockPreviewColor = nvgRGBAf(0, 0, 1, 0.25f);
	renderer->style.windowHeaderBackgroundColor = nvgRGBf(0.2039f, 0.2863f, 0.3686f);
	renderer->style.windowBorderColor = nvgRGBf(0.2784f, 0.4000f, 0.4902f);
	renderer->style.checkBoxBoxSize = 16;
	renderer->style.checkBoxBoxRightMargin = 8;
	renderer->style.groupBoxMargin = 8;
	renderer->style.groupBoxTextLeftMargin = 20;
	renderer->style.groupBoxTextBorderSpacing = 5;
	renderer->style.listItemLeftPadding = 4;
	renderer->style.menuBarPadding = 6;
	renderer->style.radioButtonOuterRadius = 8;
	renderer->style.radioButtonInnerRadius = 4;
	renderer->style.radioButtonSpacing = 8;

	return renderer;
}

void wz_renderer_destroy(struct wzRenderer *renderer)
{
	if (renderer)
	{
		if (renderer->vg)
		{
			renderer->destroy(renderer->vg);
		}

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
	return renderer->vg;
}

void wz_renderer_set_style(struct wzRenderer *renderer, wzRendererStyle style)
{
	WZ_ASSERT(renderer);
	renderer->style = style;
}

wzRendererStyle wz_renderer_get_style(const struct wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	return renderer->style;
}

void wz_renderer_begin_frame(struct wzRenderer *renderer, const struct wzMainWindow *mainWindow)
{
	wzSize windowSize;

	WZ_ASSERT(renderer);
	windowSize = wz_widget_get_size((const struct wzWidget *)mainWindow);
	nvgBeginFrame(renderer->vg, windowSize.w, windowSize.h, 1);
}

void wz_renderer_end_frame(struct wzRenderer *renderer)
{
	nvgEndFrame(renderer->vg);
}

void wz_renderer_toggle_text_cursor(struct wzRenderer *renderer)
{
	WZ_ASSERT(renderer);
	renderer->showTextCursor = !renderer->showTextCursor;
}

int wz_renderer_get_line_height(struct wzRenderer *renderer, const char *fontFace, float fontSize)
{
	float lineHeight;

	WZ_ASSERT(renderer);
	nvgFontSize(renderer->vg, fontSize == 0 ? renderer->defaultFontSize : fontSize);
	wz_renderer_set_font_face(renderer, fontFace);
	nvgTextMetrics(renderer->vg, NULL, NULL, &lineHeight);
	return (int)lineHeight;
}

void wz_renderer_measure_text(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int *width, int *height)
{
	WZ_ASSERT(renderer);
	if (width)
	{
		nvgFontSize(renderer->vg, fontSize == 0 ? renderer->defaultFontSize : fontSize);
		wz_renderer_set_font_face(renderer, fontFace);
		*width = (int)nvgTextBounds(renderer->vg, 0, 0, text, n == 0 ? NULL : &text[n], NULL);
	}

	if (height)
	{
		*height = (int)(fontSize == 0 ? renderer->defaultFontSize : fontSize);
	}
}

wzLineBreakResult wz_renderer_line_break_text(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int lineWidth)
{
	struct NVGtextRow row;
	wzLineBreakResult result;

	WZ_ASSERT(renderer);

	if (text && nvgTextBreakLines(renderer->vg, text, n == 0 ? NULL : &text[n], (float)lineWidth, &row, 1) > 0)
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

int wz_renderer_create_image(struct wzRenderer *renderer, const char *filename, int *width, int *height)
{
	int handle, i;

	WZ_ASSERT(renderer);
	handle = -1;

	// Check cache.
	for (i = 0; i < renderer->nImages; i++)
	{
		if (strcmp(renderer->images[i].filename, filename) == 0)
		{
			handle = renderer->images[i].handle;
			break;
		}
	}

	if (handle == -1)
	{
		// Not found, create and cache it.
		renderer->images[renderer->nImages].handle = nvgCreateImage(renderer->vg, filename, 0);
		strcpy(renderer->images[renderer->nImages].filename, filename);
		handle = renderer->images[renderer->nImages].handle;
		renderer->nImages++;
	}

	nvgImageSize(renderer->vg, handle, width, height);
	return handle;
}

void wz_renderer_set_font_face(struct wzRenderer *renderer, const char *face)
{
	int id;
	
	WZ_ASSERT(renderer);
	id = wz_nanovg_create_font(renderer, face);

	// Use the first font if creating failed.
	if (id == -1)
		id = 0;

	nvgFontFaceId(renderer->vg, id);
}

void wz_renderer_print_box(struct wzRenderer *renderer, wzRect rect, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength)
{
	WZ_ASSERT(renderer);
	nvgFontSize(renderer->vg, fontSize == 0 ? renderer->defaultFontSize : fontSize);
	wz_renderer_set_font_face(renderer, fontFace);
	nvgTextAlign(renderer->vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	nvgTextLineHeight(renderer->vg, 1.0f);
	nvgFontBlur(renderer->vg, 0);
	nvgFillColor(renderer->vg, color);
	nvgTextBox(renderer->vg, (float)rect.x, (float)rect.y, (float)rect.w, text, textLength == 0 ? NULL : &text[textLength]);
}

void wz_renderer_print(struct wzRenderer *renderer, int x, int y, int align, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength)
{
	WZ_ASSERT(renderer);
	nvgFontSize(renderer->vg, fontSize == 0 ? renderer->defaultFontSize : fontSize);
	wz_renderer_set_font_face(renderer, fontFace);
	nvgTextAlign(renderer->vg, align);
	nvgFontBlur(renderer->vg, 0);
	nvgFillColor(renderer->vg, color);
	nvgText(renderer->vg, (float)x, (float)y, text, textLength == 0 ? NULL : &text[textLength]);
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
