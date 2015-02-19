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
#include <wz_renderer_nanovg.h>

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

static void CustomDrawListItemCallback(wz::IRenderer *renderer, wz::Rect clip, const wz::List *list, const char *fontFace, float fontSize, int itemIndex, const uint8_t *itemData)
{
	int image, width, height;
	wz::Rect rect;

	wz::NVGRenderer *nvg = (wz::NVGRenderer *)renderer;
	image = nvg->createImage((const char *)itemData, &width, &height);
	rect.x = clip.x + (int)(clip.w / 2.0f - width / 2.0f);
	rect.y = clip.y + (int)(clip.h / 2.0f - height / 2.0f);
	rect.w = width;
	rect.h = height;
	nvg->drawImage(rect, image);
}

class GUI
{
public:
	GUI(int windowWidth, int windowHeight, wz::IRenderer *renderer) : mainWindow(renderer)
	{
		mainWindow.setSize(windowWidth, windowHeight);
		mainWindow.createMenuButton("File");
		mainWindow.createMenuButton("Edit");
		mainWindow.createMenuButton("View");
		mainWindow.createMenuButton("Options");
		mainWindow.createMenuButton("Window");
		createButtonFrame();
		createCheckBoxFrame();
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
		frame->setStretch(wz::Stretch::All);
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

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
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

	void createCheckBoxFrame()
	{
		wz::Frame *frame = createFrame("CheckBox");

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
		frame->add(layout);

		wz::CheckBox *checkbox = new wz::CheckBox("Toggle me!");
		layout->add(checkbox);
	}

	void createComboFrame()
	{
		wz::Frame *frame = createFrame("Combo");

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
		frame->add(layout);

		wz::Combo *combo1 = new wz::Combo((uint8_t *)listData, sizeof(const char *), 17);
		combo1->setPosition(800, 50);
		layout->add(combo1);

		wz::Combo *combo2 = new wz::Combo((uint8_t *)listData, sizeof(const char *), 17);
		combo2->setFont("visitor1", 12);
		layout->add(combo2);
	}

	void createGroupBoxFrame()
	{
		wz::Frame *frame = createFrame("GroupBox");

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
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

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
		frame->add(layout);

		layout->add(new wz::Label("Normal label"));

		wz::Label *multilineLabel = new wz::Label("Multiline label with color. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
		multilineLabel->setMultiline(true);
		multilineLabel->setTextColor(0, 0.5f, 0);
		multilineLabel->setWidth(400);
		layout->add(multilineLabel);

		wz::Label *customLabel = new wz::Label("Label with custom font and color");
		customLabel->setTextColor(1, 0.5f, 0.5f);
		customLabel->setFont("visitor1", 32);
		layout->add(customLabel);
	}

	void createListFrame()
	{
		wz::Frame *frame = createFrame("List");

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Horizontal);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
		frame->add(layout);

		wz::List *list1 = new wz::List((uint8_t *)listData, sizeof(const char *), 17);
		list1->setSize(120, 200);
		layout->add(list1);

		wz::List *list2 = new wz::List((uint8_t *)listData, sizeof(const char *), 17);
		list2->setSize(240, 300);
		list2->setFont("visitor1", 32);
		layout->add(list2);

		wz::List *list3 = new wz::List((uint8_t *)customListData, sizeof(const char *), 3);
		list3->setItemHeight(40);
		list3->setDrawItemCallback(CustomDrawListItemCallback);
		list3->setSize(50, 200);
		layout->add(list3);
	}

	void createRadioButtonFrame()
	{
		wz::Frame *frame = createFrame("Radio Button");

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
		frame->add(layout);

		layout->add(new wz::RadioButton("Option 1"));
		layout->add(new wz::RadioButton("Option 2"));
		layout->add(new wz::RadioButton("Option 3"));
	}

	void createScrollerFrame()
	{
		wz::Frame *frame = createFrame("Scroller");

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
		frame->add(layout);

		wz::Scroller *scroller1 = new wz::Scroller(wz::ScrollerDirection::Vertical);
		scroller1->setMaxValue(100);
		scroller1->setValue(20);
		scroller1->setStepValue(10);
		scroller1->setHeight(200);
		layout->add(scroller1);

		wz::Scroller *scroller2 = new wz::Scroller(wz::ScrollerDirection::Horizontal);
		scroller2->setMaxValue(100);
		scroller2->setValue(50);
		scroller2->setStepValue(10);
		scroller2->setWidth(200);
		layout->add(scroller2);
	}

	void createSpinnerFrame()
	{
		wz::Frame *frame = createFrame("Spinner");

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
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
		tabbed->setUniformMargin(8);
		tabbed->setStretch(wz::Stretch::All);
		frame->add(tabbed);

		wz::Tab *firstTab = new wz::Tab();
		tabbed->addTab(firstTab);
		firstTab->setLabel("Tab 1");

		wz::Tab *secondTab = new wz::Tab();
		tabbed->addTab(secondTab);
		secondTab->setLabel("Another Tab");

		wz::Tab *thirdTab = new wz::Tab();
		tabbed->addTab(thirdTab);
		thirdTab->setLabel("TabTabTab");
	}

	void createTextEditFrame()
	{
		wz::Frame *frame = createFrame("Text Edit");

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
		frame->add(layout);

		wz::TextEdit *textEdit1 = new wz::TextEdit(false, "this is a very long string so scrolling can be tested");
		textEdit1->setWidth(300);
		layout->add(textEdit1);

		wz::TextEdit *textEdit2 = new wz::TextEdit(false, "text edit with a custom font");
		textEdit2->setFont("visitor1", 32);
		textEdit2->setStretch(wz::Stretch::Width);
		layout->add(textEdit2);

		wz::TextEdit *textEdit3 = new wz::TextEdit(true, "NanoVG is small antialiased vector graphics rendering library for OpenGL. It has lean API modeled after HTML5 canvas API. It is aimed to be a practical and fun toolset for building scalable user interfaces and visualizations.");
		textEdit3->setSize(200, 100);
		layout->add(textEdit3);
	}

	void createWindowFrame()
	{
		wz::Frame *frame = createFrame("Window");

		wz::StackLayout *frameLayout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		frameLayout->setSpacing(8);
		frameLayout->setUniformMargin(8);
		frameLayout->setStretch(wz::Stretch::All);
		frame->add(frameLayout);

		wz::CheckBox *showWindow1 = new wz::CheckBox("Show Window 1");
		showWindow1->addEventHandler(wz::EventType::ButtonClicked, this, &GUI::showWindow1Toggled);
		frameLayout->add(showWindow1);

		wz::CheckBox *showWindow2 = new wz::CheckBox("Show Window 2");
		showWindow2->addEventHandler(wz::EventType::ButtonClicked, this, &GUI::showWindow2Toggled);
		frameLayout->add(showWindow2);
	}

	void createParentLayoutFrame()
	{
		wz::Frame *rootFrame = createFrame("Parent Layout");

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
		rootFrame->add(layout);

		{
			wz::Frame *frame = new wz::Frame();
			frame->setStretch(wz::Stretch::All);
			layout->add(frame);

			wz::Button *button = new wz::Button("Align Left");
			button->setAlign(wz::Align::Left);
			frame->add(button);

			button = new wz::Button("Align Center");
			button->setAlign(wz::Align::Center);
			frame->add(button);

			button = new wz::Button("Align Right");
			button->setAlign(wz::Align::Right);
			frame->add(button);
		}

		{
			wz::Frame *frame = new wz::Frame();
			frame->setStretch(wz::Stretch::All);
			layout->add(frame);

			wz::Button *button = new wz::Button("Align Top");
			button->setAlign(wz::Align::Top);
			frame->add(button);

			button = new wz::Button("Align Middle");
			button->setAlign(wz::Align::Middle);
			frame->add(button);

			button = new wz::Button("Align Bottom");
			button->setAlign(wz::Align::Bottom);
			frame->add(button);
		}
	}

	void createStackLayoutFrame()
	{
		wz::Frame *frame = createFrame("Stack Layout");

		wz::StackLayout *frameLayout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		frameLayout->setSpacing(8);
		frameLayout->setUniformMargin(8);
		frameLayout->setStretch(wz::Stretch::All);
		frame->add(frameLayout);

		{
			wz::Label *label = new wz::Label("Horizontal Stack Layout");
			frameLayout->add(label);

			wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Horizontal);
			layout->setSpacing(8);
			layout->setStretch(wz::Stretch::Width);
			layout->setHeight(100);
			frameLayout->add(layout);

			wz::Button *button = new wz::Button("Default");
			layout->add(button);

			button = new wz::Button("Align Top");
			button->setAlign(wz::Align::Top);
			layout->add(button);

			button = new wz::Button("Align Middle");
			button->setAlign(wz::Align::Middle);
			layout->add(button);

			button = new wz::Button("Align Bottom");
			button->setAlign(wz::Align::Bottom);
			layout->add(button);

			button = new wz::Button("Stretch Width");
			button->setStretch(wz::Stretch::Width);
			layout->add(button);

			button = new wz::Button("Stretch Height");
			button->setStretch(wz::Stretch::Height);
			layout->add(button);
		}

		{
			wz::Label *label = new wz::Label("Vertical Stack Layout");
			label->setMargin(16, 0, 0, 0);
			frameLayout->add(label);

			wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
			layout->setSpacing(8);
			layout->setStretch(wz::Stretch::Height);
			layout->setWidth(300);
			frameLayout->add(layout);

			wz::Button *button = new wz::Button("Default");
			layout->add(button);

			button = new wz::Button("Align Left");
			button->setAlign(wz::Align::Left);
			layout->add(button);

			button = new wz::Button("Align Center");
			button->setAlign(wz::Align::Center);
			layout->add(button);

			button = new wz::Button("Align Right");
			button->setAlign(wz::Align::Right);
			layout->add(button);

			button = new wz::Button("Stretch Width");
			button->setStretch(wz::Stretch::Width);
			layout->add(button);

			button = new wz::Button("Stretch Height");
			button->setStretch(wz::Stretch::Height);
			layout->add(button);
		}
	}

