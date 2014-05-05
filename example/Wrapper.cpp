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
#include <stdint.h>
#include <string.h>
#include <SDL.h>
#include "Wrapper.h"
#include "Example.h"

Context::Context()
{
	context_ = wz_context_create();
}

Context::~Context()
{
	wz_context_destroy(context_);
}

//------------------------------------------------------------------------------

Desktop::Desktop(Context *context)
{
	desktop_ = wz_desktop_create(context->get());
}

Desktop::~Desktop()
{
	wz_widget_destroy((struct wzWidget *)desktop_);
}

void Desktop::mouseMove(int x, int y, int dx, int dy)
{
	wz_desktop_mouse_move(desktop_, x, y, dx, dy);
}

void Desktop::mouseButtonDown(int button, int x, int y)
{
	wz_desktop_mouse_button_down(desktop_, button, x, y);
}

void Desktop::mouseButtonUp(int button, int x, int y)
{
	wz_desktop_mouse_button_up(desktop_, button, x, y);
}

void Desktop::draw()
{
	wz_desktop_draw(desktop_);
}

//------------------------------------------------------------------------------

void WindowDraw(struct wzWidget *widget)
{
	Window *window = (Window *)wz_widget_get_metadata(widget);
	window->draw();
}

Window::Window(Widget *parent, char *title)
{
	struct wzWidget *widget;
	wzSize size;

	strcpy(title_, title);
	window_ = wz_window_create(parent->getContext());
	widget = (struct wzWidget *)window_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, WindowDraw);
	wz_window_set_border_size(window_, 4);

	// Calculate header height based on label text plus padding.
	MeasureText(title_, &size.w, &size.h);
	size.h += 6;
	wz_window_set_header_height(window_, size.h);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void Window::setRect(int x, int y, int w, int h)
{
	wzRect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	wz_widget_set_rect((struct wzWidget *)window_, rect);
}

void Window::draw()
{
	wzRect rect = wz_widget_get_absolute_rect((struct wzWidget *)window_);
	
	// Background.
	SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&rect);

	// Border.
	const int borderSize = wz_window_get_border_size(window_);
	SDL_SetRenderDrawColor(g_renderer, 128, 128, 128, 255);

	// Border top.
	wzRect borderRect = rect;
	borderRect.h = borderSize;
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&borderRect);

	// Border bottom.
	borderRect.y = rect.y + rect.h - borderSize;
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&borderRect);

	// Border left.
	borderRect = rect;
	borderRect.w = borderSize;
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&borderRect);

	// Border right.
	borderRect.x = rect.x + rect.w - borderSize;
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&borderRect);

	// Header.
	wzRect headerRect = wz_window_get_header_rect(window_);
	SDL_SetRenderDrawColor(g_renderer, 255, 232, 166, 255);
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&headerRect);
	TextPrintf(headerRect.x + 10, headerRect.y + headerRect.h / 2, TA_LEFT, TA_CENTER, 0, 0, 0, title_);
}

//------------------------------------------------------------------------------

void ButtonDraw(struct wzWidget *widget)
{
	Button *button = (Button *)wz_widget_get_metadata(widget);
	button->draw();
}

