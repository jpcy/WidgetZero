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
#include <stdarg.h>
#include <stdint.h>
#include <SDL.h>
#include <wz_sdl2.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define WZ_GL_MAX_ERROR_MESSAGE 1024
#define WZ_GL_NUM_GLYPHS 256

typedef enum
{
	TA_NONE = 0,
	TA_LEFT = 1,
	TA_TOP = 1,
	TA_CENTER = 2,
	TA_RIGHT = 3,
	TA_BOTTOM = 3
}
TextAlignment;

typedef struct
{
	SDL_Renderer *renderer;
	stbtt_fontinfo font;
	float fontHeight;
	uint8_t *fontFileBuffer;
	stbtt_bakedchar glyphs[WZ_GL_NUM_GLYPHS];
	SDL_Surface *glyphAtlasSurface;
	SDL_Texture *glyphAtlasTexture;
}
wzRendererData;

static char errorMessage[WZ_GL_MAX_ERROR_MESSAGE];

/*
================================================================================

MISC. UTILITY FUNCTIONS

================================================================================
*/

static void wzgl_measure_text(struct wzRenderer *renderer, const char *text, int *width, int *height);

static void wzgl_printf(struct wzRenderer *renderer, int x, int y, TextAlignment halign, TextAlignment valign, uint8_t r, uint8_t g, uint8_t b, const char *format, ...)
{
	wzRendererData *rendererData;
	static char buffer[2048];
	va_list args;
	float cursorX, cursorY;
	size_t i;

	assert(renderer);
	rendererData = (wzRendererData *)renderer->data;

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	SDL_SetTextureColorMod(rendererData->glyphAtlasTexture, r, g, b);
	cursorX = (float)x;
	cursorY = (float)y;

	if (halign == TA_CENTER || halign == TA_RIGHT)
	{
		int width, height;
		wzgl_measure_text(renderer, buffer, &width, &height);

		if (halign == TA_CENTER)
		{
			cursorX -= width / 2.0f;
		}
		else if (halign == TA_RIGHT)
		{
			cursorX -= width;
		}
	}

	if (valign == TA_TOP || valign == TA_CENTER)
	{
		float scale;
		int ascent, descent, lineGap;

		scale = stbtt_ScaleForMappingEmToPixels(&rendererData->font, rendererData->fontHeight);
		stbtt_GetFontVMetrics(&rendererData->font, &ascent, &descent, &lineGap);

		if (valign == TA_TOP)
		{
			cursorY += ascent * scale;
		}
		else if (valign == TA_CENTER)
		{
			cursorY += (ascent + descent) * scale / 2.0f;
		}
	}

	for (i = 0; i < strlen(buffer); i++)
	{
		stbtt_bakedchar *g;
		SDL_Rect src, dest;

		g = &rendererData->glyphs[buffer[i]];

		src.x = g->x0;
		src.y = g->y0;
		src.w = g->x1 - g->x0;
		src.h = g->y1 - g->y0;

		dest.x = (int)(cursorX + g->xoff);
		dest.y = (int)(cursorY + g->yoff);
		dest.w = src.w;
		dest.h = src.h;

		SDL_RenderCopy(rendererData->renderer, rendererData->glyphAtlasTexture, &src, &dest);
		cursorX += g->xadvance;
	}
}

static void wzgl_clip_reset(SDL_Renderer *renderer)
{
	SDL_Rect rect;

	assert(renderer);
	rect.x = rect.y = rect.w = rect.h = 0;
	SDL_RenderSetClipRect(renderer, &rect);
}

static void wzgl_clip_to_rect(SDL_Renderer *renderer, wzRect rect)
{
	assert(renderer);
	SDL_RenderSetClipRect(renderer, (const SDL_Rect *)&rect);
}

static bool wzgl_clip_to_rect_intersection(SDL_Renderer *renderer, wzRect rect1, wzRect rect2)
{
	wzRect intersection;

	assert(renderer);

	if (!SDL_IntersectRect((const SDL_Rect *)&rect1, (const SDL_Rect *)&rect2, (SDL_Rect *)&intersection))
	{
		return false;
	}

	SDL_RenderSetClipRect(renderer, (const SDL_Rect *)&intersection);
	return true;
}

