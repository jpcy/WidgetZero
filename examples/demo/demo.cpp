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

#ifdef _MSC_VER
#include <gl2.h>
#else
#include <GL/gl.h>
#endif

#include <SDL.h>

#include <wz.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244) // "conversion from 'int' to 'float', possible loss of data"
#endif

#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static const int textCursorBlinkInterval = 500;

static const char *customListData[3] =
{
	"../examples/data/accept.png",
	"../examples/data/delete.png",
	"../examples/data/error.png"
};

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

static void CustomDrawListItemCallback(struct wz::wzRenderer *renderer, wz::Rect clip, const struct wz::ListImpl *list, const char *fontFace, float fontSize, int itemIndex, const uint8_t *itemData)
{
	int image, width, height;
	wz::Rect rect;
	
	image = wz::wz_renderer_create_image(renderer, (const char *)itemData, &width, &height);
	rect.x = clip.x + (int)(clip.w / 2.0f - width / 2.0f);
	rect.y = clip.y + (int)(clip.h / 2.0f - height / 2.0f);
	rect.w = width;
	rect.h = height;
	wz::wz_renderer_draw_image(wz::wz_renderer_get_context(renderer), rect, image);
}

class GUI
{
public:
	GUI(int windowWidth, int windowHeight, wz::wzRenderer *renderer) : mainWindow(renderer), renderer(renderer)
	{
		mainWindow.setSize(windowWidth, windowHeight);
		mainWindow.createMenuButton("File");
		mainWindow.createMenuButton("Edit");
		mainWindow.createMenuButton("View");
		mainWindow.createMenuButton("Options");
		mainWindow.createMenuButton("Window");
		createButtonFrame();
		createCheckboxFrame();
		createComboFrame();
		createGroupBoxFrame();
		createLabelFrame();
		createListFrame();
		createRadioButtonFrame();
		createScrollerFrame();
		createSpinnerFrame();
		createTabbedFrame();
		createTextEditFrame();
		createWindowFrame();
		createParentLayoutFrame();
		createStackLayoutFrame();
		createWidgetCategoryWindow();
		createWindow1();
		createWindow2();
		setFrame(0);
	}

	wz::MainWindow mainWindow;

private:
	wz::Frame *createFrame(const char *label)
	{
		wz::Frame *frame = new wz::Frame();
		frame->setStretch(wz::WZ_STRETCH);
		mainWindow.add(frame);

		WidgetCategoryListItem category;
		category.label = label;
		category.frame = frame;
		widgetCategories.push_back(category);

		return frame;
	}

	void createButtonFrame()
	{
		wz::Frame *frame = createFrame("Button");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		layout->add(new wz::Button("Button with a label"));
		layout->add(new wz::Button("Button with a label and icon", "../examples/data/accept.png"));
		layout->add(new wz::Button("", "../examples/data/accept.png"));

		wz::Button *paddedButton = new wz::Button("Custom padding");
		paddedButton->setPadding(20, 40, 20, 40);
		layout->add(paddedButton);

		wz::ToggleButton *toggleButton = new wz::ToggleButton("Toggle Button", std::string());
		layout->add(toggleButton);
	}

	void createCheckboxFrame()
	{
		wz::Frame *frame = createFrame("Checkbox");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		wz::Checkbox *checkbox = new wz::Checkbox("Toggle me!");
		layout->add(checkbox);
	}

	void createComboFrame()
	{
		wz::Frame *frame = createFrame("Combo");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		wz::Combo *combo1 = new wz::Combo();
		combo1->setItems((uint8_t *)listData, sizeof(const char *), 17)->setPosition(800, 50);
		layout->add(combo1);

		wz::Combo *combo2 = new wz::Combo();
		combo2->setItems((uint8_t *)listData, sizeof(const char *), 17)->setFont("visitor1", 12);
		layout->add(combo2);
	}

	void createGroupBoxFrame()
	{
		wz::Frame *frame = createFrame("GroupBox");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		wz::GroupBox *groupBox1 = new wz::GroupBox("With a Label");
		groupBox1->add(new wz::Label("Default margin"));
		layout->add(groupBox1);

		wz::GroupBox *groupBox = new wz::GroupBox();
		layout->add(groupBox);
	}

