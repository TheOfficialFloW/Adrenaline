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
#include <vshctrl.h>

#include "main.h"
#include "menu.h"

PSP_MODULE_INFO("VshCtrlSatelite", 0, 1, 0);

char *cpuspeeds[] = { "Default", "20/10", "75/37", "100/50", "133/66", "222/111", "266/133", "300/150", "333/166" };
char *umdmodes[] = { "Inferno", "M33 Driver", "Sony NP9660" };

AdrenalineConfig config;

void SuspendDevice();
void RecoveryMenu();
void RestartVSH();

Entry entries[] = {
	{ "CPU CLOCK XMB", NULL, cpuspeeds, sizeof(cpuspeeds), &config.vshcpuspeed, 0 },
	{ "CPU CLOCK GAME", NULL, cpuspeeds, sizeof(cpuspeeds), &config.umdisocpuspeed, 0 },
	{ "UMD ISO MODE", NULL, umdmodes, sizeof(umdmodes), &config.umdmode, 0 },
	{ "SUSPEND DEVICE", SuspendDevice, NULL, 0, NULL, 1 },
	{ "RECOVERY MENU", RecoveryMenu, NULL, 0, NULL, 1 },
	{ "RESTART VSH", RestartVSH, NULL, 0, NULL, 1 },
	{ "EXIT", NULL, NULL, 0, NULL, 1 },
};

int exit_mode = -1;

u32 cur_buttons = -1, button_on = 0;

SceUID vshmenu_thid;

int ctrl_handler(SceCtrlData *pad_data, int count) {
	button_on = pad_data->Buttons & ~cur_buttons;
	cur_buttons = pad_data->Buttons;

	int i;
	for (i = 0; i < count; i++) {
		pad_data[i].Buttons &= ~ALL_CTRL;
	}

	return 0;
}

void RecoveryMenu() {
	sctrlSESetBootConfFileIndex(BOOT_RECOVERY);
	sctrlKernelExitVSH(NULL);
}

void SuspendDevice() {
	scePowerRequestSuspend();
}

void RestartVSH() {
	sctrlKernelExitVSH(NULL);
}

int VshMenu_Thread() {
	sceKernelChangeThreadPriority(0, 8);
	sctrlSEGetConfig(&config);
	vctrlVSHRegisterVshMenu(ctrl_handler);

	int exit_sel = 0, menu_mode = 0;

	MenuReset(entries, sizeof(entries));

	while (exit_mode < 0) {
		sceDisplayWaitVblankStart();

		if (menu_mode > 0) {
			MenuDisplay();
		}

		switch(menu_mode) {
			case 0:
				if (!(cur_buttons & ALL_CTRL)) {
					menu_mode++;
				}
				break;

			case 1:
				exit_sel = MenuCtrl();
				if (exit_sel >= 0) {
					menu_mode++;
				}
				break;

			case 2:
				if (!(cur_buttons & ALL_CTRL)) {
					exit_mode = exit_sel;
				}
				break;
		}
	}

	sctrlSESetConfig(&config);

	MenuExitFunction(exit_mode);

	vctrlVSHExitVSHMenu(&config);

	return sceKernelExitDeleteThread(0);
}

int module_start() {
	vshmenu_thid = sceKernelCreateThread("VshMenu_Thread", VshMenu_Thread, 0x10, 0x1000, 0, NULL);
	if (vshmenu_thid >= 0)
		sceKernelStartThread(vshmenu_thid, 0, NULL);

	return 0;
}

int module_stop() {
	exit_mode = 0;

	SceUInt timeout = 100000;
	if (sceKernelWaitThreadEnd(vshmenu_thid, &timeout) < 0)
		sceKernelTerminateDeleteThread(vshmenu_thid);

	return 0;
}