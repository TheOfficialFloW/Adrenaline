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

#ifndef __UTILS_H__
#define __UTILS_H__

#define debugPrintf(...) \
{ \
	char msg[128]; \
	snprintf(msg, sizeof(msg), __VA_ARGS__); \
	debug_printf(msg); \
}

void debug_printf(char *msg);
int ReadFile(char *file, void *buf, int size);
int WriteFile(char *file, void *buf, int size);

#endif