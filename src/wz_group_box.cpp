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
#include "wz.h"
#pragma hdrstop

namespace wz {

GroupBox::GroupBox(const std::string &label) : label_(label)
{
	type_ = WidgetType::GroupBox;
}

void GroupBox::setLabel(const char *label)
{
	label_ = label;

	// Update the margin.
	if (renderer_)
	{
		refreshPadding();
	}
}

const char *GroupBox::getLabel() const
{
	return label_.c_str();
}

void GroupBox::setContent(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->getType() == WidgetType::MainWindow || widget->getType() == WidgetType::Window)
		return;

	if (!children_.empty())
	{
		removeChildWidget(children_[0]);
	}

	addChildWidget(widget);
}

void GroupBox::removeContent(Widget *widget)
{
	WZ_ASSERT(widget);
	removeChildWidget(widget);
}

void GroupBox::onRendererChanged()
{
	refreshPadding();
}

void GroupBox::draw(Rect clip)
{
	renderer_->drawGroupBox(this, clip);
}

Size GroupBox::measure()
{
	Size content(padding_.left + padding_.right, padding_.top + padding_.bottom);

	if (!children_.empty())
	{
		content += children_[0]->getMeasuredSize();
	}

	const Size measured = renderer_->measureGroupBox(this);
	return Size(WZ_MAX(content.w, measured.w), WZ_MAX(content.h, measured.h));
}

void GroupBox::refreshPadding()
{
	Border margin = renderer_->getGroupBoxMargin(this);

	if (!label_.empty())
	{
		margin.top += getLineHeight();
	}

	setPadding(margin);
}

} // namespace wz