/*
================================================================================

RENDERER

================================================================================
*/

static void wzgl_measure_text(struct wzRenderer *renderer, const char *text, int *width, int *height)
{
	wzRendererData *rendererData;
	float total;
	size_t i;

	assert(renderer);
	rendererData = (wzRendererData *)renderer->data;
	total = 0;

	for (i = 0; i < strlen(text); i++)
	{
		stbtt_bakedchar *g = &rendererData->glyphs[text[i]];
		total += g->xadvance;
	}

	if (width)
	{
		*width = (int)total;
	}

	if (height)
	{
		*height = (int)rendererData->fontHeight;
	}
}

static void wzgl_draw_dock_icon(struct wzRenderer *renderer, wzRect rect)
{
	SDL_Renderer *sdl;

	assert(renderer);
	sdl = ((wzRendererData *)renderer->data)->renderer;
	wzgl_clip_reset(sdl);
	SDL_SetRenderDrawBlendMode(sdl, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(sdl, 64, 64, 64, 128);
	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);
	SDL_SetRenderDrawBlendMode(sdl, SDL_BLENDMODE_NONE);
}

static void wzgl_draw_dock_preview(struct wzRenderer *renderer, wzRect rect)
{
	SDL_Renderer *sdl;

	assert(renderer);
	sdl = ((wzRendererData *)renderer->data)->renderer;
	wzgl_clip_reset(sdl);
	SDL_SetRenderDrawBlendMode(sdl, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(sdl, 0, 0, 128, 64);
	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);
	SDL_SetRenderDrawBlendMode(sdl, SDL_BLENDMODE_NONE);
}

static void wzgl_draw_window(struct wzRenderer *renderer, wzRect clip, struct wzWindow *window, const char *title)
{
	SDL_Renderer *sdl;
	wzRect rect;
	int borderSize;
	wzRect borderRect, headerRect;

	assert(renderer);
	assert(window);

	sdl = ((wzRendererData *)renderer->data)->renderer;
	wzgl_clip_reset(sdl);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)window);
	
	// Background.
	SDL_SetRenderDrawColor(sdl, 255, 255, 255, 255);
	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);

	// Border.
	borderSize = wz_window_get_border_size(window);
	SDL_SetRenderDrawColor(sdl, 128, 128, 128, 255);

	// Border top.
	borderRect = rect;
	borderRect.h = borderSize;
	SDL_RenderFillRect(sdl, (SDL_Rect *)&borderRect);

	// Border bottom.
	borderRect.y = rect.y + rect.h - borderSize;
	SDL_RenderFillRect(sdl, (SDL_Rect *)&borderRect);

	// Border left.
	borderRect = rect;
	borderRect.w = borderSize;
	SDL_RenderFillRect(sdl, (SDL_Rect *)&borderRect);

	// Border right.
	borderRect.x = rect.x + rect.w - borderSize;
	SDL_RenderFillRect(sdl, (SDL_Rect *)&borderRect);

	// Header.
	headerRect = wz_window_get_header_rect(window);

	if (headerRect.w > 0 && headerRect.h > 0)
	{
		wzgl_clip_to_rect(sdl, headerRect);
		SDL_SetRenderDrawColor(sdl, 255, 232, 166, 255);
		SDL_RenderFillRect(sdl, (SDL_Rect *)&headerRect);
		wzgl_printf(renderer, headerRect.x + 10, headerRect.y + headerRect.h / 2, TA_LEFT, TA_CENTER, 0, 0, 0, title);
	}
}

