/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common.h>

#include "blit.h"

#include "main.h"
#include "menu.h"

typedef struct {
	Entry *entries;
	int n_entries;
} MenuStruct;

MenuStruct menu_struct;

int sel = 0;

void ExtendLength() {
	int i, j;
	int longest_len = 0;

	// Get longest length
	for (i = 0; i < menu_struct.n_entries; i++) {
		if (menu_struct.entries[i].options) {
			int len = strlen(menu_struct.entries[i].name);
			if (len > longest_len)
				longest_len = len;
		}
	}

	// One more space
	longest_len += 1;

	// Extend all names
	for (i = 0; i < menu_struct.n_entries; i++) {
		if (menu_struct.entries[i].options) {
			for (j = strlen(menu_struct.entries[i].name); j < longest_len; j++) {
				menu_struct.entries[i].name[j] = ' ';
			}
		}
	}
}

void MenuReset(Entry *entries, int size_entries) {
	menu_struct.entries = entries;
	menu_struct.n_entries = size_entries / sizeof(Entry);
	sel = 0;

	ExtendLength();
}

void MenuExitFunction(int exit_mode) {
	if (menu_struct.entries[exit_mode].function) {
		menu_struct.entries[exit_mode].function();
	}
}

int MenuCtrl() {
	int direction = 0;
	if (button_on & PSP_CTRL_DOWN) direction = +1;
	if (button_on & PSP_CTRL_UP) direction = -1;

	sel = (menu_struct.n_entries + sel + direction) % menu_struct.n_entries;

	direction = -2;
	if (button_on & PSP_CTRL_LEFT) direction = -1;
	if (button_on & PSP_CTRL_CROSS) direction = 0;
	if (button_on & PSP_CTRL_RIGHT) direction = +1;

	if (button_on & PSP_CTRL_SELECT || button_on & PSP_CTRL_HOME) {
		direction = 0;
		sel = menu_struct.n_entries - 1;
	}

	if (direction > -2) {
		if (menu_struct.entries[sel].options) {
			int max = menu_struct.entries[sel].size_options / sizeof(char **);
			(*menu_struct.entries[sel].value) = (max + (*menu_struct.entries[sel].value) + direction) % max;
		}

		if (menu_struct.entries[sel].exit) {
			return sel;
		} else {
			MenuExitFunction(sel);
		}
	}

	return -1;
}

int MenuDisplay() {
	if (blit_setup() < 0)
		return -1;

	blit_string(CENTER(19), 6 * 8, 0x00FFFFFF, 0x8000FF00, "ADRENALINE VSH MENU");

	int i;
	for (i = 0; i < menu_struct.n_entries; i++) {
		u32 bc = (i == sel) ? 0x00FF8080 : 0xC00000FF;

		int y = (8 + i) * 8;

		int len = strlen(menu_struct.entries[i].name);

		int center = len;
		if (menu_struct.entries[i].options) {
			center += 8;
		}

		int x = CENTER(center);

		blit_string(x, y, 0x00FFFFFF, bc, menu_struct.entries[i].name);

		if (menu_struct.entries[i].options) {
			int max = menu_struct.entries[i].size_options / sizeof(char **);
			blit_string(x + (len + 1) * 8, y, 0x00FFFFFF, bc, menu_struct.entries[i].options[(*menu_struct.entries[i].value) % max]);
		}
	}

	return 0;
}