	void createWidgetCategoryWindow()
	{
		wz::Window *window = new wz::Window("Widgets");
		window->setWidth(200);
		mainWindow.add(window);
		mainWindow.dockWindow(window, wz::DockPosition::West);

		wz::List *list = new wz::List((uint8_t *)&widgetCategories[0], sizeof(WidgetCategoryListItem), widgetCategories.size());
		list->setUniformMargin(8);
		list->setStretch(wz::Stretch::All);
		list->setFontSize(18);
		list->setSelectedItem(0);
		list->addEventHandler(wz::EventType::ListItemSelected, this, &GUI::widgetCategoryChanged);
		window->add(list);
	}

	void createWindow1()
	{
		window1 = new wz::Window("Test Window");
		window1->setRect(650, 100, 300, 300);
		window1->setVisible(false);
		mainWindow.add(window1);

		wz::StackLayout *layout = new wz::StackLayout(wz::StackLayoutDirection::Vertical);
		layout->setSpacing(8);
		layout->setUniformMargin(8);
		layout->setStretch(wz::Stretch::All);
		window1->add(layout);

		wz::TextEdit *textEdit = new wz::TextEdit(false, "this is a very long string so scrolling can be tested");
		textEdit->setStretch(wz::Stretch::Width);
		layout->add(textEdit);

		wz::Button *button = new wz::Button("Another Button", "../examples/data/accept.png");
		button->setStretch(wz::Stretch::Width);
		layout->add(button);

		wz::CheckBox *checkbox = new wz::CheckBox("CheckBox");
		checkbox->setAlign(wz::Align::Center);
		layout->add(checkbox);

		wz::Combo *combo = new wz::Combo((uint8_t *)listData, sizeof(const char *), 17);
		combo->setAlign(wz::Align::Right);
		combo->setFont("visitor1", 12);
		layout->add(combo);

		wz::Button *button2 = new wz::Button("Yet Another Button");
		button2->setStretch(wz::Stretch::All);
		layout->add(button2);
	}

