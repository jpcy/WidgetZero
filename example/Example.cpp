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

struct BenchmarkSample
{
	BenchmarkSample() : totalMs_(0), num_(0), averageMs_(0), startTime_(0) {}

	void start()
	{
		startTime_ = SDL_GetPerformanceCounter();
	}

	void end()
	{
		totalMs_ += SDL_GetPerformanceCounter() - startTime_;
		num_++;
	}

	void calculateAverage()
	{
		if (totalMs_ == 0)
		{
			averageMs_ = 0;
		}
		else
		{
			averageMs_ = totalMs_ / (float)SDL_GetPerformanceFrequency() * 1000.0f / (float)num_;
		}

		totalMs_ = 0;
		num_ = 0;
	}

	float getAverage() const { return averageMs_; }

private:
	uint64_t totalMs_;
	int num_;
	float averageMs_;
	uint64_t startTime_;
};

struct Benchmark
{
	BenchmarkSample draw;
	BenchmarkSample frame;
	BenchmarkSample input;
};

static Benchmark benchmark;

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

		radioButtonLayout.reset(new wz::StackLayout(groupBox.get(), wz::StackLayout::Vertical));
		radioButtonLayout->setAutosize(WZ_AUTOSIZE);
		radioButton1.reset(new wz::RadioButton(radioButtonLayout.get(), "Option 1", &radioButtonGroup));
		radioButton2.reset(new wz::RadioButton(radioButtonLayout.get(), "Option 2", &radioButtonGroup));
		radioButton2->setMargin(8, 0, 0, 0);
		radioButton3.reset(new wz::RadioButton(radioButtonLayout.get(), "Option 3", &radioButtonGroup));
		radioButton3->setMargin(8, 0, 0, 0);

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

		combo.reset(new wz::Combo(desktop.get(), listData, 17));
		combo->setRect(800, 50, 150, 20);

		childWindow.reset(new wz::Window(desktop.get(), "Test Window"));
		childWindow->setRect(650, 100, 300, 300);

		childLayout.reset(new wz::StackLayout(childWindow.get(), wz::StackLayout::Vertical));
		childLayout->setMargin(8);
		childLayout->setAutosize(WZ_AUTOSIZE);

		childTextEdit.reset(new wz::TextEdit(childLayout.get(), "Text Box"));
		childTextEdit->setStretch(WZ_STRETCH_HORIZONTAL);

		childWindowButton.reset(new wz::Button(childLayout.get(), "Another Button"));
		childWindowButton->setMargin(8, 0, 0, 0);
		childWindowButton->setStretch(WZ_STRETCH_HORIZONTAL);

		childWindowCheckbox.reset(new wz::Checkbox(childLayout.get(), "Checkbox"));
		childWindowCheckbox->setMargin(8, 0, 0, 0);
		childWindowCheckbox->setAlign(WZ_ALIGN_CENTER);

		childCombo.reset(new wz::Combo(childLayout.get(), listData, 17));
		childCombo->setMargin(8, 0, 0, 0);
		childCombo->setAlign(WZ_ALIGN_RIGHT);

		childWindowButton2.reset(new wz::Button(childLayout.get(), "Yet Another Button"));
		childWindowButton2->setStretch(WZ_STRETCH);
		childWindowButton2->setMargin(8, 0, 0, 0);

		childWindow2.reset(new wz::Window(desktop.get(), "Window with a long title"));
		childWindow2->setRect(590, 500, 200, 200);

		tabbed.reset(new wz::Tabbed(childWindow2.get()));
		tabbed->setMargin(8);
		tabbed->setAutosize(WZ_AUTOSIZE);
		wz::TabPage *firstTabPage = tabbed->addTab("Tab 1");
		wz::TabPage *secondTabPage = tabbed->addTab("Another Tab");
		tabbed->addTab("TabTabTab");

		secondTabPageButton.reset(new wz::Button(secondTabPage, "Button Button Button"));
		secondTabPageButton->setPosition(10, 10);

		combo2.reset(new wz::Combo(firstTabPage, listData, 17));
		combo2->setRect(10, 10, 150, 20);

		childWindow3.reset(new wz::Window(desktop.get(), "Window 3"));
		childWindow3->setRect(800, 500, 200, 200);

		childList.reset(new wz::List(childWindow3.get(), listData, 17));
		childList->setMargin(8);
		childList->setAutosize(WZ_AUTOSIZE);

		horizontalStackLayout.reset(new wz::StackLayout(desktop.get(), wz::StackLayout::Horizontal));
		horizontalStackLayout->setRect(50, 550, 400, 100);

		hslButton1.reset(new wz::Button(horizontalStackLayout.get(), "Button A"));
		hslButton1->setStretch(WZ_STRETCH_VERTICAL);

		hslButton2.reset(new wz::Button(horizontalStackLayout.get(), "Button B"));
		hslButton2->setMargin(0, 0, 0, 8);
		hslButton2->setAlign(WZ_ALIGN_MIDDLE);

		hslButton3.reset(new wz::Button(horizontalStackLayout.get(), "Button C"));
		hslButton3->setMargin(0, 0, 0, 8);
		hslButton3->setAlign(WZ_ALIGN_BOTTOM);
	}

	std::auto_ptr<wz::Desktop> desktop;
	std::auto_ptr<wz::Button> button;
	std::auto_ptr<wz::Checkbox> checkbox;
	std::auto_ptr<wz::GroupBox> groupBox;

	wz::RadioButtonGroup radioButtonGroup;
	std::auto_ptr<wz::StackLayout> radioButtonLayout;
	std::auto_ptr<wz::RadioButton> radioButton1, radioButton2, radioButton3;

	std::auto_ptr<wz::Scroller> scroller;
	std::auto_ptr<wz::Label> label;
	std::auto_ptr<wz::Scroller> scrollerHorizontal;
	std::auto_ptr<wz::List> list;
	std::auto_ptr<wz::Tabbed> tabbed;
	std::auto_ptr<wz::Button> secondTabPageButton;
	std::auto_ptr<wz::Combo> combo;
	std::auto_ptr<wz::Combo> combo2;
	std::auto_ptr<wz::Window> childWindow;
	std::auto_ptr<wz::StackLayout> childLayout;
	std::auto_ptr<wz::TextEdit> childTextEdit;
	std::auto_ptr<wz::Button> childWindowButton;
	std::auto_ptr<wz::Checkbox> childWindowCheckbox;
	std::auto_ptr<wz::List> childList;
	std::auto_ptr<wz::Combo> childCombo;
	std::auto_ptr<wz::Button> childWindowButton2;
	std::auto_ptr<wz::Window> childWindow2;
	std::auto_ptr<wz::Window> childWindow3;

	std::auto_ptr<wz::StackLayout> horizontalStackLayout;
	std::auto_ptr<wz::Button> hslButton1, hslButton2, hslButton3;
};

