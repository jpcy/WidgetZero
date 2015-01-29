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

namespace wz {

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
	renderer = new struct wzRenderer;
	renderer->destroy = destroy;
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

		delete renderer;
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

LineBreakResult wz_renderer_line_break_text(struct wzRenderer *renderer, const char *fontFace, float fontSize, const char *text, int n, int lineWidth)
{
	struct NVGtextRow row;
	LineBreakResult result;

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

void wz_renderer_print_box(struct wzRenderer *renderer, Rect rect, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength)
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

void wz_renderer_clip_to_rect(struct NVGcontext *vg, Rect rect)
{
	WZ_ASSERT(vg);
	nvgScissor(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
}

bool wz_renderer_clip_to_rect_intersection(struct NVGcontext *vg, Rect rect1, Rect rect2)
{
	Rect intersection;

	WZ_ASSERT(vg);

	if (!Rect::intersect(rect1, rect2, &intersection))
	{
		return false;
	}

	nvgScissor(vg, (float)intersection.x, (float)intersection.y, (float)intersection.w, (float)intersection.h);
	return true;
}

void wz_renderer_draw_filled_rect(struct NVGcontext *vg, Rect rect, struct NVGcolor color)
{
	nvgBeginPath(vg);
	nvgRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f);
	nvgFillColor(vg, color);
	nvgFill(vg);
}

void wz_renderer_draw_rect(struct NVGcontext *vg, Rect rect, struct NVGcolor color)
{
	nvgBeginPath(vg);
	nvgRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.w - 1.0f, rect.h - 1.0f);
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

void wz_renderer_draw_image(struct NVGcontext *vg, Rect rect, int image)
{
	int w, h;
	struct NVGpaint paint;

	nvgImageSize(vg, image, &w, &h);
	paint = nvgImagePattern(vg, (float)rect.x, (float)rect.y, (float)w, (float)h, 0, image, 1);
	nvgBeginPath(vg);
	nvgRect(vg, (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h);
	nvgFillPaint(vg, paint);
	nvgFill(vg);
}

void wz_renderer_create_rect_path(struct NVGcontext *vg, Rect rect, float r, int sides, int roundedCorners)
{
	const float x = rect.x + 0.5f;
	const float y = rect.y + 0.5f;
	const float w = rect.w - 1.0f;
	const float h = rect.h - 1.0f;
	const float rx = WZ_MIN(r, WZ_ABS(w) * 0.5f) * WZ_SIGN(w);
	const float ry = WZ_MIN(r, WZ_ABS(h) * 0.5f) * WZ_SIGN(h);
	const float NVG_KAPPA90 = 0.5522847493f;

	nvgBeginPath(vg);
	
	if (roundedCorners & WZ_CORNER_TL)
	{
		nvgMoveTo(vg, x, y + ry);
	}
	else
	{
		nvgMoveTo(vg, x, y);
	}

	if (roundedCorners & WZ_CORNER_BL)
	{
		// left straight
		if (sides & WZ_SIDE_LEFT)
		{
			nvgLineTo(vg, x, y + h - ry);
		}
		else
		{
			nvgMoveTo(vg, x, y + h - ry);
		}

		nvgBezierTo(vg, x, y+h-ry*(1-NVG_KAPPA90), x+rx*(1-NVG_KAPPA90), y+h, x+rx, y+h); // bottom left arc
	}
	else
	{
		if (sides & WZ_SIDE_LEFT)
		{
			nvgLineTo(vg, x, y + h);
		}
		else
		{
			nvgMoveTo(vg, x, y + h);
		}
	}

	if (roundedCorners & WZ_CORNER_BR)
	{
		// bottom straight
		if (sides & WZ_SIDE_BOTTOM)
		{
			nvgLineTo(vg, x+w-rx, y+h);
		}
		else
		{
			nvgMoveTo(vg, x+w-rx, y+h);
		}

		nvgBezierTo(vg, x+w-rx*(1-NVG_KAPPA90), y+h, x+w, y+h-ry*(1-NVG_KAPPA90), x+w, y+h-ry); // bottom right arc
	}
	else
	{
		if (sides & WZ_SIDE_BOTTOM)
		{
			nvgLineTo(vg, x + w, y + h);
		}
		else
		{
			nvgMoveTo(vg, x + w, y + h);
		}
	}

	if (roundedCorners & WZ_CORNER_TR)
	{
		// right straight
		if (sides & WZ_SIDE_RIGHT)
		{
			nvgLineTo(vg, x+w, y+ry);
		}
		else
		{
			nvgMoveTo(vg, x+w, y+ry);
		}

		nvgBezierTo(vg, x+w, y+ry*(1-NVG_KAPPA90), x+w-rx*(1-NVG_KAPPA90), y, x+w-rx, y); // top right arc
	}
	else
	{
		if (sides & WZ_SIDE_RIGHT)
		{
			nvgLineTo(vg, x + w, y);
		}
		else
		{
			nvgMoveTo(vg, x + w, y);
		}
	}

	if (roundedCorners & WZ_CORNER_TL)
	{
		// top straight
		if (sides & WZ_SIDE_TOP)
		{
			nvgLineTo(vg, x+rx, y);
		}
		else
		{
			nvgMoveTo(vg, x+rx, y);
		}

		nvgBezierTo(vg, x+rx*(1-NVG_KAPPA90), y, x, y+ry*(1-NVG_KAPPA90), x, y+ry); // top left arc
	}
	else if (sides & WZ_SIDE_TOP)
	{
		nvgLineTo(vg, x, y);
	}
}

} // namespace wz