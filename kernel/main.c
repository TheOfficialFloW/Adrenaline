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

#include <psp2kern/ctrl.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/cpu.h>

#include <stdio.h>
#include <string.h>

#include <taihen.h>

#include "utils.h"

#include "../adrenaline_compat.h"

int ksceKernelGetProcessInfo(SceUID pid, void *info);

static tai_hook_ref_t ksceKernelAllocMemBlockRef;
static tai_hook_ref_t ksceKernelFreeMemBlockRef;
static tai_hook_ref_t ksceKernelUnmapMemBlockRef;
static tai_hook_ref_t SceGrabForDriver_E9C25A28_ref;
static tai_hook_ref_t sm_stuff_ref;
static tai_hook_ref_t ksceSblAimgrIsDEXRef;
static tai_hook_ref_t ksceKernelStartPreloadedModulesRef;

static int hooks[8];
static int n_hooks = 0;

static SceUID extra_1_blockid = -1, extra_2_blockid = -1;

static SceUID ksceKernelAllocMemBlockPatched(const char *name, SceKernelMemBlockType type, int size, SceKernelAllocMemBlockKernelOpt *optp) {
	SceUID blockid = TAI_CONTINUE(SceUID, ksceKernelAllocMemBlockRef, name, type, size, optp);

	uint32_t addr;
	ksceKernelGetMemBlockBase(blockid, (void *)&addr);

	if (addr == 0x23000000) {
		extra_1_blockid = blockid;
	} else if (addr == 0x24000000) {
		extra_2_blockid = blockid;
	}

	return blockid;
}

static int ksceKernelFreeMemBlockPatched(SceUID uid) {
	if (uid == extra_1_blockid)
		return 0;

	int res = TAI_CONTINUE(int, ksceKernelFreeMemBlockRef, uid);

	if (uid == extra_2_blockid) {
		ksceKernelFreeMemBlock(extra_1_blockid);
		extra_1_blockid = -1;
		extra_2_blockid = -1;
	}

	return res;
}

static int ksceKernelUnmapMemBlockPatched(SceUID uid) {
	return 0;
}

static int SceGrabForDriver_E9C25A28_patched(int unk, uint32_t paddr) {
	if (unk == 2 && paddr == 0x21000001)
		paddr = 0x22000001;

	return TAI_CONTINUE(int, SceGrabForDriver_E9C25A28_ref, unk, paddr);
}

static int sm_stuff_patched() {
	uint32_t a;

	a = 0;
	ksceKernelMemcpyUserToKernel(&a, 0x70FC0000, sizeof(uint32_t));

	if (a != 0) {
		// jal 0x88FC0000
		a = 0x0E3F0000;
		ksceKernelMemcpyKernelToUser(0x70602D58, &a, sizeof(uint32_t));
		ksceKernelCpuDcacheWritebackRange((void *)0x70602D58, sizeof(uint32_t));
	}

	return TAI_CONTINUE(int, sm_stuff_ref);
}

static int ksceSblAimgrIsDEXPatched() {
	TAI_CONTINUE(int, ksceSblAimgrIsDEXRef);
	return 1;
}

static int getShellPid() {
	uint32_t info[0xE8/4];
	memset(info, 0, sizeof(info));
	info[0] = sizeof(info);

	SceUID current_pid = 0, parent_pid = 0;

	do {
		current_pid = parent_pid;

		if (ksceKernelGetProcessInfo(current_pid, info) < 0)
			return -1;

		parent_pid = info[5];

		if (parent_pid == -1)
			return -1;
	} while (parent_pid != KERNEL_PID);

	return current_pid;
}

