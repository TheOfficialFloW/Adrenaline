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

#ifndef __UTILS_H__
#define __UTILS_H__

extern uint32_t old_buttons, current_buttons, pressed_buttons, hold_buttons, hold2_buttons, released_buttons;
extern int SCE_CTRL_ENTER, SCE_CTRL_CANCEL;

void _init_vita_newlib(void);

int debugPrintf(char *text, ...);
int ReadFile(char *file, void *buf, int size);
int WriteFile(char *file, void *buf, int size);

void readPad();
int doubleClick(uint32_t buttons, uint64_t max_time);

void getSizeString(char string[16], uint64_t size);
void getTimeString(char string[16], int time_format, SceDateTime *time);
void getDateString(char string[24], int date_format, SceDateTime *time);

void SetPspemuFrameBuffer(void *base);

char *getPspemuMemoryStickLocation();

uint32_t encode_movw(uint8_t rd, uint16_t imm16);
uint32_t encode_movt(uint8_t rd, uint16_t imm16);
uint32_t encode_bl(uint32_t patch_offset, uint32_t target_offset);

#endif
