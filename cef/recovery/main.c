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
#include "menu.h"
#include "installer.h"
#include "plugins.h"
#include "utils.h"

#include "options.h"

PSP_MODULE_INFO("Recovery mode", 0, 1, 0);
PSP_HEAP_SIZE_MAX();

static int usbStatus = 0;

int recovery_exit = 0;

u32 button_assign_value = 0;

AdrenalineConfig config;

Entry main_entries[] = {
	{ "Toggle USB", ToggleUSB, NULL, 0, NULL },
	{ "Configuration ->", Configuration, NULL, 0, NULL },
	{ "Run program at /PSP/GAME/RECOVERY/EBOOT.PBP", RunRecovery, NULL, 0, NULL },
	{ "Advanced ->", Advanced, NULL, 0, NULL },
	{ "CPU speed ->", CpuSpeed, NULL, 0, NULL },
	{ "Plugins ->", Plugins, NULL, 0, NULL },
	{ "Registry hacks ->", RegistryHacks, NULL, 0, NULL },
	{ "Exit", Exit, NULL, 0, NULL },
};

Entry configuration_entries[] = {
	{ "Back", MainMenu, NULL, 0, NULL },
	{ "Skip Sony logo", NULL, disenabled, sizeof(disenabled), &config.skiplogo },
	{ "Skip gameboot", NULL, disenabled, sizeof(disenabled), &config.skipgameboot },
	{ "Hide corrupt icons", NULL, disenabled, sizeof(disenabled), &config.hidecorrupt },
	{ "Hide MAC address", NULL, disenabled, sizeof(disenabled), &config.hidemacaddr },
	{ "Autorun program at /PSP/GAME/BOOT/EBOOT.PBP", NULL, disenabled, sizeof(disenabled), &config.startupprog },
	{ "UMD mode", NULL, umdmodes, sizeof(umdmodes), &config.umdmode },
	{ "Fake region", NULL, regions, sizeof(regions), &config.fakeregion },
	{ "Hide DLC's in game menu", NULL, disenabled, sizeof(disenabled), &config.hidedlcs },
	{ "Hide PIC0.PNG and PIC1.PNG in game menu", NULL, disenabled, sizeof(disenabled), &config.hidepic0pic1 },
	{ "Use extended colors", NULL, extendedcolors, sizeof(extendedcolors), &config.useextendedcolors },
	{ "Use Sony PSP OSK", NULL, disenabled, sizeof(disenabled), &config.usesonypsposk },
	{ "Use NoDRM engine", NULL, endisabled, sizeof(endisabled), &config.notusenodrmengine },
};

Entry advanced_entries[] = {
	{ "Back", MainMenu, NULL, 0, NULL },
	{ "Advanced configuration ->", AdvancedConfiguration, NULL, 0, NULL },
	{ "Reset settings", ResetSettings, NULL, 0, NULL },
};

Entry advanced_configuration_entries[] = {
	{ "Back", Advanced, NULL, 0, NULL },
	{ "Force high memory layout", NULL, disenabled, sizeof(disenabled), &config.forcehighmemory },
	{ "Execute BOOT.BIN in UMD/ISO", NULL, disenabled, sizeof(disenabled), &config.executebootbin },
	{ "XMB  plugins", NULL, endisabled, sizeof(endisabled), &config.notusexmbplugins },
	{ "GAME plugins", NULL, endisabled, sizeof(endisabled), &config.notusegameplugins },
	{ "POPS plugins", NULL, endisabled, sizeof(endisabled), &config.notusepopsplugins },
};

Entry cpu_speed_entries[] = {
	{ "Back", MainMenu, NULL, 0, NULL },
	{ "Speed in XMB", NULL, cpuspeeds, sizeof(cpuspeeds), &config.vshcpuspeed },
	{ "Speed in UMD/ISO", NULL, cpuspeeds, sizeof(cpuspeeds), &config.umdisocpuspeed },
};

Entry registry_hacks_entries[] = {
	{ "Back", MainMenu, NULL, 0, NULL },
	{ "Button assign", SetButtonAssign, buttonassign, sizeof(buttonassign), (int *)&button_assign_value },
	{ "Activate WMA", SetWMA, NULL, 0, NULL },
	{ "Activate Flash Player", SetFlashPlayer, NULL, 0, NULL },
};

