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
#include <psp2/apputil.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/rtc.h>
#include <psp2/system_param.h>
#include <psp2/sysmodule.h>
#include <psp2/power.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <vita2d.h>

#include "main.h"
#include "menu.h"
#include "states.h"
#include "usb.h"
#include "utils.h"

AdrenalineStateEntry *states;

static int option_sel =  0;
static int rel_pos = 0, base_pos = 0;

int open_options = 0;

static char *option_entries_new[] = {
  "Save State",
  "Cancel",
};

static char *option_entries_exist[] = {
  "Load State",
  "Overwrite State",
  "Delete State",
  "Cancel",
};

#define N_OPTION_ENTRIES_NEW (sizeof(option_entries_new) / sizeof(char **))
#define N_OPTION_ENTRIES_EXIST (sizeof(option_entries_exist) / sizeof(char **))

#define OPTION_MODE_NEW 0
#define OPTION_MODE_EXIST 1

static char **option_entries;
static int n_options = 0;
static int option_mode = 0;

void makeSaveStatePath(char *path, int num) {
  sprintf(path, "%s/PSP/SAVESTATE/STATE%02d.BIN", getPspemuMemoryStickLocation(), num);
}

extern void *pops_data;

static uint32_t convert565To8888(uint16_t color) {
  uint8_t red_value = (color & 0xF800) >> 11;
  uint8_t green_value = (color & 0x7E0) >> 5;
  uint8_t blue_value = (color & 0x1F);

  uint8_t alpha = 0xFF;
  uint8_t red = red_value << 3;
  uint8_t green = green_value << 2;
  uint8_t blue = blue_value << 3;

  return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

int saveFrameBuffer(SceUID fd) {
  sceCompatLCDCSync();

  uint32_t *buf = malloc(SCREENSHOT_SIZE);
  if (!buf)
    return -1;

  int i = 0;

  int draw_native = *(uint32_t *)CONVERT_ADDRESS(DRAW_NATIVE);
  SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_INPUT_MODE, ADRENALINE_SIZE);

  if (draw_native) {
    int y;
    for (y = 0; y < SCREEN_HEIGHT; y += 4) {
      int x;
      for (x = 0; x < SCREEN_WIDTH; x += 4) {
        uint16_t color = ((uint16_t *)CONVERT_ADDRESS(NATIVE_FRAMEBUFFER))[x + SCREEN_WIDTH * y];
        buf[i++] = convert565To8888(color);
      }
    }
  } else if (adrenaline->pops_mode) {
    int y;
    for (y = 0; y < SCREEN_HEIGHT; y += 4) {
      int x;
      for (x = 0; x < SCREEN_WIDTH; x += 4) {
        uint32_t color = ((uint32_t *)pops_data)[x + SCREEN_LINE * y];
        buf[i++] = 0xFF000000 | color; // Add alpha channel
      }
    }
  } else {
    int y;
    for (y = 0; y < PSP_SCREEN_HEIGHT; y += 2) {
      int x;
      for (x = 0; x < PSP_SCREEN_WIDTH; x += 2) {
        uint32_t color = ((uint32_t *)SCE_PSPEMU_FRAMEBUFFER)[x + PSP_SCREEN_LINE * y];
        buf[i++] = color;
      }
    }
  }

  sceIoWrite(fd, buf, SCREENSHOT_SIZE);

  free(buf);

  return 0;
}

void saveState(int num) {
  char path[128];

  // Make dir
  sprintf(path, "%s/PSP/SAVESTATE", getPspemuMemoryStickLocation());
  sceIoMkdir(path, 0777);

  makeSaveStatePath(path, num);

  SceUID fd = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
  if (fd >= 0) {
    AdrenalineStateHeader header;
    memset(&header, 0, sizeof(AdrenalineStateHeader));
    sceIoWrite(fd, &header, sizeof(AdrenalineStateHeader));
    saveFrameBuffer(fd);
    sceIoClose(fd);
  }

  SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_OUTPUT_MODE, ADRENALINE_SIZE);
  adrenaline->num = num;
  ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);

  stopUsb(usbdevice_modid);
  SendAdrenalineRequest(ADRENALINE_PSP_CMD_SAVESTATE);
}

