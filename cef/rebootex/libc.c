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

unsigned int _strlen(const char *s) {
	int len = 0;
	while (*s) {
		s++;
		len++;
	}

	return len;
}

int _strcmp(const char *s1, const char *s2) {
	int val = 0;
	const u8 *u1, *u2;

	u1 = (u8 *)s1;
	u2 = (u8 *)s2;

	while (1) {
		if (*u1 != *u2) {
			val = (int) *u1 - (int) *u2;
			break;
		}

		if ((*u1 == 0) && (*u2 == 0)) {
			break;
		}

		u1++;
		u2++;
	}

	return val;
}

void *_memcpy(void *dst, const void *src, int len) {
	void *pRet = dst;
	const char *usrc = (const char *)src;
	char *udst = (char *)dst;

	while (len > 0) {
		*udst++ = *usrc++;
		len--;
	}

	return pRet;
}

void *_memset(void *b, int c, size_t len) {
	void *pRet = b;
	u8 *ub = (u8 *)b;

	while (len > 0) {
		*ub++ = (u8)c;
		len--;
	}

	return pRet;
}

void *_memmove(void *dst, const void *src, size_t len) {
	void *pRet = dst;
	char *udst;
	const char *usrc;

	if (dst < src) {
		// Copy forwards
		udst = (char *)dst;
		usrc = (const char *)src;
		while (len > 0) {
			*udst++ = *usrc++;
			len--;
		}
	} else {
		// Copy backwards
		udst = ((char *)dst) + len;
		usrc = ((const char *)src) + len;
		while (len > 0) {
			*--udst = *--usrc;
			len--;
		}
	}
	
	return pRet;
}