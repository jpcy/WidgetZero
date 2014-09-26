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
#ifndef WZ_RENDERER_H
#define WZ_RENDERER_H

#include <wz.h>

#define WZ_NANOVG_MAX_PATH 256
#define WZ_NANOVG_MAX_IMAGES 1024
#define WZ_NANOVG_MAX_ERROR_MESSAGE 1024

#define WZ_STYLE_TEXT_COLOR nvgRGBf(0.8f, 0.8f, 0.8f)
#define WZ_STYLE_DARK_BORDER_COLOR nvgRGB(25, 25, 25)
#define WZ_STYLE_LIGHT_BORDER_COLOR nvgRGBf(0.8f, 0.8f, 0.8f)
#define WZ_STYLE_HOVER_COLOR nvgRGBf(0.5f, 0.76f, 0.9f)
#define WZ_STYLE_CORNER_RADIUS 4.0f

typedef struct
{
	int handle;
	char filename[WZ_NANOVG_MAX_PATH];
}
wzImage;

struct wzRenderer
{
	wzNanoVgGlDestroy destroy;
	struct NVGcontext *vg;
	wzImage images[WZ_NANOVG_MAX_IMAGES];
	int nImages;
	char fontDirectory[WZ_NANOVG_MAX_PATH];
	float defaultFontSize;
};

#endif // WZ_RENDERER_H
