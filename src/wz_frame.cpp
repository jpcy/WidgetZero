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
#include "wz_pch.h"
#pragma hdrstop

namespace wz {

FrameImpl::FrameImpl()
{
	type = WZ_TYPE_FRAME;
}

Frame::Frame()
{
	impl = new FrameImpl;
	wz_widget_set_size_args(impl, 200, 200);
}

Frame::~Frame()
{
	if (!wz_widget_get_main_window(impl))
	{
		wz_widget_destroy(impl);
	}
}

Widget *Frame::add(Widget *widget)
{
	wz_frame_add((FrameImpl *)impl, widget->impl);
	return widget;
}

void Frame::remove(Widget *widget)
{
	wz_frame_remove((FrameImpl *)impl, widget->impl);
}

void wz_frame_add(struct FrameImpl *frame, struct WidgetImpl *widget)
{
	WZ_ASSERT(frame);
	WZ_ASSERT(widget);

	if (widget->type == WZ_TYPE_MAIN_WINDOW || widget->type == WZ_TYPE_WINDOW)
		return;

	wz_widget_add_child_widget(frame, widget);
}

void wz_frame_remove(struct FrameImpl *frame, struct WidgetImpl *widget)
{
	WZ_ASSERT(frame);
	WZ_ASSERT(widget);
	wz_widget_remove_child_widget(frame, widget);
}

} // namespace wz
