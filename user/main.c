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
#include <psp2/audioout.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/power.h>
#include <psp2/screenshot.h>
#include <psp2/shellutil.h>
#include <psp2/udcd.h>
#include <psp2/usbstorvstor.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/dmac.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <taihen.h>

#include "main.h"
#include "flashfs.h"
#include "msfs.h"
#include "menu.h"
#include "states.h"
#include "usb.h"
#include "utils.h"

#include "msfs.h"

#include "lz4/lz4.h"

#include "files/payloadex.h"
#include "startdat.h"

int _newlib_heap_size_user = 8 * 1024 * 1024;

int (* ScePspemuDevide)(uint64_t x, uint64_t y);
int (* ScePspemuErrorExit)(int error);
int (* ScePspemuConvertAddress)(uint32_t addr, int mode, uint32_t cache_size);
int (* ScePspemuWritebackCache)(void *addr, int size);
int (* ScePspemuKermitWaitAndGetRequest)(int mode, SceKermitRequest **request);
int (* ScePspemuKermitSendResponse)(int mode, SceKermitRequest *request, uint64_t response);
int (* ScePspemuConvertStatTimeToUtc)(SceIoStat *stat);
int (* ScePspemuConvertStatTimeToLocaltime)(SceIoStat *stat);
int (* ScePspemuSettingsHandler)(int a1, int a2, int a3, int a4);
int (* ScePspemuPausePops)(int pause);

int (* sceCompatGetDevInf)(SceIoDevInfo *info);
int (* sceCompatCache)(int mode, uint32_t addr, uint32_t size);
int (* sceCompatLCDCSync)();
int (* sceCompatInterrupt)(int num);

static SceUID hooks[16];
static SceUID uids[64];
static int n_hooks = 0;
static int n_uids = 0;

static tai_hook_ref_t sceCompatSuspendResumeRef;
static tai_hook_ref_t sceCompatWriteSharedCtrlRef;
static tai_hook_ref_t sceCompatWaitSpecialRequestRef;
static tai_hook_ref_t sceShellUtilRegisterSettingsHandlerRef;
static tai_hook_ref_t sceKernelCreateThreadRef;
static tai_hook_ref_t sceIoOpenRef;
static tai_hook_ref_t sceIoGetstatRef;
static tai_hook_ref_t sceAudioOutOpenPortRef;
static tai_hook_ref_t sceAudioOutOutputRef;
static tai_hook_ref_t sceCtrlPeekBufferNegative2Ref;
static tai_hook_ref_t sceDisplaySetFrameBufForCompatRef;

static tai_hook_ref_t ScePspemuInitTitleSpecificInfoRef;
static tai_hook_ref_t ScePspemuGetStartupPngRef;
static tai_hook_ref_t ScePspemuGetTitleidRef;
static tai_hook_ref_t ScePspemuInitAudioOutRef;
static tai_hook_ref_t ScePspemuConvertAddressRef;
static tai_hook_ref_t ScePspemuDecodePopsAudioRef;

uint32_t text_addr, text_size, data_addr, data_size;

static int lock_power = 0;

static char app_titleid[12];

SceUID usbdevice_modid = -1;

AdrenalineConfig config;

extern int menu_open;

void GetFunctions() {
	ScePspemuDevide = (void *)text_addr + 0x39F0 + 0x1;
	ScePspemuErrorExit = (void *)text_addr + 0x4104 + 0x1;
	ScePspemuConvertAddress = (void *)text_addr + 0x6364 + 0x1;
	ScePspemuWritebackCache = (void *)text_addr + 0x6490 + 0x1;
	ScePspemuKermitWaitAndGetRequest = (void *)text_addr + 0x64D0 + 0x1;
	ScePspemuKermitSendResponse = (void *)text_addr + 0x6560 + 0x1;
	ScePspemuConvertStatTimeToUtc = (void *)text_addr + 0x8664 + 0x1;
	ScePspemuConvertStatTimeToLocaltime = (void *)text_addr + 0x8680 + 0x1;
	ScePspemuPausePops = (void *)text_addr + 0x300C0 + 0x1;

	sceCompatGetDevInf = (void *)text_addr + 0x6ECC;
	sceCompatCache = (void *)text_addr + 0x6F3C;
	sceCompatLCDCSync = (void *)text_addr + 0x700C;
	sceCompatInterrupt = (void *)text_addr + 0x705C;
}

void SendAdrenalineRequest(int cmd) {
	SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, SCE_PSPEMU_CACHE_NONE | SCE_PSPEMU_CACHE_INVALIDATE, ADRENALINE_SIZE);
	adrenaline->psp_cmd = cmd;
	ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);

	sceCompatInterrupt(KERMIT_VIRTUAL_INTR_IMPOSE_CH1);
}

#define LZ4_ACCELERATION 8
#define SAVESTATE_TEMP_SIZE (32 * 1024 * 1024)