static int ksceKernelStartPreloadedModulesPatched(SceUID pid) {
	int res = TAI_CONTINUE(int, ksceKernelStartPreloadedModulesRef, pid);

	char titleid[32];
	ksceKernelGetProcessTitleId(pid, titleid, sizeof(titleid));

	if (strcmp(titleid, "main") == 0) {
		ksceKernelLoadStartModuleForPid(pid, "ux0:app/" ADRENALINE_TITLEID "/sce_module/adrenaline_vsh.suprx", 0, NULL, 0, NULL, NULL);
	} else if (strcmp(titleid, ADRENALINE_TITLEID) == 0) {
		ksceKernelLoadStartModuleForPid(pid, "ux0:app/" ADRENALINE_TITLEID "/sce_module/adrenaline_user.suprx", 0, NULL, 0, NULL, NULL);
	}

	return res;
}

int kuCtrlPeekBufferPositive(int port, SceCtrlData *pad_data, int count) {
	uint32_t state;
	ENTER_SYSCALL(state);

	SceCtrlData pad;
	uint32_t off;

	// set cpu offset to zero
	asm volatile ("mrc p15, 0, %0, c13, c0, 4" : "=r" (off));
	asm volatile ("mcr p15, 0, %0, c13, c0, 4" :: "r" (0));

	int res = ksceCtrlPeekBufferPositive(port, &pad, count);

	// restore cpu offset
	asm volatile ("mcr p15, 0, %0, c13, c0, 4" :: "r" (off));

	ksceKernelMemcpyKernelToUser((uintptr_t)pad_data, &pad, sizeof(SceCtrlData));

	EXIT_SYSCALL(state);
	return res;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
	int res;

	// Load plugin for SceShell
	ksceKernelLoadStartModuleForPid(getShellPid(), "ux0:app/" ADRENALINE_TITLEID "/sce_module/adrenaline_vsh.suprx", 0, NULL, 0, NULL, NULL);

	// Tai module info
	tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);
	res = taiGetModuleInfoForKernel(KERNEL_PID, "SceCompat", &tai_info);
	if (res < 0)
		return res;

	// SceCompat
	hooks[n_hooks++] = taiHookFunctionOffsetForKernel(KERNEL_PID, &sm_stuff_ref, tai_info.modid, 0, 0x2AA4, 1, sm_stuff_patched);

	// SceSysmemForDriver
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceKernelAllocMemBlockRef, "SceCompat", 0x6F25E18A, 0xC94850C9, ksceKernelAllocMemBlockPatched);
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceKernelFreeMemBlockRef, "SceCompat", 0x6F25E18A, 0x009E1C61, ksceKernelFreeMemBlockPatched);
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceKernelUnmapMemBlockRef, "SceCompat", 0x6F25E18A, 0xFFCD9B60, ksceKernelUnmapMemBlockPatched);

	// SceGrabForDriver
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &SceGrabForDriver_E9C25A28_ref, "SceCompat", 0x81C54BED, 0xE9C25A28, SceGrabForDriver_E9C25A28_patched);

	// SceSblAIMgrForDriver
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceSblAimgrIsDEXRef, "SceCompat", 0xFD00C69A, 0xF4B98F66, ksceSblAimgrIsDEXPatched);

	// SceKernelModulemgr
	hooks[n_hooks++] = taiHookFunctionExportForKernel(KERNEL_PID, &ksceKernelStartPreloadedModulesRef, "SceKernelModulemgr", 0xC445FA63, 0x432DCC7A, ksceKernelStartPreloadedModulesPatched);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
	taiHookReleaseForKernel(hooks[--n_hooks], ksceKernelStartPreloadedModulesRef);
	taiHookReleaseForKernel(hooks[--n_hooks], ksceSblAimgrIsDEXRef);
	taiHookReleaseForKernel(hooks[--n_hooks], SceGrabForDriver_E9C25A28_ref);
	taiHookReleaseForKernel(hooks[--n_hooks], ksceKernelUnmapMemBlockRef);
	taiHookReleaseForKernel(hooks[--n_hooks], ksceKernelFreeMemBlockRef);
	taiHookReleaseForKernel(hooks[--n_hooks], ksceKernelAllocMemBlockRef);
	taiHookReleaseForKernel(hooks[--n_hooks], sm_stuff_ref);

	return SCE_KERNEL_STOP_SUCCESS;
}