	void createLabelFrame()
	{
		wz::Frame *frame = createFrame("Label");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		layout->add(new wz::Label("Normal label"));

		wz::Label *multilineLabel = new wz::Label("Multiline label with color. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
		multilineLabel->setMultiline(true)->setTextColor(0, 0.5f, 0)->setWidth(400);
		layout->add(multilineLabel);

		wz::Label *customLabel = new wz::Label("Label with custom font and color");
		customLabel->setTextColor(1, 0.5f, 0.5f)->setFont("visitor1", 32);
		layout->add(customLabel);
	}

	void createListFrame()
	{
		wz::Frame *frame = createFrame("List");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_HORIZONTAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		wz::List *list1 = new wz::List();
		list1->setItems((uint8_t *)listData, sizeof(const char *), 17)->setSize(120, 200);
		layout->add(list1);

		wz::List *list2 = new wz::List();
		list2->setItems((uint8_t *)listData, sizeof(const char *), 17)->setSize(240, 300)->setFont("visitor1", 32);
		layout->add(list2);

		wz::List *list3 = new wz::List();
		list3->setItems((uint8_t *)customListData, sizeof(const char *), 3)->setItemHeight(40)->setDrawItemCallback(CustomDrawListItemCallback)->setSize(50, 200);
		layout->add(list3);
	}

	void createRadioButtonFrame()
	{
		wz::Frame *frame = createFrame("Radio Button");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		layout->add(new wz::RadioButton("Option 1"));
		layout->add(new wz::RadioButton("Option 2"));
		layout->add(new wz::RadioButton("Option 3"));
	}

	void createScrollerFrame()
	{
		wz::Frame *frame = createFrame("Scroller");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		wz::Scroller *scroller1 = new wz::Scroller(wz::WZ_SCROLLER_VERTICAL);
		scroller1->setMaxValue(100)->setValue(20)->setStepValue(10)->setHeight(200);
		layout->add(scroller1);

		wz::Scroller *scroller2 = new wz::Scroller(wz::WZ_SCROLLER_HORIZONTAL);
		scroller2->setMaxValue(100)->setValue(50)->setStepValue(10)->setWidth(200);
		layout->add(scroller2);
	}

	void createSpinnerFrame()
	{
		wz::Frame *frame = createFrame("Spinner");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		wz::Spinner *spinner = new wz::Spinner();
		spinner->setValue(42);
		layout->add(spinner);

		wz::Spinner *spinner2 = new wz::Spinner();
		spinner2->setFont("visitor1", 32);
		layout->add(spinner2);
	}

	void createTabbedFrame()
	{
		wz::Frame *frame = createFrame("Tabbed");

		wz::Tabbed *tabbed = new wz::Tabbed();
		tabbed->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(tabbed);

		wz::Tab *firstTab = tabbed->addTab(new wz::Tab());
		firstTab->setLabel("Tab 1");

		wz::Tab *secondTab = tabbed->addTab(new wz::Tab());
		secondTab->setLabel("Another Tab");

		tabbed->addTab(new wz::Tab())->setLabel("TabTabTab");
	}

	void createTextEditFrame()
	{
		wz::Frame *frame = createFrame("Text Edit");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(layout);

		wz::TextEdit *textEdit1 = new wz::TextEdit("this is a very long string so scrolling can be tested", false);
		textEdit1->setWidth(300);
		layout->add(textEdit1);

		wz::TextEdit *textEdit2 = new wz::TextEdit("text edit with a custom font", false);
		textEdit2->setFont("visitor1", 32)->setStretch(wz::WZ_STRETCH_WIDTH);
		layout->add(textEdit2);

		wz::TextEdit *textEdit3 = new wz::TextEdit("NanoVG is small antialiased vector graphics rendering library for OpenGL. It has lean API modeled after HTML5 canvas API. It is aimed to be a practical and fun toolset for building scalable user interfaces and visualizations.", true);
		textEdit3->setSize(200, 100);
		layout->add(textEdit3);
	}

	void createWindowFrame()
	{
		wz::Frame *frame = createFrame("Window");

		wz::StackLayout *frameLayout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		frameLayout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(frameLayout);

		wz::Checkbox *showWindow1 = new wz::Checkbox("Show Window 1");
		showWindow1->addEventHandler(wz::WZ_EVENT_BUTTON_CLICKED, this, &GUI::showWindow1Toggled);
		frameLayout->add(showWindow1);

		wz::Checkbox *showWindow2 = new wz::Checkbox("Show Window 2");
		showWindow2->addEventHandler(wz::WZ_EVENT_BUTTON_CLICKED, this, &GUI::showWindow2Toggled);
		frameLayout->add(showWindow2);
	}

	void createParentLayoutFrame()
	{
		wz::Frame *rootFrame = createFrame("Parent Layout");

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		rootFrame->add(layout);

		{
			wz::Frame *frame = new wz::Frame();
			frame->setStretch(wz::WZ_STRETCH);
			layout->add(frame);

			frame->add(new wz::Button("Align Left"))->setAlign(wz::WZ_ALIGN_LEFT);
			frame->add(new wz::Button("Align Center"))->setAlign(wz::WZ_ALIGN_CENTER);
			frame->add(new wz::Button("Align Right"))->setAlign(wz::WZ_ALIGN_RIGHT);
		}

		{
			wz::Frame *frame = new wz::Frame();
			frame->setStretch(wz::WZ_STRETCH);
			layout->add(frame);

			frame->add(new wz::Button("Align Top"))->setAlign(wz::WZ_ALIGN_TOP);
			frame->add(new wz::Button("Align Middle"))->setAlign(wz::WZ_ALIGN_MIDDLE);
			frame->add(new wz::Button("Align Bottom"))->setAlign(wz::WZ_ALIGN_BOTTOM);
		}
	}

	void createStackLayoutFrame()
	{
		wz::Frame *frame = createFrame("Stack Layout");

		wz::StackLayout *frameLayout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		frameLayout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		frame->add(frameLayout);

		{
			wz::Label *label = new wz::Label("Horizontal Stack Layout");
			frameLayout->add(label);

			wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_HORIZONTAL);
			layout->setSpacing(8)->setStretch(wz::WZ_STRETCH_WIDTH)->setHeight(100);
			frameLayout->add(layout);

			layout->add(new wz::Button("Default"));
			layout->add(new wz::Button("Align Top"))->setAlign(wz::WZ_ALIGN_TOP);
			layout->add(new wz::Button("Align Middle"))->setAlign(wz::WZ_ALIGN_MIDDLE);
			layout->add(new wz::Button("Align Bottom"))->setAlign(wz::WZ_ALIGN_BOTTOM);
			layout->add(new wz::Button("Stretch Width"))->setStretch(wz::WZ_STRETCH_WIDTH);
			layout->add(new wz::Button("Stretch Height"))->setStretch(wz::WZ_STRETCH_HEIGHT);
		}

		{
			wz::Label *label = new wz::Label("Vertical Stack Layout");
			label->setMargin(16, 0, 0, 0);
			frameLayout->add(label);

			wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
			layout->setSpacing(8)->setStretch(wz::WZ_STRETCH_HEIGHT)->setWidth(300);
			frameLayout->add(layout);

			layout->add(new wz::Button("Default"));
			layout->add(new wz::Button("Align Left"))->setAlign(wz::WZ_ALIGN_LEFT);
			layout->add(new wz::Button("Align Center"))->setAlign(wz::WZ_ALIGN_CENTER);
			layout->add(new wz::Button("Align Right"))->setAlign(wz::WZ_ALIGN_RIGHT);
			layout->add(new wz::Button("Stretch Width"))->setStretch(wz::WZ_STRETCH_WIDTH);
			layout->add(new wz::Button("Stretch Height"))->setStretch(wz::WZ_STRETCH_HEIGHT);
		}
	}

