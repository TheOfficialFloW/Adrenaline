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

#include "main.h"

SceUID heapid = -1;

void oe_free(void *ptr) {
	sceKernelFreeHeapMemory661(heapid, ptr);
}

void *oe_malloc(SceSize size) {
	return sceKernelAllocHeapMemory661(heapid, size);
}

int mallocinit() {
	int keyconfig = sceKernelInitKeyConfig();
	if ((keyconfig == PSP_INIT_KEYCONFIG_POPS) || (keyconfig == PSP_INIT_KEYCONFIG_GAME && sceKernelInitApitype() == PSP_INIT_APITYPE_UMDEMU_MS1)) {
		return 0;
	}

	heapid = sceKernelCreateHeap661(PSP_MEMORY_PARTITION_KERNEL, 256 * 1024, 1, "");

	return (heapid < 0) ? heapid : 0;
}