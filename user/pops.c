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

#include <psp2/audioout.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/dmac.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "menu.h"
#include "utils.h"

int ScePspemuInitAudioOutPatched() {
  int res = TAI_CONTINUE(int, ScePspemuInitAudioOutRef);

  res = ScePspemuInitPops();
  if (res < 0)
    return res;

  SceUID blockid = sceKernelAllocMemBlock("ScePspemuMcWork", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, 0x40000, NULL);
  if (blockid < 0)
    return blockid;

  sceKernelGetMemBlockBase(blockid, (void *)(data_addr + 0x10100));

  int (* init_sth)() = NULL;

  if (module_nid == 0x2714F07D) { // 3.60 retail
    init_sth = (void *)(text_addr + 0x31F90 + 0x1);
  } else if (module_nid == 0x3F75D4D3) { // 3.65/3.67/3.68 retail
    init_sth = (void *)(text_addr + 0x31FA4 + 0x1);
  }

  init_sth(*(uint32_t *)(data_addr + 0x10100), 0x1E000);

  res = ScePspemuInitPocs();
  if (res < 0)
    return res;

  return res;
}

static int pops_audio_port = -1;

int sceAudioOutOpenPortPatched(int type, int len, int freq, int mode) {
  int res = TAI_CONTINUE(int, sceAudioOutOpenPortRef, type, len, freq, mode);

  // Use voice port
  if (res == SCE_AUDIO_OUT_ERROR_PORT_FULL && type == SCE_AUDIO_OUT_PORT_TYPE_BGM) {
    pops_audio_port = TAI_CONTINUE(int, sceAudioOutOpenPortRef, SCE_AUDIO_OUT_PORT_TYPE_VOICE, len, freq, mode);
    return pops_audio_port;
  }

  return res;
}

int sceAudioOutOutputPatched(int port, const void *buf) {
  SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_INPUT_MODE, ADRENALINE_SIZE);

  if (port == pops_audio_port && !adrenaline->pops_mode) {
    sceDisplayWaitVblankStart();
    return 0;
  }

  return TAI_CONTINUE(int, sceAudioOutOutputRef, port, buf);
}

int ScePspemuDecodePopsAudioPatched(int a1, int a2, int a3, int a4) {
  SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_INPUT_MODE, ADRENALINE_SIZE);

  if (!adrenaline->pops_mode) {
    return 0;
  }

  return TAI_CONTINUE(int, ScePspemuDecodePopsAudioRef, a1, a2, a3, a4);
}

int sceCtrlPeekBufferNegative2Patched(int port, SceCtrlData *pad_data, int count) {
  int res = TAI_CONTINUE(int, sceCtrlPeekBufferNegative2Ref, port, pad_data, count);

  if (res == 0x80340001) {
    if (!sceKernelIsPSVitaTV()) {
      if (config.use_ds3_ds4 && port == 1) {
        return TAI_CONTINUE(int, sceCtrlPeekBufferNegative2Ref, 0, pad_data, count);
      } else {
        uint8_t *val = (uint8_t *)ScePspemuConvertAddress(0xABCD00A7, KERMIT_OUTPUT_MODE, 1);
        *val = 0;
        ScePspemuWritebackCache(val, 1);
      }
    }
  }

  return res;
}

char *ScePspemuGetTitleidPatched() {
  SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_INPUT_MODE, ADRENALINE_SIZE);
  return adrenaline->titleid;
}

int ScePspemuConvertAddressPatched(uint32_t addr, int mode, uint32_t cache_size) {
  if (addr >= 0x09FE0000 && addr < 0x09FE01B0) {
    addr = 0x0BCD0000 | (addr & 0xFFFF);
  }

  return TAI_CONTINUE(int, ScePspemuConvertAddressRef, addr, mode, cache_size);
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
  char *p = strrchr(file, '/');
  if (p) {
    static char new_file[256];

    SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_INPUT_MODE, ADRENALINE_SIZE);

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
    } else if (strcmp(p+1, "PARAM.SFO") == 0 ||
               strcmp(p+1, "SCEVMC0.VMP") == 0 ||
               strcmp(p+1, "SCEVMC1.VMP") == 0) {
      snprintf(new_file, sizeof(new_file), "%s/PSP/SAVEDATA/%s/%s", getPspemuMemoryStickLocation(), adrenaline->titleid, p+1);
      file = new_file;
    }
  }

  return TAI_CONTINUE(SceUID, sceIoOpenRef, file, flags, mode);
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat) {
  char *p = strrchr(file, '/');
  if (p) {
    static char new_file[256];

    SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_INPUT_MODE, ADRENALINE_SIZE);

    if (strcmp(p+1, "PARAM.SFO") == 0 ||
        strcmp(p+1, "SCEVMC0.VMP") == 0 ||
        strcmp(p+1, "SCEVMC1.VMP") == 0) {
      snprintf(new_file, sizeof(new_file), "%s/PSP/SAVEDATA/%s/%s", getPspemuMemoryStickLocation(), adrenaline->titleid, p+1);
      file = new_file;
    }
  }

  return TAI_CONTINUE(int, sceIoGetstatRef, file, stat);
}

extern void *pops_data;

int sceDisplaySetFrameBufForCompatPatched(int a1, int a2, int a3, int a4, int a5, SceDisplayFrameBuf *pParam) {
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