	void createWidgetCategoryWindow()
	{
		wz::Window *window = new wz::Window("Widgets");
		window->setWidth(200);
		mainWindow.add(window);
		mainWindow.dockWindow(window, wz::WZ_DOCK_POSITION_WEST);

		wz::List *list = new wz::List();
		list->setMargin(8);
		list->setItems((uint8_t *)&widgetCategories[0], sizeof(WidgetCategoryListItem), widgetCategories.size());
		list->setStretch(wz::WZ_STRETCH)->setFontSize(18);
		list->setSelectedItem(0);
		list->addEventHandler(wz::WZ_EVENT_LIST_ITEM_SELECTED, this, &GUI::widgetCategoryChanged);
		window->add(list);
	}

	void createWindow1()
	{
		window1 = new wz::Window("Test Window");
		window1->setRect(650, 100, 300, 300);
		window1->setVisible(false);
		mainWindow.add(window1);

		wz::StackLayout *layout = new wz::StackLayout(wz::WZ_STACK_LAYOUT_VERTICAL);
		layout->setSpacing(8)->setMargin(8)->setStretch(wz::WZ_STRETCH);
		window1->add(layout);

		wz::TextEdit *textEdit = new wz::TextEdit("this is a very long string so scrolling can be tested", false);
		textEdit->setStretch(wz::WZ_STRETCH_WIDTH);
		layout->add(textEdit);

		wz::Button *button = new wz::Button("Another Button", "../examples/data/accept.png");
		button->setStretch(wz::WZ_STRETCH_WIDTH);
		layout->add(button);

		wz::Checkbox *checkbox = new wz::Checkbox("Checkbox");
		checkbox->setAlign(wz::WZ_ALIGN_CENTER);
		layout->add(checkbox);

		wz::Combo *combo = new wz::Combo();
		combo->setItems((uint8_t *)listData, sizeof(const char *), 17)->setAlign(wz::WZ_ALIGN_RIGHT)->setFont("visitor1", 12);
		layout->add(combo);

		wz::Button *button2 = new wz::Button("Yet Another Button");
		button2->setStretch(wz::WZ_STRETCH);
		layout->add(button2);
	}

