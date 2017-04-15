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

#ifndef __STRING_CLONE_H__
#define __STRING_CLONE_H__

char *strtok_r_clone(char *string, const char *seps, char **context);
char *strtok_clone(char *str, const char *seps);
void atob_clone(char *a0, int *a1);
size_t strspn_clone(const char *s, const char *accept);
size_t strcspn_clone(const char *s, const char *reject);
char *strncat_clone(char *s, const char *append, size_t count);

#endif