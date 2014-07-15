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
	GUI(int windowWidth, int windowHeight, wzRenderer *renderer) : mainWindow(renderer)
	{
		mainWindow.setSize(windowWidth, windowHeight);

		{
			wz::Button *button = new wz::Button(renderer, "Test Button");
			button->setPosition(100, 100);
			mainWindow.add(button);

			const wzRect buttonRect = button->getRect();
			wz::Checkbox *checkbox = new wz::Checkbox(renderer, "Toggle me!");
			checkbox->setPosition(buttonRect.x, buttonRect.y + buttonRect.h + 16);
			mainWindow.add(checkbox);

			wz::Scroller *scroller1 = new wz::Scroller(renderer);
			scroller1->setType(WZ_SCROLLER_VERTICAL)->setMaxValue(100)->setValue(20)->setStepValue(10)->setPosition(300, 50)->setHeight(200);
			mainWindow.add(scroller1);

			wz::Scroller *scroller2 = new wz::Scroller(renderer);
			scroller2->setType(WZ_SCROLLER_HORIZONTAL)->setMaxValue(100)->setValue(50)->setStepValue(10)->setPosition(500, 50)->setWidth(200);
			mainWindow.add(scroller2);

			wz::Label *label = new wz::Label(renderer, "Label test");
			label->setTextColor(255, 128, 128)->setPosition(350, 50);
			mainWindow.add(label);

			wz::List *list = new wz::List(renderer);
			list->setItems(listData, 17)->setRect(400, 300, 150, 150);
			mainWindow.add(list);

			wz::Combo *combo = new wz::Combo(renderer);
			combo->setItems(listData, 17)->setRect(800, 50, 150, 20);
			mainWindow.add(combo);
		}

		{
			wz::GroupBox *groupBox = new wz::GroupBox(renderer, "Test GroupBox");
			groupBox->setPosition(100, 300);
			mainWindow.add(groupBox);

			wz::StackLayout *layout = new wz::StackLayout(WZ_STACK_LAYOUT_VERTICAL);
			layout->setSpacing(8)->setStretch(WZ_STRETCH);
			groupBox->add(layout);

			wz::RadioButton *rb1 = new wz::RadioButton(renderer, "Option 1");
			rb1->setGroup(&radioButtonGroup);
			layout->add(rb1);

			wz::RadioButton *rb2 = new wz::RadioButton(renderer, "Option 2");
			rb2->setGroup(&radioButtonGroup);
			layout->add(rb2);

			wz::RadioButton *rb3 = new wz::RadioButton(renderer, "Option 3");
			rb3->setGroup(&radioButtonGroup);
			layout->add(rb3);
		}

		{
			wz::Window *window = new wz::Window(renderer, "Test Window");
			window->setRect(650, 100, 300, 300);
			mainWindow.add(window);

			wz::StackLayout *layout = new wz::StackLayout(WZ_STACK_LAYOUT_VERTICAL);
			layout->setSpacing(8)->setMargin(8)->setStretch(WZ_STRETCH);
			window->add(layout);

			wz::TextEdit *textEdit = new wz::TextEdit(renderer, "this is a very long string so scrolling can be tested");
			textEdit->setStretch(WZ_STRETCH_WIDTH);
			layout->add(textEdit);

			wz::Button *button = new wz::Button(renderer, "Another Button");
			button->setStretch(WZ_STRETCH_WIDTH);
			layout->add(button);

			wz::Checkbox *checkbox = new wz::Checkbox(renderer, "Checkbox");
			checkbox->setAlign(WZ_ALIGN_CENTER);
			layout->add(checkbox);

			wz::Combo *combo = new wz::Combo(renderer);
			combo->setItems(listData, 17)->setAlign(WZ_ALIGN_RIGHT);
			layout->add(combo);

			wz::Button *button2 = new wz::Button(renderer, "Yet Another Button");
			button2->setStretch(WZ_STRETCH);
			layout->add(button2);
		}

		{
			wz::Window *window = new wz::Window(renderer, "Window with a long title");
			window->setRect(590, 500, 200, 200);
			mainWindow.add(window);

			wz::Tabbed *tabbed = new wz::Tabbed(renderer);
			tabbed->setMargin(8)->setStretch(WZ_STRETCH);
			window->add(tabbed);

			wz::Tab *firstTab = tabbed->addTab(new wz::Tab());
			firstTab->setLabel("Tab 1");

			wz::Tab *secondTab = tabbed->addTab(new wz::Tab());
			secondTab->setLabel("Another Tab");

			tabbed->addTab(new wz::Tab())->setLabel("TabTabTab");

			wz::Combo *combo = new wz::Combo(renderer);
			combo->setItems(listData, 17)->setRect(10, 10, 150, 20);
			firstTab->add(combo);

			secondTab->add(new wz::Button(renderer, "Button Button Button"))->setPosition(10, 10);
		}

		{
			wz::Window *window = new wz::Window(renderer, "Window 3");
			window->setRect(800, 500, 200, 200);
			mainWindow.add(window);

			wz::List *list = new wz::List(renderer);
			list->setItems(listData, 17)->setMargin(8)->setStretch(WZ_STRETCH);
			window->add(list);
		}

		{
			wz::StackLayout *layout = new wz::StackLayout(WZ_STACK_LAYOUT_HORIZONTAL);
			layout->setSpacing(8)->setRect(50, 550, 400, 100);
			mainWindow.add(layout);

			layout->add(new wz::Button(renderer, "Button A"))->setStretch(WZ_STRETCH_HEIGHT);
			layout->add(new wz::Button(renderer, "Button B"))->setAlign(WZ_ALIGN_MIDDLE);
			layout->add(new wz::Button(renderer, "Button C"))->setAlign(WZ_ALIGN_BOTTOM);
		}
	}

	wz::MainWindow mainWindow;
	wz::RadioButtonGroup radioButtonGroup;
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
	int tick = 0;

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
				gui.mainWindow.setSize(e.window.data1, e.window.data2);
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				benchmark.input.start();
				gui.mainWindow.mouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
				benchmark.input.end();
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				benchmark.input.start();
				gui.mainWindow.mouseButtonDown(e.button.button, e.button.x, e.button.y);
				benchmark.input.end();
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				benchmark.input.start();
				gui.mainWindow.mouseButtonUp(e.button.button, e.button.x, e.button.y);
				benchmark.input.end();
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				benchmark.input.start();
				gui.mainWindow.mouseWheelMove(e.wheel.x, e.wheel.y);
				benchmark.input.end();
			}
			else if (e.type == SDL_KEYDOWN)
			{
				benchmark.input.start();
				gui.mainWindow.keyDown(ConvertKey(e.key.keysym.sym));
				benchmark.input.end();
			}
			else if (e.type == SDL_KEYUP)
			{
				benchmark.input.start();
				gui.mainWindow.keyUp(ConvertKey(e.key.keysym.sym));
				benchmark.input.end();
			}
			else if (e.type == SDL_TEXTINPUT)
			{
				benchmark.input.start();
				gui.mainWindow.textInput(e.text.text);
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
			gui.mainWindow.beginFrame();
			gui.mainWindow.drawFrame();

			sprintf(buffer, "draw: %0.2fms", benchmark.draw.getAverage());
			renderer->debug_draw_text(renderer, buffer, 0, 0);

			sprintf(buffer, "frame: %0.2fms", benchmark.frame.getAverage());
			renderer->debug_draw_text(renderer, buffer, 0, 20);

			sprintf(buffer, "input: %0.2fms", benchmark.input.getAverage());
			renderer->debug_draw_text(renderer, buffer, 0, 40);

			gui.mainWindow.endFrame();
			benchmark.draw.end();

			SDL_GL_SwapWindow(window);
			SDL_SetCursor(cursors[gui.mainWindow.getCursor()]);
			accumulatedTime -= frameTime;
			benchmark.frame.end();
			tick++;

			if ((tick % 30) == 0)
			{
				gui.mainWindow.setShowCursor(!gui.mainWindow.getShowCursor());
			}
		}
	}

	wzgl_destroy_renderer(renderer);
	return 0;
}