static void wzgl_draw_button(struct wzRenderer *renderer, wzRect clip, struct wzButton *button, const char *label)
{
	SDL_Renderer *sdl;
	wzRect rect;
	bool hover, pressed;

	assert(renderer);
	assert(button);
	sdl = ((wzRendererData *)renderer->data)->renderer;
	wzgl_clip_to_rect(sdl, clip);

	rect = wz_widget_get_absolute_rect((struct wzWidget *)button);
	hover = wz_widget_get_hover((struct wzWidget *)button);
	pressed = wz_button_is_pressed(button);

	// Background.
	if (pressed && hover)
	{
		SDL_SetRenderDrawColor(sdl, 127, 194, 229, 255);
	}
	else if (hover)
	{
		SDL_SetRenderDrawColor(sdl, 188, 229, 252, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(sdl, 218, 218, 218, 255);
	}

	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);

	// Border.
	if (pressed && hover)
	{
		SDL_SetRenderDrawColor(sdl, 44, 98, 139, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(sdl, 112, 112, 112, 255);
	}

	SDL_RenderDrawRect(sdl, (SDL_Rect *)&rect);

	// Label.
	wzgl_printf(renderer, rect.x + rect.w / 2, rect.y + rect.h / 2, TA_CENTER, TA_CENTER, 0, 0, 0, label);
}

static void wzgl_draw_checkbox(struct wzRenderer *renderer, wzRect clip, struct wzButton *checkbox, const char *label)
{
	SDL_Renderer *sdl;
	wzRect rect;
	bool hover;
	SDL_Rect boxRect;
	int boxSize, boxRightMargin;

	assert(renderer);
	sdl = ((wzRendererData *)renderer->data)->renderer;
	wzgl_clip_to_rect(sdl, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)checkbox);
	hover = wz_widget_get_hover((struct wzWidget *)checkbox);

	// Box.
	boxSize = 16;
	boxRightMargin = 8;
	boxRect.x = rect.x;
	boxRect.y = (int)(rect.y + rect.h / 2.0f - boxSize / 2.0f);
	boxRect.w = boxSize;
	boxRect.h = boxSize;

	// Box background.
	if (wz_button_is_pressed(checkbox) && hover)
	{
		SDL_SetRenderDrawColor(sdl, 127, 194, 229, 255);
		SDL_RenderFillRect(sdl, &boxRect);
	}
	else if (hover)
	{
		SDL_SetRenderDrawColor(sdl, 188, 229, 252, 255);
		SDL_RenderFillRect(sdl, &boxRect);
	}

	// Box border.
	SDL_SetRenderDrawColor(sdl, 0, 0, 0, 255);
	SDL_RenderDrawRect(sdl, &boxRect);

	// Box checkmark.
	if (wz_button_is_set(checkbox))
	{
		boxRect.x = rect.x + 4;
		boxRect.y = (int)(rect.y + rect.h / 2.0f - boxSize / 2.0f) + 4;
		boxRect.w = boxSize - 8;
		boxRect.h = boxSize - 8;
		SDL_SetRenderDrawColor(sdl, 0, 0, 0, 255);
		SDL_RenderFillRect(sdl, &boxRect);
	}

	// Label.
	wzgl_printf(renderer, rect.x + boxSize + boxRightMargin, rect.y + rect.h / 2, TA_LEFT, TA_CENTER, 0, 0, 0, label);
}

static void wzgl_draw_combo(struct wzRenderer *renderer, wzRect clip, struct wzCombo *combo, const char *item)
{
	SDL_Renderer *sdl;
	wzRect rect;
	bool hover;

	assert(renderer);
	assert(combo);
	sdl = ((wzRendererData *)renderer->data)->renderer;

	wzgl_clip_to_rect(sdl, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)combo);
	hover = wz_widget_get_hover((struct wzWidget *)combo);

	// Background.
	if (hover)
	{
		SDL_SetRenderDrawColor(sdl, 188, 229, 252, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(sdl, 218, 218, 218, 255);
	}

	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);

	// Border.
	if (hover)
	{
		SDL_SetRenderDrawColor(sdl, 44, 98, 139, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(sdl, 112, 112, 112, 255);
	}

	SDL_RenderDrawRect(sdl, (SDL_Rect *)&rect);

	// Selected item.
	if (item)
		wzgl_printf(renderer, rect.x + 10, rect.y + rect.h / 2, TA_LEFT, TA_CENTER, 0, 0, 0, item);
}

