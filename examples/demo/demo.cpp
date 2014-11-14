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

struct wzMainWindow *mainWindow;
const int textCursorBlinkInterval = 500;
const char *customListData[] =
{
	"../examples/data/accept.png",
	"../examples/data/delete.png",
	"../examples/data/error.png"
};

#define CUSTOM_LIST_DATA_LENGTH (sizeof(customListData) / sizeof(customListData[0]))

const char *listData[] =
{
	"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten"
};

#define LIST_DATA_LENGTH (sizeof(listData) / sizeof(listData[0]))

struct WidgetCategoryListItem
{
	const char *label;
	struct wzFrame *frame;
};

struct wzWindow *window1, *window2;
#define MAX_WIDGET_CATEGORIES 32
struct WidgetCategoryListItem widgetCategories[MAX_WIDGET_CATEGORIES];
size_t nWidgetCategories = 0;

void CustomDrawListItemCallback(struct wzRenderer *renderer, wzRect clip, const struct wzList *list, const char *fontFace, float fontSize, int itemIndex, const uint8_t *itemData)
{
	int image, width, height;
	wzRect rect;
	
	image = wz_renderer_create_image(renderer, (const char *)itemData, &width, &height);
	rect.x = clip.x + (int)(clip.w / 2.0f - width / 2.0f);
	rect.y = clip.y + (int)(clip.h / 2.0f - height / 2.0f);
	rect.w = width;
	rect.h = height;
	wz_renderer_draw_image(wz_renderer_get_context(renderer), rect, image);
}

struct wzFrame *CreateFrame(const char *label)
{
	struct wzFrame *frame = wz_frame_create();
	wz_widget_set_stretch((struct wzWidget *)frame, WZ_STRETCH);
	wz_main_window_add(mainWindow, (struct wzWidget *)frame);

	widgetCategories[nWidgetCategories].label = label;
	widgetCategories[nWidgetCategories].frame = frame;
	nWidgetCategories++;

	return frame;
}

void SetFrame(int index)
{
	size_t i;

	for (i = 0; i < nWidgetCategories; i++)
	{
		wz_widget_set_visible((struct wzWidget *)widgetCategories[i].frame, i == index);
	}
}

void CreateButtonFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	struct wzButton *button;
	wzWidgetStyle style;
	
	frame = CreateFrame("Button");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	wz_stack_layout_add(layout, (struct wzWidget *)wz_button_create("Button with a label", NULL));
	wz_stack_layout_add(layout, (struct wzWidget *)wz_button_create("Button with a label and icon", "../examples/data/accept.png"));
	wz_stack_layout_add(layout, (struct wzWidget *)wz_button_create(NULL, "../examples/data/accept.png"));

	button = wz_button_create("Custom padding", NULL);
	wz_button_set_padding_args(button, 20, 40, 20, 40);
	wz_stack_layout_add(layout, (struct wzWidget *)button);

	button = wz_button_create("Toggle Button", NULL);
	wz_button_set_set_behavior(button, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);
	wz_stack_layout_add(layout, (struct wzWidget *)button);

	button = wz_button_create("Custom style", NULL);
	style = wz_widget_get_style((const struct wzWidget *)button);
	style.button.textColor = nvgRGBf(1, 0, 1);
	style.button.bgColor1 = nvgRGB(100, 0, 0);
	style.button.bgColor2 = nvgRGB(0, 100, 0);
	style.button.bgPressedColor1 = nvgRGB(80, 0, 0);
	style.button.bgPressedColor2 = nvgRGB(0, 80, 0);
	wz_widget_set_style((struct wzWidget *)button, style);
	wz_stack_layout_add(layout, (struct wzWidget *)button);
}

void CreateCheckboxFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	
	frame = CreateFrame("Checkbox");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	wz_stack_layout_add(layout, (struct wzWidget *)wz_check_box_create("Toggle me!"));
}

void CreateComboFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	struct wzCombo *combo1, *combo2;
	
	frame = CreateFrame("Combo");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	combo1 = wz_combo_create((uint8_t *)listData, sizeof(const char *), LIST_DATA_LENGTH);
	wz_stack_layout_add(layout, (struct wzWidget *)combo1);

	combo2 = wz_combo_create((uint8_t *)listData, sizeof(const char *), LIST_DATA_LENGTH);
	wz_widget_set_font((struct wzWidget *)combo2, "visitor1", 12);
	wz_stack_layout_add(layout, (struct wzWidget *)combo2);
}

void CreateGroupBoxFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	struct wzGroupBox *groupBox1;
	struct wzGroupBox *groupBox2;

	frame = CreateFrame("GroupBox");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	groupBox1 = wz_group_box_create("With a Label");
	wz_widget_set_size_args((struct wzWidget *)groupBox1, 200, 200);
	wz_stack_layout_add(layout, (struct wzWidget *)groupBox1);
	
	wz_group_box_add(groupBox1, (struct wzWidget *)wz_label_create("Default margin"));

	groupBox2 = wz_group_box_create(NULL);
	wz_widget_set_size_args((struct wzWidget *)groupBox2, 200, 200);
	wz_stack_layout_add(layout, (struct wzWidget *)groupBox2);
}

void CreateLabelFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	struct wzLabel *label;

	frame = CreateFrame("Label");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	wz_stack_layout_add(layout, (struct wzWidget *)wz_label_create("Normal label"));

	label = wz_label_create("Multiline label with color. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
	wz_label_set_multiline(label, true);
	wz_label_set_text_color(label, nvgRGBf(0, 0.5f, 0));
	wz_widget_set_width((struct wzWidget *)label, 400);
	wz_stack_layout_add(layout, (struct wzWidget *)label);

	label = wz_label_create("Label with custom font and color");
	wz_label_set_text_color(label, nvgRGBf(1, 0.5f, 0.5f));
	wz_widget_set_font((struct wzWidget *)label, "visitor1", 32);
	wz_stack_layout_add(layout, (struct wzWidget *)label);
}

void CreateListFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	struct wzList *list;
	
	frame = CreateFrame("List");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_HORIZONTAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	list = wz_list_create((uint8_t *)listData, sizeof(const char *), LIST_DATA_LENGTH);
	wz_widget_set_size_args((struct wzWidget *)list, 120, 200);
	wz_stack_layout_add(layout, (struct wzWidget *)list);

	list = wz_list_create((uint8_t *)listData, sizeof(const char *), LIST_DATA_LENGTH);
	wz_widget_set_size_args((struct wzWidget *)list, 240, 300);
	wz_widget_set_font((struct wzWidget *)list, "visitor1", 32);
	wz_stack_layout_add(layout, (struct wzWidget *)list);

	list = wz_list_create((uint8_t *)customListData, sizeof(const char *), CUSTOM_LIST_DATA_LENGTH);
	wz_list_set_item_height(list, 40);
	wz_list_set_draw_item_callback(list, CustomDrawListItemCallback);
	wz_widget_set_size_args((struct wzWidget *)list, 50, 200);
	wz_stack_layout_add(layout, (struct wzWidget *)list);
}

void CreateRadioButtonFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	
	frame = CreateFrame("Radio Button");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	wz_stack_layout_add(layout, (struct wzWidget *)wz_radio_button_create("Option 1"));
	wz_stack_layout_add(layout, (struct wzWidget *)wz_radio_button_create("Option 2"));
	wz_stack_layout_add(layout, (struct wzWidget *)wz_radio_button_create("Option 3"));
}

void CreateScrollerFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	struct wzScroller *scroller;
	
	frame = CreateFrame("Scroller");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	scroller = wz_scroller_create(WZ_SCROLLER_VERTICAL, 20, 10, 100);
	wz_widget_set_height((struct wzWidget *)scroller, 200);
	wz_stack_layout_add(layout, (struct wzWidget *)scroller);

	scroller = wz_scroller_create(WZ_SCROLLER_HORIZONTAL, 50, 10, 100);
	wz_widget_set_width((struct wzWidget *)scroller, 200);
	wz_stack_layout_add(layout, (struct wzWidget *)scroller);
}

void CreateSpinnerFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	struct wzSpinner *spinner;
	
	frame = CreateFrame("Spinner");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	spinner = wz_spinner_create();
	wz_spinner_set_value(spinner, 42);
	wz_stack_layout_add(layout, (struct wzWidget *)spinner);

	spinner = wz_spinner_create();
	wz_widget_set_font((struct wzWidget *)spinner, "visitor1", 32);
	wz_stack_layout_add(layout, (struct wzWidget *)spinner);
}

