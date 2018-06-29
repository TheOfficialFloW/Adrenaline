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

#ifndef __STATES_H__
#define __STATES_H__

#include <vita2d.h>

#include "../adrenaline_compat.h"

#define MAX_STATES 32
#define MAX_POSITION 3

#define RIGHT_ARROW "\xE2\x96\xB6"
#define LEFT_ARROW "\xE2\x97\x80"

#define SCREENSHOT_WIDTH 240
#define SCREENSHOT_HEIGHT 136
#define SCREENSHOT_SIZE (SCREENSHOT_WIDTH * SCREENSHOT_HEIGHT * 4)

#define ADRENALINE_SAVESTATE_MAGIC 0x54535653
#define ADRENALINE_SAVESTATE_VERSION ADRENALINE_VERSION

typedef struct {
  uint32_t magic;          // 0x00
  uint32_t version;        // 0x04
  char title[0x80];        // 0x08
  uint32_t sp;          // 0x88
  uint32_t ra;          // 0x8C
  uint32_t screenshot_offset;    // 0x90
  uint32_t screenshot_size;    // 0x94
  uint32_t descriptors_offset;  // 0x98
  uint32_t descriptors_size;    // 0x9C
  uint32_t ram_part1_offset;    // 0xA0
  uint32_t ram_part1_size;    // 0xA4
  uint32_t ram_part2_offset;    // 0xA8
  uint32_t ram_part2_size;    // 0xAC
  char reserved[0x50];      // 0xB0
} AdrenalineStateHeader;      // 0x100

typedef struct {
  int num;
  vita2d_texture *tex;
  char title[128];
  SceOff size;
  SceDateTime time;
} AdrenalineStateEntry;

extern int open_options;

void makeSaveStatePath(char *path, int num);

int initStates();
void finishStates();
void drawStates();
void ctrlStates();

#endif