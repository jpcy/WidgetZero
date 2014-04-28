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
#include <widgetzero/wz.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static SDL_Renderer *renderer;

static char fontFilename[] = "../example/data/DejaVuSans.ttf";
static stbtt_fontinfo font;
static uint8_t *fontFileBuffer;
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

void TextPrintf(int x, int y, const char *format, ...)
{
	static char buffer[2048];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	float currentX = (float)x;

	for (size_t i = 0; i < strlen(buffer); i++)
	{
		stbtt_bakedchar *g = &glyphs[buffer[i]];

		SDL_Rect src;
		src.x = g->x0;
		src.y = g->y0;
		src.w = g->x1 - g->x0;
		src.h = g->y1 - g->y0;

		SDL_Rect dest;
		dest.x = (int)(currentX + g->xoff);
		dest.y = (int)((float)y + g->yoff);
		dest.w = src.w;
		dest.h = src.h;

		SDL_RenderCopy(renderer, glyphAtlasTexture, &src, &dest);
		currentX += g->xadvance;
	}
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

	if (SDL_CreateWindowAndRenderer(1024, 768, 0, &window, &renderer) < 0)
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
	fread(fontFileBuffer, length, 1, f);
	fclose(f);

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
	stbtt_BakeFontBitmap(fontFileBuffer, 0, 32.0f, (unsigned char *)glyphAtlasSurface->pixels, glyphAtlasSurface->w, glyphAtlasSurface->h, 0, nGlyphs, glyphs);
	free(fontFileBuffer);

	// Create a texture from the glyph atlas.
	glyphAtlasTexture = SDL_CreateTextureFromSurface(renderer, glyphAtlasSurface);

	if (!glyphAtlasTexture)
	{
		ShowSdlError();
		return 1;
	}

	SDL_SetTextureBlendMode(glyphAtlasTexture, SDL_BLENDMODE_BLEND);

	for (;;)
	{
		SDL_Event e;

		if (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				break;
		}

		SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
		SDL_RenderClear(renderer);
		
		SDL_SetTextureColorMod(glyphAtlasTexture, 128, 0, 0);
		TextPrintf(50, 50, "Testing (red)");

		SDL_SetTextureColorMod(glyphAtlasTexture, 0, 0, 192);
		TextPrintf(50, 100, "Another test (blue)");

		SDL_RenderPresent(renderer);
	}

	return 0;
}
