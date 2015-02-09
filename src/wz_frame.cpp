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
#include "wz_internal.h"
#pragma hdrstop

namespace wz {

FrameImpl::FrameImpl()
{
	type = WZ_TYPE_FRAME;
}

void FrameImpl::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->impl->type == WZ_TYPE_MAIN_WINDOW || widget->impl->type == WZ_TYPE_WINDOW)
		return;

	addChildWidget(widget);
}

void FrameImpl::add(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	addChildWidget(widget);
}

void FrameImpl::remove(Widget *widget)
{
	WZ_ASSERT(widget);
	removeChildWidget(widget);
}

void FrameImpl::remove(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	removeChildWidget(widget);
}

/*
================================================================================

PUBLIC INTERFACE

================================================================================
*/

Frame::Frame()
{
	impl.reset(new FrameImpl);
	impl->setSize(200, 200);
}

Frame::~Frame()
{
}

Widget *Frame::add(Widget *widget)
{
	((FrameImpl *)impl.get())->add(widget);
	return widget;
}

void Frame::remove(Widget *widget)
{
	((FrameImpl *)impl.get())->remove(widget);
}

} // namespace wz
