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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "wz_widget.h"

struct wzRadioButtonGroup
{
	struct wzButton **buttons;
};

struct wzRadioButtonGroup *wz_radio_button_group_create()
{
	struct wzRadioButtonGroup *group = malloc(sizeof(struct wzRadioButtonGroup));
	group->buttons = NULL;
	return group;
}

void wz_radio_button_group_destroy(struct wzRadioButtonGroup *group)
{
	free(group);
}

static void wz_radio_button_group_button_clicked(wzEvent e)
{
	int i;
	struct wzRadioButtonGroup *group;
	
	group = (struct wzRadioButtonGroup *)wz_widget_get_internal_metadata(e.base.widget);

	// Unset all the other buttons in the group.
	for (i = 0; i < wz_arr_len(group->buttons); i++)
	{
		if (group->buttons[i] != e.button.button)
		{
			wz_button_set(group->buttons[i], false);
		}
	}
}

void wz_radio_button_group_add_button(struct wzRadioButtonGroup *group, struct wzButton *button)
{
	int i;

	assert(group);
	assert(button);

	// Don't add it if it already exists.
	for (i = 0; i < wz_arr_len(group->buttons); i++)
	{
		if (group->buttons[i] == button)
			return;
	}

	wz_button_set_set_behavior(button, WZ_BUTTON_SET_BEHAVIOR_STICKY);
	wz_button_add_callback_clicked(button, wz_radio_button_group_button_clicked);
	wz_widget_set_internal_metadata((struct wzWidget *)button, group);
	wz_arr_push(group->buttons, button);

	// Set the first button added.
	if (wz_arr_len(group->buttons) == 1)
	{
		wz_button_set(button, true);
	}
}

void wz_radio_button_group_remove_button(struct wzRadioButtonGroup *group, struct wzButton *button)
{
	wz_arr_delete(group->buttons, button);
}
