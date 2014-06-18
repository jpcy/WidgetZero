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
#include "wz_arr.h"
#include "wz_widget.h"

struct wzVerticalStackLayout
{
	struct wzWidget base;
};

static void wz_vertical_stack_layout_set_rect(struct wzWidget *widget, wzRect rect)
{
	int n, h, y, i;

	assert(widget);
	widget->rect = rect;
	n = wz_arr_len(widget->children);

	// Calculate the height left after subracting all child widget top and bottom margins.
	h = rect.h;

	for (i = 0; i < n; i++)
	{
		h -= widget->children[i]->margin.top + widget->children[i]->margin.bottom;
	}

	// Layout the children.
	y = 0;

	for (i = 0; i < n; i++)
	{
		struct wzWidget *child;
		wzRect childRect;

		child = widget->children[i];
		y += child->margin.top;
		childRect.x = child->margin.left;
		childRect.y = y;
		childRect.w = rect.w - (child->margin.left + child->margin.right);
		childRect.h = (int)(h / (float)n);
		wz_widget_set_rect(widget->children[i], childRect);
		y += childRect.h + child->margin.bottom;
	}
}

struct wzVerticalStackLayout *wz_vertical_stack_layout_create(struct wzDesktop *desktop)
{
	struct wzVerticalStackLayout *layout;

	assert(desktop);
	layout = (struct wzVerticalStackLayout *)malloc(sizeof(struct wzVerticalStackLayout));
	memset(layout, 0, sizeof(struct wzVerticalStackLayout));
	layout->base.type = WZ_TYPE_VERTICAL_STACK_LAYOUT;
	layout->base.desktop = desktop;
	layout->base.vtable.set_rect = wz_vertical_stack_layout_set_rect;
	return layout;
}
