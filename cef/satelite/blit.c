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

extern u8 msx[];

int pwidth, pheight, bufferwidth, pixelformat;
u32 *vram32;

__attribute__((noinline)) u32 adjust_alpha(u32 col) {
	u32 alpha = col >> 24;

	if (alpha == 0) return col;
	if (alpha == 0xFF) return col;

	u32 c1 = col & 0x00FF00FF;
	u32 c2 = col & 0x0000FF00;
	u8 mul = (u8)(255 - alpha);
	c1 = ((c1 * mul) >> 8) & 0x00FF00FF;
	c2 = ((c2 * mul) >> 8) & 0x0000FF00;
	return (alpha << 24) | c1 | c2;
}

int blit_setup() {
	int unk;
	sceDisplayGetMode(&unk, &pwidth, &pheight);
	sceDisplayGetFrameBuf((void *)&vram32, &bufferwidth, &pixelformat, unk);
	if (bufferwidth == 0 || pixelformat != 3) return -1;

	return 0;
}

int blit_string(int sx, int sy, int fcolor, int bcolor, const char *msg) {
	u32 fg_col = adjust_alpha(fcolor);
	u32 bg_col = adjust_alpha(bcolor);

	if (bufferwidth == 0 || pixelformat != 3) return -1;

	int x;
	for (x = 0; msg[x] && x < (pwidth / 8); x++) {
		char code = msg[x] & 0x7F; // 7bit ANK

		int y;
		for (y = 0; y < 8; y++) {
			int offset = (sy + y) * bufferwidth + sx + x * 8;
			u8 font = y >= 7 ? 0x00 : msx[code * 8 + y];

			int p;
			for (p = 0; p < 8; p++) {
				u32 col = (font & 0x80) ? fg_col : bg_col;

				u32 alpha = col >> 24;
				if (alpha == 0) {
					vram32[offset] = col;
				} else if (alpha != 0xFF) {
					u32 c2 = vram32[offset];
					u32 c1 = c2 & 0x00FF00FF;
					c2 = c2 & 0x0000FF00;
					c1 = ((c1 * alpha) >> 8) & 0x00FF00FF;
					c2 = ((c2 * alpha) >> 8) & 0x0000FF00;
					vram32[offset] = (col & 0xFFFFFF) + c1 + c2;
				}

				font <<= 1;
				offset++;
			}
		}
	}

	return x;
}