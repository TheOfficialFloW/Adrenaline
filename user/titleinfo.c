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

int ScePspemuInitTitleSpecificInfoPatched(const char *titleid, SceUID uid) {
  int res = 0;
  uint32_t *info;

  if (module_nid == 0x2714F07D) { // 3.60 retail
    // Make __sce_menuinfo path
    snprintf((char *)(data_addr + 0x11C7D0C), 0x80, "ms0:PSP/GAME/%s/__sce_menuinfo", titleid);

    info = (uint32_t *)(data_addr + 0x1156450);
  } else if (module_nid == 0x3F75D4D3) { // 3.65/3.67 retail
    // Make __sce_menuinfo path
    snprintf((char *)(data_addr + 0x11C7E0C), 0x80, "ms0:PSP/GAME/%s/__sce_menuinfo", titleid);

    info = (uint32_t *)(data_addr + 0x1156550);
  }

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