int AdrenalineCompat(SceSize args, void *argp) {
	void *savestate_data = NULL;

	// Allocate savestate temp memory
	SceUID blockid = sceKernelAllocMemBlock("ScePspemuSavestateTemp", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, SAVESTATE_TEMP_SIZE, NULL);
	if (blockid < 0)
		return blockid;

	// Get base address
	sceKernelGetMemBlockBase(blockid, (void **)&savestate_data);

	while (1) {
		// Wait and get kermit request
		SceKermitRequest *request;
		ScePspemuKermitWaitAndGetRequest(KERMIT_MODE_EXTRA_2, &request);

		SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, SCE_PSPEMU_CACHE_NONE | SCE_PSPEMU_CACHE_INVALIDATE, ADRENALINE_SIZE);

		int res = -1;
		
		if (request->cmd == ADRENALINE_VITA_CMD_SAVESTATE) {
			void *ram = (void *)ScePspemuConvertAddress(0x88000000, SCE_PSPEMU_CACHE_NONE, PSP_RAM_SIZE);

			char path[128];
			makeSaveStatePath(path, adrenaline->num);

			SceUID fd = sceIoOpen(path, SCE_O_WRONLY, 0777);
			if (fd >= 0) {
				// Header
				AdrenalineStateHeader header;
				memset(&header, 0, sizeof(AdrenalineStateHeader));
				header.magic = ADRENALINE_SAVESTATE_MAGIC;
				header.version = ADRENALINE_SAVESTATE_VERSION;
				header.screenshot_offset = sizeof(AdrenalineStateHeader);
				header.screenshot_size = SCREENSHOT_SIZE;
				header.descriptors_offset = header.screenshot_offset + header.screenshot_size;
				header.descriptors_size = MAX_DESCRIPTORS * sizeof(ScePspemuMsfsDescriptor);
				header.ram_part1_offset = header.descriptors_offset + header.descriptors_size;
				// header.ram_part2_offset = header.ram_part1_offset + ram_part1_size;
				// header.ram_part1_size = compressed_size_part1;
				// header.ram_part1_siz2 = compressed_size_part2;
				header.sp = adrenaline->sp;
				header.ra = adrenaline->ra;
				strcpy(header.title, adrenaline->title);

				// Write descriptors
				ScePspemuMsfsDescriptor *descriptors = ScePspemuMsfsGetFileDescriptors();
				sceIoLseek(fd, header.descriptors_offset, SCE_SEEK_SET);
				sceIoWrite(fd, descriptors, header.descriptors_size);

				// Write compressed RAM
				uint32_t compressed_size_part1 = LZ4_compress_fast(ram, savestate_data, SAVESTATE_TEMP_SIZE, SAVESTATE_TEMP_SIZE, LZ4_ACCELERATION);
				sceIoLseek(fd, header.ram_part1_offset, SCE_SEEK_SET);
				sceIoWrite(fd, savestate_data, compressed_size_part1);

				uint32_t compressed_size_part2 = LZ4_compress_fast(ram + SAVESTATE_TEMP_SIZE, savestate_data, SAVESTATE_TEMP_SIZE, SAVESTATE_TEMP_SIZE, LZ4_ACCELERATION);
				header.ram_part2_offset = header.ram_part1_offset + compressed_size_part1;
				sceIoLseek(fd, header.ram_part2_offset, SCE_SEEK_SET);
				sceIoWrite(fd, savestate_data, compressed_size_part2);

				// Write header
				header.ram_part1_size = compressed_size_part1;
				header.ram_part2_size = compressed_size_part2;
				sceIoLseek(fd, 0, SCE_SEEK_SET);
				sceIoWrite(fd, &header, sizeof(AdrenalineStateHeader));

				sceIoClose(fd);
			}

			adrenaline->vita_response = ADRENALINE_VITA_RESPONSE_SAVED;
			ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);
			continue;
		} else if (request->cmd == ADRENALINE_VITA_CMD_LOADSTATE) {
			void *ram = (void *)ScePspemuConvertAddress(0x88000000, SCE_PSPEMU_CACHE_INVALIDATE, PSP_RAM_SIZE);

			char path[128];
			makeSaveStatePath(path, adrenaline->num);

			SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
			if (fd >= 0) {
				AdrenalineStateHeader header;

				// Read header
				sceIoLseek(fd, 0, SCE_SEEK_SET);
				sceIoRead(fd, &header, sizeof(AdrenalineStateHeader));

				// Read compressed RAM
				sceIoLseek(fd, header.ram_part1_offset, SCE_SEEK_SET);
				sceIoRead(fd, savestate_data, header.ram_part1_size);
				LZ4_decompress_fast(savestate_data, ram, SAVESTATE_TEMP_SIZE);

				sceIoLseek(fd, header.ram_part2_offset, SCE_SEEK_SET);
				sceIoRead(fd, savestate_data, header.ram_part2_size);
				LZ4_decompress_fast(savestate_data, ram + SAVESTATE_TEMP_SIZE, SAVESTATE_TEMP_SIZE);

				// Read descriptors
				ScePspemuMsfsDescriptor *descriptors = malloc(MAX_DESCRIPTORS * sizeof(ScePspemuMsfsDescriptor));
				if (descriptors) {
					sceIoLseek(fd, header.descriptors_offset, SCE_SEEK_SET);
					sceIoRead(fd, descriptors, header.descriptors_size);
					ScePspemuMsfsSetFileDescriptors(descriptors);
					free(descriptors);
				}

				// Set registers
				adrenaline->sp = header.sp;
				adrenaline->ra = header.ra;

				sceIoClose(fd);
			}

			ScePspemuWritebackCache(ram, PSP_RAM_SIZE);

			adrenaline->vita_response = ADRENALINE_VITA_RESPONSE_LOADED;
			ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);
			continue;
		} else if (request->cmd == ADRENALINE_VITA_CMD_GET_USB_STATE) {
			SceUdcdDeviceState state;
			sceUdcdGetDeviceState(&state);

			// Response
			res = state.state | state.cable | state.connection | state.use_usb_charging;
		} else if (request->cmd == ADRENALINE_VITA_CMD_START_USB) {
			// Start usb
			if (usbdevice_modid < 0 && !sceKernelIsPSVitaTV()) {
				char *path;
				
				if (config.ms_location == MEMORY_STICK_LOCATION_UR0) {
					path = "sdstor0:int-lp-ign-user";
				} else if (config.ms_location == MEMORY_STICK_LOCATION_IMC0) {
					path = "sdstor0:int-lp-ign-userext";
				} else {
					path = "sdstor0:xmc-lp-ign-userext";

					SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
					
					if (fd < 0)
						path = "sdstor0:int-lp-ign-userext";
					else
						sceIoClose(fd);
				}

				usbdevice_modid = startUsb("ux0:adrenaline/usbdevice.skprx", path, SCE_USBSTOR_VSTOR_TYPE_FAT);

				// Response
				res = (usbdevice_modid < 0) ? usbdevice_modid : 0;
			} else {
				// error already started
				res = -1;
			}
		} else if (request->cmd == ADRENALINE_VITA_CMD_STOP_USB) {
			// Stop usb
			res = stopUsb(usbdevice_modid);
			if (res >= 0)
				usbdevice_modid = -1;
		} else if (request->cmd == ADRENALINE_VITA_CMD_PAUSE_POPS) {
			ScePspemuPausePops(1);
			SetPspemuFrameBuffer((void *)SCE_PSPEMU_FRAMEBUFFER);
			adrenaline->draw_psp_screen_in_pops = 1;
			ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);
			res = 0;
		} else if (request->cmd == ADRENALINE_VITA_CMD_RESUME_POPS) {
			if (!menu_open)
				ScePspemuPausePops(0);
			SetPspemuFrameBuffer((void *)SCE_PSPEMU_FRAMEBUFFER);
			adrenaline->draw_psp_screen_in_pops = 0;
			ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);
			res = 0;
		} else if (request->cmd == ADRENALINE_VITA_CMD_POWER_SHUTDOWN) {
			scePowerRequestStandby();
			res = 0;
		} else if (request->cmd == ADRENALINE_VITA_CMD_POWER_REBOOT) {
			scePowerRequestColdReset();
			res = 0;
		}

		ScePspemuKermitSendResponse(KERMIT_MODE_EXTRA_2, request, (uint64_t)res);
	}

	return sceKernelExitDeleteThread(0);
}

