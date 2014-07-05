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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "wz_widget.h"

struct wzFrame
{
	struct wzWidget base;
	struct wzWidget *content;
};

static struct wzWidget *wz_frame_get_content_widget(struct wzWidget *widget)
{
	assert(widget);
	return ((struct wzFrame *)widget)->content;
}

struct wzFrame *wz_frame_create()
{
	struct wzFrame *frame;

	frame = (struct wzFrame *)malloc(sizeof(struct wzFrame));
	memset(frame, 0, sizeof(struct wzFrame));
	frame->base.type = WZ_TYPE_FRAME;
	frame->base.vtable.get_content_widget = wz_frame_get_content_widget;

	// Create content widget.
	frame->content = (struct wzWidget *)malloc(sizeof(struct wzWidget));
	memset(frame->content, 0, sizeof(struct wzWidget));
	frame->content->autosize = WZ_AUTOSIZE;
	wz_widget_add_child_widget_internal((struct wzWidget *)frame, frame->content);

	return frame;
}