	void createWindow2()
	{
		window2 = new wz::Window("Window with a long title");
		window2->setRect(590, 500, 200, 200);
		window2->setVisible(false);
		mainWindow.add(window2);

		wz::Tabbed *tabbed = new wz::Tabbed();
		tabbed->setUniformMargin(8);
		tabbed->setStretch(wz::Stretch::All);
		window2->add(tabbed);

		wz::Tab *firstTab = new wz::Tab();
		tabbed->addTab(firstTab);
		firstTab->setLabel("Tab 1");

		wz::Tab *secondTab = new wz::Tab();
		tabbed->addTab(secondTab);
		secondTab->setLabel("Another Tab");

		wz::Tab *thirdTab = new wz::Tab();
		tabbed->addTab(thirdTab);
		thirdTab->setLabel("TabTabTab");

		wz::Combo *combo = new wz::Combo((uint8_t *)listData, sizeof(const char *), 17);
		combo->setPosition(10, 10);
		firstTab->add(combo);

		secondTab->add(new wz::Button("Button Button Button"))->setPosition(10, 10);
	}

	void showWindow1Toggled(wz::Event e)
	{
		window1->setVisible(e.button.isSet);
	}

	void showWindow2Toggled(wz::Event e)
	{
		window2->setVisible(e.button.isSet);
	}

