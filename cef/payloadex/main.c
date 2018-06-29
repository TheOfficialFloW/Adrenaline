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
#include "libc.h"

#define REBOOT_MODULE "/rtm.prx"

int (* DcacheClear)(void) = (void *)0x886018AC;
int (* IcacheClear)(void) = (void *)0x88601E40;

int (* DecryptExecutable)(void *buf, int size, int *retSize);

void (* SetMemoryPartitionTable)(void *sysmem_config, SceSysmemPartTable *table);
int (* sceKernelBootLoadFile)(BootFile *file, void *a1, void *a2, void *a3, void *t0);

RebootexConfig *rebootex_config = (RebootexConfig *)0x88FB0000;

void ClearCaches() {
	DcacheClear();
	IcacheClear();
}

void SetMemoryPartitionTablePatched(void *sysmem_config, SceSysmemPartTable *table) {
	SetMemoryPartitionTable(sysmem_config, table);

	// Add partition 11
	table->extVshell.addr = 0x8A000000;
	table->extVshell.size = 20 * 1024 * 1024;
}

int PatchSysMem(void *a0, void *sysmem_config) {
	int (* module_bootstart)(SceSize args, void *sysmem_config) = (void *)_lw((u32)a0 + 0x28);

	u32 i;
	for (i = 0; i < 0x14000; i += 4) {
		u32 addr = 0x88000000 + i;

		// Patch to add new partition
		if (_lw(addr) == 0x14600003) {
			K_HIJACK_CALL(addr - 0x1C, SetMemoryPartitionTablePatched, SetMemoryPartitionTable);
			continue;
		}
	}

	ClearCaches();

	return module_bootstart(4, sysmem_config);
}

int DecryptExecutablePatched(void *buf, int size, int *retSize) {
	if (*(u16 *)((u32)buf + 0x150) == 0x8B1F) {
		*retSize = *(u32 *)((u32)buf + 0xB0);
		_memcpy(buf, (void *)((u32)buf + 0x150), *retSize);
		return 0;
	}

	return DecryptExecutable(buf, size, retSize);
}

int PatchLoadCore(int (* module_bootstart)(SceSize args, void *argp), void *argp) {
	u32 text_addr = ((u32)module_bootstart) - 0xAF8;

	u32 i;
	for (i = 0; i < 0x8000; i += 4) {
		u32 addr = text_addr + i;

		// Allow custom modules
		if (_lw(addr) == 0xAE2D0048) {
			DecryptExecutable = (void *)K_EXTRACT_CALL(addr + 8);
			MAKE_CALL(addr + 8, DecryptExecutablePatched);
			break;
		}
	}

	ClearCaches();

	return module_bootstart(8, argp);
}

int InsertModule(void *buf, char *new_module, char *module_after, int flags) {
	BtcnfHeader *header = (BtcnfHeader *)buf;

	ModuleEntry *modules = (ModuleEntry *)((u32)header + header->modulestart);
	ModeEntry *modes = (ModeEntry *)((u32)header + header->modestart);

	char *modnamestart = (char *)((u32)header + header->modnamestart);
	char *modnameend = (char *)((u32)header + header->modnameend);

	if (header->signature != BTCNF_MAGIC)
		return -1;

	int i;
	for (i = 0; i < header->nmodules; i++) {
		if (_strcmp(modnamestart + modules[i].stroffset, module_after) == 0) {
			break;
		}
	}

	if (i == header->nmodules)
		return -2;

	int len = _strlen(new_module) + 1;

	// Add new_module name at end
	_memcpy((void *)modnameend, (void *)new_module, len);

	// Move module_after forward
	_memmove(&modules[i + 1], &modules[i], (header->nmodules - i) * sizeof(ModuleEntry) + len + modnameend - modnamestart);

	// Add new_module information
	modules[i].stroffset = modnameend - modnamestart;
	modules[i].flags = flags;

	// Update header
	header->nmodules++;
	header->modnamestart += sizeof(ModuleEntry);
	header->modnameend += (len + sizeof(ModuleEntry));

	// Update modes
	int j;
	for (j = 0; j < header->nmodes; j++) {
		modes[j].maxsearch++;
	}

	return 0;
}