	void createWindow2()
	{
		window2 = new wz::Window("Window with a long title");
		window2->setRect(590, 500, 200, 200);
		window2->setVisible(false);
		mainWindow.add(window2);

		wz::Tabbed *tabbed = new wz::Tabbed();
		tabbed->setMargin(8)->setStretch(wz::WZ_STRETCH);
		window2->add(tabbed);

		wz::Tab *firstTab = tabbed->addTab(new wz::Tab());
		firstTab->setLabel("Tab 1");

		wz::Tab *secondTab = tabbed->addTab(new wz::Tab());
		secondTab->setLabel("Another Tab");

		tabbed->addTab(new wz::Tab())->setLabel("TabTabTab");

		wz::Combo *combo = new wz::Combo();
		combo->setItems((uint8_t *)listData, sizeof(const char *), 17)->setPosition(10, 10);
		firstTab->add(combo);

		secondTab->add(new wz::Button("Button Button Button"))->setPosition(10, 10);
	}

	void showWindow1Toggled(wz::Event *e)
	{
		window1->setVisible(e->button.isSet);
	}

	void showWindow2Toggled(wz::Event *e)
	{
		window2->setVisible(e->button.isSet);
	}

	void widgetCategoryChanged(wz::Event *e)
	{
		setFrame(e->list.selectedItem);
	}

	void setFrame(int index)
	{
		for (size_t i = 0; i < widgetCategories.size(); i++)
		{
			widgetCategories[i].frame->setVisible(i == index ? true : false);
		}
	}

	struct WidgetCategoryListItem
	{
		const char *label;
		wz::Frame *frame;
	};

	wz::wzRenderer *renderer;
	wz::Window *window1, *window2;
	std::vector<WidgetCategoryListItem> widgetCategories;
	bool showProfiling_;
};

static void ShowError(const char *message)
{
	fprintf(stderr, "%s\n", message);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
}

static Uint32 TextCursorBlinkCallback(Uint32 interval, void *param)
{
	((wz::MainWindow *)param)->toggleTextCursor();
    
	SDL_Event ev;
    ev.type = SDL_USEREVENT;
    ev.user.type = SDL_USEREVENT;
    ev.user.code = 0;
    ev.user.data1 = NULL;
    ev.user.data2 = NULL;
    SDL_PushEvent(&ev);

    return interval;
}

