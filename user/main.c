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

#include <psp2/appmgr.h>
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
#include <psp2/kernel/clib.h>
#include <psp2/kernel/dmac.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "pops.h"
#include "titleinfo.h"
#include "flashfs.h"
#include "msfs.h"
#include "menu.h"
#include "states.h"
#include "usb.h"
#include "utils.h"

#include "msfs.h"

#include "lz4/lz4.h"

#include "startdat.h"

INCLUDE_EXTERN_RESOURCE(payloadex_bin);

int _newlib_heap_size_user = 8 * 1024 * 1024;

int (* ScePspemuDivide)(uint64_t x, uint64_t y);
int (* ScePspemuErrorExit)(int error);
int (* ScePspemuConvertAddress)(uint32_t addr, int mode, uint32_t cache_size);
int (* ScePspemuWritebackCache)(void *addr, int size);
int (* ScePspemuKermitWaitAndGetRequest)(int mode, SceKermitRequest **request);
int (* ScePspemuKermitSendResponse)(int mode, SceKermitRequest *request, uint64_t response);
int (* ScePspemuConvertStatTimeToUtc)(SceIoStat *stat);
int (* ScePspemuConvertStatTimeToLocaltime)(SceIoStat *stat);
int (* ScePspemuSettingsHandler)(int a1, int a2, int a3, int a4);
int (* ScePspemuSetDisplayConfig)();
int (* ScePspemuPausePops)(int pause);
int (* ScePspemuInitPops)();
int (* ScePspemuInitPocs)();

tai_hook_ref_t sceCompatSuspendResumeRef;
tai_hook_ref_t sceCompatWriteSharedCtrlRef;
tai_hook_ref_t sceCompatWaitSpecialRequestRef;
tai_hook_ref_t sceShellUtilRegisterSettingsHandlerRef;
tai_hook_ref_t sceKernelCreateThreadRef;
tai_hook_ref_t sceIoOpenRef;
tai_hook_ref_t sceIoGetstatRef;
tai_hook_ref_t sceAudioOutOpenPortRef;
tai_hook_ref_t sceAudioOutOutputRef;
tai_hook_ref_t sceDisplaySetFrameBufForCompatRef;

tai_hook_ref_t ScePspemuInitTitleSpecificInfoRef;
tai_hook_ref_t ScePspemuGetStartupPngRef;
tai_hook_ref_t ScePspemuGetTitleidRef;
tai_hook_ref_t ScePspemuInitAudioOutRef;
tai_hook_ref_t ScePspemuConvertAddressRef;
tai_hook_ref_t ScePspemuDecodePopsAudioRef;
tai_hook_ref_t ScePspemuGetParamRef;

static SceUID hooks[64];
static SceUID uids[64];
static int n_hooks = 0;
static int n_uids = 0;

uint32_t module_nid;
uint32_t text_addr, text_size, data_addr, data_size;

static int lock_power = 0;

SceUID usbdevice_modid = -1;

AdrenalineConfig config;

extern int menu_open;

void GetFunctions() {
  ScePspemuDivide                     = (void *)(text_addr + 0x39F0 + 0x1);
  ScePspemuErrorExit                  = (void *)(text_addr + 0x4104 + 0x1);
  ScePspemuConvertAddress             = (void *)(text_addr + 0x6364 + 0x1);
  ScePspemuWritebackCache             = (void *)(text_addr + 0x6490 + 0x1);
  ScePspemuKermitWaitAndGetRequest    = (void *)(text_addr + 0x64D0 + 0x1);
  ScePspemuKermitSendResponse         = (void *)(text_addr + 0x6560 + 0x1);
  ScePspemuConvertStatTimeToUtc       = (void *)(text_addr + 0x8664 + 0x1);
  ScePspemuConvertStatTimeToLocaltime = (void *)(text_addr + 0x8680 + 0x1);

  if (module_nid == 0x2714F07D) { // 3.60 retail
    ScePspemuSetDisplayConfig           = (void *)(text_addr + 0x20E50 + 0x1);
    ScePspemuPausePops                  = (void *)(text_addr + 0x300C0 + 0x1);
    ScePspemuInitPops                   = (void *)(text_addr + 0x30678 + 0x1);
    ScePspemuInitPocs                   = (void *)(text_addr + 0x227C4 + 0x1);
  } else if (module_nid == 0x3F75D4D3) { // 3.65-3.70 retail
    ScePspemuSetDisplayConfig           = (void *)(text_addr + 0x20E54 + 0x1);
    ScePspemuPausePops                  = (void *)(text_addr + 0x300D4 + 0x1);
    ScePspemuInitPops                   = (void *)(text_addr + 0x3068C + 0x1);
    ScePspemuInitPocs                   = (void *)(text_addr + 0x227D0 + 0x1);
  }
}

