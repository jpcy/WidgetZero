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

class GroupBoxContent : public Widget
{
public:
	virtual Size measure()
	{
		Size s;

		for (size_t i = 0; i < children_.size(); i++)
		{
			s += children_[i]->measure();
		}

		return s;
	}
};

GroupBox::GroupBox(const std::string &label) : label_(label)
{
	type_ = WidgetType::GroupBox;

	// Create content widget.
	content_ = new GroupBoxContent;
	content_->setStretch(Stretch::All);
	addChildWidget(content_);
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

	if (widget->getType() == WidgetType::MainWindow || widget->getType() == WidgetType::Window)
		return;

	content_->addChildWidget(widget);
}

void GroupBox::remove(Widget *widget)
{
	WZ_ASSERT(widget);
	content_->removeChildWidget(widget);
}

Size GroupBox::measureContent()
{
	// Content margin.
	const Border m = content_->getMargin();
	Size s(m.left + m.right, m.top + m.bottom);

	// Content itself.
	s += content_->measure();

	return s;
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
	Border margin = renderer_->getGroupBoxMargin(this);

	if (!label_.empty())
	{
		margin.top += getLineHeight();
	}

	content_->setMargin(margin);
}

} // namespace wz
