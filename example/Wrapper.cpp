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

Window::Window(Context *context)
{
	window_ = wz_window_create(context->get());
}

Window::~Window()
{
	wz_window_destroy(window_);
}

void Window::mouseMove(int x, int y)
{
	wz_window_mouse_move(window_, x, y);
}

void Window::mouseButtonDown(int button, int x, int y)
{
	wz_window_mouse_button_down(window_, button, x, y);
}

void Window::mouseButtonUp(int button, int x, int y)
{
	wz_window_mouse_button_up(window_, button, x, y);
}

void Window::draw()
{
	wz_window_draw(window_);
}

//------------------------------------------------------------------------------

wzSize ButtonAutosize(struct wzWidget *widget)
{
	Button *button = (Button *)wz_widget_get_metadata(widget);
	return button->autosize();
}

void ButtonDraw(struct wzWidget *widget)
{
	Button *button = (Button *)wz_widget_get_metadata(widget);
	button->draw();
}

Button::Button(Window *window, const char *label)
{
	struct wzWidget *widget;

	strcpy(label_, label);
	button_ = wz_button_create(window->get());
	widget = (struct wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_autosize_function(widget, ButtonAutosize);
	wz_widget_set_draw_function(widget, ButtonDraw);
}

void Button::setPosition(int x, int y)
{
	wz_widget_set_position((struct wzWidget *)button_, x, y);
}

wzSize Button::autosize()
{
	wzSize size;
	MeasureText(label_, &size.w, &size.h);

	// Add padding.
	size.w += 16;
	size.h += 8;
	return size;
}

void Button::draw()
{
	wzRect rect = wz_widget_get_rect((struct wzWidget *)button_);
	
	// Background.
	if (wz_button_is_pressed(button_))
	{
		SDL_SetRenderDrawColor(g_renderer, 127, 194, 229, 255);
	}
	else if (wz_widget_get_hover((struct wzWidget *)button_))
	{
		SDL_SetRenderDrawColor(g_renderer, 188, 229, 252, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(g_renderer, 218, 218, 218, 255);
	}

	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&rect);

	// Border.
	if (wz_button_is_pressed(button_))
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

wzSize GroupBoxAutosize(struct wzWidget *widget)
{
	GroupBox *groupBox = (GroupBox *)wz_widget_get_metadata(widget);
	return groupBox->autosize();
}

void GroupBoxDraw(struct wzWidget *widget)
{
	GroupBox *groupBox = (GroupBox *)wz_widget_get_metadata(widget);
	groupBox->draw();
}

GroupBox::GroupBox(Window *window, const char *label)
{
	struct wzWidget *widget;

	strcpy(label_, label);
	groupBox_ = wz_groupbox_create(window->get());
	widget = (struct wzWidget *)groupBox_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_autosize_function(widget, GroupBoxAutosize);
	wz_widget_set_draw_function(widget, GroupBoxDraw);
}

void GroupBox::setPosition(int x, int y)
{
	wz_widget_set_position((struct wzWidget *)groupBox_, x, y);
}

wzSize GroupBox::autosize()
{
	wzSize size;
	size.w = 200;
	size.h = 200;
	return size;
}

void GroupBox::draw()
{
	const int textLeftMargin = 20;
	const int textBorderSpacing = 5;

	wzRect rect = wz_widget_get_rect((struct wzWidget *)groupBox_);
	
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
