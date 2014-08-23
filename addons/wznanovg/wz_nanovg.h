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
#ifndef _WZ_GL_H_
#define _WZ_GL_H_

#include <GL/glew.h>
#include <nanovg.h>
#include <wz.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NVGcontext;

struct wzRenderer *wz_nanovg_create_renderer(const char *fontDirectory, const char *defaultFontFace, float defaultFontSize);
void wz_nanovg_destroy_renderer(struct wzRenderer *renderer);
const char *wz_nanovg_get_error();
struct NVGcontext *wz_nanovg_get_context(struct wzRenderer *renderer);

int wz_nanovg_create_image(struct wzRenderer *renderer, const char *filename, int *width, int *height);
void wz_nanovg_print_box(struct wzRenderer *renderer, wzRect rect, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength);
void wz_nanovg_print(struct wzRenderer *renderer, int x, int y, int align, const char *fontFace, float fontSize, struct NVGcolor color, const char *text, size_t textLength);
void wz_nanovg_clip_to_rect(struct NVGcontext *vg, wzRect rect);
bool wz_nanovg_clip_to_rect_intersection(struct NVGcontext *vg, wzRect rect1, wzRect rect2);
void wz_nanovg_draw_filled_rect(struct NVGcontext *vg, wzRect rect, struct NVGcolor color);
void wz_nanovg_draw_rect(struct NVGcontext *vg, wzRect rect, struct NVGcolor color);
void wz_nanovg_draw_line(struct NVGcontext *vg, int x1, int y1, int x2, int y2, struct NVGcolor color);
void wz_nanovg_draw_image(struct NVGcontext *vg, wzRect rect, int image);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _WZ_GL_H_