static int doubleClick(uint32_t buttons, uint64_t max_time) {
	static uint32_t old_buttons, current_buttons, released_buttons;
	static uint64_t last_time = 0;
	static int clicked = 0;
	int double_clicked = 0;

	SceCtrlData pad;
	kuCtrlPeekBufferPositive(0, &pad, 1);

	old_buttons = current_buttons;
	current_buttons = pad.buttons;
	released_buttons = ~current_buttons & old_buttons;

	if (released_buttons & buttons) {
		if (clicked) {
			if ((sceKernelGetProcessTimeWide()-last_time) < max_time) {
				double_clicked = 1;
				clicked = 0;
				last_time = 0;
			} else {
				clicked = 1;
				last_time = sceKernelGetProcessTimeWide();
			}
		} else {
			clicked = 1;
			last_time = sceKernelGetProcessTimeWide();
		}
	}

	return double_clicked;
}

static int AdrenalineExit(SceSize args, void *argp) {
	while (1) {
		// Double click detection
		if (menu_open == 0) {
			if (doubleClick(SCE_CTRL_PS_BTN, 300 * 1000)) {
				stopUsb(usbdevice_modid);

				if (sceAppMgrLaunchAppByName2(app_titleid, NULL, NULL) < 0)
					ScePspemuErrorExit(0);
			}
		}

		sceDisplayWaitVblankStart();
	}

	return sceKernelExitDeleteThread(0);
}

void lockPower() {
	lock_power = 1;
}

void unlockPower() {
	lock_power = 0;
}

static int AdrenalinePowerTick(SceSize args, void *argp) {
	while (1) {
		if (lock_power) {
			sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
		}

		sceKernelDelayThread(10 * 1000 * 1000);
	}

	return sceKernelExitDeleteThread(0);
}

static int InitAdrenaline() {
	// Set GPU frequency to highest
	scePowerSetGpuClockFrequency(222);

	// Enable screenshot
	sceScreenShotEnable();

	// Lock USB connection and PS button
	sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION | SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);

	// Create and start AdrenalinePowerTick thread
	SceUID thid = sceKernelCreateThread("AdrenalinePowerTick", AdrenalinePowerTick, 0x10000100, 0x1000, 0, 0, NULL);
	if (thid >= 0)
		sceKernelStartThread(thid, 0, NULL);

	// Create and start AdrenalineCompat thread
	SceUID compat_thid = sceKernelCreateThread("AdrenalineCompat", AdrenalineCompat, 0x60, 0x8000, 0, 0, NULL);
	if (compat_thid >= 0)
		sceKernelStartThread(compat_thid, 0, NULL);

	// Create and start AdrenalineDraw thread
	SceUID draw_thid = sceKernelCreateThread("AdrenalineDraw", AdrenalineDraw, 0xA0, 0x10000, 0, 0, NULL);
	if (draw_thid >= 0)
		sceKernelStartThread(draw_thid, 0, NULL);

	return 0;
}

int sceCompatSuspendResumePatched(int unk) {
	// Lock USB connection and PS button
	sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION | SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);

	if (!menu_open)
		ScePspemuPausePops(0);

	return TAI_CONTINUE(int, sceCompatSuspendResumeRef, unk);
}

static int sceCompatWriteSharedCtrlPatched(SceCtrlDataPsp *pad_data) {
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);

	pad_data->Rx = pad.rx;
	pad_data->Ry = pad.ry;

	kuCtrlPeekBufferPositive(0, &pad, 1);

	pad_data->Buttons &= ~SCE_CTRL_PS_BTN;
	pad_data->Buttons |= (pad.buttons & SCE_CTRL_PS_BTN);

	if (menu_open) {
		pad_data->Buttons = SCE_CTRL_PS_BTN;
		pad_data->Lx = 128;
		pad_data->Ly = 128;
		pad_data->Rx = 128;
		pad_data->Ry = 128;
	}

	return TAI_CONTINUE(int, sceCompatWriteSharedCtrlRef, pad_data);
}

