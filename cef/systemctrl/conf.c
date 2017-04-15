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

int sctrlSEGetConfigEx(AdrenalineConfig *config, int size) {
	int k1 = pspSdkSetK1(0);
	
	memset(config, 0, size);
	SceUID fd = sceIoOpen("flash1:/config.adrenaline", PSP_O_RDONLY, 0);
	if (fd < 0) {
		pspSdkSetK1(k1);
		return -1;
	}

	int read = sceIoRead(fd, config, size);

	sceIoClose(fd);
	pspSdkSetK1(k1);

	return read;
}

int sctrlSESetConfigEx(AdrenalineConfig *config, int size) {
	int k1 = pspSdkSetK1(0);
		
	SceUID fd = sceIoOpen("flash1:/config.adrenaline", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0) {
		pspSdkSetK1(k1);
		return -1;
	}

	config->magic[0] = ADRENALINE_CFG_MAGIC_1;
	config->magic[1] = ADRENALINE_CFG_MAGIC_2;

	if (sceIoWrite(fd, config, size) < size) {
		sceIoClose(fd);
		pspSdkSetK1(k1);
		return -1;
	}

	sceIoClose(fd);
	pspSdkSetK1(k1);

	return 0;
}

int sctrlSEGetConfig(AdrenalineConfig *config) {
	return sctrlSEGetConfigEx(config, sizeof(AdrenalineConfig));
}

int sctrlSESetConfig(AdrenalineConfig *config) {
	return sctrlSESetConfigEx(config, sizeof(AdrenalineConfig));
}