void CreateTabbedFrame()
{
	struct wzFrame *frame;
	struct wzTabbed *tabbed;
	struct wzButton *tabButton;
	struct wzWidget *tabPage;
	
	frame = CreateFrame("Tabbed");

	tabbed = wz_tabbed_create();
	wz_widget_set_margin_uniform((struct wzWidget *)tabbed, 8);
	wz_widget_set_stretch((struct wzWidget *)tabbed, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)tabbed);

	wz_tabbed_add_tab(tabbed, &tabButton, &tabPage);
	wz_button_set_label(tabButton, "Tab 1");

	wz_tabbed_add_tab(tabbed, &tabButton, &tabPage);
	wz_button_set_label(tabButton, "Another Tab");

	wz_tabbed_add_tab(tabbed, &tabButton, &tabPage);
	wz_button_set_label(tabButton, "TabTabTab");
}

void CreateTextEditFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	struct wzTextEdit *textEdit;

	frame = CreateFrame("Text Edit");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	textEdit = wz_text_edit_create(false, 256);
	wz_text_edit_set_text(textEdit, "this is a very long string so scrolling can be tested");
	wz_widget_set_width((struct wzWidget *)textEdit, 300);
	wz_stack_layout_add(layout, (struct wzWidget *)textEdit);

	textEdit = wz_text_edit_create(false, 256);
	wz_text_edit_set_text(textEdit, "text edit with a custom font");
	wz_widget_set_font((struct wzWidget *)textEdit, "visitor1", 32);
	wz_widget_set_stretch((struct wzWidget *)textEdit, WZ_STRETCH_WIDTH);
	wz_stack_layout_add(layout, (struct wzWidget *)textEdit);

	textEdit = wz_text_edit_create(true, 256);
	wz_text_edit_set_text(textEdit, "NanoVG is small antialiased vector graphics rendering library for OpenGL. It has lean API modeled after HTML5 canvas API. It is aimed to be a practical and fun toolset for building scalable user interfaces and visualizations.");
	wz_widget_set_size_args((struct wzWidget *)textEdit, 200, 100);
	wz_stack_layout_add(layout, (struct wzWidget *)textEdit);
}

void ShowWindow1(wzEvent *e)
{
	wz_widget_set_visible((struct wzWidget *)window1, e->button.isSet);
}

void ShowWindow2(wzEvent *e)
{
	wz_widget_set_visible((struct wzWidget *)window2, e->button.isSet);
}

void CreateWindowFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *layout;
	struct wzButton *showWindow;
	
	frame = CreateFrame("Window");

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)layout);

	showWindow = wz_check_box_create("Show Window 1");
	wz_button_add_callback_clicked(showWindow, ShowWindow1);
	wz_stack_layout_add(layout, (struct wzWidget *)showWindow);

	showWindow = wz_check_box_create("Show Window 2");
	wz_button_add_callback_clicked(showWindow, ShowWindow2);
	wz_stack_layout_add(layout, (struct wzWidget *)showWindow);
}

