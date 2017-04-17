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
#include "adrenaline.h"
#include "executable_patch.h"
#include "libraries_patch.h"
#include "io_patch.h"
#include "init_patch.h"
#include "power_patch.h"
#include "utility_patch.h"
#include "sysmodpatches.h"
#include "nodrm.h"
#include "malloc.h"

PSP_MODULE_INFO("SystemControl", 0x1007, 1, 0);

int (* PrologueModule)(void *modmgr_param, SceModule2 *mod);

STMOD_HANDLER module_handler;

RebootexConfig rebootex_config;

AdrenalineConfig config;

int idle = 0;

#ifdef DEBUG

void logmsg(char *msg) {
	int k1 = pspSdkSetK1(0);

	SceUID fd = sceIoOpen("ms0:/systemctrl_log.txt", PSP_O_WRONLY | PSP_O_CREAT, 0777);
	if (fd >= 0) {
		sceIoLseek(fd, 0, PSP_SEEK_END);
		sceIoWrite(fd, msg, strlen(msg));
		sceIoClose(fd);
	}

	pspSdkSetK1(k1);
}

#endif

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int read = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return read;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

void ClearCaches() {
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
}

int PrologueModulePatched(void *modmgr_param, SceModule2 *mod) {
	int res = PrologueModule(modmgr_param, mod);

	if (res >= 0 && module_handler)
		module_handler(mod);

	return res;
}

STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler) {
	STMOD_HANDLER prev = module_handler;
	module_handler = (STMOD_HANDLER)((u32)handler | 0x80000000);
	return prev;
}

int sceResmgrDecryptIndexPatched(void *buf, int size, int *retSize) {
	int k1 = pspSdkSetK1(0);
	*retSize = ReadFile("flash0:/vsh/etc/version.txt", buf, size);
	pspSdkSetK1(k1);
	return 0;
}

int memcmp_patched(const void *b1, const void *b2, size_t len) {
	u32 tag = 0x4C9494F0;
	if (memcmp(&tag, b2, len) == 0) {
		static u8 kernel661_keys[0x10] = { 0x76, 0xF2, 0x6C, 0x0A, 0xCA, 0x3A, 0xBA, 0x4E, 0xAC, 0x76, 0xD2, 0x40, 0xF5, 0xC3, 0xBF, 0xF9 };
		memcpy((void *)0xBFC00220, kernel661_keys, sizeof(kernel661_keys));
		return 0;
	}

	return memcmp(b1, b2, len);
}

void PatchMemlmd() {
	SceModule2 *mod = sceKernelFindModuleByName661("sceMemlmd");
	u32 text_addr = mod->text_addr;

	// Allow 6.61 kernel modules
	MAKE_CALL(text_addr + 0x2C8, memcmp_patched);
}

void PatchInterruptMgr() {
	SceModule2 *mod = sceKernelFindModuleByName661("sceInterruptManager");
	u32 text_addr = mod->text_addr;

	// Allow execution of syscalls in kernel mode
	_sw(0x408F7000, text_addr + 0xE98);
	_sw(0, text_addr + 0xE9C);
}

void PatchModuleMgr() {
	SceModule2 *mod = sceKernelFindModuleByName661("sceModuleManager");
	u32 text_addr = mod->text_addr;

	// Patch to allow a full coverage of loaded modules
	PrologueModule = (void *)text_addr + 0x8270;
	MAKE_CALL(text_addr + 0x7158, PrologueModulePatched);

	// Patch PartitionCheck
	HIJACK_FUNCTION(text_addr + 0x811C, PartitionCheckPatched, PartitionCheck);

	// Dummy patch for LEDA
	MAKE_JUMP(sctrlHENFindImport("sceModuleManager", "ThreadManForKernel", 0x446D8DE6), sceKernelCreateThread);
}