static int sceCompatWaitSpecialRequestPatched(int mode) {
	ScePspemuBuildFlash0();

	uint32_t *m = (uint32_t *)ScePspemuConvertAddress(0x88FC0000, SCE_PSPEMU_CACHE_INVALIDATE, size_payloadex);
	memcpy(m, payloadex, size_payloadex);
	ScePspemuWritebackCache(m, size_payloadex);

	void *n = (void *)ScePspemuConvertAddress(0x88FB0000, SCE_PSPEMU_CACHE_INVALIDATE, 0x100);
	memset(n, 0, 0x100);

	strcpy((char *)(n+4), app_titleid);

	SceCtrlData pad;
	kuCtrlPeekBufferPositive(0, &pad, 1);

	if (pad.buttons & SCE_CTRL_RTRIGGER)
		((uint32_t *)n)[0] = 4; // Recovery mode

	SceIoStat stat;
	memset(&stat, 0, sizeof(SceIoStat));
	if (sceIoGetstat("ux0:adrenaline/flash0", &stat) < 0)
		((uint32_t *)n)[0] = 4; // Recovery mode

	ScePspemuWritebackCache(n, 0x100);

	// Init Adrenaline
	InitAdrenaline();

	// Create and start AdrenalineExit thread
	SceUID thid = sceKernelCreateThread("AdrenalineExit", AdrenalineExit, 0x10000100, 0x1000, 0, 0, NULL);
	if (thid >= 0)
		sceKernelStartThread(thid, 0, NULL);

	// Clear 0x8A000000 memory
	sceDmacMemset((void *)0x63000000, 0, 16 * 1024 * 1024);
	sceCompatCache(2, 0x63000000, 16 * 1024 * 1024);

	return TAI_CONTINUE(int, sceCompatWaitSpecialRequestRef, mode);
}

static int sceShellUtilRegisterSettingsHandlerPatched(int (* handler)(int a1, int a2, int a3, int a4), int unk) {
	if (handler) {
		ScePspemuSettingsHandler = handler;
		handler = ScePspemuCustomSettingsHandler;
	}

	return TAI_CONTINUE(int, sceShellUtilRegisterSettingsHandlerRef, handler, unk);
}

static SceUID sceKernelCreateThreadPatched(const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, int cpuAffinityMask, const SceKernelThreadOptParam *option) {
	if (strcmp(name, "ScePspemuRemoteMsfs") == 0) {
		entry = (SceKernelThreadEntry)ScePspemuRemoteMsfs;
	}

	return TAI_CONTINUE(SceUID, sceKernelCreateThreadRef, name, entry, initPriority, stackSize, attr, cpuAffinityMask, option);
}

// data_addr: 83200200

static int ScePspemuInitTitleSpecificInfoPatched(const char *titleid, SceUID uid) {
	int res = 0;

	// Make __sce_menuinfo path
	snprintf((char *)(data_addr + 0x11C7D0C), 0x80, "ms0:PSP/GAME/%s/__sce_menuinfo", titleid);

	uint32_t *info = (uint32_t *)(data_addr + 0x1156450);

	// Video delay
	// Buzz!: Brain Bender: 3000. Fixes buggy PMF sequence
	info[0x00] = 3000;

	// Use current titleid for adhoc if 
	// it's set to any other value than 0xFFFFFFFF
	info[0x01] = 0xFFFFFFFF;

	// IO read delay
	info[0x02] = 0xFFFFFFFF;

	// One of those games. Adhoc related
	// 0x0: ULJS00218
	// 0x1: ULES01275
	// 0x2: ULES00703
	// 0x3: ULES00125
	// 0x4: UCJS10003
	info[0x03] = 0xFFFFFFFF;

	// Unused. Msfs related
	info[0x04] = 0xFFFFFFFF;

	// Net send delay
	info[0x05] = 0xFFFFFFFF;

	// Audio delay. Used in socom
	info[0x06] = 0xFFFFFFFF;

	// Net send related
	info[0x07] = 0xFFFFFFFF;

	// Delay. Not sure for what
	info[0x08] = 0xFFFFFFFF;

	// Msfs lseek patch. Value 0 or 1. Used in Ratched & Clank
	info[0x09] = 0xFFFFFFFF;

	// Use current titleid for adhoc if 
	// it's set to any other value than 0xFFFFFFFF
	info[0x0A] = 0xFFFFFFFF;

	// Game patches
	// 0x01: UCUS98687 Twisted Metal: Head-On
	// 0x02: UCES00018 Twisted Metal: Head-On
	// 0x03: NPJG00115 INFLUENCE
	// 0x04: ULJM05500 Monster Hunter Portable 2nd G
	// 0x05: ULJM05800 Monster Hunter Portable 3rd
	// 0x06: ULES00851 Monster Hunter Freedom 2
	// 0x07: ULES01213 Monster Hunter Freedom Unite
	// 0x08: UCES01563 Geronimo Stilton: Return to the Kingdom of Fantasy
	// 0x09: NPUG80850 Geronimo Stilton: Return to the Kingdom of Fantasy
	// 0x0A: NPJH00039 Hatsune Miku: Project Diva - Tsuika Gakkyoku Shuu Deluxe Pack 1 - Miku Uta, Okawar
	// 0x0B: NPJH00040 Hatsune Miku: Project Diva - Tsuika Gakkyoku Shuu Deluxe Pack 2 - Motto Okawari Rin, Len, Luka
	// 0x0C: NPJH50594 Jikkyou Powerful Pro Yakyuu 2012
	// 0x0D: NPJH50708 Jikkyou Powerful Pro Yakyuu 2012 Ketteiban
	// 0x0E: ULES00981 Star Wars: The Force Unleashed
	// 0x0F: ULUS10345 Star Wars: The Force Unleashed
	// 0x10: ULUS10088 Field Commander
	// 0x11: NPUH10091 Pool Hall Pro
	// 0x12: ULES00821 World of Pool
	info[0x0B] = 0xFFFFFFFF;

	// This enables audio in MotorStorm
	info[0x0C] = 0x1000;

	// Net termination delay
	info[0x0D] = 0xFFFFFFFF;

	// Use ME 2. Used in Harvest Moon
	info[0x0E] = 0xFFFFFFFF;

	// Wlan related. Only used in B-Boy
	info[0x0F] = 0xFFFFFFFF;

	// SHA-1 size
	info[0x10] = 0xFFFFFFFF;
	
	// SHA-1 hash digest
	info[0x11] = 0xFFFFFFFF;

	// Io cache file buffer size. Used in LocoRoco
	info[0x12] = 0xFFFFFFFF;

	// The game Thrillville sets this to 1
	info[0x13] = 0xFFFFFFFF;

	// Video delay. Used in Dangan-Ronpa
	info[0x14] = 0xFFFFFFFF;

	// If set to 0, the wlan switch is turned off. Used in Metal Slug
	info[0x15] = 0xFFFFFFFF;

	// Unknown. Video related
	info[0x16] = 0xFFFFFFFF;

	// Delay before act.dat read
	// This is only used in one unknown game
	info[0x17] = 0xFFFFFFFF;

	// Video flag
	// 0x2: This will cause problems with PMF. KillZone wont't work for example
	// 0x400: This will cause problems with PMF.
	info[0x18] = 0xFFFFFFFF;

	// Unknown. Adhoc related?
	info[0x19] = 0xFFFFFFFF;

	// Unknown. Adhoc related?
	info[0x1A] = 0xFFFFFFFF;

	// Unknown. Adhoc related?
	info[0x1B] = 0xFFFFFFFF;

	// Unknown. Adhoc related?
	info[0x1C] = 0xFFFFFFFF;

	// Title ID for adhoc
	info[0x1D] = 0xFFFFFFFF;

	// Used for peripheral
	info[0x1E] = 0xFFFFFFFF;

	// Not use msfs file size limit
	info[0x1F] = 0xFFFFFFFF;

	return res;
}

