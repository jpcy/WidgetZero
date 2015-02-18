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

/*
================================================================================

MENU BAR BUTTON

================================================================================
*/

MenuBarButton::MenuBarButton(MenuBar *menuBar)
{
	type_ = WZ_TYPE_BUTTON;
	isPressed_ = isSet_ = false;
	menuBar_ = menuBar;
}

void MenuBarButton::setLabel(const char *label)
{
	label_ = label;
	resizeToMeasured();
}

const char *MenuBarButton::getLabel() const
{
	return label_.c_str();
}

bool MenuBarButton::isPressed() const
{
	return isPressed_;
}

void MenuBarButton::onMouseButtonDown(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1)
	{
		isPressed_ = true;

		// Lock input to the menu bar, not this button.
		mainWindow_->pushLockInputWidget(menuBar_);
	}
}

void MenuBarButton::onMouseButtonUp(int mouseButton, int mouseX, int mouseY)
{
	if (mouseButton == 1 && isPressed_)
	{
		isPressed_ = false;
		mainWindow_->popLockInputWidget(parent_->getParent());
	}
}

void MenuBarButton::onMouseHoverOn()
{
	// See if any of the other buttons in the menubar are pressed.
	// If one is pressed, unpress it and press this one instead.
	for (size_t i = 0; i < menuBar_->getChildren()[0]->getChildren().size(); i++)
	{
		MenuBarButton *otherButton = (MenuBarButton *)menuBar_->getChildren()[0]->getChildren()[i];

		if (otherButton == this || !otherButton->isPressed_)
			continue;

		otherButton->isPressed_ = false;
		isPressed_ = true;
		return;
	}
}

void MenuBarButton::draw(Rect clip)
{
	renderer_->drawMenuBarButton(this, clip);
}

Size MenuBarButton::measure()
{
	return renderer_->measureMenuBarButton(this);
}

/*
================================================================================

MENU BAR

================================================================================
*/

MenuBar::MenuBar()
{
	type_ = WZ_TYPE_MENU_BAR;

	layout_ = new StackLayout(WZ_STACK_LAYOUT_HORIZONTAL, 0);
	layout_->setStretch(WZ_STRETCH);
	addChildWidget(layout_);
}

MenuBarButton *MenuBar::createButton()
{
	MenuBarButton *button = new MenuBarButton(this);
	button->setStretch(WZ_STRETCH_HEIGHT);
	layout_->add(button);
	return button;
}

void MenuBar::onRendererChanged()
{
	setHeight(getLineHeight() + WZ_SKIN_MENU_BAR_PADDING);
}

void MenuBar::draw(Rect clip)
{
	renderer_->drawMenuBar(this, clip);
}

Size MenuBar::measure()
{
	return renderer_->measureMenuBar(this);
}

} // namespace wz
