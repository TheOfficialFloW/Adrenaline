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

#ifndef __MAIN_H__
#define __MAIN_H__

#define KIRK7_HEADER_SIZE 0x14

#define VERSION_KEY_SIZE 0x10
#define CONTENT_ID_SIZE 0x30

typedef struct {
	int type;
	u8 key[16];
	u8 pad[16];
	int pad_size;
} MAC_KEY;

int sceDrmBBMacInit(MAC_KEY *mkey, int type);
int sceDrmBBMacUpdate(MAC_KEY *mkey, u8 *buf, int size);
int sceDrmBBMacFinal(MAC_KEY *mkey, u8 *buf, u8 *vkey);

#endif