void loadState(int num) {
  SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_OUTPUT_MODE, ADRENALINE_SIZE);
  adrenaline->num = num;
  ScePspemuWritebackCache(adrenaline, ADRENALINE_SIZE);

  stopUsb(usbdevice_modid);
  SendAdrenalineRequest(ADRENALINE_PSP_CMD_LOADSTATE);
}

void deleteState(int num) {
  char path[128];
  makeSaveStatePath(path, num);

  // Remove file
  sceIoRemove(path);

  // Remove from list
  states[num].num = -1;

  if (states[num].tex)
    vita2d_free_texture(states[num].tex);
}

void finishStates() {
  if (states) {
    int i;
    for (i = 0; i < MAX_STATES; i++) {
      if (states[i].num != -1) {
        if (states[i].tex)
          vita2d_free_texture(states[i].tex);
      }
    }

    free(states);
    states = NULL;
  }
}

int initStates() {
  option_sel = 0;
  open_options = 0;

  states = malloc(MAX_STATES * sizeof(AdrenalineStateEntry));
  if (!states)
    return -1;

  // Clear
  int i;
  for (i = 0; i < MAX_STATES; i++) {
    states[i].num = -1;
  }

  // Load savestates
  char folder[128];
  sprintf(folder, "%s/PSP/SAVESTATE", getPspemuMemoryStickLocation());

  SceUID dfd = sceIoDopen(folder);
  if (dfd >= 0) {
    int res = 0;

    do {
      SceIoDirent dir;
      memset(&dir, 0, sizeof(SceIoDirent));

      res = sceIoDread(dfd, &dir);
      if (res > 0) {
        if (strncmp(dir.d_name, "STATE", 5) == 0) {
          char num_str[3];
          num_str[0] = dir.d_name[5];
          num_str[1] = dir.d_name[6];
          num_str[2] = '\0';

          if (num_str[0] >= '0' && num_str[0] <= '9' &&
            num_str[1] >= '0' && num_str[1] <= '9') {
            int num = strtol(num_str, 0, 10);
            if (num >= MAX_STATES)
              continue;

            char path[128];
            makeSaveStatePath(path, num);

            SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
            if (fd < 0)
              continue;

            // Read header
            AdrenalineStateHeader header;
            sceIoRead(fd, &header, sizeof(AdrenalineStateHeader));

            // Check header info
            if (header.magic == ADRENALINE_SAVESTATE_MAGIC) { // header.version == ADRENALINE_SAVESTATE_VERSION
              // Set number
              states[num].num = num;

              // Get title
              strcpy(states[num].title, header.title);

              // Read framebuffer
              vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW);
              states[num].tex = vita2d_create_empty_texture(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT);
              if (states[num].tex) {
                sceIoLseek(fd, header.screenshot_offset, SCE_SEEK_SET);
                sceIoRead(fd, vita2d_texture_get_datap(states[num].tex), header.screenshot_size);
              }

              // Set file stat
              SceIoStat stat;
              memset(&stat, 0, sizeof(SceIoStat));
              if (sceIoGetstat(path, &stat) >= 0) {
                states[num].size = stat.st_size;
                states[num].time = stat.st_mtime;
              }
            }

            sceIoClose(fd);
          }
        }
      }
    } while (res > 0);

    sceIoDclose(dfd);
  }

  return 0;
}