void MainMenu() {
	MenuReset(main_entries, sizeof(main_entries), "Main menu", 1);
}

void ToggleUSB() {
	if (!usbStatus) {
		printf(" > USB enabled");
		sctrlStartUsb();
		usbStatus = 1;
		sceKernelDelayThread(1 * 1000 * 1000);
	} else {
		printf(" > USB disabled");
		sctrlStopUsb();
		usbStatus = 0;
		sceKernelDelayThread(1 * 1000 * 1000);
	}
}

void Configuration() {
	MenuReset(configuration_entries, sizeof(configuration_entries), "Configuration", 3);
}

void RunRecovery() {
	sctrlStopUsb();

	sctrlSESetConfig(&config);

	static u32 vshmain_args[0x100];
	memset(vshmain_args, 0, sizeof(vshmain_args));

	vshmain_args[0] = sizeof(vshmain_args);
	vshmain_args[1] = 0x20;
	vshmain_args[16] = 1;

	struct SceKernelLoadExecVSHParam param;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.argp = "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP";
	param.args = strlen(param.argp) + 1;
	param.vshmain_args = vshmain_args;
	param.vshmain_args_size = sizeof(vshmain_args);
	param.key = "game";

	sctrlKernelLoadExecVSHMs2(param.argp, &param);
}

void Advanced() {
	MenuReset(advanced_entries, sizeof(advanced_entries), "Advanced", 3);
}

void AdvancedConfiguration() {
	MenuReset(advanced_configuration_entries, sizeof(advanced_configuration_entries), "Advanced configuration", 3);
}

void CpuSpeed() {
	MenuReset(cpu_speed_entries, sizeof(cpu_speed_entries), "CPU speed", 3);
}

void RegistryHacks() {
	GetRegistryData("/CONFIG/SYSTEM/XMB", "button_assign", REG_TYPE_INT, &button_assign_value, sizeof(u32));
	MenuReset(registry_hacks_entries, sizeof(registry_hacks_entries), "Registry hacks", 3);
}

void SetButtonAssign(int sel) {
	SetRegistryData("/CONFIG/SYSTEM/XMB", "button_assign", REG_TYPE_INT, (void *)&button_assign_value, sizeof(u32));
}

void SetWMA(int sel) {
	u32 value = 0;
	GetRegistryData("/CONFIG/MUSIC", "wma_play", REG_TYPE_INT, &value, sizeof(u32));

	if (value == 1) {
		printf(" > WMA was already activated.");
	} else {
		printf(" > Activating WMA...");
		value = 1;
		SetRegistryData("/CONFIG/MUSIC", "wma_play", REG_TYPE_INT, (void *)&value, sizeof(u32));
	}

	sceKernelDelayThread(1 * 1000 * 1000);
	MenuResetSelection();
}

void SetFlashPlayer(int sel) {
	u32 value = 0;
	GetRegistryData("/CONFIG/BROWSER", "flash_activated", REG_TYPE_INT, &value, sizeof(u32));

	if (value == 1) {
		printf(" > Flash Player was already activated.");
	} else {
		printf(" > Activating Flash Player...");
		value = 1;
		SetRegistryData("/CONFIG/BROWSER", "flash_activated", REG_TYPE_INT, (void *)&value, sizeof(u32));
		SetRegistryData("/CONFIG/BROWSER", "flash_play", REG_TYPE_INT, (void *)&value, sizeof(u32));
	}

	sceKernelDelayThread(1 * 1000 * 1000);
	MenuResetSelection();
}

void Exit() {
	printf(" > Exiting recovery...");
	sceKernelDelayThread(700 * 1000);

	recovery_exit = 1;
}

int main(int argc, char *argv[]) {
	pspDebugScreenInit();

	SceIoStat stat;
	memset(&stat, 0, sizeof(SceIoStat));
	if (sceIoGetstat("ms0:/__ADRENALINE__/flash0", &stat) < 0)
		Installer();

	sctrlSEGetConfig(&config);

	MainMenu();

	while (!recovery_exit) {
		MenuDisplayCtrl();
	}

	sctrlStopUsb();

	sctrlSESetConfig(&config);

	sctrlKernelExitVSH(NULL);

	return 0;
}