Button::Button(Widget *parent, const char *label)
{
	struct wzWidget *widget;
	wzSize size;

	strcpy(label_, label);
	button_ = wz_button_create(parent->getContext());
	widget = (struct wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, ButtonDraw);

	// Calculate size based on label text plus padding.
	MeasureText(label_, &size.w, &size.h);
	size.w += 16;
	size.h += 8;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

Button::Button(wzButton *button, const char *label)
{
	strcpy(label_, label);
	button_ = button;
	struct wzWidget *widget = (struct wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, ButtonDraw);
}

void Button::setPosition(int x, int y)
{
	wz_widget_set_position_args((struct wzWidget *)button_, x, y);
}

wzRect Button::getRect()
{
	return wz_widget_get_absolute_rect((struct wzWidget *)button_);
}

void Button::draw()
{
	wzRect rect = wz_widget_get_absolute_rect((struct wzWidget *)button_);
	const bool hover = wz_widget_get_hover((struct wzWidget *)button_);
	const bool pressed = wz_button_is_pressed(button_);

	// Background.
	if (pressed && hover)
	{
		SDL_SetRenderDrawColor(g_renderer, 127, 194, 229, 255);
	}
	else if (hover)
	{
		SDL_SetRenderDrawColor(g_renderer, 188, 229, 252, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(g_renderer, 218, 218, 218, 255);
	}

	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&rect);

	// Border.
	if (pressed && hover)
	{
		SDL_SetRenderDrawColor(g_renderer, 44, 98, 139, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(g_renderer, 112, 112, 112, 255);
	}

	SDL_RenderDrawRect(g_renderer, (SDL_Rect *)&rect);

	// Label.
	TextPrintf(rect.x + rect.w / 2, rect.y + rect.h / 2, TA_CENTER, TA_CENTER, 0, 0, 0, label_);
}

//------------------------------------------------------------------------------

void CheckboxDraw(struct wzWidget *widget)
{
	Checkbox *checkbox = (Checkbox *)wz_widget_get_metadata(widget);
	checkbox->draw();
}

Checkbox::Checkbox(Widget *parent, const char *label)
{
	struct wzWidget *widget;
	wzSize size;

	strcpy(label_, label);
	button_ = wz_button_create(parent->getContext());
	widget = (struct wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, CheckboxDraw);
	wz_button_set_toggle_behavior(button_, true);

	// Calculate size.
	MeasureText(label_, &size.w, &size.h);
	size.w += boxSize + boxRightMargin;
	size.w += 16;
	size.h += 8;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void Checkbox::setPosition(int x, int y)
{
	wz_widget_set_position_args((struct wzWidget *)button_, x, y);
}

void Checkbox::draw()
{
	wzRect rect = wz_widget_get_absolute_rect((struct wzWidget *)button_);
	const bool hover = wz_widget_get_hover((struct wzWidget *)button_);

	// Box.
	SDL_Rect boxRect;
	boxRect.x = rect.x;
	boxRect.y = (int)(rect.y + rect.h / 2.0f - boxSize / 2.0f);
	boxRect.w = boxSize;
	boxRect.h = boxSize;

	// Box background.
	if (wz_button_is_pressed(button_) && hover)
	{
		SDL_SetRenderDrawColor(g_renderer, 127, 194, 229, 255);
		SDL_RenderFillRect(g_renderer, &boxRect);
	}
	else if (hover)
	{
		SDL_SetRenderDrawColor(g_renderer, 188, 229, 252, 255);
		SDL_RenderFillRect(g_renderer, &boxRect);
	}

	// Box border.
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(g_renderer, &boxRect);

	// Box checkmark.
	if (wz_button_is_set(button_))
	{
		boxRect.x = rect.x + 4;
		boxRect.y = (int)(rect.y + rect.h / 2.0f - boxSize / 2.0f) + 4;
		boxRect.w = boxSize - 8;
		boxRect.h = boxSize - 8;
		SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(g_renderer, &boxRect);
	}

	// Label.
	TextPrintf(rect.x + boxSize + boxRightMargin, rect.y + rect.h / 2, TA_LEFT, TA_CENTER, 0, 0, 0, label_);
}

//------------------------------------------------------------------------------

void GroupBoxDraw(struct wzWidget *widget)
{
	GroupBox *groupBox = (GroupBox *)wz_widget_get_metadata(widget);
	groupBox->draw();
}

GroupBox::GroupBox(Widget *parent, const char *label)
{
	struct wzWidget *widget;
	wzSize size;

	strcpy(label_, label);
	groupBox_ = wz_groupbox_create(parent->getContext());
	widget = (struct wzWidget *)groupBox_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, GroupBoxDraw);

	size.w = 200;
	size.h = 200;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void GroupBox::setPosition(int x, int y)
{
	wz_widget_set_position_args((struct wzWidget *)groupBox_, x, y);
}

void GroupBox::draw()
{
	const int textLeftMargin = 20;
	const int textBorderSpacing = 5;

	wzRect rect = wz_widget_get_absolute_rect((struct wzWidget *)groupBox_);
	
	// Background.
	SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&rect);

	// Border - left, bottom, right, top left, top right.
	int textWidth, textHeight;
	MeasureText(label_, &textWidth, &textHeight);
	SDL_SetRenderDrawColor(g_renderer, 98, 135, 157, 255);
	SDL_RenderDrawLine(g_renderer, rect.x, rect.y + textHeight / 2, rect.x, rect.y + rect.h);
	SDL_RenderDrawLine(g_renderer, rect.x, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h);
	SDL_RenderDrawLine(g_renderer, rect.x + rect.w, rect.y + textHeight / 2, rect.x + rect.w, rect.y + rect.h);
	SDL_RenderDrawLine(g_renderer, rect.x, rect.y + textHeight / 2, rect.x + textLeftMargin - textBorderSpacing, rect.y + textHeight / 2);
	SDL_RenderDrawLine(g_renderer, rect.x + textLeftMargin + textWidth + textBorderSpacing * 2, rect.y + textHeight / 2, rect.x + rect.w, rect.y + textHeight / 2);

	// Label.
	TextPrintf(rect.x + textLeftMargin, rect.y, TA_LEFT, TA_TOP, 0, 0, 0, label_);
}

//------------------------------------------------------------------------------

static void ScrollerDraw(struct wzWidget *widget)
{
	Scroller *scroller = (Scroller *)wz_widget_get_metadata(widget);
	scroller->draw();
}

Scroller::Scroller(Widget *parent, wzScrollerType type, int value, int stepValue, int maxValue)
{
	scroller_ = wz_scroller_create(parent->getContext(), type);
	wz_scroller_set_max_value(scroller_, maxValue);
	wz_scroller_set_value(scroller_, value);
	wz_scroller_set_step_value(scroller_, stepValue);
	struct wzWidget *widget = (struct wzWidget *)scroller_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, ScrollerDraw);
	wz_scroller_set_nub_size(scroller_, 16);

	decrementButton.reset(new Button(wz_scroller_get_decrement_button(scroller_), "-"));
	incrementButton.reset(new Button(wz_scroller_get_increment_button(scroller_), "+"));

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wzSize buttonSize;
	buttonSize.w = 16;
	buttonSize.h = 16;
	wz_widget_set_size((struct wzWidget *)wz_scroller_get_decrement_button(scroller_), buttonSize);
	wz_widget_set_size((struct wzWidget *)wz_scroller_get_increment_button(scroller_), buttonSize);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

Scroller::Scroller(wzScroller *scroller)
{
	scroller_ = scroller;
	struct wzWidget *widget = (struct wzWidget *)scroller_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, ScrollerDraw);
	wz_scroller_set_nub_size(scroller_, 16);

	decrementButton.reset(new Button(wz_scroller_get_decrement_button(scroller_), "-"));
	incrementButton.reset(new Button(wz_scroller_get_increment_button(scroller_), "+"));

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wzSize buttonSize;
	buttonSize.w = 16;
	buttonSize.h = 16;
	wz_widget_set_size((struct wzWidget *)wz_scroller_get_decrement_button(scroller_), buttonSize);
	wz_widget_set_size((struct wzWidget *)wz_scroller_get_increment_button(scroller_), buttonSize);
}

void Scroller::setRect(int x, int y, int w, int h)
{
	wzRect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	wz_widget_set_rect((struct wzWidget *)scroller_, rect);
}

void Scroller::draw()
{
	wzRect rect = wz_widget_get_absolute_rect((struct wzWidget *)scroller_);
	
	// Background.
	SDL_SetRenderDrawColor(g_renderer, 192, 192, 192, 255);
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&rect);

	// Nub.
	wzRect nubRect = wz_scroller_get_nub_rect(scroller_);
	SDL_SetRenderDrawColor(g_renderer, 218, 218, 218, 255);
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&nubRect);
	SDL_SetRenderDrawColor(g_renderer, 112, 112, 112, 255);
	SDL_RenderDrawRect(g_renderer, (SDL_Rect *)&nubRect);
}