	void widgetCategoryChanged(wz::Event e)
	{
		setFrame(e.list.selectedItem);
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

static wz::Key::Enum ConvertKey(SDL_Keycode sym)
{
	static int keys[] =
	{
		SDLK_LEFT, wz::Key::LeftArrow,
		SDLK_RIGHT, wz::Key::RightArrow,
		SDLK_UP, wz::Key::UpArrow,
		SDLK_DOWN, wz::Key::DownArrow, 
		SDLK_HOME, wz::Key::Home,
		SDLK_END, wz::Key::End,
		SDLK_RETURN, wz::Key::Enter,
		SDLK_RETURN, wz::Key::Enter,
		SDLK_KP_ENTER, wz::Key::Enter,
		SDLK_DELETE, wz::Key::Delete,
		SDLK_BACKSPACE, wz::Key::Backspace,
		SDLK_LSHIFT, wz::Key::LeftShift,
		SDLK_RSHIFT, wz::Key::RightShift,
		SDLK_LCTRL, wz::Key::LeftControl,
		SDLK_RCTRL, wz::Key::RightControl
	};

	for (size_t i = 0; i < sizeof(keys) / sizeof(int); i += 2)
	{
		if (keys[i] == sym)
		{
			int key = keys[i + 1];

			if (SDL_GetModState() & KMOD_SHIFT)
			{
				key |= wz::Key::ShiftBit;
			}

			if (SDL_GetModState() & KMOD_CTRL)
			{
				key |= wz::Key::ControlBit;
			}

			return (wz::Key::Enum)key;
		}
	}

	return wz::Key::Unknown;
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
	SDL_Cursor *cursors[wz::Cursor::NumCursors];
	cursors[wz::Cursor::Default] = SDL_GetDefaultCursor();
	cursors[wz::Cursor::Ibeam] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	cursors[wz::Cursor::Resize_N_S] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursors[wz::Cursor::Resize_E_W] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	cursors[wz::Cursor::Resize_NE_SW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	cursors[wz::Cursor::Resize_NW_SE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);

	// Create the renderer.
	wz::NVGRenderer *renderer = new wz::NVGRenderer(nvgCreateGL2, nvgDeleteGL2, NVG_ANTIALIAS, "../examples/data", "DejaVuSans", 16.0f);

	if (renderer->getError())
	{
		ShowError(renderer->getError());
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

	delete renderer;
	return 0;
}
