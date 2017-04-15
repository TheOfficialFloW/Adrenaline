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

static char *__strtok_context;

char *strtok_r_clone(char *string, const char *seps, char **context) {
	char *head;  // start of word
	char *tail;  // end of word

	// If we're starting up, initialize context
	if (string)
		*context = string;

	// Get potential start of this next word
	head = *context;
	if (head == NULL)
		return NULL;

	// Skip any leading separators
	while (*head && strchr(seps, *head)) {
		head++;
	}

	 // Did we hit the end?
	if (*head == 0) {
		// Nothing left
		*context = NULL;
		return NULL;
	}

	// skip over word
	tail = head;
	while (*tail && !strchr(seps, *tail)) {
		tail++;
	}

	// Save head for next time in context
	if (*tail == 0) {
		*context = NULL;
	} else {
		*tail = 0;
		tail++;
		*context = tail;
	}

	// Return current word
	return head;
}

char *strtok_clone(char *str, const char *seps) {
	return strtok_r_clone(str, seps, &__strtok_context);
}

void atob_clone(char *a0, int *a1) {
	char *p;
	*a1 = strtol(a0, &p, 10);
}

size_t strspn_clone(const char *s, const char *accept) {
    const char *c;
    for (c = s; *c; c++) {
        if (!strchr(accept, *c))
			return c - s;
    }

    return c - s;
}

size_t strcspn_clone(const char *s, const char *reject) {
    const char *c;
    for (c = s; *c; c++) {
        if (strchr(reject, *c))
			return c - s;
    }

    return c - s;
}

char *strncat_clone(char *s, const char *append, size_t count) {
	char *pRet = s;

	while (*s)
		s++;

	while ((*append) && (count > 0)) {
		*s++ = *append++;
		count--;
	}

	*s = 0;

	return pRet;
}