static void wzgl_draw_groupbox(struct wzRenderer *renderer, wzRect clip, struct wzGroupBox *groupBox, const char *label)
{
	SDL_Renderer *sdl;
	wzRect rect;
	int textLeftMargin, textBorderSpacing;
	int textWidth, textHeight;

	assert(renderer);
	sdl = ((wzRendererData *)renderer->data)->renderer;

	wzgl_clip_to_rect(sdl, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)groupBox);
	
	// Background.
	SDL_SetRenderDrawColor(sdl, 255, 255, 255, 255);
	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);

	// Border - left, bottom, right, top left, top right.
	textLeftMargin = 20;
	textBorderSpacing = 5;
	wzgl_measure_text(renderer, label, &textWidth, &textHeight);
	SDL_SetRenderDrawColor(sdl, 98, 135, 157, 255);
	SDL_RenderDrawLine(sdl, rect.x, rect.y + textHeight / 2, rect.x, rect.y + rect.h);
	SDL_RenderDrawLine(sdl, rect.x, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h);
	SDL_RenderDrawLine(sdl, rect.x + rect.w, rect.y + textHeight / 2, rect.x + rect.w, rect.y + rect.h);
	SDL_RenderDrawLine(sdl, rect.x, rect.y + textHeight / 2, rect.x + textLeftMargin - textBorderSpacing, rect.y + textHeight / 2);
	SDL_RenderDrawLine(sdl, rect.x + textLeftMargin + textWidth + textBorderSpacing * 2, rect.y + textHeight / 2, rect.x + rect.w, rect.y + textHeight / 2);

	// Label.
	wzgl_printf(renderer, rect.x + textLeftMargin, rect.y, TA_LEFT, TA_TOP, 0, 0, 0, label);
}

static void wzgl_draw_scroller(struct wzRenderer *renderer, wzRect clip, struct wzScroller *scroller)
{
	SDL_Renderer *sdl;
	wzRect rect, nubRect;

	assert(renderer);
	assert(scroller);
	sdl = ((wzRendererData *)renderer->data)->renderer;

	wzgl_clip_to_rect(sdl, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)scroller);
	
	// Background.
	SDL_SetRenderDrawColor(sdl, 192, 192, 192, 255);
	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);

	// Nub.
	nubRect = wz_scroller_get_nub_rect(scroller);
	SDL_SetRenderDrawColor(sdl, 218, 218, 218, 255);
	SDL_RenderFillRect(sdl, (SDL_Rect *)&nubRect);
	SDL_SetRenderDrawColor(sdl, 112, 112, 112, 255);
	SDL_RenderDrawRect(sdl, (SDL_Rect *)&nubRect);
}

static void wzgl_draw_label(struct wzRenderer *renderer, wzRect clip, struct wzLabel *label, const char *text, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_Renderer *sdl;
	wzRect rect;

	assert(renderer);
	assert(label);
	sdl = ((wzRendererData *)renderer->data)->renderer;

	wzgl_clip_to_rect(sdl, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)label);
	wzgl_printf(renderer, rect.x, rect.y, TA_LEFT, TA_TOP, r, g, b, text);
}

