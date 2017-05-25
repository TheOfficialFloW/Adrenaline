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
#include "../../adrenaline_compat.h"

#include "rebootex.h"

int (* _sceChkregGetPsCode)(u8 *pscode);

int (* RunReboot)(u32 *params);
int (* DecodeKL4E)(void *dest, u32 size_dest, void *src, u32 size_src);

int (* SetIdleCallback)(int flags);

int (* scePowerSetClockFrequency_k)(int cpufreq, int ramfreq, int busfreq);

int (* sceSystemFileGetIndex)(void *sfo, void *a1, void *a2);

u32 FindPowerFunction(u32 nid) {
	return FindProc("scePower_Service", "scePower", nid);
}

static u32 FindPowerDriverFunction(u32 nid) {
	return FindProc("scePower_Service", "scePower_driver", nid);
}

typedef struct PartitionData {
	u32 unk[5];
	u32 size;
} PartitionData;

typedef struct SysMemPartition {
	struct SysMemPartition *next;
	u32	address;
	u32 size;
	u32 attributes;
	PartitionData *data;
} SysMemPartition;

void ApplyMemory() {
	if (rebootex_config.ram2 != 0 && (rebootex_config.ram2 + rebootex_config.ram11) <= 52) {
		SysMemPartition *(* GetPartition)(int partition) = (void *)0x88003F20;
		SysMemPartition *partition;
		u32 user_size;

		user_size = (rebootex_config.ram2 * 1024 * 1024);
		partition = GetPartition(PSP_MEMORY_PARTITION_USER);
		partition->size = user_size;
		partition->data->size = (((user_size >> 8) << 9) | 0xFC);

		partition = GetPartition(11);
		partition->size = (rebootex_config.ram11 * 1024 * 1024);
		partition->address = 0x88800000 + user_size;
		partition->data->size = (((partition->size >> 8) << 9) | 0xFC);
	}
}

void UnprotectExtraMemory() {
	u32 *prot = (u32 *)0xBC000040;

	int i;
	for (i = 0; i < 0x10; i++) {
		prot[i] = 0xFFFFFFFF;
	}
}

int sctrlHENSetMemory(u32 p2, u32 p11) {
	if (p2 == 0)
		return 0x80000107;

	if ((p2 + p11) > 52)
		return 0x80000107;

	int k1 = pspSdkSetK1(0);

	rebootex_config.ram2 = p2;
	rebootex_config.ram11 = p11;

	pspSdkSetK1(k1);
	return 0;
}

int sceSystemFileGetIndexPatched(void *sfo, void *a1, void *a2) {
	int largememory = 0;

	if (rebootex_config.ram2 == 0) {
		SFOHeader *header = (SFOHeader *)sfo;
		SFODir *entries = (SFODir *)(sfo + sizeof(SFOHeader));

		int i;
		for (i = 0; i < header->nitems; i++) {
			if (strcmp(sfo + header->fields_table_offs + entries[i].field_offs, "MEMSIZE") == 0) {
				memcpy(&largememory, sfo + header->values_table_offs + entries[i].val_offs, 4);
			}
		}

		if (largememory) {
			sctrlHENSetMemory(52, 0);
			ApplyMemory();
		}
	} else {
		ApplyMemory();
		rebootex_config.ram2 = 0;
	}

	return sceSystemFileGetIndex(sfo, a1, a2);
}

int RunRebootPatched(u32 *params) {
	if (strncmp((char *)params[2], "disc0:/", 7) != 0) {
		if (rebootex_config.bootfileindex != BOOT_RECOVERY) {
			rebootex_config.bootfileindex = BOOT_NORMAL;
		}

		memset(rebootex_config.umdfilename, 0, 0x48);
	}

	return RunReboot(params);
}

int DecodeKL4EPatched(void *dest, u32 size_dest, void *src, u32 size_src) {
	memcpy((void *)0x88FC0000, rebootex, size_rebootex);
	memcpy((void *)0x88FB0000, &rebootex_config, sizeof(RebootexConfig));
	return DecodeKL4E(dest, size_dest, src, size_src);
}

void PatchLoadExec(u32 text_addr) {
	// Allow loadexec in whatever user level. Ignore K1 Check
	_sh(0x1000, text_addr + 0x16A6);
	_sh(0x1000, text_addr + 0x241E);
	_sh(0x1000, text_addr + 0x2622);

	// Allow loadexec in whatever user level. Make sceKernelGetUserLevel return 4
	MAKE_DUMMY_FUNCTION(text_addr + 0x3690, 4);

	// Remove apitype check in FW's above 2.60
	memset((void *)text_addr + 0x2964, 0, 0x20);

	// Patch to do things before reboot
	RunReboot = (void *)text_addr + 0x2B04;
	MAKE_CALL(text_addr + 0x29BC, RunRebootPatched);

	// Ignore kermit calls
	_sw(0, text_addr + 0x2B9C);

	// Redirect pointer to 0x88FC0000
	DecodeKL4E = (void *)text_addr + 0;
	MAKE_CALL(text_addr + 0x2E00, DecodeKL4EPatched);
	_sh(0x88FC, text_addr + 0x2E4C);

	// Fix type check
	_sw(0x24050002, text_addr + 0x2D24); //ori $a1, $v1, 0x2 -> li $a1, 2
	_sw(0x12E500B7, text_addr + 0x2D28); //bnez $s7, loc_XXXXXXXX -> beq $s7, $a1, loc_XXXXXXXX
	_sw(0xAC570018, text_addr + 0x2D2C); //sw $a1, 24($v0) -> sw $s7, 24($v0)

	// Some registers are reserved. Use other registers to avoid malfunction
	_sw(0x24050200, text_addr + 0x3380); //li $s0, 0x200 -> li $a1, 0x200
	_sw(0x12650003, text_addr + 0x3384); //beq $s3, $s0, loc_XXXXXXXX - > beq $s3, $a1, loc_XXXXXXXX
	_sw(0x241E0210, text_addr + 0x3388); //li $s5, 0x210 -> li $fp, 0x210
	_sw(0x567EFFDE, text_addr + 0x338C); //bne $s3, $s5, loc_XXXXXXXX -> bne $s3, $fp, loc_XXXXXXXX

	// Allow LoadExecVSH type 1. Ignore peripheralCommon KERMIT_CMD_ERROR_EXIT
	MAKE_JUMP(text_addr + 0x3394, text_addr + 0x2BA4);
	_sw(0x24170001, text_addr + 0x3398); //li $s7, 1

	ClearCaches();
}