static int ScePspemuGetStartupPngPatched(int num, void *png_buf, int *png_size, int *unk) {
	int num_startup_png = TAI_CONTINUE(int, ScePspemuGetStartupPngRef, num, png_buf, png_size, unk);

	if (config.skip_logo) {
		num_startup_png = 0;
	} else {
		// Insert custom startdat.png
		memcpy(png_buf, startdat, size_startdat);
		*png_size = size_startdat;
		num_startup_png = 1;
	}

	return num_startup_png;
}

static int ScePspemuInitAudioOutPatched() {
	int res = TAI_CONTINUE(int, ScePspemuInitAudioOutRef);

	int (* ScePspemuInitPops)() = (void *)text_addr + 0x30678 + 0x1;
	int (* ScePspemuInitPocs)() = (void *)text_addr + 0x227C4 + 0x1;

	res = ScePspemuInitPops();
	if (res < 0)
		return res;

	SceUID blockid = sceKernelAllocMemBlock("ScePspemuMcWork", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, 0x40000, NULL);
	if (blockid < 0)
		return blockid;

	sceKernelGetMemBlockBase(blockid, (void *)(data_addr + 0x10100));

	int (* sub_811B2390)() = (void *)text_addr + 0x31F90 + 0x1;
	sub_811B2390(*(uint32_t *)(data_addr + 0x10100), 0x1E000);

	res = ScePspemuInitPocs();
	if (res < 0)
		return res;

	return res;
}

static int pops_audio_port = -1;

static int sceAudioOutOpenPortPatched(int type, int len, int freq, int mode) {
	int res = TAI_CONTINUE(int, sceAudioOutOpenPortRef, type, len, freq, mode);

	// Use voice port
	if (res == SCE_AUDIO_OUT_ERROR_PORT_FULL && type == SCE_AUDIO_OUT_PORT_TYPE_BGM) {
		pops_audio_port = TAI_CONTINUE(int, sceAudioOutOpenPortRef, SCE_AUDIO_OUT_PORT_TYPE_VOICE, len, freq, mode);
		return pops_audio_port;
	}

	return res;
}

static int sceAudioOutOutputPatched(int port, const void *buf) {
	SceAdrenaline *adrenaline = (SceAdrenaline *)CONVERT_ADDRESS(ADRENALINE_ADDRESS);

	if (port == pops_audio_port && !adrenaline->pops_mode) {
		sceDisplayWaitVblankStart();
		return 0;
	}

	return TAI_CONTINUE(int, sceAudioOutOutputRef, port, buf);
}

static int ScePspemuDecodePopsAudioPatched(int a1, int a2, int a3, int a4) {
	SceAdrenaline *adrenaline = (SceAdrenaline *)CONVERT_ADDRESS(ADRENALINE_ADDRESS);

	if (!adrenaline->pops_mode) {
		return 0;
	}

	return TAI_CONTINUE(int, ScePspemuDecodePopsAudioRef, a1, a2, a3, a4);
}

