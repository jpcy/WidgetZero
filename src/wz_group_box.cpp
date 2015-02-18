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
#include "wz_renderer_nanovg.h"

namespace wz {

GroupBox::GroupBox(const std::string &label) : label_(label)
{
	type_ = WZ_TYPE_GROUP_BOX;

	// Create content widget.
	content_ = new Widget;
	content_->setStretch(WZ_STRETCH);
	addChildWidget(content_);

	setSize(200, 200);
}

void GroupBox::setLabel(const char *label)
{
	label_ = label;

	// Update the margin.
	if (renderer_)
	{
		refreshMargin();
	}
}

const char *GroupBox::getLabel() const
{
	return label_.c_str();
}

void GroupBox::add(Widget *widget)
{
	WZ_ASSERT(widget);

	if (widget->getType() == WZ_TYPE_MAIN_WINDOW || widget->getType() == WZ_TYPE_WINDOW)
		return;

	content_->addChildWidget(widget);
}

void GroupBox::remove(Widget *widget)
{
	WZ_ASSERT(widget);
	content_->removeChildWidget(widget);
}

void GroupBox::onRendererChanged()
{
	refreshMargin();
}

void GroupBox::draw(Rect clip)
{
	renderer_->drawGroupBox(this, clip);
}

Size GroupBox::measure()
{
	return renderer_->measureGroupBox(this);
}

void GroupBox::refreshMargin()
{
	Border margin;
	margin.top = margin.bottom = margin.left = margin.right = WZ_SKIN_GROUP_BOX_MARGIN;

	if (!label_.empty())
	{
		margin.top = getLineHeight() + WZ_SKIN_GROUP_BOX_MARGIN;
	}

	content_->setMargin(margin);
}

} // namespace wz