int sceChkregGetPsCodePatched(u8 *pscode) {
	int res = _sceChkregGetPsCode(pscode);

	if (config.fakeregion) {
		pscode[2] = (config.fakeregion < 12) ? (config.fakeregion + 2) : (config.fakeregion - 11);
	}

	// Fix pscode
	pscode[3] = 0;
	pscode[4] = 1;

	return res;
}

void PatchChkreg() {
	MAKE_DUMMY_FUNCTION(K_EXTRACT_IMPORT(&sceChkregCheckRegion661), 1);
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceChkregGetPsCode661), sceChkregGetPsCodePatched, _sceChkregGetPsCode);
	ClearCaches();
}

int SetIdleCallbackPatched(int flags) {
	// Only allow idle callback for music player sleep-timer
	if (flags & 8) {
		return SetIdleCallback(flags);
	}

	return 0;
}

int exit_callback(int arg1, int arg2, void *common) {
	sceKernelSuspendAllUserThreads();
	SceAdrenaline *adrenaline = (SceAdrenaline *)ADRENALINE_ADDRESS;
	adrenaline->pops_mode = 0;
	SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS);

	static u32 vshmain_args[0x100];
	memset(vshmain_args, 0, sizeof(vshmain_args));

	vshmain_args[0] = sizeof(vshmain_args);
	vshmain_args[1] = 0x20;
	vshmain_args[16] = 1;

	struct SceKernelLoadExecVSHParam param;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.argp = NULL;
	param.args = 0;
	param.vshmain_args = vshmain_args;
	param.vshmain_args_size = sizeof(vshmain_args);
	param.key = "vsh";

	sctrlKernelExitVSH(&param);

	return 0;
}

int CallbackThread(SceSize args, void *argp) {
	SceUID cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	if (cbid < 0)
		return cbid;

	int (* sceKernelRegisterExitCallback)() = (void *)FindProc("sceLoadExec", "LoadExecForUser", 0x4AC57943);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

SceUID SetupCallbacks() {
	SceUID thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	return thid;
}

int sceKernelWaitEventFlagPatched(int evid, u32 bits, u32 wait, u32 *outBits, SceUInt *timeout) {
	int res = sceKernelWaitEventFlag(evid, bits, wait, outBits, timeout);

	if (*outBits & 0x1) {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_PAUSE_POPS);
	} else if (*outBits & 0x2) {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS);
	}

	return res;
}

void PatchImposeDriver(u32 text_addr) {
	// Hide volume bar
	_sw(0, text_addr + 0x4AEC);

	HIJACK_FUNCTION(text_addr + 0x381C, SetIdleCallbackPatched, SetIdleCallback);

	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
		SetupCallbacks();
		MAKE_DUMMY_FUNCTION(text_addr + 0x91C8, PSP_INIT_KEYCONFIG_GAME);
		REDIRECT_FUNCTION(text_addr + 0x92B0, sceKernelWaitEventFlagPatched);
	}

	ClearCaches();
}

void PatchMediaSync(u32 text_addr) {
	// Dummy function that checks flash0 files
	_sw(0x00001021, text_addr + 0xC8);

	// Fixes: ELF boot, boot not from /PSP/GAME/
	_sw(0x00008821, text_addr + 0x864);
	_sw(0x00008821, text_addr + 0x988);

	// Avoid SCE_MEDIASYNC_ERROR_INVALID_MEDIA
	_sh(0x5000, text_addr + 0x3C6);
	_sh(0x1000, text_addr + 0xDCA);

	K_HIJACK_CALL(text_addr + 0x97C, sceSystemFileGetIndexPatched, sceSystemFileGetIndex);

	ClearCaches();
}

void SetSpeed(int cpu, int bus) {
	if (cpu == 20  || cpu == 75 || cpu == 100 || cpu == 133 || cpu == 333 || cpu == 300 || cpu == 266 || cpu == 222) {
		scePowerSetClockFrequency_k = (void *)FindPowerFunction(0x737486F2);
		scePowerSetClockFrequency_k(cpu, cpu, bus);

		if (sceKernelInitKeyConfig() != PSP_INIT_KEYCONFIG_VSH) {
			MAKE_DUMMY_FUNCTION((u32)scePowerSetClockFrequency_k, 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0x545A7F3C), 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0xB8D7B3FB), 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0x843FBF43), 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0xEBD177D6), 0);
			ClearCaches();
		}
	}
}

void sctrlHENSetSpeed(int cpu, int bus) {
	scePowerSetClockFrequency_k = (void *)FindPowerFunction(0x545A7F3C);
	scePowerSetClockFrequency_k(cpu, cpu, bus);
}