void CreateStackLayoutFrame()
{
	struct wzFrame *frame;
	struct wzStackLayout *frameLayout;

	frame = CreateFrame("Stack Layout");

	frameLayout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)frameLayout, 8);
	wz_widget_set_stretch((struct wzWidget *)frameLayout, WZ_STRETCH);
	wz_frame_add(frame, (struct wzWidget *)frameLayout);

	{
		struct wzStackLayout *layout;
		struct wzButton *button;

		wz_stack_layout_add(frameLayout, (struct wzWidget *)wz_label_create("Horizontal Stack Layout"));

		layout = wz_stack_layout_create(WZ_STACK_LAYOUT_HORIZONTAL, 8);
		wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH_WIDTH);
		wz_widget_set_height((struct wzWidget *)layout, 100);
		wz_stack_layout_add(frameLayout, (struct wzWidget *)layout);

		wz_stack_layout_add(layout, (struct wzWidget *)wz_button_create("Default", NULL));

		button = wz_button_create("Align Top", NULL);
		wz_widget_set_align((struct wzWidget *)button, WZ_ALIGN_TOP);
		wz_stack_layout_add(layout, (struct wzWidget *)button);

		button = wz_button_create("Align Middle", NULL);
		wz_widget_set_align((struct wzWidget *)button, WZ_ALIGN_MIDDLE);
		wz_stack_layout_add(layout, (struct wzWidget *)button);

		button = wz_button_create("Align Bottom", NULL);
		wz_widget_set_align((struct wzWidget *)button, WZ_ALIGN_BOTTOM);
		wz_stack_layout_add(layout, (struct wzWidget *)button);

		button = wz_button_create("Stretch Width", NULL);
		wz_widget_set_stretch((struct wzWidget *)button, WZ_STRETCH_WIDTH);
		wz_stack_layout_add(layout, (struct wzWidget *)button);

		button = wz_button_create("Stretch Height", NULL);
		wz_widget_set_stretch((struct wzWidget *)button, WZ_STRETCH_HEIGHT);
		wz_stack_layout_add(layout, (struct wzWidget *)button);
	}

	{
		struct wzLabel *label;
		struct wzStackLayout *layout;
		struct wzButton *button;

		label = wz_label_create("Vertical Stack Layout");
		wz_widget_set_margin_args((struct wzWidget *)label, 16, 0, 0, 0);
		wz_stack_layout_add(frameLayout, (struct wzWidget *)label);

		layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
		wz_widget_set_width((struct wzWidget *)layout, 300);
		wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH_HEIGHT);
		wz_stack_layout_add(frameLayout, (struct wzWidget *)layout);

		wz_stack_layout_add(layout, (struct wzWidget *)wz_button_create("Default", NULL));

		button = wz_button_create("Align Left", NULL);
		wz_widget_set_align((struct wzWidget *)button, WZ_ALIGN_LEFT);
		wz_stack_layout_add(layout, (struct wzWidget *)button);

		button = wz_button_create("Align Center", NULL);
		wz_widget_set_align((struct wzWidget *)button, WZ_ALIGN_CENTER);
		wz_stack_layout_add(layout, (struct wzWidget *)button);

		button = wz_button_create("Align Right", NULL);
		wz_widget_set_align((struct wzWidget *)button, WZ_ALIGN_RIGHT);
		wz_stack_layout_add(layout, (struct wzWidget *)button);

		button = wz_button_create("Stretch Width", NULL);
		wz_widget_set_stretch((struct wzWidget *)button, WZ_STRETCH_WIDTH);
		wz_stack_layout_add(layout, (struct wzWidget *)button);

		button = wz_button_create("Stretch Height", NULL);
		wz_widget_set_stretch((struct wzWidget *)button, WZ_STRETCH_HEIGHT);
		wz_stack_layout_add(layout, (struct wzWidget *)button);
	}
}

void CreateWindow1()
{
	struct wzStackLayout *layout;
	struct wzTextEdit *textEdit;
	struct wzButton *button;
	struct wzButton *checkbox;
	struct wzCombo *combo;

	window1 = wz_window_create("Test Window");
	wz_widget_set_rect_args((struct wzWidget *)window1, 650, 100, 300, 300);
	wz_widget_set_visible((struct wzWidget *)window1, false);
	wz_main_window_add(mainWindow, (struct wzWidget *)window1);

	layout = wz_stack_layout_create(WZ_STACK_LAYOUT_VERTICAL, 8);
	wz_widget_set_margin_uniform((struct wzWidget *)layout, 8);
	wz_widget_set_stretch((struct wzWidget *)layout, WZ_STRETCH);
	wz_window_add(window1, (struct wzWidget *)layout);

	textEdit = wz_text_edit_create(false, 256);
	wz_text_edit_set_text(textEdit, "this is a very long string so scrolling can be tested");
	wz_widget_set_stretch((struct wzWidget *)textEdit, WZ_STRETCH_WIDTH);
	wz_stack_layout_add(layout, (struct wzWidget *)textEdit);

	button = wz_button_create("Another Button", "../examples/data/accept.png");
	wz_widget_set_stretch((struct wzWidget *)button, WZ_STRETCH_WIDTH);
	wz_stack_layout_add(layout, (struct wzWidget *)button);

	checkbox = wz_check_box_create("Checkbox");
	wz_widget_set_align((struct wzWidget *)checkbox, WZ_ALIGN_CENTER);
	wz_stack_layout_add(layout, (struct wzWidget *)checkbox);

	combo = wz_combo_create((uint8_t *)listData, sizeof(const char *), LIST_DATA_LENGTH);
	wz_widget_set_align((struct wzWidget *)combo, WZ_ALIGN_RIGHT);
	wz_widget_set_font((struct wzWidget *)combo, "visitor1", 12);
	wz_stack_layout_add(layout, (struct wzWidget *)combo);

	button = wz_button_create("Yet Another Button", NULL);
	wz_widget_set_stretch((struct wzWidget *)button, WZ_STRETCH);
	wz_stack_layout_add(layout, (struct wzWidget *)button);
}

