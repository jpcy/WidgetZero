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
#include <stdarg.h>
#include <stdint.h>
#include <memory>
#include <GL/glew.h>
#include <SDL.h>
#include <wz_cpp.h>
#include <wz_gl.h>

static const float frameTime = 1000 / 60.0f;

static void ShowError(const char *message)
{
	fprintf(stderr, "%s\n", message);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
}

int main(int argc, char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

	atexit(SDL_Quit);

	SDL_Window *window = SDL_CreateWindow("WidgetZero Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!window)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(window);

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

	int windowWidth, windowHeight;
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);

	// Setup cursors.
	SDL_Cursor *cursors[WZ_NUM_CURSORS];
	cursors[WZ_CURSOR_DEFAULT] = SDL_GetDefaultCursor();
	cursors[WZ_CURSOR_RESIZE_N_S] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursors[WZ_CURSOR_RESIZE_E_W] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	cursors[WZ_CURSOR_RESIZE_NE_SW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	cursors[WZ_CURSOR_RESIZE_NW_SE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);

	// Create the wzgl renderer.
	wzRenderer *wzRenderer = wzgl_create_renderer();

	if (!wzRenderer)
	{
		ShowError(wzgl_get_error());
		return 1;
	}

	// Create wzcpp objects.
	wz::Desktop desktop(wzRenderer);
	desktop.setSize(windowWidth, windowHeight);

	wz::Button button(&desktop, "Test Button");
	button.setPosition(100, 100);

	wzRect buttonRect = button.getRect();
	wz::Checkbox checkbox(&desktop, "Toggle me!");
	checkbox.setPosition(buttonRect.x, buttonRect.y + buttonRect.h + 16);

	wz::GroupBox groupBox(&desktop, "Test GroupBox");
	groupBox.setPosition(100, 300);

	wz::Scroller scroller(&desktop, WZ_SCROLLER_VERTICAL, 20, 10, 100);
	scroller.setRect(300, 50, 16, 200);

	wz::Label scrollerLabel(&desktop);
	scrollerLabel.setTextColor(255, 128, 128);
	scrollerLabel.setPosition(350, 50);

	wz::Scroller scrollerHorizontal(&desktop, WZ_SCROLLER_HORIZONTAL, 50, 10, 100);
	scrollerHorizontal.setRect(500, 50, 200, 16);

	wz::Label scrollerHorizontalLabel(&desktop);
	scrollerHorizontalLabel.setTextColor(128, 128, 255);
	scrollerHorizontalLabel.setPosition(500, 100);

	const char *listData[17] =
	{
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
		"Sunday",
		"One",
		"Two",
		"Three",
		"Four",
		"Five",
		"Six",
		"Seven",
		"Eight",
		"Nine",
		"Ten"
	};

	wz::List list(&desktop, listData, 17);
	list.setRect(400, 300, 150, 150);

	wz::Tabbed tabbed(&desktop);
	tabbed.setRect(350, 500, 200, 250);
	wz::TabPage *firstTabPage = tabbed.addTab("Tab 1");
	wz::TabPage *secondTabPage = tabbed.addTab("Another Tab");
	tabbed.addTab("TabTabTab");

	wz::Button secondTabPageButton(secondTabPage, "Button Button Button");
	secondTabPageButton.setPosition(10, 10);

	wz::Combo combo(&desktop, listData, 17);
	combo.setRect(800, 50, 150, 20);

	wz::Combo combo2(firstTabPage, listData, 17);
	combo2.setRect(10, 10, 150, 20);

	wz::Window childWindow(&desktop, "Test Window");
	childWindow.setRect(650, 100, 300, 300);

	wz::Button childWindowButton(&childWindow, "Another Button");
	childWindowButton.setPosition(20, 20);

	wz::List childList(&childWindow, listData, 17);
	childList.setRect(20, 70, 150, 150);

	wz::Combo childCombo(&childWindow, listData, 17);
	childCombo.setRect(20, 240, 150, 20);

	wz::Window childWindow2(&desktop, "Window #2");
	childWindow2.setRect(800, 500, 200, 200);

	uint32_t lastTime = SDL_GetTicks();
	float accumulatedTime = 0;

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
			else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				glViewport(0, 0, e.window.data1, e.window.data2);
				desktop.setSize(e.window.data1, e.window.data2);
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				desktop.mouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				desktop.mouseButtonDown(e.button.button, e.button.x, e.button.y);
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				desktop.mouseButtonUp(e.button.button, e.button.x, e.button.y);
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				desktop.mouseWheelMove(e.wheel.x, e.wheel.y);
			}
		}

		if (quit)
			break;

		uint32_t currentTime = SDL_GetTicks();
		accumulatedTime += (float)(currentTime - lastTime);
		lastTime = currentTime;

		while (accumulatedTime > frameTime)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			desktop.draw();
			SDL_GL_SwapWindow(window);
			SDL_SetCursor(cursors[wz_desktop_get_cursor((wzDesktop *)desktop.getWidget())]);

			scrollerLabel.setText("Scroll value: %d", scroller.getValue());
			scrollerHorizontalLabel.setText("Scroll value: %d", scrollerHorizontal.getValue());

			accumulatedTime -= frameTime;
		}
	}

	wzgl_destroy_renderer(wzRenderer);
	return 0;
}