void PatchLoadCore() {
	SceModule2 *mod = sceKernelFindModuleByName661("sceLoaderCore");
	u32 text_addr = mod->text_addr;

	// Patch to allow homebrews
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceKernelCheckExecFile661), sceKernelCheckExecFilePatched, _sceKernelCheckExecFile);
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceKernelProbeExecutableObject661), sceKernelProbeExecutableObjectPatched, _sceKernelProbeExecutableObject);

	// Allow custom modules
	PspUncompress = (void *)text_addr + 0x55E4;
	MAKE_CALL(text_addr + 0x4360, PspUncompressPatched);

	// Patch relocation check in switch statement (7 -> 0)
	_sw(_lw(text_addr + 0x7F10), text_addr + 0x7F2C);

	// Allow kernel modules to have syscall imports
	_sw(0x3C090000, text_addr + 0x3D70);

	// Allow lower devkit version
	_sw(0x1000FFCB, text_addr + 0x6AD0);

	// Allow higher devkit version
	_sw(0, text_addr + 0x7308);

	// Patch to resolve NIDs
	_sw(0x02203021, text_addr + 0x3A40); //move $a2, $s1
	search_nid_in_entrytable = (void *)text_addr + 0xEA8;
	MAKE_CALL(text_addr + 0x3A44, search_nid_in_entrytable_patched);
	_sw(0x02403821, text_addr + 0x3A48); //move $a3, $s2
	_sw(0, text_addr + 0x3BAC);
	_sw(0, text_addr + 0x3BB4);

	// Patch to undo prometheus patches
	HIJACK_FUNCTION(text_addr + 0x3BB8, aLinkLibEntriesPatched, aLinkLibEntries);

	// Patch call to init module_bootstart
	MAKE_CALL(text_addr + 0x1A28, PatchInit);
	_sw(0x02E02021, text_addr + 0x1A2C); //move $a0, $s7

	// Restore original call
	MAKE_CALL(text_addr + 0x5864, FindProc("sceMemlmd", "memlmd", 0xEF73E85B));

	// Find removed nids
	LoadCoreForKernel_nids[0].function = (void *)text_addr + 0x73B0;
	LoadCoreForKernel_nids[1].function = (void *)text_addr + 0x73F0;
}

int (* sceKernelVolatileMemTryLock)(int unk, void **ptr, int *size);

int sceKernelVolatileMemTryLockPatched(int unk, void **ptr, int *size) {
	int res = 0;

	int i;
	for (i = 0; i < 0x10; i++) {
		res = sceKernelVolatileMemTryLock(unk, ptr, size);
		if (res >= 0)
			break;

		sceKernelDelayThread(100);
	}

	return res;
}

void PatchVolatileMemBug() {
	if (sceKernelBootFrom() == PSP_BOOT_DISC) {
		sceKernelVolatileMemTryLock = (void *)FindProc("sceSystemMemoryManager", "sceSuspendForUser", 0xA14F40B2);
		sctrlHENPatchSyscall((u32)sceKernelVolatileMemTryLock, sceKernelVolatileMemTryLockPatched);
		ClearCaches();
	}
}

int sceUmdRegisterUMDCallBackPatched(int cbid) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelNotifyCallback(cbid, PSP_UMD_NOT_PRESENT);
	pspSdkSetK1(k1);
	return res;
}

int cpu_list[] = { 0, 20, 75, 100, 133, 222, 266, 300, 333 };
int bus_list[] = { 0, 10, 37,  50,  66, 111, 133, 150, 166 };

#define N_CPU (sizeof(cpu_list) / sizeof(int))

void OnSystemStatusIdle() {
	initAdrenalineInfo();

	PatchVolatileMemBug();

	if (sceKernelBootFrom() == PSP_BOOT_DISC) {
		SetSpeed(cpu_list[config.umdisocpuspeed % N_CPU], bus_list[config.umdisocpuspeed % N_CPU]);
	}
}

int (* sceMeAudio_driver_C300D466)(int codec, int unk, void *info);

int sceMeAudio_driver_C300D466_Patched(int codec, int unk, void *info) {
	int res = sceMeAudio_driver_C300D466(codec, unk, info);

	if (res < 0 && codec == 0x1002 && unk == 2)
		return 0;

	return res;
}