void SendAdrenalineRequest(int cmd) {
  SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_OUTPUT_MODE, ADRENALINE_SIZE);
  adrenaline->psp_cmd = cmd;
  ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);

  sceCompatInterrupt(KERMIT_VIRTUAL_INTR_IMPOSE_CH1);
}

#define LZ4_ACCELERATION 8
#define SAVESTATE_TEMP_SIZE (32 * 1024 * 1024)

int SaveState(SceAdrenaline *adrenaline, void *savestate_data) {
  void *ram = (void *)ScePspemuConvertAddress(0x88000000, KERMIT_INPUT_MODE, PSP_RAM_SIZE);

  char path[128];
  makeSaveStatePath(path, adrenaline->num);

  SceUID fd = sceIoOpen(path, SCE_O_WRONLY, 0777);
  if (fd < 0)
    return fd;

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

  return 0;
}

int LoadState(SceAdrenaline *adrenaline, void *savestate_data) {
  void *ram = (void *)ScePspemuConvertAddress(0x88000000, KERMIT_OUTPUT_MODE, PSP_RAM_SIZE);

  char path[128];
  makeSaveStatePath(path, adrenaline->num);

  SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
  if (fd < 0)
    return fd;

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

  ScePspemuWritebackCache(ram, PSP_RAM_SIZE);

  return 0;
}

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

    SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_INPUT_MODE | KERMIT_OUTPUT_MODE, ADRENALINE_SIZE);

    int res = -1;

    if (request->cmd == ADRENALINE_VITA_CMD_SAVESTATE) {
      SaveState(adrenaline, savestate_data);
      adrenaline->vita_response = ADRENALINE_VITA_RESPONSE_SAVED;
      ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);

      // Continue, do not send response
      continue;
    } else if (request->cmd == ADRENALINE_VITA_CMD_LOADSTATE) {
      LoadState(adrenaline, savestate_data);
      adrenaline->vita_response = ADRENALINE_VITA_RESPONSE_LOADED;
      ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);

      // Continue, do not send response
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

        if (config.usbdevice == USBDEVICE_MODE_INTERNAL_STORAGE) {
          path = "sdstor0:int-lp-ign-user";
        } else if (config.usbdevice == USBDEVICE_MODE_SD2VITA) {
          path = "sdstor0:gcd-lp-ign-entire";
        } else if (config.usbdevice == USBDEVICE_MODE_PSVSD) {
          path = "sdstor0:uma-pp-act-a";
        } else {
          path = "sdstor0:xmc-lp-ign-userext";

          SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);

          if (fd < 0)
            path = "sdstor0:int-lp-ign-userext";
          else
            sceIoClose(fd);
        }

        usbdevice_modid = startUsb("ux0:app/" ADRENALINE_TITLEID "/sce_module/usbdevice.skprx", path, SCE_USBSTOR_VSTOR_TYPE_FAT);

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
      sceDisplayWaitVblankStart();
      sceDisplayWaitVblankStart();
      SetPspemuFrameBuffer((void *)SCE_PSPEMU_FRAMEBUFFER);
      adrenaline->draw_psp_screen_in_pops = 1;
      ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);
      res = 0;
    } else if (request->cmd == ADRENALINE_VITA_CMD_RESUME_POPS) {
      if (!menu_open)
        ScePspemuPausePops(0);
      sceDisplayWaitVblankStart();
      sceDisplayWaitVblankStart();
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

  pad_data->Buttons &= ~SCE_CTRL_PSBUTTON;
  pad_data->Buttons |= (pad.buttons & SCE_CTRL_PSBUTTON);

  if (menu_open) {
    pad_data->Buttons = SCE_CTRL_PSBUTTON;
    pad_data->Lx = 128;
    pad_data->Ly = 128;
    pad_data->Rx = 128;
    pad_data->Ry = 128;
  }

  return TAI_CONTINUE(int, sceCompatWriteSharedCtrlRef, pad_data);
}

