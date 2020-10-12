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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <common.h>

#include "executable_patch.h"

// #define DEBUG

#ifdef DEBUG

void logmsg(char *msg);

#define log(...) \
{ \
	char msg[256]; \
	sprintf(msg,__VA_ARGS__); \
	logmsg(msg); \
}

#else

#define log(...);

#endif

extern RebootexConfig rebootex_config;
extern AdrenalineConfig config;

u32 sctrlHENFindImport(const char *szMod, const char *szLib, u32 nid);

SceUID sceKernelCreateHeap661(SceUID partitionid, SceSize size, int unk, const char *name);
void *sceKernelAllocHeapMemory661(SceUID heapid, SceSize size);
int sceKernelFreeHeapMemory661(SceUID heapid, void *block);
SceUID sceKernelAllocPartitionMemory661(SceUID partitionid, const char *name, int type, SceSize size, void *addr);
int sceKernelFreePartitionMemory661(SceUID blockid);
void *sceKernelGetBlockHeadAddr661(SceUID blockid);
int sceKernelSetDdrMemoryProtection661(void *addr, int size, int prot);
int sceKernelGetSystemStatus661();
void *sceKernelGetGameInfo661();

SceModule2 *sceKernelFindModuleByName661(const char *modname);
SceModule2 *sceKernelFindModuleByAddress661(u32 addr);
SceModule2 *sceKernelFindModuleByUID661(SceUID modid);
int sceKernelCheckExecFile661(void *buf, SceLoadCoreExecFileInfo *execInfo);
int sceKernelProbeExecutableObject661(void *buf, SceLoadCoreExecFileInfo *execInfo);

int sceKernelQuerySystemCall661(void *function);

int sceKermitSendRequest661(SceKermitRequest *request, u32 mode, u32 cmd, u32 args, u32 is_callback, u64 *resp);
int sceKermitRegisterVirtualIntrHandler661(int num, int (* handler)());

int sceKermitMemorySetArgument661(SceKermitRequest *request, int argc, const void *data, int size, int mode);

SceUID sceKernelLoadModuleBufferBootInitBtcnf661(int bufsize, void *buf, int flags, SceKernelLMOption *option);
SceUID sceKernelLoadModuleBuffer661(void *buf, SceSize bufsize, int flags, SceKernelLMOption *option);
SceUID sceKernelLoadModuleWithApitype2661(int apitype, const char *path, int flags, SceKernelLMOption *option);
SceUID sceKernelLoadModuleMs2661(int apitype, const char *path, int flags, SceKernelLMOption *option);
SceUID sceKernelLoadModule661(const char *path, int flags, SceKernelLMOption *option);
SceUID sceKernelStartModule661(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

int sceKernelExitVSHVSH661(struct SceKernelLoadExecVSHParam *param);

int scePowerRequestStandby661();
int scePowerRequestSuspend661();
int scePowerRequestColdReset661(int a0);
int scePowerIsPowerOnline661();
int scePowerIsBatteryCharging661();
int scePowerGetBatteryLifeTime661();
int scePowerGetBatteryChargingStatus661();
int scePowerSetIdleCallback661(int slot, int flags, u64 time, int (* callback)(int slot, u32 diff, int arg, int *unk), int arg);
int scePowerGetBatteryTemp661();
int scePowerGetBatteryVolt661();

int sceChkregCheckRegion661();
int sceChkregGetPsCode661(u8 *pscode);

int sceDisplaySetFrameBuf661(void *topaddr, int bufferwidth, int pixelformat, int sync);

int ReadFile(char *file, void *buf, int size);
int WriteFile(char *file, void *buf, int size);

void ClearCaches();

#endif