void CreateWindow2()
{
	struct wzTabbed *tabbed;
	struct wzButton *tabButton;
	struct wzWidget *tabPage;
	struct wzCombo *combo;
	struct wzButton *button;

	window2 = wz_window_create("Window with a long title");
	wz_widget_set_rect_args((struct wzWidget *)window2, 590, 500, 200, 200);
	wz_widget_set_visible((struct wzWidget *)window2, false);
	wz_main_window_add(mainWindow, (struct wzWidget *)window2);

	tabbed = wz_tabbed_create();
	wz_widget_set_margin_uniform((struct wzWidget *)tabbed, 8);
	wz_widget_set_stretch((struct wzWidget *)tabbed, WZ_STRETCH);
	wz_window_add(window2, (struct wzWidget *)tabbed);

	wz_tabbed_add_tab(tabbed, &tabButton, &tabPage);
	wz_button_set_label(tabButton, "Tab 1");

	combo = wz_combo_create((uint8_t *)listData, sizeof(const char *), LIST_DATA_LENGTH);
	wz_widget_set_position_args((struct wzWidget *)combo, 10, 10);
	wz_tab_page_add(tabPage, (struct wzWidget *)combo);

	wz_tabbed_add_tab(tabbed, &tabButton, &tabPage);
	wz_button_set_label(tabButton, "Another Tab");

	button = wz_button_create("Button Button Button", NULL);
	wz_widget_set_position_args((struct wzWidget *)button, 10, 10);
	wz_tab_page_add(tabPage, (struct wzWidget *)button);

	wz_tabbed_add_tab(tabbed, &tabButton, &tabPage);
	wz_button_set_label(tabButton, "TabTabTab");
}

void WidgetCategoryChanged(wzEvent *e)
{
	SetFrame(e->list.selectedItem);
}

void CreateWidgetCategoryWindow()
{
	struct wzWindow *window;
	struct wzList *list;

	window = wz_window_create("Widgets");
	wz_widget_set_width((struct wzWidget *)window, 200);
	wz_main_window_add(mainWindow, (struct wzWidget *)window);
	wz_main_window_dock_window(mainWindow, window, WZ_DOCK_POSITION_WEST);

	list = wz_list_create((uint8_t *)&widgetCategories[0], sizeof(struct WidgetCategoryListItem), nWidgetCategories);
	wz_list_set_selected_item(list, 0);
	wz_list_add_callback_item_selected(list, &WidgetCategoryChanged);
	wz_widget_set_margin_uniform((struct wzWidget *)list, 8);
	wz_widget_set_stretch((struct wzWidget *)list, WZ_STRETCH);
	wz_widget_set_font_size((struct wzWidget *)list, 18);
	wz_window_add(window, (struct wzWidget *)list);
}

void CreateGui(int windowWidth, int windowHeight, struct wzRenderer *renderer)
{
	struct wzMenuBar *menuBar;

	mainWindow = wz_main_window_create(renderer);
	wz_widget_set_size_args((struct wzWidget *)mainWindow, windowWidth, windowHeight);

	menuBar = wz_menu_bar_create();
	wz_main_window_set_menu_bar(mainWindow, menuBar);
	wz_menu_bar_button_set_label(wz_menu_bar_create_button(menuBar), "File");
	wz_menu_bar_button_set_label(wz_menu_bar_create_button(menuBar), "Edit");
	wz_menu_bar_button_set_label(wz_menu_bar_create_button(menuBar), "View");
	wz_menu_bar_button_set_label(wz_menu_bar_create_button(menuBar), "Options");
	wz_menu_bar_button_set_label(wz_menu_bar_create_button(menuBar), "Window");

	CreateButtonFrame();
	CreateCheckboxFrame();
	CreateComboFrame();
	CreateGroupBoxFrame();
	CreateLabelFrame();
	CreateListFrame();
	CreateRadioButtonFrame();
	CreateScrollerFrame();
	CreateSpinnerFrame();
	CreateTabbedFrame();
	CreateTextEditFrame();
	CreateWindowFrame();
	CreateStackLayoutFrame();
	CreateWindow1();
	CreateWindow2();
	CreateWidgetCategoryWindow();
	SetFrame(0);
}

void ShowError(const char *message)
{
	fprintf(stderr, "%s\n", message);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
}

Uint32 TextCursorBlinkCallback(Uint32 interval, void *param)
{
	SDL_Event ev;

	wz_main_window_toggle_text_cursor((struct wzMainWindow *)param);
    
    ev.type = SDL_USEREVENT;
    ev.user.type = SDL_USEREVENT;
    ev.user.code = 0;
    ev.user.data1 = NULL;
    ev.user.data2 = NULL;
    SDL_PushEvent(&ev);

    return interval;
}