static wz::Key ConvertKey(SDL_Keycode sym)
{
	static int keys[] =
	{
		SDLK_LEFT, wz::WZ_KEY_LEFT,
		SDLK_RIGHT, wz::WZ_KEY_RIGHT,
		SDLK_UP, wz::WZ_KEY_UP,
		SDLK_DOWN, wz::WZ_KEY_DOWN, 
		SDLK_HOME, wz::WZ_KEY_HOME,
		SDLK_END, wz::WZ_KEY_END,
		SDLK_RETURN, wz::WZ_KEY_ENTER,
		SDLK_RETURN, wz::WZ_KEY_ENTER,
		SDLK_KP_ENTER, wz::WZ_KEY_ENTER,
		SDLK_DELETE, wz::WZ_KEY_DELETE,
		SDLK_BACKSPACE, wz::WZ_KEY_BACKSPACE,
		SDLK_LSHIFT, wz::WZ_KEY_LSHIFT,
		SDLK_RSHIFT, wz::WZ_KEY_RSHIFT,
		SDLK_LCTRL, wz::WZ_KEY_LCONTROL,
		SDLK_RCTRL, wz::WZ_KEY_RCONTROL
	};

	for (size_t i = 0; i < sizeof(keys) / sizeof(int); i += 2)
	{
		if (keys[i] == sym)
		{
			int key = keys[i + 1];

			if (SDL_GetModState() & KMOD_SHIFT)
			{
				key |= wz::WZ_KEY_SHIFT_BIT;
			}

			if (SDL_GetModState() & KMOD_CTRL)
			{
				key |= wz::WZ_KEY_CONTROL_BIT;
			}

			return (wz::Key)key;
		}
	}

	return wz::WZ_KEY_UNKNOWN;
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

#ifdef _MSC_VER
	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		ShowError("ogl_LoadFunctions failed");
		return 1;
	}
#endif

	glClearColor(0.2510f, 0.2510f, 0.2510f, 1);

	int windowWidth, windowHeight;
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);

	// Setup cursors.
	SDL_Cursor *cursors[wz::WZ_NUM_CURSORS];
	cursors[wz::WZ_CURSOR_DEFAULT] = SDL_GetDefaultCursor();
	cursors[wz::WZ_CURSOR_IBEAM] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	cursors[wz::WZ_CURSOR_RESIZE_N_S] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursors[wz::WZ_CURSOR_RESIZE_E_W] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	cursors[wz::WZ_CURSOR_RESIZE_NE_SW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	cursors[wz::WZ_CURSOR_RESIZE_NW_SE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);

	// Create the renderer.
	wz::wzRenderer *renderer = wz::wz_renderer_create(nvgCreateGL2, nvgDeleteGL2, NVG_ANTIALIAS, "../examples/data", "DejaVuSans", 16.0f);

	if (!renderer)
	{
		ShowError(wz::wz_renderer_get_error());
		return 1;
	}

	GUI gui(windowWidth, windowHeight, renderer);
	const SDL_TimerID textCursorTimer = SDL_AddTimer(textCursorBlinkInterval, TextCursorBlinkCallback, &gui.mainWindow);

	for (;;)
	{
		SDL_Event e;

		SDL_WaitEvent(&e);

		if (e.type == SDL_QUIT)
		{
			SDL_RemoveTimer(textCursorTimer);
			break;
		}
		else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED)
		{
			glViewport(0, 0, e.window.data1, e.window.data2);
			gui.mainWindow.setSize(e.window.data1, e.window.data2);
		}
		else if (e.type == SDL_MOUSEMOTION)
		{
			gui.mainWindow.mouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			gui.mainWindow.mouseButtonDown(e.button.button, e.button.x, e.button.y);
		}
		else if (e.type == SDL_MOUSEBUTTONUP)
		{
			gui.mainWindow.mouseButtonUp(e.button.button, e.button.x, e.button.y);
		}
		else if (e.type == SDL_MOUSEWHEEL)
		{
			gui.mainWindow.mouseWheelMove(e.wheel.x, e.wheel.y);
		}
		else if (e.type == SDL_KEYDOWN)
		{
			gui.mainWindow.keyDown(ConvertKey(e.key.keysym.sym));
		}
		else if (e.type == SDL_KEYUP)
		{
			gui.mainWindow.keyUp(ConvertKey(e.key.keysym.sym));
		}
		else if (e.type == SDL_TEXTINPUT)
		{
			gui.mainWindow.textInput(e.text.text);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		gui.mainWindow.drawFrame();
		SDL_GL_SwapWindow(window);
		SDL_SetCursor(cursors[gui.mainWindow.getCursor()]);
	}

	wz_renderer_destroy(renderer);
	return 0;
}