int Scroller::getValue() const
{
	return wz_scroller_get_value(scroller_);
}

//------------------------------------------------------------------------------

static void ListDraw(struct wzWidget *widget)
{
	List *list = (List *)wz_widget_get_metadata(widget);
	list->draw();
}

List::List(Widget *parent, const char **items, int nItems)
{
	list_ = wz_list_create(parent->getContext());
	wz_list_set_num_items(list_, nItems);
	wz_list_set_item_height(list_, itemHeight);
	struct wzWidget *widget = (struct wzWidget *)list_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, ListDraw);
	wz_widget_add_child_widget(parent->getWidget(), widget);
	items_ = items;
	scroller_.reset(new Scroller(wz_list_get_scroller(list_)));

	wzSize scrollerSize;
	scrollerSize.w = 16;
	scrollerSize.h = 0;
	wz_widget_set_size(scroller_->getWidget(), scrollerSize);
}

void List::setRect(int x, int y, int w, int h)
{
	wzRect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	wz_widget_set_rect((struct wzWidget *)list_, rect);

	wzRect itemsRect;
	itemsRect.x = x + itemsMargin;
	itemsRect.y = y + itemsMargin;
	itemsRect.w = w - itemsMargin * 2;
	itemsRect.h = h - itemsMargin * 2;
	wz_list_set_items_rect(list_, itemsRect);
}

