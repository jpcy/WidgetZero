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

static const char *listData[17] =
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

class GUI
{
public:
	GUI(int windowWidth, int windowHeight, wzRenderer *renderer)
	{
		desktop.reset(new wz::Desktop(renderer));
		desktop->setSize(windowWidth, windowHeight);

		button.reset(new wz::Button(desktop.get(), "Test Button"));
		button->setPosition(100, 100);

		wzRect buttonRect = button->getRect();
		checkbox.reset(new wz::Checkbox(desktop.get(), "Toggle me!"));
		checkbox->setPosition(buttonRect.x, buttonRect.y + buttonRect.h + 16);

		groupBox.reset(new wz::GroupBox(desktop.get(), "Test GroupBox"));
		groupBox->setPosition(100, 300);

		scroller.reset(new wz::Scroller(desktop.get(), WZ_SCROLLER_VERTICAL, 20, 10, 100));
		scroller->setRect(300, 50, 16, 200);

		label.reset(new wz::Label(desktop.get()));
		label->setText("Label test");
		label->setTextColor(255, 128, 128);
		label->setPosition(350, 50);

		scrollerHorizontal.reset(new wz::Scroller(desktop.get(), WZ_SCROLLER_HORIZONTAL, 50, 10, 100));
		scrollerHorizontal->setRect(500, 50, 200, 16);

		list.reset(new wz::List(desktop.get(), listData, 17));
		list->setRect(400, 300, 150, 150);

		tabbed.reset(new wz::Tabbed(desktop.get()));
		tabbed->setRect(350, 500, 200, 250);
		wz::TabPage *firstTabPage = tabbed->addTab("Tab 1");
		wz::TabPage *secondTabPage = tabbed->addTab("Another Tab");
		tabbed->addTab("TabTabTab");

		secondTabPageButton.reset(new wz::Button(secondTabPage, "Button Button Button"));
		secondTabPageButton->setPosition(10, 10);

		combo.reset(new wz::Combo(desktop.get(), listData, 17));
		combo->setRect(800, 50, 150, 20);

		combo2.reset(new wz::Combo(firstTabPage, listData, 17));
		combo2->setRect(10, 10, 150, 20);

		childWindow.reset(new wz::Window(desktop.get(), "Test Window"));
		childWindow->setRect(650, 100, 300, 300);

		childWindowButton.reset(new wz::Button(childWindow.get(), "Another Button"));
		childWindowButton->setPosition(20, 20);

		childCombo.reset(new wz::Combo(childWindow.get(), listData, 17));
		childCombo->setRect(20, 70, 150, 20);

		childWindow2.reset(new wz::Window(desktop.get(), "Window with a long title"));
		childWindow2->setRect(590, 500, 200, 200);

		childWindow3.reset(new wz::Window(desktop.get(), "Window 3"));
		childWindow3->setRect(800, 500, 200, 200);

		childList.reset(new wz::List(childWindow3.get(), listData, 17));
		childList->setMargin(8);
		childList->setAutosize(WZ_AUTOSIZE_WIDTH | WZ_AUTOSIZE_HEIGHT);
	}

	std::auto_ptr<wz::Desktop> desktop;
	std::auto_ptr<wz::Button> button;
	std::auto_ptr<wz::Checkbox> checkbox;
	std::auto_ptr<wz::GroupBox> groupBox;
	std::auto_ptr<wz::Scroller> scroller;
	std::auto_ptr<wz::Label> label;
	std::auto_ptr<wz::Scroller> scrollerHorizontal;
	std::auto_ptr<wz::List> list;
	std::auto_ptr<wz::Tabbed> tabbed;
	std::auto_ptr<wz::Button> secondTabPageButton;
	std::auto_ptr<wz::Combo> combo;
	std::auto_ptr<wz::Combo> combo2;
	std::auto_ptr<wz::Window> childWindow;
	std::auto_ptr<wz::Button> childWindowButton;
	std::auto_ptr<wz::List> childList;
	std::auto_ptr<wz::Combo> childCombo;
	std::auto_ptr<wz::Window> childWindow2;
	std::auto_ptr<wz::Window> childWindow3;
};

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
	wzRenderer *renderer = wzgl_create_renderer();

	if (!renderer)
	{
		ShowError(wzgl_get_error());
		return 1;
	}

	GUI gui(windowWidth, windowHeight, renderer);
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
				gui.desktop->setSize(e.window.data1, e.window.data2);
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				gui.desktop->mouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				gui.desktop->mouseButtonDown(e.button.button, e.button.x, e.button.y);
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				gui.desktop->mouseButtonUp(e.button.button, e.button.x, e.button.y);
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				gui.desktop->mouseWheelMove(e.wheel.x, e.wheel.y);
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
			gui.desktop->draw();
			SDL_GL_SwapWindow(window);
			SDL_SetCursor(cursors[wz_desktop_get_cursor((wzDesktop *)gui.desktop->getWidget())]);
			accumulatedTime -= frameTime;
		}
	}

	wzgl_destroy_renderer(renderer);
	return 0;
}