wzKey ConvertKey(SDL_Keycode sym)
{
	static int keys[] =
	{
		SDLK_LEFT, WZ_KEY_LEFT,
		SDLK_RIGHT, WZ_KEY_RIGHT,
		SDLK_UP, WZ_KEY_UP,
		SDLK_DOWN, WZ_KEY_DOWN, 
		SDLK_HOME, WZ_KEY_HOME,
		SDLK_END, WZ_KEY_END,
		SDLK_RETURN, WZ_KEY_ENTER,
		SDLK_RETURN, WZ_KEY_ENTER,
		SDLK_KP_ENTER, WZ_KEY_ENTER,
		SDLK_DELETE, WZ_KEY_DELETE,
		SDLK_BACKSPACE, WZ_KEY_BACKSPACE,
		SDLK_LSHIFT, WZ_KEY_LSHIFT,
		SDLK_RSHIFT, WZ_KEY_RSHIFT,
		SDLK_LCTRL, WZ_KEY_LCONTROL,
		SDLK_RCTRL, WZ_KEY_RCONTROL
	};

	size_t i;

	for (i = 0; i < sizeof(keys) / sizeof(int); i += 2)
	{
		if (keys[i] == sym)
		{
			int key = keys[i + 1];

			if (SDL_GetModState() & KMOD_SHIFT)
			{
				key |= WZ_KEY_SHIFT_BIT;
			}

			if (SDL_GetModState() & KMOD_CTRL)
			{
				key |= WZ_KEY_CONTROL_BIT;
			}

			return (wzKey)key;
		}
	}

	return WZ_KEY_UNKNOWN;
}

int main(int argc, char **argv)
{
	SDL_Window *window;
	SDL_GLContext glContext;
	int windowWidth, windowHeight;
	SDL_Cursor *cursors[WZ_NUM_CURSORS];
	struct wzRenderer *renderer;
	SDL_TimerID textCursorTimer;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		ShowError(SDL_GetError());
		SDL_ClearError();
		return 1;
	}

	atexit(SDL_Quit);

	window = SDL_CreateWindow("WidgetZero Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

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

#ifdef _MSC_VER
	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		ShowError("ogl_LoadFunctions failed");
		return 1;
	}
#endif

	glClearColor(0.2510f, 0.2510f, 0.2510f, 1);
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);

	// Setup cursors.
	cursors[WZ_CURSOR_DEFAULT] = SDL_GetDefaultCursor();
	cursors[WZ_CURSOR_IBEAM] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	cursors[WZ_CURSOR_RESIZE_N_S] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	cursors[WZ_CURSOR_RESIZE_E_W] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	cursors[WZ_CURSOR_RESIZE_NE_SW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	cursors[WZ_CURSOR_RESIZE_NW_SE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);

	// Create the renderer.
	renderer = wz_renderer_create(nvgCreateGL2, nvgDeleteGL2, NVG_ANTIALIAS, "../examples/data", "DejaVuSans", 16.0f);

	if (!renderer)
	{
		ShowError(wz_renderer_get_error());
		return 1;
	}

	CreateGui(windowWidth, windowHeight, renderer);
	textCursorTimer = SDL_AddTimer(textCursorBlinkInterval, TextCursorBlinkCallback, mainWindow);

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
			wz_widget_set_size_args((struct wzWidget *)mainWindow, e.window.data1, e.window.data2);
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
		else if (e.type == SDL_MOUSEWHEEL)
		{
			wz_main_window_mouse_wheel_move(mainWindow, e.wheel.x, e.wheel.y);
		}
		else if (e.type == SDL_KEYDOWN)
		{
			wz_main_window_key_down(mainWindow, ConvertKey(e.key.keysym.sym));
		}
		else if (e.type == SDL_KEYUP)
		{
			wz_main_window_key_up(mainWindow, ConvertKey(e.key.keysym.sym));
		}
		else if (e.type == SDL_TEXTINPUT)
		{
			wz_main_window_text_input(mainWindow, e.text.text);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		wz_main_window_draw_frame(mainWindow);
		SDL_GL_SwapWindow(window);
		SDL_SetCursor(cursors[wz_main_window_get_cursor(mainWindow)]);
	}

	wz_renderer_destroy(renderer);
	return 0;
}