void List::draw()
{
	wzRect rect = wz_widget_get_absolute_rect((struct wzWidget *)list_);
	
	// Background.
	SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&rect);

	// Border.
	SDL_SetRenderDrawColor(g_renderer, 130, 135, 144, 255);
	SDL_RenderDrawRect(g_renderer, (SDL_Rect *)&rect);

	// Items.
	int nItems = wz_list_get_num_items(list_);
	wzRect itemsRect = wz_list_get_absolute_items_rect(list_);
	int y = itemsRect.y;

	SDL_Rect oldClipRect;
	SDL_RenderGetClipRect(g_renderer, &oldClipRect);
	SDL_RenderSetClipRect(g_renderer, (SDL_Rect *)&itemsRect);

	for (int i = wz_list_get_first_item(list_); i < nItems; i++)
	{
		// Outside widget?
		if (y > itemsRect.y + itemsRect.h)
			break;

		SDL_Rect itemRect;
		itemRect.x = itemsRect.x;
		itemRect.y = y;
		itemRect.w = itemsRect.w;
		itemRect.h = itemHeight;

		if (i == wz_list_get_pressed_item(list_) || i == wz_list_get_selected_item(list_))
		{
			SDL_SetRenderDrawColor(g_renderer, 127, 194, 229, 255);
			SDL_RenderFillRect(g_renderer, &itemRect);
		}
		else if (i == wz_list_get_hovered_item(list_))
		{
			SDL_SetRenderDrawColor(g_renderer, 188, 229, 252, 255);
			SDL_RenderFillRect(g_renderer, &itemRect);
		}

		TextPrintf(itemsRect.x + itemLeftPadding, y + itemHeight / 2, TA_LEFT, TA_CENTER, 0, 0, 0, items_[i]);
		y += itemHeight;
	}

	SDL_RenderSetClipRect(g_renderer, &oldClipRect);
}
