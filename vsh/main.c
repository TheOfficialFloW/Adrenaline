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

#include <psp2/appmgr.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <string.h>

#include <taihen.h>

#include "utils.h"
#include "../adrenaline_compat.h"

char *sceLsdbGetName(void *a1);

static tai_hook_ref_t sceSysmoduleLoadModuleInternalWithArgRef;
static tai_hook_ref_t sceSysmoduleUnloadModuleInternalWithArgRef;
static tai_hook_ref_t scePafMiscLoadModuleRef;
static tai_hook_ref_t sceLsdbGetTypeRef;

static SceUID hooks[4];

static int sceLsdbGetTypePatched(void *a1) {
	int val = TAI_CONTINUE(int, sceLsdbGetTypeRef, a1);

	if (val == 1) { // type normal
		char *name = sceLsdbGetName(a1);
		if (name && strcmp(name, ADRENALINE_TITLEID) == 0) {
			return 2; // type pspemu
		}
	}

	return val;
}

static int scePafMiscLoadModulePatched(int a1, const char *path, int a3, int a4, int a5) {
	int res = TAI_CONTINUE(int, scePafMiscLoadModuleRef, a1, path, a3, a4, a5);

	if (sceClibStrncmp(path, "vs0:vsh/shell/livespace_db.suprx", 32) == 0) {
		hooks[3] = taiHookFunctionImport(&sceLsdbGetTypeRef, "SceShell", 0x6BC25E17, 0xDEC358E4, sceLsdbGetTypePatched);
	}

	return res;
}

static int sceSysmoduleLoadModuleInternalWithArgPatched(SceUInt32 id, SceSize args, void *argp, void *unk) {
	int res = TAI_CONTINUE(int, sceSysmoduleLoadModuleInternalWithArgRef, id, args, argp, unk);

	if (res >= 0 && id == 0x80000008) {
		hooks[2] = taiHookFunctionImport(&scePafMiscLoadModuleRef, "SceShell", 0x3D643CE8, 0xB3B5DF38, scePafMiscLoadModulePatched);
	}

	return res;
}

static int sceSysmoduleUnloadModuleInternalWithArgPatched(SceUInt32 id, SceSize args, void *argp, void *unk) {
	int res = TAI_CONTINUE(int, sceSysmoduleUnloadModuleInternalWithArgRef, id, args, argp, unk);

	if (res >= 0 && id == 0x80000008) {
		if (hooks[2] >= 0)
			taiHookRelease(hooks[2], scePafMiscLoadModuleRef);
	}

	return res;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
	tai_module_info_t info;
	info.size = sizeof(info);
	if (taiGetModuleInfo("SceLsdb", &info) < 0) {
		hooks[0] = taiHookFunctionImport(&sceSysmoduleLoadModuleInternalWithArgRef, "SceShell", 0x03FCF19D, 0xC3C26339, sceSysmoduleLoadModuleInternalWithArgPatched);
		hooks[1] = taiHookFunctionImport(&sceSysmoduleUnloadModuleInternalWithArgRef, "SceShell", 0x03FCF19D, 0xA2F40C4C, sceSysmoduleUnloadModuleInternalWithArgPatched);
	} else {
		hooks[3] = taiHookFunctionImport(&sceLsdbGetTypeRef, "SceShell", 0x6BC25E17, 0xDEC358E4, sceLsdbGetTypePatched);
	}

	// Destroy
	sceAppMgrDestroyAppByName(ADRENALINE_TITLEID);

	// TODO: autoreload

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
	if (hooks[3] >= 0)
		taiHookRelease(hooks[3], sceLsdbGetTypeRef);
	if (hooks[1] >= 0)
		taiHookRelease(hooks[1], sceSysmoduleUnloadModuleInternalWithArgRef);
	if (hooks[0] >= 0)
		taiHookRelease(hooks[0], sceSysmoduleLoadModuleInternalWithArgRef);

	return SCE_KERNEL_STOP_SUCCESS;
}