static void wzgl_draw_list(struct wzRenderer *renderer, wzRect clip, struct wzList *list, const char **items)
{
	SDL_Renderer *sdl;
	wzRect rect, itemsRect;
	int itemsMargin, itemHeight, itemLeftPadding;
	int nItems, scrollerValue, y, i;

	assert(renderer);
	assert(list);
	sdl = ((wzRendererData *)renderer->data)->renderer;

	wzgl_clip_to_rect(sdl, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)list);
	
	// Background.
	SDL_SetRenderDrawColor(sdl, 255, 255, 255, 255);
	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);

	// Border.
	SDL_SetRenderDrawColor(sdl, 130, 135, 144, 255);
	SDL_RenderDrawRect(sdl, (SDL_Rect *)&rect);

	// Items.
	itemsMargin = 2;
	itemHeight = 18;
	itemLeftPadding = 4;
	itemsRect = wz_list_get_absolute_items_rect(list);

	if (!wzgl_clip_to_rect_intersection(sdl, clip, itemsRect))
		return;

	nItems = wz_list_get_num_items(list);
	scrollerValue = wz_scroller_get_value(wz_list_get_scroller(list));
	y = itemsRect.y - (scrollerValue % itemHeight);

	for (i = wz_list_get_first_item(list); i < nItems; i++)
	{
		SDL_Rect itemRect;

		// Outside widget?
		if (y > itemsRect.y + itemsRect.h)
			break;

		itemRect.x = itemsRect.x;
		itemRect.y = y;
		itemRect.w = itemsRect.w;
		itemRect.h = itemHeight;

		if (i == wz_list_get_selected_item(list))
		{
			SDL_SetRenderDrawColor(sdl, 127, 194, 229, 255);
			SDL_RenderFillRect(sdl, &itemRect);
		}
		else if (i == wz_list_get_pressed_item(list) || i == wz_list_get_hovered_item(list))
		{
			SDL_SetRenderDrawColor(sdl, 188, 229, 252, 255);
			SDL_RenderFillRect(sdl, &itemRect);
		}

		wzgl_printf(renderer, itemsRect.x + itemLeftPadding, y + itemHeight / 2, TA_LEFT, TA_CENTER, 0, 0, 0, items[i]);
		y += itemHeight;
	}
}

