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
#include <SDL.h>
#include <widgetzero/wzsdl2.h>

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

	SDL_Window *window;
	SDL_Renderer *renderer;

	if (SDL_CreateWindowAndRenderer(1024, 768, SDL_WINDOW_RESIZABLE, &window, &renderer) < 0)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

	SDL_SetWindowTitle(window, "WidgetZero Example");

	// Setup cursors.
	SDL_Cursor *cursors[WZ_NUM_CURSORS];
	cursors[WZ_CURSOR_DEFAULT] = SDL_GetDefaultCursor();
	cursors[WZ_CURSOR_RESIZE_N_S] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursors[WZ_CURSOR_RESIZE_E_W] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	cursors[WZ_CURSOR_RESIZE_NE_SW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	cursors[WZ_CURSOR_RESIZE_NW_SE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);

	// Create wzsdl2 objects.
	wz::Renderer wzRenderer(renderer);

	std::string result = wzRenderer.initialize("../example/data/DejaVuSans.ttf", 16.0f);

	if (!result.empty())
	{
		ShowError(result.c_str());
		return 1;
	}

	wz::Desktop desktop(&wzRenderer);
	int windowWidth, windowHeight;
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);
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

	wz::Combo combo(&desktop, listData, 17);
	combo.setRect(800, 50, 150, 20);

	wz::Combo combo2(&desktop, listData, 17);
	combo2.setRect(20, 600, 150, 20);

	wz::Window childWindow(&desktop, "Test Window");
	childWindow.setRect(650, 100, 300, 300);

	wz::Button childWindowButton(&childWindow, "Another Button");
	childWindowButton.setPosition(20, 20);

	wz::List childList(&childWindow, listData, 17);
	childList.setRect(20, 70, 150, 150);

	wz::Combo childCombo(&childWindow, listData, 17);
	childCombo.setRect(20, 240, 150, 20);

	wz::Window childWindow2(&desktop, "Window #2");
	childWindow2.setRect(650, 500, 200, 200);

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
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderClear(renderer);
			desktop.draw();
			SDL_RenderPresent(renderer);
			SDL_SetCursor(cursors[wz_desktop_get_cursor((wzDesktop *)desktop.getWidget())]);

			scrollerLabel.setText("Scroll value: %d", scroller.getValue());
			scrollerHorizontalLabel.setText("Scroll value: %d", scrollerHorizontal.getValue());

			accumulatedTime -= frameTime;
		}
	}

	return 0;
}