static int sceCtrlPeekBufferNegative2Patched(int port, SceCtrlData *pad_data, int count) {
	int res = TAI_CONTINUE(int, sceCtrlPeekBufferNegative2Ref, port, pad_data, count);

	if (res == 0x80340001) {
		if (!sceKernelIsPSVitaTV()) {
			if (config.use_ds3_ds4 && port == 1) {
				return TAI_CONTINUE(int, sceCtrlPeekBufferNegative2Ref, 0, pad_data, count);
			} else {
				*(uint8_t *)(CONVERT_ADDRESS(0xABCD00A7)) = 0;
			}
		}
	}

	return res;
}

static char *ScePspemuGetTitleidPatched() {
	SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, SCE_PSPEMU_CACHE_NONE, ADRENALINE_SIZE);
	return adrenaline->titleid;
}

static int ScePspemuConvertAddressPatched(uint32_t addr, int mode, uint32_t cache_size) {
	if (addr >= 0x09FE0000 && addr < 0x09FE01B0) {
		addr = 0x0BCD0000 | (addr & 0xFFFF);
	}

	return TAI_CONTINUE(int, ScePspemuConvertAddressRef, addr, mode, cache_size);
}

static SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
	char *p = strrchr(file, '/');
	if (p) {
		static char new_file[256];

		SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, SCE_PSPEMU_CACHE_NONE, ADRENALINE_SIZE);

		if (strcmp(p+1, "__sce_menuinfo") == 0) {			
			char *filename = adrenaline->filename;			
			if (strncmp(filename, "ms0:/", 5) == 0) {
				char *q = strrchr(filename, '/');
				if (q) {
					char path[128];
					strncpy(path, filename+5, q-(filename+5));
					path[q-(filename+5)] = '\0';
					
					snprintf(new_file, sizeof(new_file), "%s/%s/__sce_menuinfo", getPspemuMemoryStickLocation(), path);
					file = new_file;
				}
			}
		} else if (strcmp(p+1, "PARAM.SFO") == 0 || strcmp(p+1, "SCEVMC0.VMP") == 0 || strcmp(p+1, "SCEVMC1.VMP") == 0) {
			snprintf(new_file, sizeof(new_file), "%s/PSP/SAVEDATA/%s/%s", getPspemuMemoryStickLocation(), adrenaline->titleid, p+1);
			file = new_file;
		}
	}

	return TAI_CONTINUE(SceUID, sceIoOpenRef, file, flags, mode);
}

static int sceIoGetstatPatched(const char *file, SceIoStat *stat) {
	char *p = strrchr(file, '/');
	if (p) {
		static char new_file[256];

		SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, SCE_PSPEMU_CACHE_NONE, ADRENALINE_SIZE);

		if (strcmp(p+1, "PARAM.SFO") == 0 || strcmp(p+1, "SCEVMC0.VMP") == 0 || strcmp(p+1, "SCEVMC1.VMP") == 0) {
			snprintf(new_file, sizeof(new_file), "%s/PSP/SAVEDATA/%s/%s", getPspemuMemoryStickLocation(), adrenaline->titleid, p+1);
			file = new_file;
		}
	}

	return TAI_CONTINUE(int, sceIoGetstatRef, file, stat);
}

extern void *pops_data;

