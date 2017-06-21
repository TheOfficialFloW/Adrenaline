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

#include "adrenaline_suprx.h"
#include "adrenaline_skprx.h"
#include "usbdevice_skprx.h"

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
	{ "ms0:/__ADRENALINE__/adrenaline.suprx", adrenaline_suprx, sizeof(adrenaline_suprx) },
	{ "ms0:/__ADRENALINE__/adrenaline.skprx", adrenaline_skprx, sizeof(adrenaline_skprx) },
	{ "ms0:/__ADRENALINE__/usbdevice.skprx", usbdevice_skprx, sizeof(usbdevice_skprx) },
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

	if (sctrlSEGetVersion() >= 0x00050000) {
		ErrorExit(5000, "This update or a higher one was already applied.\n");
	}

	printf("6.61 Adrenaline-5.1 Installer\n");
	printf("Changes:\n\n");

	if (sctrlSEGetVersion() <= 0x00050000) {
		printf("- Added ability to skip adrenaline boot logo.\n");
		printf("- Added message for original filter.\n");
		printf("- Fixed bug where payloadex was not updated and caused some bugs.\n");
		printf("- Fixed '20000006' bug on PS TV. Network update will work on PS TV in the future.\n");
		printf("- Changed CPU clock back to 333 MHz.\n");
		printf("\n");
	}

	if (sctrlSEGetVersion() <= 0x00040002) {
		printf("- Added 'Hide DLC's in game menu' functionality.\n");
		printf("- Readded 'Original' graphics filtering, since PS1 games have got framedrops using custom filters.\n");
		printf("- Fixed corrupted icons bug that was introduced in the previous update.\n");
		printf("- Fixed bug where the framebuffer was corrupted after loading savestate.\n");
		printf("- Adrenaline icon is now hidden in game menu.\n");
		printf("\n");
	}

	if (sctrlSEGetVersion() <= 0x00040001) {
		printf("- Added support for ISO sorting using 'Game Categories Lite' plugin.\n");
		printf("- Fixed compatiblity with 'Kingdom Hearts: Birth by Sleep' english patch.\n");
		printf("\n");
	}

	if (sctrlSEGetVersion() <= 0x00040000) {
		printf("- Fixed bug where holding R trigger while launching Adrenaline didn't open the recovery menu.\n");
		printf("- Fixed msfs truncation bug that caused savedata corruption for Little Big Planet and maybe other games.\n");
		printf("- Fixed wrong scale of PS1 games on PS TV.\n");
		printf("\n");
	}

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