static void wzgl_draw_tab_button(struct wzRenderer *renderer, wzRect clip, struct wzButton *tabButton, const char *label)
{
	SDL_Renderer *sdl;
	wzRect rect;
	bool hover, set;

	assert(renderer);
	assert(tabButton);
	sdl = ((wzRendererData *)renderer->data)->renderer;

	wzgl_clip_to_rect(sdl, clip);

	rect = wz_widget_get_absolute_rect((struct wzWidget *)tabButton);
	hover = wz_widget_get_hover((struct wzWidget *)tabButton);
	set = wz_button_is_set(tabButton);

	// Background.
	if (set)
	{
		SDL_SetRenderDrawColor(sdl, 127, 194, 229, 255);
	}
	else if (hover)
	{
		SDL_SetRenderDrawColor(sdl, 188, 229, 252, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(sdl, 218, 218, 218, 255);
	}

	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);

	// Border.
	if (set)
	{
		SDL_SetRenderDrawColor(sdl, 44, 98, 139, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(sdl, 112, 112, 112, 255);
	}

	SDL_RenderDrawRect(sdl, (SDL_Rect *)&rect);

	// Label.
	wzgl_printf(renderer, rect.x + rect.w / 2, rect.y + rect.h / 2, TA_CENTER, TA_CENTER, 0, 0, 0, label);
}

static void wzgl_draw_tab_page(struct wzRenderer *renderer, wzRect clip, struct wzWidget *tabPage)
{
	SDL_Renderer *sdl;
	wzRect rect;

	assert(renderer);
	assert(tabPage);
	sdl = ((wzRendererData *)renderer->data)->renderer;

	wzgl_clip_to_rect(sdl, clip);
	rect = wz_widget_get_absolute_rect((struct wzWidget *)tabPage);

	SDL_SetRenderDrawColor(sdl, 224, 224, 224, 255);
	SDL_RenderFillRect(sdl, (SDL_Rect *)&rect);

	SDL_SetRenderDrawColor(sdl, 112, 112, 112, 255);
	SDL_RenderDrawRect(sdl, (SDL_Rect *)&rect);
}

static void wzgl_reset_clipping(struct wzRenderer *renderer)
{
	assert(renderer);
	wzgl_clip_reset(((wzRendererData *)renderer->data)->renderer);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

struct wzRenderer *wzgl_create_renderer(SDL_Renderer *sdl, const char *fontFilename, float fontHeight)
{
	struct wzRenderer *renderer;
	wzRendererData *rendererData;
	FILE *f;
	size_t length, bytesRead;
	SDL_Palette palette;
	SDL_Color colors[256];
	int i;

	// Alloc renderer.
	renderer = malloc(sizeof(struct wzRenderer));
	rendererData = renderer->data = malloc(sizeof(wzRendererData));
	memset(rendererData, 0, sizeof(wzRendererData));
	rendererData->renderer = sdl;

	// Read font file into buffer.
	f = fopen(fontFilename, "rb");

	if (!f)
	{
		sprintf(errorMessage, "Error loading font '%s'", fontFilename);
		wzgl_destroy_renderer(renderer);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	length = (size_t)ftell(f);
	fseek(f, 0, SEEK_SET);
	rendererData->fontFileBuffer = (uint8_t *)malloc(length);
	bytesRead = fread(rendererData->fontFileBuffer, 1, length, f);
	fclose(f);

    if (bytesRead != length)
    {
		sprintf(errorMessage, "Error reading font file '%s'. Tried to read %d bytes, got %d.", fontFilename, length, bytesRead);
		wzgl_destroy_renderer(renderer);
		return NULL;
    }

	// Create glyph atlas and set a greyscale palette.	
	rendererData->glyphAtlasSurface = SDL_CreateRGBSurface(0, 512, 512, 8, 0, 0, 0, 0);

	if (!rendererData->glyphAtlasSurface)
	{
		strcpy(errorMessage, SDL_GetError());
		wzgl_destroy_renderer(renderer);
		return NULL;
	}
	
	palette.ncolors = 256;
	palette.colors = colors;
	
	for (i = 0; i < palette.ncolors; i++)
	{
		colors[i].r = colors[i].g = colors[i].b = colors[i].a = i;
	}
	
	SDL_SetSurfacePalette(rendererData->glyphAtlasSurface, &palette);

	// Create font and bake glyphs to the atlas.
	stbtt_InitFont(&rendererData->font, rendererData->fontFileBuffer, stbtt_GetFontOffsetForIndex(rendererData->fontFileBuffer, 0));
	stbtt_BakeFontBitmap(rendererData->fontFileBuffer, 0, fontHeight, (unsigned char *)rendererData->glyphAtlasSurface->pixels, rendererData->glyphAtlasSurface->w, rendererData->glyphAtlasSurface->h, 0, WZ_GL_NUM_GLYPHS, rendererData->glyphs);
	rendererData->fontHeight = fontHeight;

	// Create a texture from the glyph atlas.
	rendererData->glyphAtlasTexture = SDL_CreateTextureFromSurface(rendererData->renderer, rendererData->glyphAtlasSurface);

	if (!rendererData->glyphAtlasTexture)
	{
		strcpy(errorMessage, SDL_GetError());
		wzgl_destroy_renderer(renderer);
		return NULL;
	}

	SDL_SetTextureBlendMode(rendererData->glyphAtlasTexture, SDL_BLENDMODE_BLEND);

	// Set renderer function pointers.
	renderer->measure_text = wzgl_measure_text;
	renderer->draw_dock_icon = wzgl_draw_dock_icon;
	renderer->draw_dock_preview = wzgl_draw_dock_preview;
	renderer->draw_window = wzgl_draw_window;
	renderer->draw_button = wzgl_draw_button;
	renderer->draw_checkbox = wzgl_draw_checkbox;
	renderer->draw_combo = wzgl_draw_combo;
	renderer->draw_groupbox = wzgl_draw_groupbox;
	renderer->draw_scroller = wzgl_draw_scroller;
	renderer->draw_label = wzgl_draw_label;
	renderer->draw_list = wzgl_draw_list;
	renderer->draw_tab_button = wzgl_draw_tab_button;
	renderer->draw_tab_page = wzgl_draw_tab_page;
	renderer->reset_clipping = wzgl_reset_clipping;

	return renderer;
}

const char *wzgl_get_error()
{
	return errorMessage;
}

void wzgl_destroy_renderer(struct wzRenderer *renderer)
{
	if (renderer)
	{
		free(renderer->data);
		free(renderer);
	}
}