static int sceDisplaySetFrameBufForCompatPatched(int a1, int a2, int a3, int a4, int a5, SceDisplayFrameBuf *pParam) {
	if (config.graphics_filtering != 0) {
		if (pParam == NULL) {
			static SceDisplayFrameBuf param;
			param.size = sizeof(SceDisplayFrameBuf);
			param.base = pops_data;
			param.pitch = SCREEN_LINE;
			param.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
			param.width = SCREEN_WIDTH;
			param.height = SCREEN_HEIGHT;
			pParam = &param;
		}
	}

	return TAI_CONTINUE(int, sceDisplaySetFrameBufForCompatRef, a1, a2, a3, a4, a5, pParam);
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
	int res;

	// Get app titleid
	sceAppMgrGetNameById(sceKernelGetProcessId(), app_titleid);

	// Init vita newlib
	_init_vita_newlib();

	// Read config
	memset(&config, 0, sizeof(AdrenalineConfig));
	ReadFile("ux0:adrenaline/adrenaline.bin", &config, sizeof(AdrenalineConfig));

	// Use ux0 if imc0 is unavailable
	if (config.ms_location == MEMORY_STICK_LOCATION_IMC0 && !hasImc0())
		config.ms_location = MEMORY_STICK_LOCATION_UX0;

	// Tai module info
	tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);
	res = taiGetModuleInfo("ScePspemu", &tai_info);
	if (res < 0)
		return res;

	// Module info
	SceKernelModuleInfo mod_info;
	mod_info.size = sizeof(SceKernelModuleInfo);
	res = sceKernelGetModuleInfo(tai_info.modid, &mod_info);
	if (res < 0)
		return res;

	// Addresses
	text_addr = (uint32_t)mod_info.segments[0].vaddr;
	text_size = (uint32_t)mod_info.segments[0].memsz;

	data_addr = (uint32_t)mod_info.segments[1].vaddr;
	data_size = (uint32_t)mod_info.segments[1].memsz;

	// Get functions
	GetFunctions();

	// SceCompat
	hooks[n_hooks++] = taiHookFunctionImport(&sceCompatSuspendResumeRef, "ScePspemu", 0x0F35909D, 0x324112CA, sceCompatSuspendResumePatched);
	hooks[n_hooks++] = taiHookFunctionImport(&sceCompatWriteSharedCtrlRef, "ScePspemu", 0x0F35909D, 0x2306FFED, sceCompatWriteSharedCtrlPatched);	
	hooks[n_hooks++] = taiHookFunctionImport(&sceCompatWaitSpecialRequestRef, "ScePspemu", 0x0F35909D, 0x714F7ED6, sceCompatWaitSpecialRequestPatched);

	// SceShellUtil
	hooks[n_hooks++] = taiHookFunctionImport(&sceShellUtilRegisterSettingsHandlerRef, "ScePspemu", 0xD2B1C8AE, 0xCE35B2B8, sceShellUtilRegisterSettingsHandlerPatched);

	// SceLibKernel
	hooks[n_hooks++] = taiHookFunctionImport(&sceKernelCreateThreadRef, "ScePspemu", 0xCAE9ACE6, 0xC5C11EE7, sceKernelCreateThreadPatched);
	hooks[n_hooks++] = taiHookFunctionImport(&sceIoOpenRef, "ScePspemu", 0xCAE9ACE6, 0x6C60AC61, sceIoOpenPatched);
	hooks[n_hooks++] = taiHookFunctionImport(&sceIoGetstatRef, "ScePspemu", 0xCAE9ACE6, 0xBCA5B623, sceIoGetstatPatched);

	// SceAudio
	hooks[n_hooks++] = taiHookFunctionImport(&sceAudioOutOpenPortRef, "ScePspemu", 0x438BB957, 0x5BC341E4, sceAudioOutOpenPortPatched);
	hooks[n_hooks++] = taiHookFunctionImport(&sceAudioOutOutputRef, "ScePspemu", 0x438BB957, 0x02DB3F5F, sceAudioOutOutputPatched);

	// SceCtrl
	hooks[n_hooks++] = taiHookFunctionImport(&sceCtrlPeekBufferNegative2Ref, "ScePspemu", 0xD197E3C7, 0x81A89660, sceCtrlPeekBufferNegative2Patched);

	// SceDisplayUser
	hooks[n_hooks++] = taiHookFunctionImport(&sceDisplaySetFrameBufForCompatRef, "ScePspemu", 0x4FAACD11, 0x8C36B628, sceDisplaySetFrameBufForCompatPatched);

	// ScePspemu
	hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuInitTitleSpecificInfoRef, tai_info.modid, 0, 0x20374, 0x1, ScePspemuInitTitleSpecificInfoPatched);
	hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuGetStartupPngRef, tai_info.modid, 0, 0x3C88, 0x1, ScePspemuGetStartupPngPatched);
	hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuGetTitleidRef, tai_info.modid, 0, 0x205FC, 0x1, ScePspemuGetTitleidPatched);
	hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuInitAudioOutRef, tai_info.modid, 0, 0xD190, 0x1, ScePspemuInitAudioOutPatched);
	hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuConvertAddressRef, tai_info.modid, 0, 0x6364, 0x1, ScePspemuConvertAddressPatched);
	hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuDecodePopsAudioRef, tai_info.modid, 0, 0x2D62C, 0x1, ScePspemuDecodePopsAudioPatched);

	// 0x01C00000 -> 0x03C00000
	uint32_t cmp_a4_3C00000 = 0x7F70F1B3;
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x6394, &cmp_a4_3C00000, sizeof(cmp_a4_3C00000));

	uint32_t cmp_v2_3C00000 = 0x7F70F1B5;
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x6434, &cmp_v2_3C00000, sizeof(cmp_v2_3C00000));

	uint32_t cmp_a3_3C00000 = 0x7F70F1B2;
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x6534, &cmp_a3_3C00000, sizeof(cmp_a3_3C00000));

	////////////////////////////////////////////////////////////////////////

	// Use different mode for ScePspemuRemotePocs
	uint16_t movs_a1_E = 0x200E;
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811A2B34 - 0x81180400, &movs_a1_E, sizeof(movs_a1_E));

	// g_is_pops patches

	uint32_t movs_a4_1_nop_opcode = 0xBF002301;
	uint32_t movs_a1_0_nop_opcode = 0xBF002000;
	uint32_t movs_a1_1_nop_opcode = 0xBF002001;

	// Resume stuff. PROBABLY SHOULD DO POPS AND PSP MODE STUFF
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811846F0 - 0x81180400, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

	// Unknown. Mode 4, 5
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81185B2E - 0x81180400, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

	// Set cache address for pops stuff
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81185BC0 - 0x81180400, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

	// Read savedata and menu info. Should be enabled, otherwise an error will occur
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81185FBA - 0x81180400, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

	// Get app state for pops
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81186052 - 0x81180400, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

	// Unknown
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x8118624A - 0x81180400, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

	///////////////////////////

	// isPops patches

	// Peripheral

	// Use vibration
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81196DF6 - 0x81180400, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

	// Unknown check for POPS mode
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81196EEC - 0x81180400, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

	// Unknown check for PSP mode. If false return 0x80010089
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81196F6C - 0x81180400, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

	// Unknown check for PSP mode. If false return 0
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81196F86 - 0x81180400, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

	// Unknown check for PSP mode. If false return 0
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x8119703E - 0x81180400, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

	////////////////////

	// Init ScePspemuMenuWork
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x8119865E - 0x81180400, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

	// Read savedata and menu info
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811A161E - 0x81180400, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

	// POPS Settings menu function
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81197F32 - 0x81180400, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

	/////////////////////

	// Settings related. Screenshot is enabled/disabled here. Responsible for __sce_menuinfo saving
	uint32_t bl_is_pops_patched_opcode_1 = encode_bl(text_addr + 0x811A1422 - 0x81180400, text_addr + 0x811A0784 - 0x81180400);
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811A1422 - 0x81180400, &bl_is_pops_patched_opcode_1, sizeof(bl_is_pops_patched_opcode_1));

	uint32_t bl_is_pops_patched_opcode_2 = encode_bl(text_addr + 0x811A144C - 0x81180400, text_addr + 0x811A0784 - 0x81180400);
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811A144C - 0x81180400, &bl_is_pops_patched_opcode_2, sizeof(bl_is_pops_patched_opcode_2));

	// Switch between PSP mode settings and POPS mode settings
	uint32_t bl_is_pops_patched_opcode_3 = encode_bl(text_addr + 0x81197FEA - 0x81180400, text_addr + 0x811A0784 - 0x81180400);
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x81197FEA - 0x81180400, &bl_is_pops_patched_opcode_3, sizeof(bl_is_pops_patched_opcode_3));

	// Draw dialog on PSP screen or POPS screen
	uint32_t bl_is_pops_patched_opcode_4 = encode_bl(text_addr + 0x811987A4 - 0x81180400, text_addr + 0x811A0784 - 0x81180400);
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811987A4 - 0x81180400, &bl_is_pops_patched_opcode_4, sizeof(bl_is_pops_patched_opcode_4));

	// ctrlEmulation. If not patched, buttons assignment in ps1emu don't work
	uint32_t bl_is_pops_patched_opcode_5 = encode_bl(text_addr + 0x811A0B10 - 0x81180400, text_addr + 0x811A0784 - 0x81180400);
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811A0B10 - 0x81180400, &bl_is_pops_patched_opcode_5, sizeof(bl_is_pops_patched_opcode_5));

	// Use available code memory at text_addr + 0x811A0784 - 0x81180400 (ScePspemuInitTitleSpecificInfo)
	// For custom function: isPopsPatched
	uint32_t isPopsPatched[4];
	uint32_t pops_mode_offset = CONVERT_ADDRESS(ADRENALINE_ADDRESS) + offsetof(SceAdrenaline, pops_mode);
	isPopsPatched[0] = encode_movw(0, pops_mode_offset & 0xFFFF);
	isPopsPatched[1] = encode_movt(0, pops_mode_offset >> 0x10);
	isPopsPatched[2] = 0xBF006800; // ldr a1, [a1]
	isPopsPatched[3] = 0xBF004770; // bx lr
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811A0784 - 0x81180400, isPopsPatched, sizeof(isPopsPatched));

	if (!sceKernelIsPSVitaTV()) {
		// Fake isVitaTV for pops ctrl
		uint32_t bl_is_vita_tv_patched_opcode_1 = encode_bl(text_addr + 0x811AF9BC - 0x81180400, text_addr + 0x811A0794 - 0x81180400);
		uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811AF9BC - 0x81180400, &bl_is_vita_tv_patched_opcode_1, sizeof(bl_is_vita_tv_patched_opcode_1));
		uint32_t bl_is_vita_tv_patched_opcode_2 = encode_bl(text_addr + 0x811AFB4E - 0x81180400, text_addr + 0x811A0794 - 0x81180400);
		uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811AFB4E - 0x81180400, &bl_is_vita_tv_patched_opcode_2, sizeof(bl_is_vita_tv_patched_opcode_2));

		// Use available code memory at text_addr + 0x811A0794 - 0x81180400 (ScePspemuInitTitleSpecificInfo)
		// For custom function: isVitaTVPatched
		uint32_t isVitaTVPatched[4];
		uint32_t use_ds3_ds4_offset = (uint32_t)&config.use_ds3_ds4;
		isVitaTVPatched[0] = encode_movw(0, use_ds3_ds4_offset & 0xFFFF);
		isVitaTVPatched[1] = encode_movt(0, use_ds3_ds4_offset >> 0x10);
		isVitaTVPatched[2] = 0xBF006800; // ldr a1, [a1]
		isVitaTVPatched[3] = 0xBF004770; // bx lr
		uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811A0794 - 0x81180400, isVitaTVPatched, sizeof(isVitaTVPatched));
	}

	// Fake vita mode for ctrlEmulation
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811A0B3C - 0x81180400, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811A0C4E - 0x81180400, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
	uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x811B05DC - 0x81180400, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
	int i;
	for (i = n_uids-1; i >= 0; i++) {
		taiInjectRelease(uids[i]);
	}

	taiHookRelease(hooks[--n_hooks], ScePspemuDecodePopsAudioRef);
	taiHookRelease(hooks[--n_hooks], ScePspemuConvertAddressRef);
	taiHookRelease(hooks[--n_hooks], ScePspemuInitAudioOutRef);
	taiHookRelease(hooks[--n_hooks], ScePspemuGetTitleidRef);
	taiHookRelease(hooks[--n_hooks], ScePspemuGetStartupPngRef);
	taiHookRelease(hooks[--n_hooks], ScePspemuInitTitleSpecificInfoRef);

	taiHookRelease(hooks[--n_hooks], sceDisplaySetFrameBufForCompatRef);
	taiHookRelease(hooks[--n_hooks], sceCtrlPeekBufferNegative2Ref);
	taiHookRelease(hooks[--n_hooks], sceAudioOutOutputRef);
	taiHookRelease(hooks[--n_hooks], sceAudioOutOpenPortRef);
	taiHookRelease(hooks[--n_hooks], sceIoGetstatRef);
	taiHookRelease(hooks[--n_hooks], sceIoOpenRef);
	taiHookRelease(hooks[--n_hooks], sceKernelCreateThreadRef);
	taiHookRelease(hooks[--n_hooks], sceShellUtilRegisterSettingsHandlerRef);
	taiHookRelease(hooks[--n_hooks], sceCompatWaitSpecialRequestRef);
	taiHookRelease(hooks[--n_hooks], sceCompatWriteSharedCtrlRef);
	taiHookRelease(hooks[--n_hooks], sceCompatSuspendResumeRef);

	return SCE_KERNEL_STOP_SUCCESS;
}
