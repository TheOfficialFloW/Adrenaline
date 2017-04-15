/*
	Adrenaline
	Copyright (C) 2016-2017, TheFloW

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

#include "main.h"
#include "menu.h"
#include "utils.h"

#include "options.h"

#define MAX_PLUGINS_PER_MODE 8
#define MAX_PLUGINS MAX_PLUGINS_PER_MODE * 3

typedef struct {
	char name[64];
	int activated;
} PluginsInfo;
PluginsInfo plugins_info[MAX_PLUGINS];

char plugins_path[MAX_PLUGINS][64]; //8 vsh, 8 game, 8 pops

Entry plugins_tool_entries[MAX_PLUGINS + 1]; //+ Back entry

int n_vsh = 0, n_game = 0, n_pops = 0;

void trim(char *str) {
	int i;
	for (i = strlen(str) - 1; i >= 0; i--) {
		if (str[i] == 0x20 || str[i] == '\t') {
			str[i] = 0;
		} else {
			break;
		}
	}
}

int GetPlugin(char *buf, int size, char *str, int *activated) {
	int n = 0, i = 0;
	char *s = str;

	while (1) {
		if (i >= size) {
			break;
		}

		char ch = buf[i];

		if (ch < 0x20 && ch != '\t') {
			if (n != 0) {
				i++;
				break;
			}
		} else {
			*str++ = ch;
			n++;
		}

		i++;
	}

	trim(s);

	*activated = 0;

	if (i > 0) {
		char *p = strpbrk(s, " \t");
		if (p) {
			char *q = p + 1;

			while (*q < 0) {
				q++;
			}

			if (strcmp(q, "1") == 0) {
				*activated = 1;
			}

			*p = 0;
		}
	}

	return i;
}

void SavePlugins(char *mode, int start, int end) {
	char file[64];
	sprintf(file, "ms0:/seplugins/%s.txt", mode);

	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd >= 0) {
		int i;
		for (i = start; i < end; i++) {
			if (plugins_path[i][0] != '\0') {
				char string[128];
				sprintf(string, "%s %d\r\n", plugins_path[i], plugins_info[i].activated);
				sceIoWrite(fd, string, strlen(string));
			}
		}

		sceIoClose(fd);
	}
}

int ReadPlugins(char *mode, int get_name, int n) {
	int i = 0;

	char file[64];
	sprintf(file, "ms0:/seplugins/%s.txt", mode);

	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	if (fd >= 0) {
		char buffer[1024];
		int size = sceIoRead(fd, buffer, sizeof(buffer));
		char *p = buffer;

		int res = 0;

		do {
			res = GetPlugin(p, size, plugins_path[n], &plugins_info[n].activated);

			if (res > 0) {
				char *q = strrchr(plugins_path[n], '/');
				if (q) {
					if (get_name) {
						sprintf(plugins_info[n].name, "%s [%s]", q + 1, mode);
					}

					i++;
					n++;
				}

				size -= res;
				p += res;
			}
		} while (res > 0 && i < MAX_PLUGINS_PER_MODE);

		sceIoClose(fd);
	}

	return i;
}

void SetPlugins(int sel) {
	char *mode = NULL;
	int start = 0, end = 0;

	sel -= 1;

	if (sel >= 0 && sel < n_vsh) {
		mode = "vsh";
		start = 0;
		end = n_vsh;
	} else if (sel >= n_vsh && sel < n_vsh+n_game) {
		mode = "game";
		start = n_vsh;
		end = n_vsh+n_game;
	} else if (sel >= n_vsh+n_game && sel < n_vsh+n_game+n_pops) {
		mode = "pops";
		start = n_vsh+n_game;
		end = n_vsh+n_game+n_pops;
	}

	// Save
	SavePlugins(mode, start, end);
}

void Plugins() {
	memset(plugins_tool_entries, 0, sizeof(plugins_tool_entries));
	memset(plugins_path, 0, sizeof(plugins_path));
	memset(plugins_info, 0, sizeof(plugins_info));

	n_vsh = ReadPlugins("VSH", 1, 0);
	n_game = ReadPlugins("GAME", 1, n_vsh);
	n_pops = ReadPlugins("POPS", 1, n_vsh+n_game);

	plugins_tool_entries[0].name = "Back";
	plugins_tool_entries[0].function = (void *)MainMenu;

	int i;
	for (i = 0; i < (n_vsh+n_game+n_pops); i++) {
		plugins_tool_entries[i + 1].name = plugins_info[i].name;
		plugins_tool_entries[i + 1].function = (void *)SetPlugins;
		plugins_tool_entries[i + 1].options = disenabled;
		plugins_tool_entries[i + 1].size_options = sizeof(disenabled);
		plugins_tool_entries[i + 1].value = &plugins_info[i].activated;
	}

	MenuReset(plugins_tool_entries, (n_vsh+n_game+n_pops + 1) * sizeof(Entry), "Plugins manager", 3);
}