void drawStates() {
  if (!states)
    return;

  float k = 92.0f / 136.0f;

  int i;
  for (i = 0; i < 3; i++) {
    if (i == rel_pos)
      vita2d_draw_rectangle(WINDOW_X, FONT_Y_LINE(2 + i * 5) + 3.0f - FONT_Y_SPACE/2.0f, WINDOW_WIDTH, FONT_Y_SPACE * 5, COLOR_ALPHA(0xFFFF1F7F, 0x8F));

    if (states[base_pos + i].num == -1) {
      vita2d_draw_rectangle(75.0f, FONT_Y_LINE(2 + i * 5) + 3.0f, 240.0f * k, 92.0f, COLOR_ALPHA(GRAY, 0x7F));
      pgf_draw_textf(250.0f, FONT_Y_LINE(2 + i * 5), WHITE, FONT_SIZE, "New State %d", base_pos + i);
    } else {
      // Screenshot
      if (states[base_pos + i].tex)
        vita2d_draw_texture_scale(states[base_pos + i].tex, 75.0f, FONT_Y_LINE(2 + i * 5) + 3.0f, k, k);

      // Title
      pgf_draw_text(250.0f, FONT_Y_LINE(2 + i * 5), WHITE, FONT_SIZE, states[base_pos + i].title);

      // Date & time
      char date_string[16];
      getDateString(date_string, date_format, &states[base_pos + i].time);

      char time_string[24];
      getTimeString(time_string, time_format, &states[base_pos + i].time);

      pgf_draw_textf(250.0f, FONT_Y_LINE(3 + i * 5), WHITE, FONT_SIZE, "%s %s", date_string, time_string);

      // Size
      char string[16];
      getSizeString(string, states[base_pos + i].size);
      pgf_draw_text(250.0f, FONT_Y_LINE(4 + i * 5), WHITE, FONT_SIZE, string);
    }
  }

  // Show options
  if (open_options) {
    int i;
    for (i = 0; i < n_options; i++) {
      uint32_t color = (i == option_sel) ? GREEN : WHITE;
      pgf_draw_text(ALIGN_RIGHT(885.0f, vita2d_pgf_text_width(font, FONT_SIZE, option_entries[i])), FONT_Y_LINE(2 + rel_pos * 5 + i), color, FONT_SIZE, option_entries[i]);
    }
  }
}

void ctrlStates() {
  if (!states)
    return;

  if (open_options) {
    if (released_pad[PAD_CANCEL]) {
      open_options = 0;
    }

    if (released_pad[PAD_ENTER]) {
      if (option_mode == OPTION_MODE_NEW) {
        switch (option_sel) {
          case 0:
          {
            ExitAdrenalineMenu();
            saveState(base_pos+rel_pos);
            break;
          }

          case 1:
          {
            open_options = 0;
            break;
          }
        }
      } else if (option_mode == OPTION_MODE_EXIST) {
        switch (option_sel) {
          case 0:
          {
            ExitAdrenalineMenu();
            loadState(base_pos+rel_pos);
            break;
          }

          case 1:
          {
            ExitAdrenalineMenu();
            saveState(base_pos+rel_pos);
            break;
          }

          case 2:
          {
            deleteState(base_pos+rel_pos);
            open_options = 0;
            break;
          }

          case 3:
          {
            open_options = 0;
            break;
          }
        }
      }
    }

    if (hold_pad[PAD_UP] || hold_pad[PAD_LEFT_ANALOG_UP]) {
      if (option_sel > 0)
        option_sel--;
    }

    if (hold_pad[PAD_DOWN] || hold_pad[PAD_LEFT_ANALOG_DOWN]) {
      if (option_sel < n_options-1)
        option_sel++;
    }
  } else {
    if (hold_pad[PAD_UP] || hold_pad[PAD_LEFT_ANALOG_UP]) {
      if (rel_pos > 0) {
        rel_pos--;
      } else if (base_pos > 0) {
        base_pos--;
      }
    }

    if (hold_pad[PAD_DOWN] || hold_pad[PAD_LEFT_ANALOG_DOWN]) {
      if (rel_pos < MAX_STATES-1) {
        if (rel_pos < MAX_POSITION-1) {
          rel_pos++;
        } else if (base_pos+rel_pos < MAX_STATES-1) {
          base_pos++;
        }
      }
    }

    if (released_pad[PAD_ENTER]) {
      open_options = 1;
      option_sel = 0;

      if (states[base_pos+rel_pos].num == -1) {
        option_entries = option_entries_new;
        n_options = N_OPTION_ENTRIES_NEW;
        option_mode = OPTION_MODE_NEW;
      } else {
        option_entries = option_entries_exist;
        n_options = N_OPTION_ENTRIES_EXIST;
        option_mode = OPTION_MODE_EXIST;
      }
    }
  }
}