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

/*
================================================================================

MENU BAR BUTTON

================================================================================
*/

static Size wz_menu_bar_button_measure(struct WidgetImpl *widget)
{
	Size size;
	struct MenuBarButtonImpl *button = (struct MenuBarButtonImpl *)widget;

	widget->measureText(button->label.c_str(), 0, &size.w, &size.h);
	size.w += 12;
	return size;
}

static void wz_menu_bar_button_draw(struct WidgetImpl *widget, Rect clip)
{
	struct MenuBarButtonImpl *button = (struct MenuBarButtonImpl *)widget;
	NVGRenderer *r = (NVGRenderer *)widget->renderer;
	struct NVGcontext *vg = r->getContext();
	const Rect rect = widget->getAbsoluteRect();

	nvgSave(vg);
	r->clipToRect(clip);

	if (button->isPressed())
	{
		r->drawFilledRect(rect, WZ_SKIN_MENU_BAR_SET_COLOR);
	}

	if (widget->hover)
	{
		r->drawRect(rect, WZ_SKIN_MENU_BAR_BORDER_HOVER_COLOR);
	}

	r->print(rect.x + rect.w / 2, rect.y + rect.h / 2, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, widget->fontFace, widget->fontSize, WZ_SKIN_MENU_BAR_TEXT_COLOR, button->label.c_str(), 0);

	nvgRestore(vg);
}

static void wz_menu_bar_button_mouse_button_down(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct MenuBarButtonImpl *button = (struct MenuBarButtonImpl *)widget;
	WZ_ASSERT(button);

	if (mouseButton == 1)
	{
		button->isPressed_ = true;

		// Lock input to the menu bar, not this button.
		widget->mainWindow->pushLockInputWidget(button->menuBar);
	}
}

static void wz_menu_bar_button_mouse_button_up(struct WidgetImpl *widget, int mouseButton, int mouseX, int mouseY)
{
	struct MenuBarButtonImpl *button = (struct MenuBarButtonImpl *)widget;
	WZ_ASSERT(button);

	if (mouseButton == 1 && button->isPressed_)
	{
		button->isPressed_ = false;
		widget->mainWindow->popLockInputWidget(widget->parent->parent);
	}
}

static void wz_menu_bar_button_mouse_hover_on(struct WidgetImpl *widget)
{
	struct MenuBarButtonImpl *button = (struct MenuBarButtonImpl *)widget;
	WZ_ASSERT(button);

	// See if any of the other buttons in the menubar are pressed.
	// If one is pressed, unpress it and press this one instead.
	for (size_t i = 0; i < button->menuBar->children[0]->children.size(); i++)
	{
		struct MenuBarButtonImpl *otherButton = (struct MenuBarButtonImpl *)button->menuBar->children[0]->children[i];

		if (otherButton == button || !otherButton->isPressed_)
			continue;

		otherButton->isPressed_ = false;
		button->isPressed_ = true;
		return;
	}
}

MenuBarButtonImpl::MenuBarButtonImpl()
{
	type = WZ_TYPE_BUTTON;
	isPressed_ = isSet = false;
	menuBar = NULL;
}

void MenuBarButtonImpl::setLabel(const char *label)
{
	this->label = label;
	resizeToMeasured();
}

const char *MenuBarButtonImpl::getLabel() const
{
	return label.c_str();
}

bool MenuBarButtonImpl::isPressed() const
{
	return isPressed_;
}

static struct MenuBarButtonImpl *wz_menu_bar_button_create(struct MenuBarImpl *menuBar)
{
	struct MenuBarButtonImpl *button = new struct MenuBarButtonImpl;
	button->vtable.measure = wz_menu_bar_button_measure;
	button->vtable.draw = wz_menu_bar_button_draw;
	button->vtable.mouse_button_down = wz_menu_bar_button_mouse_button_down;
	button->vtable.mouse_button_up = wz_menu_bar_button_mouse_button_up;
	button->vtable.mouse_hover_on = wz_menu_bar_button_mouse_hover_on;
	button->menuBar = menuBar;
	return button;
}

/*
================================================================================

MENU BAR

================================================================================
*/

static void wz_menu_bar_draw(struct WidgetImpl *widget, Rect clip)
{
	NVGRenderer *r = (NVGRenderer *)widget->renderer;
	struct NVGcontext *vg = r->getContext();

	nvgSave(vg);
	r->clipToRect(clip);
	r->drawFilledRect(widget->getAbsoluteRect(), WZ_SKIN_MENU_BAR_BG_COLOR);
	nvgRestore(vg);
}

static void wz_menu_bar_renderer_changed(struct WidgetImpl *widget)
{
	WZ_ASSERT(widget);
	widget->setHeight(widget->getLineHeight() + WZ_SKIN_MENU_BAR_PADDING);
}

MenuBarImpl::MenuBarImpl()
{
	type = WZ_TYPE_MENU_BAR;

	vtable.draw = wz_menu_bar_draw;
	vtable.renderer_changed = wz_menu_bar_renderer_changed;

	layout = new StackLayoutImpl(WZ_STACK_LAYOUT_HORIZONTAL, 0);
	layout->setStretch(WZ_STRETCH);
	addChildWidget(layout);
}

struct MenuBarButtonImpl *MenuBarImpl::createButton()
{
	struct MenuBarButtonImpl *button = wz_menu_bar_button_create(this);
	button->setStretch(WZ_STRETCH_HEIGHT);
	layout->add(button);
	return button;
}

} // namespace wz
