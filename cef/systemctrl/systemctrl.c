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
#include "adrenaline.h"

int sctrlKernelSetUserLevel(int level) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelGetUserLevel();

	SceModule2 *mod = sceKernelFindModuleByName661("sceThreadManager");
	u32 text_addr = mod->text_addr;

	u32 high = (((u32)_lh(text_addr + 0x358)) << 16);
	u32 low = ((u32)_lh(text_addr + 0x35C));

	if (low & 0x8000)
		high -= 0x10000;

	u32 *thstruct = (u32 *)_lw(high | low);
	thstruct[0x14/4] = (level ^ 8) << 28;

	pspSdkSetK1(k1);
	return res;
}

int sctrlHENIsSE() {
	return 1;
}

int sctrlHENIsDevhook() {
	return 0;
}

int sctrlHENGetVersion() {
	return 0x00001000;
}

int sctrlSEGetVersion() {
	return ADRENALINE_VERSION;
}

PspIoDrv *sctrlHENFindDriver(char *drvname) {
	int k1 = pspSdkSetK1(0);

	SceModule2 *mod = sceKernelFindModuleByName661("sceIOFileManager");
	u32 text_addr = mod->text_addr;

	u32 *(* GetDevice)(char *) = NULL;

	int i;
	for (i = 0; i < mod->text_size; i += 4) {
		u32 addr = text_addr + i;
		if (_lw(addr) == 0xA2200000) {
			GetDevice = (void *)K_EXTRACT_CALL(addr + 4);
			break;
		}
	}

	if (!GetDevice) {
		pspSdkSetK1(k1);
		return 0;
	}

	u32 *u = GetDevice(drvname);
	if (!u) {
		pspSdkSetK1(k1);
		return 0;
	}

	pspSdkSetK1(k1);

	return (PspIoDrv *)u[1];
}

int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelExitVSHVSH661(param);
	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param) {
	int k1 = pspSdkSetK1(0);

	SceModule2 *mod = sceKernelFindModuleByName661("sceLoadExec");
	u32 text_addr = mod->text_addr;

	int (* LoadExecVSH)(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param, int unk2) = (void *)text_addr + 0x23D0;

	int res = LoadExecVSH(apitype, file, param, 0x10000);
	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelLoadExecVSHMs1(const char *file, struct SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(PSP_INIT_APITYPE_MS1, file, param);
}

int sctrlKernelLoadExecVSHMs2(const char *file, struct SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(PSP_INIT_APITYPE_MS2, file, param);
}

int sctrlKernelLoadExecVSHMs3(const char *file, struct SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(PSP_INIT_APITYPE_MS3, file, param);
}

int sctrlKernelLoadExecVSHMs4(const char *file, struct SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(PSP_INIT_APITYPE_MS4, file, param);
}

int sctrlKernelLoadExecVSHDisc(const char *file, struct SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(PSP_INIT_APITYPE_DISC, file, param);
}

int sctrlKernelLoadExecVSHDiscUpdater(const char *file, struct SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(PSP_INIT_APITYPE_DISC_UPDATER, file, param);
}

void SetConfig(AdrenalineConfig *conf) {
	memcpy(&config, conf, sizeof(AdrenalineConfig));
}

int sctrlKernelQuerySystemCall(void *function) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelQuerySystemCall661(function);
	pspSdkSetK1(k1);
	return res;
}

void sctrlHENPatchSyscall(u32 addr, void *newaddr) {
	void *ptr;
	asm("cfc0 %0, $12\n" : "=r"(ptr));

	u32 *syscalls = (u32 *)(ptr + 0x10);

	int i;
	for (i = 0; i < 0x1000; i++) {
		if ((syscalls[i] & 0x0FFFFFFF) == (addr & 0x0FFFFFFF)) {
			syscalls[i] = (u32)newaddr;
		}
	}
}

void SetUmdFile(char *file) {
	strncpy(rebootex_config.umdfilename, file, 255);
}

char *GetUmdFile() {
	return rebootex_config.umdfilename;
}

int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs) {
	int k1 = pspSdkSetK1(0);

	SetUmdFile(file);

	pspSdkSetK1(k1);
	return 0;
}

int sctrlSEGetBootConfBootFileIndex() {
	return rebootex_config.bootfileindex;
}

void sctrlSESetBootConfFileIndex(int index) {
	rebootex_config.bootfileindex = index;
}

void sctrlHENLoadModuleOnReboot(char *module_after, void *buf, int size, int flags) {
	rebootex_config.module_after = module_after;
	rebootex_config.buf = buf;
	rebootex_config.size = size;
	rebootex_config.flags = flags;
}

int sctrlGetUsbState() {
	return SendAdrenalineCmd(ADRENALINE_VITA_CMD_GET_USB_STATE);
}

int sctrlStartUsb() {
	return SendAdrenalineCmd(ADRENALINE_VITA_CMD_START_USB);
}

int sctrlStopUsb() {
	return SendAdrenalineCmd(ADRENALINE_VITA_CMD_STOP_USB);
}

int sctrlRebootDevice() {
	return SendAdrenalineCmd(ADRENALINE_VITA_CMD_POWER_REBOOT);
}