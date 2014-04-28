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

wzSize ButtonAutosize(struct wzWidget *widget);
void ButtonDraw(struct wzWidget *widget);

Context::Context()
{
	context_ = wz_context_create();

	buttonBehavior_.base.autosize = ButtonAutosize;
	buttonBehavior_.base.draw = ButtonDraw;
	wz_context_set_widget_behavior(context_, WZ_TYPE_BUTTON, (const wzWidgetBehavior *)&buttonBehavior_);
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

void Window::draw()
{
	wz_window_draw(window_);
}

//------------------------------------------------------------------------------

Button::Button(Window *window, const char *label)
{
	strcpy(label_, label);
	button_ = wz_button_create(window->get());
	wz_widget_set_metadata((struct wzWidget *)button_, this);
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
	SDL_SetRenderDrawColor(g_renderer, 218, 218, 218, 255);
	SDL_RenderFillRect(g_renderer, (SDL_Rect *)&rect);

	// Border.
	SDL_SetRenderDrawColor(g_renderer, 112, 112, 112, 255);
	SDL_RenderDrawRect(g_renderer, (SDL_Rect *)&rect);

	// Label.
	TextPrintf(rect.x + rect.w / 2, rect.y + rect.h / 2, TA_CENTER, TA_CENTER, 0, 0, 0, label_);
}

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
