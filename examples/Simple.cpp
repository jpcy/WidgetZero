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

#ifdef _MSC_VER
#include <gl2.h>
#else
#include <GL/gl.h>
#endif

#include <SDL.h>
#include <wz.h>

#include <wz.h>
#include <wz_renderer_nanovg.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // "nonstandard extension used : nameless struct/union"
#pragma warning(disable : 4244) // "conversion from 'int' to 'float', possible loss of data"
#endif

#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

SDL_Window *sdlWindow;

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

	SDL_GLContext glContext = SDL_GL_CreateContext(sdlWindow);

	if (!glContext)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

#ifdef _MSC_VER
	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		ShowError("ogl_LoadFunctions failed");
		return 1;
	}
#endif

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	return 0;
}

int main(int, char **)
{
	// SDL init - create a window and GL context.
	int errorCode = InitializeSDL();

	if (errorCode)
		return errorCode;

	// Create the renderer.
	wz::NVGRenderer *renderer = new wz::NVGRenderer(nvgCreateGL2, nvgDeleteGL2, 0, "../examples/data", "DejaVuSans", 16.0f);

	if (renderer->getError())
	{
		ShowError(renderer->getError());
		return 1;
	}

	// Get the screen clear color from the renderer (optional).
	const wz::Color clearColor = renderer->getClearColor();
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

	// Create the main window.
	wz::MainWindow *mainWindow = new wz::MainWindow(renderer);
	mainWindow->setSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Create a button.
	wz::Button *button = new wz::Button("Click me!");
	button->setSize(200, 100);
	button->setAlign(wz::Align::Center | wz::Align::Middle);
	button->setFontSize(24);
	mainWindow->add(button);

	for (;;)
	{
		SDL_Event e;
		SDL_WaitEvent(&e);

		if (e.type == SDL_QUIT)
		{
			break;
		}
		else if (e.type == SDL_MOUSEMOTION)
		{
			mainWindow->mouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			mainWindow->mouseButtonDown(e.button.button, e.button.x, e.button.y);
		}
		else if (e.type == SDL_MOUSEBUTTONUP)
		{
			mainWindow->mouseButtonUp(e.button.button, e.button.x, e.button.y);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		mainWindow->drawFrame();
		SDL_GL_SwapWindow(sdlWindow);
	}

	delete renderer;
	return 0;
}