int OnModuleStart(SceModule2 *mod) {
	char *modname = mod->modname;
	u32 text_addr = mod->text_addr;

	if (strcmp(modname, "sceLowIO_Driver") == 0) {
		if (mallocinit() < 0) {
			while (1)
				_sw(0, 0);
		}

		// Protect pops memory
		if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
			sceKernelAllocPartitionMemory661(6, "", PSP_SMEM_Addr, 0x80000, (void *)0x09F40000);
		}

		memset((void *)0x49F40000, 0, 0x80000);
		memset((void *)0xABCD0000, 0, 0x1B0);

		PatchLowIODriver2(text_addr);
	} else if (strcmp(modname, "sceLoadExec") == 0) {
		PatchLoadExec(text_addr);
	} else if (strcmp(modname, "sceKermitMsfs_driver") == 0) {
		PatchMsfsDriver(mod);
	} else if (strcmp(modname, "scePower_Service") == 0) {
		log("Built: %s %s\n", __DATE__, __TIME__);
		log("Boot From: 0x%X\n", sceKernelBootFrom());
		log("Key Config: 0x%X\n", sceKernelInitKeyConfig());
		log("Apitype: 0x%X\n", sceKernelInitApitype());
		log("Filename: %s\n", sceKernelInitFileName());
		
		sctrlSEGetConfig(&config);

		if (config.forcehighmemory) {
			sctrlHENSetMemory(52, 0);
			ApplyMemory();
		}

		PatchPowerService(text_addr);
		PatchPowerService2(text_addr);
	} else if (strcmp(modname, "sceChkreg") == 0) {
		PatchChkreg();
	} else if (strcmp(modname, "sceMesgLed") == 0) {
		REDIRECT_FUNCTION(FindProc("sceMesgLed", "sceResmgr_driver", 0x9DC14891), sceResmgrDecryptIndexPatched);
		ClearCaches();
	} else if (strcmp(modname, "scePspNpDrm_Driver") == 0) {
		PatchNpDrmDriver(text_addr);
	} else if (strcmp(modname, "sceUmd_driver") == 0) {
		REDIRECT_FUNCTION(text_addr + 0xC80, sceUmdRegisterUMDCallBackPatched);
		ClearCaches();
	} else if(strcmp(modname, "sceMeCodecWrapper") == 0) {
		HIJACK_FUNCTION(FindProc(modname, "sceMeAudio_driver", 0xC300D466), sceMeAudio_driver_C300D466_Patched, sceMeAudio_driver_C300D466);
		ClearCaches();
	} else if (strcmp(modname, "sceUtility_Driver") == 0) {
		PatchUtility();
	} else if (strcmp(modname, "sceImpose_Driver") == 0) {
		PatchImposeDriver(text_addr);
	} else if (strcmp(modname, "sceMediaSync") == 0) {
		PatchMediaSync(text_addr);
	} else if (strcmp(modname, "sceSAScore") == 0) {
		PatchSasCore();
	} else if (strcmp(modname, "DJMAX") == 0 || strcmp(modname, "djmax") == 0) {
		MAKE_DUMMY_FUNCTION(sctrlHENFindImport(modname, "IoFileMgrForUser", 0xE3EB004C), 0);
		ClearCaches();
	} else if (strcmp(modname, "KHBBS_patch") == 0) {
		MAKE_DUMMY_FUNCTION(mod->entry_addr, 1);
		ClearCaches();
	} else if (strcmp(modname, "VLF_Module") == 0) {
		static u32 nids[] = { 0x2A245FE6, 0x7B08EAAB, 0x22050FC0, 0x158BE61A, 0xD495179F };

		int i;
		for (i = 0; i < (sizeof(nids) / sizeof(u32)); i++) {
			u32 vlf_function = FindProc(modname, "VlfGui", nids[i]);
			if (vlf_function)
				MAKE_DUMMY_FUNCTION(vlf_function, 0);
		}

		ClearCaches();
	}

	if (!idle) {
		if (sceKernelGetSystemStatus661() == 0x20000) {
			idle = 1;
			OnSystemStatusIdle();
		}
	}

	log("%s: 0x%08X\n", modname, text_addr);

	return 0;
}

int module_start(SceSize args, void *argp) {
	PatchLoadCore();
	PatchInterruptMgr();
	PatchIoFileMgr();
	PatchMemlmd();
	PatchModuleMgr();
	ClearCaches();

	sctrlHENSetStartModuleHandler((STMOD_HANDLER)OnModuleStart);

	UnprotectExtraMemory();

	initAdrenaline();

	memcpy(&rebootex_config, (void *)0x88FB0000, sizeof(RebootexConfig));

	return 0;
}