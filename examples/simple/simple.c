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
#include <wz_gl.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 100

static void ShowError(const char *message)
{
	fprintf(stderr, "%s\n", message);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
}

static void DrawButton(struct wzWidget *widget, wzRect clip)
{
	struct wzRenderer *renderer = wz_widget_get_metadata(widget);
	renderer->draw_button(renderer, clip, (struct wzButton *)widget, "Click me!");
}

int main(int argc, char **argv)
{
	SDL_Window *window;
	SDL_GLContext glContext;
	struct wzRenderer *renderer;
	struct wzMainWindow *mainWindow;
	struct wzButton *button;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

	atexit(SDL_Quit);

	window = SDL_CreateWindow("WidgetZero Example - simple", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

	if (!window)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

	glContext = SDL_GL_CreateContext(window);

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

	glClearColor(1, 1, 1, 1);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Create the wzgl renderer.
	renderer = wzgl_create_renderer();

	if (!renderer)
	{
		ShowError(wzgl_get_error());
		return 1;
	}

	// Create the main window.
	mainWindow = wz_main_window_create();
	wz_widget_set_size_args((struct wzWidget *)mainWindow, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Create a button.
	button = wz_button_create();
	wz_widget_set_rect_args((struct wzWidget *)button, WINDOW_WIDTH / 2 - BUTTON_WIDTH / 2, WINDOW_HEIGHT / 2 - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT);
	wz_widget_set_metadata((struct wzWidget *)button, renderer);
	wz_widget_set_draw_callback((struct wzWidget *)button, DrawButton);
	wz_widget_add_child_widget((struct wzWidget *)mainWindow, (struct wzWidget *)button);

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
		renderer->begin_frame(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
		wz_main_window_draw(mainWindow);
		renderer->end_frame(renderer);
		SDL_GL_SwapWindow(window);
	}

	wzgl_destroy_renderer(renderer);
	return 0;
}