static int sceCompatWaitSpecialRequestPatched(int mode) {
  ScePspemuBuildFlash0();

  void *payloadex = (void *)&_binary_flash0_payloadex_bin_start;
  int size_payloadex = (int)&_binary_flash0_payloadex_bin_size;

  uint32_t *m = (uint32_t *)ScePspemuConvertAddress(0x88FC0000, KERMIT_OUTPUT_MODE, size_payloadex);
  memcpy(m, payloadex, size_payloadex);
  ScePspemuWritebackCache(m, size_payloadex);

  void *n = (void *)ScePspemuConvertAddress(0x88FB0000, KERMIT_OUTPUT_MODE, 0x100);
  memset(n, 0, 0x100);

  SceCtrlData pad;
  kuCtrlPeekBufferPositive(0, &pad, 1);

  if (pad.buttons & SCE_CTRL_RTRIGGER)
    ((uint32_t *)n)[0] = 4; // Recovery mode

  SceIoStat stat;
  memset(&stat, 0, sizeof(SceIoStat));
  if (sceIoGetstat("ux0:app/" ADRENALINE_TITLEID "/flash0", &stat) < 0)
    ((uint32_t *)n)[0] = 4; // Recovery mode

  ScePspemuWritebackCache(n, 0x100);

  // Init Adrenaline
  InitAdrenaline();

  // Clear 0x8A000000 memory
  sceDmacMemset((void *)0x63000000, 0, 16 * 1024 * 1024);
  sceCompatCache(SCE_COMPAT_CACHE_WRITEBACK, (void *)0x63000000, 16 * 1024 * 1024);

  return TAI_CONTINUE(int, sceCompatWaitSpecialRequestRef, mode);
}

static int sceShellUtilRegisterSettingsHandlerPatched(int (* handler)(int a1, int a2, int a3, int a4), int unk) {
  if (handler) {
    ScePspemuSettingsHandler = handler;
    handler = ScePspemuCustomSettingsHandler;
  }

  return TAI_CONTINUE(int, sceShellUtilRegisterSettingsHandlerRef, handler, unk);
}

static SceUID sceKernelCreateThreadPatched(const char *name, SceKernelThreadEntry entry, int initPriority,
                      int stackSize, SceUInt attr, int cpuAffinityMask,
                      const SceKernelThreadOptParam *option) {
  if (strcmp(name, "ScePspemuRemoteMsfs") == 0) {
    entry = (SceKernelThreadEntry)ScePspemuRemoteMsfs;
  }

  return TAI_CONTINUE(SceUID, sceKernelCreateThreadRef, name, entry, initPriority, stackSize, attr, cpuAffinityMask, option);
}

