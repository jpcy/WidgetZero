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
#include <memory>
#include <SDL.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "Example.h"
#include "Wrapper.h"

SDL_Renderer *g_renderer;

static char fontFilename[] = "../example/data/DejaVuSans.ttf";
static stbtt_fontinfo font;
static uint8_t *fontFileBuffer;
static const float fontHeight = 16.0f;
static const size_t nGlyphs = 256;
static stbtt_bakedchar glyphs[nGlyphs];
static SDL_Surface *glyphAtlasSurface;
static SDL_Texture *glyphAtlasTexture;

static void ShowSdlError()
{
	fprintf(stderr, "%s\n", SDL_GetError());
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL Error", SDL_GetError(), NULL);
	SDL_ClearError();
}

void ErrorAndExit(const char *format, ...)
{
	static char buffer[2048];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	fprintf(stderr, "%s\n", buffer);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buffer, NULL);
	exit(1);
}

void TextPrintf(int x, int y, TextAlignment halign, TextAlignment valign, uint8_t r, uint8_t g, uint8_t b, const char *format, ...)
{
	static char buffer[2048];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	SDL_SetTextureColorMod(glyphAtlasTexture, r, g, b);
	float cursorX = (float)x;
	float cursorY = (float)y;

	if (halign == TA_CENTER || halign == TA_RIGHT)
	{
		int width, height;
		MeasureText(buffer, &width, &height);

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
		float scale = stbtt_ScaleForMappingEmToPixels(&font, fontHeight);
		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);

		if (valign == TA_TOP)
		{
			cursorY += ascent * scale;
		}
		else if (valign == TA_CENTER)
		{
			cursorY += (ascent + descent) * scale / 2.0f;
		}
	}

	for (size_t i = 0; i < strlen(buffer); i++)
	{
		stbtt_bakedchar *g = &glyphs[buffer[i]];

		SDL_Rect src;
		src.x = g->x0;
		src.y = g->y0;
		src.w = g->x1 - g->x0;
		src.h = g->y1 - g->y0;

		SDL_Rect dest;
		dest.x = (int)(cursorX + g->xoff);
		dest.y = (int)(cursorY + g->yoff);
		dest.w = src.w;
		dest.h = src.h;

		SDL_RenderCopy(g_renderer, glyphAtlasTexture, &src, &dest);
		cursorX += g->xadvance;
	}
}

void MeasureText(const char *text, int *width, int *height)
{
	float total = 0;

	for (size_t i = 0; i < strlen(text); i++)
	{
		stbtt_bakedchar *g = &glyphs[text[i]];
		total += g->xadvance;
	}

	*width = (int)total;
	*height = (int)fontHeight;
}

int main(int argc, char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		ShowSdlError();
		return 1;
	}

	atexit(SDL_Quit);

	SDL_Window *window;

	if (SDL_CreateWindowAndRenderer(1024, 768, 0, &window, &g_renderer) < 0)
	{
		ShowSdlError();
		return 1;
	}

	SDL_SetWindowTitle(window, "WidgetZero Example");

	// Read font file into buffer.
	FILE *f = fopen(fontFilename, "rb");

	if (!f)
		ErrorAndExit("Error loading font '%s'", fontFilename);

	fseek(f, 0, SEEK_END);
	size_t length = (size_t)ftell(f);
	fseek(f, 0, SEEK_SET);
	fontFileBuffer = (uint8_t *)malloc(length);
	size_t bytesRead = fread(fontFileBuffer, 1, length, f);
	fclose(f);

    if (bytesRead != length)
    {
        ErrorAndExit("Error reading font file '%s'. Tried to read %u bytes, got %u.", fontFilename, length, bytesRead);
    }

	// Create glyph atlas and set a greyscale palette.	
	glyphAtlasSurface = SDL_CreateRGBSurface(0, 512, 512, 8, 0, 0, 0, 0);

	if (!glyphAtlasSurface)
	{
		ShowSdlError();
		return 1;
	}

	SDL_Palette palette;
	SDL_Color colors[256];
	palette.ncolors = 256;
	palette.colors = colors;
	
	for (int i = 0; i < palette.ncolors; i++)
	{
		colors[i].r = colors[i].g = colors[i].b = colors[i].a = i;
	}
	
	SDL_SetSurfacePalette(glyphAtlasSurface, &palette);

	// Create font and bake glyphs to the atlas.
	stbtt_InitFont(&font, fontFileBuffer, stbtt_GetFontOffsetForIndex(fontFileBuffer, 0));
	stbtt_BakeFontBitmap(fontFileBuffer, 0, fontHeight, (unsigned char *)glyphAtlasSurface->pixels, glyphAtlasSurface->w, glyphAtlasSurface->h, 0, nGlyphs, glyphs);

	// Create a texture from the glyph atlas.
	glyphAtlasTexture = SDL_CreateTextureFromSurface(g_renderer, glyphAtlasSurface);

	if (!glyphAtlasTexture)
	{
		ShowSdlError();
		return 1;
	}

	SDL_SetTextureBlendMode(glyphAtlasTexture, SDL_BLENDMODE_BLEND);

	// Create wz objects.
	Context context;
	Desktop desktop(&context);

	Button button(&desktop, "Test Button");
	button.setPosition(100, 100);

	wzRect buttonRect = button.getRect();
	Checkbox checkbox(&desktop, "Toggle me!");
	checkbox.setPosition(buttonRect.x, buttonRect.y + buttonRect.h + 16);

	GroupBox groupBox(&desktop, "Test GroupBox");
	groupBox.setPosition(100, 300);

	Scroller scroller(&desktop, WZ_SCROLLER_VERTICAL, 20, 10, 100);
	scroller.setRect(300, 50, 16, 200);

	Scroller scrollerHorizontal(&desktop, WZ_SCROLLER_HORIZONTAL, 50, 10, 100);
	scrollerHorizontal.setRect(500, 50, 200, 16);

	const char *listData[17] =
	{
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
		"Sunday",
		"One",
		"Two",
		"Three",
		"Four",
		"Five",
		"Six",
		"Seven",
		"Eight",
		"Nine",
		"Ten"
	};

	List list(&desktop, listData, 17);
	list.setRect(400, 300, 150, 150);

	for (;;)
	{
		SDL_Event e;

		if (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				break;
			else if (e.type == SDL_MOUSEMOTION)
			{
				desktop.mouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				desktop.mouseButtonDown(e.button.button, e.button.x, e.button.y);
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				desktop.mouseButtonUp(e.button.button, e.button.x, e.button.y);
			}
		}

		SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
		SDL_RenderClear(g_renderer);
		desktop.draw();
		TextPrintf(350, 50, TA_LEFT, TA_TOP, 255, 128, 128, "Scroll value: %d", scroller.getValue());
		TextPrintf(500, 100, TA_LEFT, TA_TOP, 128, 128, 255, "Scroll value: %d", scrollerHorizontal.getValue());
		SDL_RenderPresent(g_renderer);
	}

	return 0;
}
