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

#ifndef __POPS_H__
#define __POPS_H__

#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

int ScePspemuInitAudioOutPatched();
int sceAudioOutOpenPortPatched(int type, int len, int freq, int mode);
int sceAudioOutOutputPatched(int port, const void *buf);
int ScePspemuDecodePopsAudioPatched(int a1, int a2, int a3, int a4);
int sceCtrlPeekBufferNegative2Patched(int port, SceCtrlData *pad_data, int count);
char *ScePspemuGetTitleidPatched();
int ScePspemuConvertAddressPatched(uint32_t addr, int mode, uint32_t cache_size);
SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode);
int sceIoGetstatPatched(const char *file, SceIoStat *stat);
int sceDisplaySetFrameBufForCompatPatched(int a1, int a2, int a3, int a4, int a5, SceDisplayFrameBuf *pParam);

#endif