static int ScePspemuGetParamPatched(char *discid, int *parentallevel, char *gamedataid, char *appver, int *bootable, int *isPops, int *isPocketStation) {
  TAI_CONTINUE(int, ScePspemuGetParamRef, discid, parentallevel, gamedataid, appver, bootable, isPops, isPocketStation);

  // originalpath
  strcpy((char *)(data_addr + 0x432C), "ux0:app/" ADRENALINE_TITLEID "/EBOOT.PBP");

  // selfpath
  strcpy((char *)(data_addr + 0x472C), "ux0:app/" ADRENALINE_TITLEID "/EBOOT.PBP");

  return 0;
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

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
  int res;

  // Init vita newlib
  _init_vita_newlib();

  // Read config
  memset(&config, 0, sizeof(AdrenalineConfig));
  ReadFile("ux0:app/" ADRENALINE_TITLEID "/adrenaline.bin", &config, sizeof(AdrenalineConfig));
  if ((uint32_t)config.psp_screen_scale_x == 0)
    config.psp_screen_scale_x = 2.0f;
  if ((uint32_t)config.psp_screen_scale_y == 0)
    config.psp_screen_scale_y = 2.0f;
  if ((uint32_t)config.ps1_screen_scale_x == 0)
    config.ps1_screen_scale_x = 1.0f;
  if ((uint32_t)config.ps1_screen_scale_y == 0)
    config.ps1_screen_scale_y = 1.0f;

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

  module_nid = tai_info.module_nid;

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

  // SceDisplayUser
  hooks[n_hooks++] = taiHookFunctionImport(&sceDisplaySetFrameBufForCompatRef, "ScePspemu", 0x4FAACD11, 0x8C36B628, sceDisplaySetFrameBufForCompatPatched);

  // ScePspemu
  if (module_nid == 0x2714F07D) { // 3.60 retail
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuInitTitleSpecificInfoRef, tai_info.modid, 0, 0x20374, 0x1, ScePspemuInitTitleSpecificInfoPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuGetStartupPngRef, tai_info.modid, 0, 0x3C88, 0x1, ScePspemuGetStartupPngPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuGetTitleidRef, tai_info.modid, 0, 0x205FC, 0x1, ScePspemuGetTitleidPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuInitAudioOutRef, tai_info.modid, 0, 0xD190, 0x1, ScePspemuInitAudioOutPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuConvertAddressRef, tai_info.modid, 0, 0x6364, 0x1, ScePspemuConvertAddressPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuDecodePopsAudioRef, tai_info.modid, 0, 0x2D62C, 0x1, ScePspemuDecodePopsAudioPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuGetParamRef, tai_info.modid, 0, 0xAAF4, 0x1, ScePspemuGetParamPatched);

    // Increase RAM size from 0x01C00000 to 0x03C00000
    uint32_t cmp_a4_3C00000 = 0x7F70F1B3;
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x6394, &cmp_a4_3C00000, sizeof(cmp_a4_3C00000));

    uint32_t cmp_v2_3C00000 = 0x7F70F1B5;
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x6434, &cmp_v2_3C00000, sizeof(cmp_v2_3C00000));

    uint32_t cmp_a3_3C00000 = 0x7F70F1B2;
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x6534, &cmp_a3_3C00000, sizeof(cmp_a3_3C00000));

    ////////////////////////////////////////////////////////////////////////

    // Use different mode for ScePspemuRemotePocs
    uint16_t movs_a1_E = 0x200E;
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x22734, &movs_a1_E, sizeof(movs_a1_E));

    // g_is_pops patches

    uint32_t movs_a4_1_nop_opcode = 0xBF002301;
    uint32_t movs_a1_0_nop_opcode = 0xBF002000;
    uint32_t movs_a1_1_nop_opcode = 0xBF002001;

    // Resume stuff. PROBABLY SHOULD DO POPS AND PSP MODE STUFF
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x42F0, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Unknown. Mode 4, 5
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x572E, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Set cache address for pops stuff
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x57C0, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Read savedata and menu info. Should be enabled, otherwise an error will occur
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x5BBA, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Get app state for pops
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x5C52, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Unknown
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x5E4A, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    ///////////////////////////

    // isPops patches

    // Peripheral

    // Use vibration
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x169F6, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for POPS mode
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x16AEC, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0x80010089
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x16B6C, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x16B86, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x16C3E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    ////////////////////

    // Init ScePspemuMenuWork
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x1825E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Read savedata and menu info
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x2121E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // POPS Settings menu function
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x17B32, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    /////////////////////

    // Settings related. Screenshot is enabled/disabled here. Responsible for __sce_menuinfo saving
    uint32_t bl_is_pops_patched_opcode_1 = encode_bl(text_addr + 0x21022, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x21022, &bl_is_pops_patched_opcode_1, sizeof(bl_is_pops_patched_opcode_1));

    uint32_t bl_is_pops_patched_opcode_2 = encode_bl(text_addr + 0x2104C, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x2104C, &bl_is_pops_patched_opcode_2, sizeof(bl_is_pops_patched_opcode_2));

    // Switch between PSP mode settings and POPS mode settings
    uint32_t bl_is_pops_patched_opcode_3 = encode_bl(text_addr + 0x17BEA, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x17BEA, &bl_is_pops_patched_opcode_3, sizeof(bl_is_pops_patched_opcode_3));

    // Draw dialog on PSP screen or POPS screen
    uint32_t bl_is_pops_patched_opcode_4 = encode_bl(text_addr + 0x183A4, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x183A4, &bl_is_pops_patched_opcode_4, sizeof(bl_is_pops_patched_opcode_4));

    // ctrlEmulation. If not patched, buttons assignment in ps1emu don't work
    uint32_t bl_is_pops_patched_opcode_5 = encode_bl(text_addr + 0x20710, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x20710, &bl_is_pops_patched_opcode_5, sizeof(bl_is_pops_patched_opcode_5));

    // Use available code memory at text_addr + 0x20384 (ScePspemuInitTitleSpecificInfo)
    // For custom function: isPopsPatched
    uint32_t isPopsPatched[4];
    uint32_t pops_mode_offset = CONVERT_ADDRESS(ADRENALINE_ADDRESS) + offsetof(SceAdrenaline, pops_mode);
    isPopsPatched[0] = encode_movw(0, pops_mode_offset & 0xFFFF);
    isPopsPatched[1] = encode_movt(0, pops_mode_offset >> 0x10);
    isPopsPatched[2] = 0xBF006800; // ldr a1, [a1]
    isPopsPatched[3] = 0xBF004770; // bx lr
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x20384, isPopsPatched, sizeof(isPopsPatched));

    // Fake vita mode for ctrlEmulation
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x2073C, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x2084E, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x301DC, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
  } else if (module_nid == 0x3F75D4D3) { // 3.65-3.70 retail
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuInitTitleSpecificInfoRef, tai_info.modid, 0, 0x20378, 0x1, ScePspemuInitTitleSpecificInfoPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuGetStartupPngRef, tai_info.modid, 0, 0x3C88, 0x1, ScePspemuGetStartupPngPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuGetTitleidRef, tai_info.modid, 0, 0x20600, 0x1, ScePspemuGetTitleidPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuInitAudioOutRef, tai_info.modid, 0, 0xD190, 0x1, ScePspemuInitAudioOutPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuConvertAddressRef, tai_info.modid, 0, 0x6364, 0x1, ScePspemuConvertAddressPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuDecodePopsAudioRef, tai_info.modid, 0, 0x2D638, 0x1, ScePspemuDecodePopsAudioPatched);
    hooks[n_hooks++] = taiHookFunctionOffset(&ScePspemuGetParamRef, tai_info.modid, 0, 0xAAF4, 0x1, ScePspemuGetParamPatched);

    // Increase RAM size from 0x01C00000 to 0x03C00000
    uint32_t cmp_a4_3C00000 = 0x7F70F1B3;
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x6394, &cmp_a4_3C00000, sizeof(cmp_a4_3C00000));

    uint32_t cmp_v2_3C00000 = 0x7F70F1B5;
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x6434, &cmp_v2_3C00000, sizeof(cmp_v2_3C00000));

    uint32_t cmp_a3_3C00000 = 0x7F70F1B2;
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x6534, &cmp_a3_3C00000, sizeof(cmp_a3_3C00000));

    ////////////////////////////////////////////////////////////////////////

    // Use different mode for ScePspemuRemotePocs
    uint16_t movs_a1_E = 0x200E;
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x22740, &movs_a1_E, sizeof(movs_a1_E));

    // g_is_pops patches

    uint32_t movs_a4_1_nop_opcode = 0xBF002301;
    uint32_t movs_a1_0_nop_opcode = 0xBF002000;
    uint32_t movs_a1_1_nop_opcode = 0xBF002001;

    // Resume stuff. PROBABLY SHOULD DO POPS AND PSP MODE STUFF
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x42F0, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Unknown. Mode 4, 5
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x572E, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Set cache address for pops stuff
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x57C0, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Read savedata and menu info. Should be enabled, otherwise an error will occur
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x5BBA, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Get app state for pops
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x5C52, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Unknown
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x5E4A, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    ///////////////////////////

    // isPops patches

    // Peripheral

    // Use vibration
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x169F6, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for POPS mode
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x16AEC, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0x80010089
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x16B6C, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x16B86, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x16C3E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    ////////////////////

    // Init ScePspemuMenuWork
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x1825E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Read savedata and menu info
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x21222, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // POPS Settings menu function
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x17B32, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    /////////////////////

    // Settings related. Screenshot is enabled/disabled here. Responsible for __sce_menuinfo saving
    uint32_t bl_is_pops_patched_opcode_1 = encode_bl(text_addr + 0x21026, text_addr + 0x20388);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x21026, &bl_is_pops_patched_opcode_1, sizeof(bl_is_pops_patched_opcode_1));

    uint32_t bl_is_pops_patched_opcode_2 = encode_bl(text_addr + 0x21050, text_addr + 0x20388);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x21050, &bl_is_pops_patched_opcode_2, sizeof(bl_is_pops_patched_opcode_2));

    // Switch between PSP mode settings and POPS mode settings
    uint32_t bl_is_pops_patched_opcode_3 = encode_bl(text_addr + 0x17BEC, text_addr + 0x20388);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x17BEC, &bl_is_pops_patched_opcode_3, sizeof(bl_is_pops_patched_opcode_3));

    // Draw dialog on PSP screen or POPS screen
    uint32_t bl_is_pops_patched_opcode_4 = encode_bl(text_addr + 0x183A4, text_addr + 0x20388);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x183A4, &bl_is_pops_patched_opcode_4, sizeof(bl_is_pops_patched_opcode_4));

    // ctrlEmulation. If not patched, buttons assignment in ps1emu don't work
    uint32_t bl_is_pops_patched_opcode_5 = encode_bl(text_addr + 0x20714, text_addr + 0x20388);
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x20714, &bl_is_pops_patched_opcode_5, sizeof(bl_is_pops_patched_opcode_5));

    // Use available code memory at text_addr + 0x20388 (ScePspemuInitTitleSpecificInfo)
    // For custom function: isPopsPatched
    uint32_t isPopsPatched[4];
    uint32_t pops_mode_offset = CONVERT_ADDRESS(ADRENALINE_ADDRESS) + offsetof(SceAdrenaline, pops_mode);
    isPopsPatched[0] = encode_movw(0, pops_mode_offset & 0xFFFF);
    isPopsPatched[1] = encode_movt(0, pops_mode_offset >> 0x10);
    isPopsPatched[2] = 0xBF006800; // ldr a1, [a1]
    isPopsPatched[3] = 0xBF004770; // bx lr
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x20388, isPopsPatched, sizeof(isPopsPatched));

    // Fake vita mode for ctrlEmulation
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x20740, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x20852, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
    uids[n_uids++] = taiInjectData(tai_info.modid, 0, 0x301F0, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
  }

  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
  int i;
  for (i = n_uids - 1; i >= 0; i++) {
    taiInjectRelease(uids[i]);
  }

  taiHookRelease(hooks[--n_hooks], ScePspemuGetParamRef);
  taiHookRelease(hooks[--n_hooks], ScePspemuDecodePopsAudioRef);
  taiHookRelease(hooks[--n_hooks], ScePspemuConvertAddressRef);
  taiHookRelease(hooks[--n_hooks], ScePspemuInitAudioOutRef);
  taiHookRelease(hooks[--n_hooks], ScePspemuGetTitleidRef);
  taiHookRelease(hooks[--n_hooks], ScePspemuGetStartupPngRef);
  taiHookRelease(hooks[--n_hooks], ScePspemuInitTitleSpecificInfoRef);

  taiHookRelease(hooks[--n_hooks], sceDisplaySetFrameBufForCompatRef);
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
