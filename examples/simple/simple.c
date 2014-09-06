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
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <SDL.h>
#include <wz.h>

#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 100

SDL_Window *sdlWindow;
SDL_GLContext glContext;
struct wzRenderer *renderer;
struct wzMainWindow *mainWindow;
struct wzButton *button;

void ShowError(const char *message)
{
	fprintf(stderr, "%s\n", message);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
}

int InitializeSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

	atexit(SDL_Quit);

	sdlWindow = SDL_CreateWindow("WidgetZero Example - simple", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

	if (!sdlWindow)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

	glContext = SDL_GL_CreateContext(sdlWindow);

	if (!glContext)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

	if (glewInit() != GLEW_OK)
	{
		ShowError("GLEW init failed");
		return 1;
	}

	glClearColor(0, 0, 0, 1);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	return 0;
}

int main(int argc, char **argv)
{
	int errorCode;

	// SDL init - create a window and GL context.
	errorCode = InitializeSDL();

	if (errorCode)
		return errorCode;

	// Create the renderer.
	renderer = wz_renderer_create(nvgCreateGL2, nvgDeleteGL2, "../examples/data", "DejaVuSans", 16.0f);

	if (!renderer)
	{
		ShowError(wz_renderer_get_error());
		return 1;
	}

	// Create the main window.
	mainWindow = wz_main_window_create(renderer);
	wz_widget_set_size_args((struct wzWidget *)mainWindow, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Create a button.
	button = wz_button_create();
	wz_widget_set_rect_args((struct wzWidget *)button, WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2, WINDOW_HEIGHT / 2 - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT);
	wz_button_set_label(button, "Click me!");
	wz_widget_set_font_size((struct wzWidget *)button, 24.0f);
	wz_main_window_add(mainWindow, (struct wzWidget *)button);

	for (;;)
	{
		bool quit = false;
		SDL_Event e;

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
				break;
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				wz_main_window_mouse_move(mainWindow, e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				wz_main_window_mouse_button_down(mainWindow, e.button.button, e.button.x, e.button.y);
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				wz_main_window_mouse_button_up(mainWindow, e.button.button, e.button.x, e.button.y);
			}
		}

		if (quit)
			break;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		wz_renderer_begin_frame(renderer, mainWindow);
		wz_main_window_draw(mainWindow);
		wz_renderer_end_frame(renderer);
		SDL_GL_SwapWindow(sdlWindow);
	}

	wz_renderer_destroy(renderer);
	return 0;
}