static void ShowError(const char *message)
{
	fprintf(stderr, "%s\n", message);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
}

static Uint32 BenchmarkTimerCallback(Uint32 interval, void *param)
{
	benchmark.draw.calculateAverage();
	benchmark.frame.calculateAverage();
	benchmark.input.calculateAverage();
    
	SDL_Event ev;
    ev.type = SDL_USEREVENT;
    ev.user.type = SDL_USEREVENT;
    ev.user.code = 0;
    ev.user.data1 = NULL;
    ev.user.data2 = NULL;
    SDL_PushEvent(&ev);

    return interval;
}

static wzKey ConvertKey(SDL_Keycode sym)
{
	static int keys[] =
	{
		SDLK_LEFT, WZ_KEY_LEFT,
		SDLK_RIGHT, WZ_KEY_RIGHT,
		SDLK_UP, WZ_KEY_UP,
		SDLK_DOWN, WZ_KEY_DOWN, 
		SDLK_HOME, WZ_KEY_HOME,
		SDLK_END, WZ_KEY_END,
		SDLK_DELETE, WZ_KEY_DELETE,
		SDLK_BACKSPACE, WZ_KEY_BACKSPACE
	};

	for (size_t i = 0; i < sizeof(keys) / sizeof(int); i += 2)
	{
		if (keys[i] == sym)
		{
			int key = keys[i + 1];

			if (SDL_GetModState() & KMOD_SHIFT)
			{
				key |= WZ_KEY_SHIFT;
			}

			if (SDL_GetModState() & KMOD_CTRL)
			{
				key |= WZ_KEY_CONTROL;
			}

			return (wzKey)key;
		}
	}

	return WZ_KEY_UNKNOWN;
}

int main(int argc, char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
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
	cursors[WZ_CURSOR_IBEAM] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
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

	SDL_AddTimer(1000, BenchmarkTimerCallback, NULL);
	char buffer[1024];
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
				benchmark.input.start();
				gui.desktop->mouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
				benchmark.input.end();
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				benchmark.input.start();
				gui.desktop->mouseButtonDown(e.button.button, e.button.x, e.button.y);
				benchmark.input.end();
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				benchmark.input.start();
				gui.desktop->mouseButtonUp(e.button.button, e.button.x, e.button.y);
				benchmark.input.end();
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				benchmark.input.start();
				gui.desktop->mouseWheelMove(e.wheel.x, e.wheel.y);
				benchmark.input.end();
			}
			else if (e.type == SDL_KEYDOWN)
			{
				benchmark.input.start();
				gui.desktop->keyDown(ConvertKey(e.key.keysym.sym));
				benchmark.input.end();
			}
			else if (e.type == SDL_KEYUP)
			{
				benchmark.input.start();
				gui.desktop->keyUp(ConvertKey(e.key.keysym.sym));
				benchmark.input.end();
			}
			else if (e.type == SDL_TEXTINPUT)
			{
				benchmark.input.start();
				gui.desktop->textInput(e.text.text);
				benchmark.input.end();
			}
		}

		if (quit)
			break;

		uint32_t currentTime = SDL_GetTicks();
		accumulatedTime += (float)(currentTime - lastTime);
		lastTime = currentTime;

		while (accumulatedTime > frameTime)
		{
			benchmark.frame.start();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			benchmark.draw.start();
			gui.desktop->draw();
			benchmark.draw.end();

			sprintf(buffer, "draw: %0.2fms", benchmark.draw.getAverage());
			renderer->debug_draw_text(renderer, buffer, 0, 0);

			sprintf(buffer, "frame: %0.2fms", benchmark.frame.getAverage());
			renderer->debug_draw_text(renderer, buffer, 0, 20);

			sprintf(buffer, "input: %0.2fms", benchmark.input.getAverage());
			renderer->debug_draw_text(renderer, buffer, 0, 40);

			SDL_GL_SwapWindow(window);
			SDL_SetCursor(cursors[wz_desktop_get_cursor((wzDesktop *)gui.desktop->getWidget())]);
			accumulatedTime -= frameTime;
			benchmark.frame.end();
		}
	}

	wzgl_destroy_renderer(renderer);
	return 0;
}
