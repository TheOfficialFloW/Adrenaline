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

#include "adrenaline_user.h"
#include "adrenaline_kernel.h"

PSP_MODULE_INFO("updater", 0x800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#define printf pspDebugScreenPrintf

int sctrlRebootDevice();

typedef struct {
	char *path;
	void *buf;
	int size;
} File;

File files[] = {
	{ "ms0:/__ADRENALINE__/sce_module/adrenaline_user.suprx", adrenaline_user, sizeof(adrenaline_user) },
	{ "ms0:/__ADRENALINE__/sce_module/adrenaline_kernel.skprx", adrenaline_kernel, sizeof(adrenaline_kernel) },
};

void ErrorExit(int milisecs, char *fmt, ...) {
	va_list list;
	char msg[256];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	printf(msg);

	sceKernelDelayThread(milisecs * 1000);
	sceKernelExitGame();
	sceKernelSleepThread();
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int main(void) {
	pspDebugScreenInit();

	if (sctrlSEGetVersion() < 0x00060004) {
		ErrorExit(5000, "This update can only be applied with v6.4 or higher.\n");
	}

	if (sctrlSEGetVersion() >= ADRENALINE_VERSION) {
		ErrorExit(5000, "This update or a higher one was already applied.\n");
	}

	printf("6.61 Adrenaline-%d.%d Installer\n", ADRENALINE_VERSION_MAJOR, ADRENALINE_VERSION_MINOR);
	printf("Changes:\n\n");

	printf("- Added support for native resolution patches.\n");
	printf("\n");

	printf("Press X to install, R to exit.\n\n");

	while (1) {
		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
			break;
		else if (pad.Buttons & PSP_CTRL_RTRIGGER)
			ErrorExit(5000, "Cancelled by user.\n");
	}

	int i;
	for (i = 0; i < (sizeof(files) / sizeof(File)); i++) {
		char *p = strrchr(files[i].path, '/');
		printf("Writing %s (%d)... ", p+1, files[i].size);
		sceKernelDelayThread(100 * 1000);

		int written = WriteFile(files[i].path, files[i].buf, files[i].size);
		if (written != files[i].size) {
			ErrorExit(5000, "Error 0x%08X\n", written);
		}

		printf("OK\n");
		sceKernelDelayThread(100 * 1000);
	}

	printf("\nUpdate complete. Press X to reboot your device.\n\n");

	while (1) {
		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
			break;
	}

	printf("Rebooting...\n");
	sceKernelDelayThread(2 * 1000 * 1000);
	sctrlRebootDevice();

	return 0;
}
