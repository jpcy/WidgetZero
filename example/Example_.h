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
#pragma once

#include <stdint.h>

struct SDL_Renderer;

extern SDL_Renderer *g_renderer;

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

void ErrorAndExit(const char *format, ...);
void TextPrintf(int x, int y, TextAlignment halign, TextAlignment valign, uint8_t r, uint8_t g, uint8_t b, const char *format, ...);
void MeasureText(const char *text, int *width, int *height);