int sceKernelCheckPspConfigPatched(void *buf, int size, int flag) {
	if (rebootex_config->module_after) {
		InsertModule(buf, REBOOT_MODULE, rebootex_config->module_after, rebootex_config->flags);
	}

	return 0;
}

int sceKernelBootLoadFilePatched(BootFile *file, void *a1, void *a2, void *a3, void *t0) {
	if (_strcmp(file->name, "pspbtcnf.bin") == 0) {
		char *name = NULL;

		switch(rebootex_config->bootfileindex) {
			case BOOT_NORMAL:
				name = "/kd/pspbtjnf.bin";
				break;
				
			case BOOT_INFERNO:
				name = "/kd/pspbtknf.bin";
				break;
				
			case BOOT_MARCH33:
				name = "/kd/pspbtlnf.bin";
				break;
				
			case BOOT_NP9660:
				name = "/kd/pspbtmnf.bin";
				break;
				
			case BOOT_RECOVERY:
				name = "/kd/pspbtrnf.bin";
				break;
		}

		if (rebootex_config->bootfileindex == BOOT_RECOVERY) {
			rebootex_config->bootfileindex = BOOT_NORMAL;
		}

		file->name = name;
	} else if (_strcmp(file->name, REBOOT_MODULE) == 0) {
		file->buffer = (void *)0x89000000;
		file->size = rebootex_config->size;
		_memcpy(file->buffer, rebootex_config->buf, file->size);
		return 0;
	}

	sceKernelBootLoadFile(file, a1, a2, a3, t0);

	return 0; //always return 0 to allow boot with unsuccessfully loaded files
}

/*
	0x89FF0000: Apitype
	0x89FF0004: vsh: 2, update: 3, pops: 4, licence: 5, app: 6, umd: 7, mlnapp: 8
	0x89FF0008: Path #1
	0x89FF0048: Path #2
	0x89FF0088: Path #3
	0x89FF00C8: SFO buffer
	0x89FF14C8: 0x000001F4
	0x89FF14CC: 0x000000DC
	0x89FF14D0: 0x00060313
	0x89FF1510: TITLEID
	0x89FF1520: 0x00000003
	0x89FF1540: Path #4
	0x89FF1590: Version
*/

int loadParamsPatched(int a0) {
	int v0 = _lw(a0 + 12);
	int v1 = _lw(a0 + 16);
	_sw(_lw(0x89FF0000), (v1 + (v0 << 5)) - 12);
	return 0;
}

int _start(void *a0, void *a1, void *a2) __attribute__((section(".text.start")));
int _start(void *a0, void *a1, void *a2) {
	int (* sceBoot)(void *a0, void *a1, void *a2);

	*(u32 *)0x89FF0000 = 0x200;
	*(u32 *)0x89FF0004 = 0x2;

	u32 i;
	for (i = 0; i < 0x4000; i += 4) {
		u32 addr = 0x88600000 + i;

		// Find sceBoot
		if (_lw(addr) == 0x27BD01C0) {
			sceBoot = (void *)(addr + 4);
			continue;
		}
		
		// Don't load pspemu params
		if (_lw(addr) == 0x240500CF) {
			MAKE_CALL(addr + 4, loadParamsPatched);
			continue;
		}

		// Patch call to SysMem module_bootstart
		if (_lw(addr) == 0x24040004) {
			_sw(0x02202021, addr); //move $a0, $s1
			MAKE_CALL(addr - 4, PatchSysMem);
			continue;
		}

		// Patch call to LoadCore module_bootstart
		if (_lw(addr) == 0x00600008) {
			_sw(0x00602021, addr - 8); //move $a0, $v1
			MAKE_JUMP(addr, PatchLoadCore);
			continue;
		}

		// Patch sceKernelCheckPspConfig
		if (_lw(addr) == 0x04400029) {
			MAKE_CALL(addr - 8, sceKernelCheckPspConfigPatched);
			continue;
		}

		// Patch sceKernelBootLoadFile
		if (_lw(addr) == 0xAFBF0000 && _lw(addr + 8) == 0x00000000) {
			sceKernelBootLoadFile = (void *)K_EXTRACT_CALL(addr + 4);
			MAKE_CALL(addr + 4, sceKernelBootLoadFilePatched);
			continue;
		}
	}

	ClearCaches();

	// Call original function
	return sceBoot(a0, a1, a2);
}