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

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/fcntl.h>

#include <stdio.h>
#include <string.h>

void debug_printf(char *msg) {
	SceUID fd = ksceIoOpen("ux0:adrenaline_kernel_log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
	if (fd >= 0) {
		ksceIoWrite(fd, msg, strlen(msg));
		ksceIoClose(fd);
	}
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = ksceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int read = ksceIoRead(fd, buf, size);

	ksceIoClose(fd);
	return read;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = ksceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = ksceIoWrite(fd, buf, size);

	ksceIoClose(fd);
	return written;
}