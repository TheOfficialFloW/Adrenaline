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

#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/rtc.h>
#include <psp2/system_param.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

uint32_t old_buttons, current_buttons, pressed_buttons, hold_buttons, hold2_buttons, released_buttons;

int SCE_CTRL_ENTER = SCE_CTRL_CROSS, SCE_CTRL_CANCEL = SCE_CTRL_CIRCLE;

void _init_vita_heap(void);
void _init_vita_reent(void);
void _init_vita_malloc(void);
void _init_vita_io(void);

void _init_vita_newlib(void) {
	_init_vita_heap();
	_init_vita_reent();
	_init_vita_malloc();
	_init_vita_io();
}

int debugPrintf(char *text, ...) {
	va_list list;
	char string[512];

	va_start(list, text);
	vsprintf(string, text, list);
	va_end(list);

	SceUID fd = sceIoOpen("ux0:data/adrenaline_user_log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
	if (fd >= 0) {
		sceIoWrite(fd, string, strlen(string));
		sceIoClose(fd);
	}

	return 0;
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int read = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return read;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

void readPad() {
	static int hold_n = 0, hold2_n = 0;

	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);

	SceCtrlData home;
	kuCtrlPeekBufferPositive(0, &home, 1);

	pad.buttons &= ~SCE_CTRL_PSBUTTON;
	pad.buttons |= (home.buttons & SCE_CTRL_PSBUTTON);

	old_buttons = current_buttons;
	current_buttons = pad.buttons;
	pressed_buttons = current_buttons & ~old_buttons;
	released_buttons = ~current_buttons & old_buttons;

	hold_buttons = pressed_buttons;
	hold2_buttons = pressed_buttons;

	if (old_buttons & current_buttons) {
		if (hold_n >= 10) {
			hold_buttons = current_buttons;
			hold_n = 6;
		}

		if (hold2_n >= 10) {
			hold2_buttons = current_buttons;
			hold2_n = 10;
		}

		hold_n++;
		hold2_n++;
	} else {
		hold_n = 0;
		hold2_n = 0;
	}
}

void getSizeString(char string[16], uint64_t size) {
	double double_size = (double)size;

	int i = 0;
	static char *units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (double_size >= 1024.0) {
		double_size /= 1024.0;
		i++;
	}

	snprintf(string, 16, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
}

void convertUtcToLocalTime(SceDateTime *time_local, SceDateTime *time_utc) {
	SceRtcTick tick;
	sceRtcGetTick(time_utc, &tick);
	sceRtcConvertUtcToLocalTime(&tick, &tick);
	sceRtcSetTick(time_local, &tick);	
}

void getTimeString(char string[16], int time_format, SceDateTime *time) {
	SceDateTime time_local;
	convertUtcToLocalTime(&time_local, time);

	switch(time_format) {
		case SCE_SYSTEM_PARAM_TIME_FORMAT_12HR:
			snprintf(string, 16, "%02d:%02d %s", (time_local.hour > 12) ? (time_local.hour-12) : ((time_local.hour == 0) ? 12 : time_local.hour), time_local.minute, time_local.hour >= 12 ? "PM" : "AM");
			break;

		case SCE_SYSTEM_PARAM_TIME_FORMAT_24HR:
			snprintf(string, 16, "%02d:%02d", time_local.hour, time_local.minute);
			break;
	}
}

void getDateString(char string[24], int date_format, SceDateTime *time) {
	SceDateTime time_local;
	convertUtcToLocalTime(&time_local, time);

	switch (date_format) {
		case SCE_SYSTEM_PARAM_DATE_FORMAT_YYYYMMDD:
			snprintf(string, 24, "%04d/%02d/%02d", time_local.year, time_local.month, time_local.day);
			break;

		case SCE_SYSTEM_PARAM_DATE_FORMAT_DDMMYYYY:
			snprintf(string, 24, "%02d/%02d/%04d", time_local.day, time_local.month, time_local.year);
			break;

		case SCE_SYSTEM_PARAM_DATE_FORMAT_MMDDYYYY:
			snprintf(string, 24, "%02d/%02d/%04d", time_local.month, time_local.day, time_local.year);
			break;
	}
}

void SetPspemuFrameBuffer(void *base) {
	SceDisplayFrameBuf framebuf;
	memset(&framebuf, 0, sizeof(SceDisplayFrameBuf));
	framebuf.size        = sizeof(SceDisplayFrameBuf);
	framebuf.base        = base;
	framebuf.pitch       = PSP_SCREEN_LINE;
	framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
	framebuf.width       = PSP_SCREEN_WIDTH;
	framebuf.height      = PSP_SCREEN_HEIGHT;
	sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);
}

char *getPspemuMemoryStickLocation() {
	switch (config.ms_location) {
		case MEMORY_STICK_LOCATION_UR0:
			return "ur0:pspemu";
		case MEMORY_STICK_LOCATION_IMC0:
			return "imc0:pspemu";
		case MEMORY_STICK_LOCATION_UMA0:
			return "uma0:pspemu";
		default:
			return "ux0:pspemu";
	}
}

#define THUMB_SHUFFLE(x) ((((x) & 0xFFFF0000) >> 16) | (((x) & 0xFFFF) << 16))

uint32_t encode_movw(uint8_t rd, uint16_t imm16) {
	uint32_t imm4 = (imm16 >> 12) & 0xF;
	uint32_t i = (imm16 >> 11) & 0x1;
	uint32_t imm3 = (imm16 >> 8) & 0x7;
	uint32_t imm8 = imm16 & 0xFF;
	return THUMB_SHUFFLE(0xF2400000 | (rd << 8) | (i << 26) | (imm4 << 16) | (imm3 << 12) | imm8);
}

uint32_t encode_movt(uint8_t rd, uint16_t imm16) {
	uint32_t imm4 = (imm16 >> 12) & 0xF;
	uint32_t i = (imm16 >> 11) & 0x1;
	uint32_t imm3 = (imm16 >> 8) & 0x7;
	uint32_t imm8 = imm16 & 0xFF;
	return THUMB_SHUFFLE(0xF2C00000 | (rd << 8) | (i << 26) | (imm4 << 16) | (imm3 << 12) | imm8);
}

uint32_t encode_bl(uint32_t patch_offset, uint32_t target_offset) {
	uint32_t displacement = target_offset - (patch_offset & ~0x1) - 4;
	uint32_t signbit = (displacement >> 31) & 0x1;
	uint32_t i1 = (displacement >> 23) & 0x1;
	uint32_t i2 = (displacement >> 22) & 0x1;
	uint32_t imm10 = (displacement >> 12) & 0x03FF;
	uint32_t imm11 = (displacement >> 1) & 0x07FF;
	uint32_t j1 = i1 ^ (signbit ^ 1);
	uint32_t j2 = i2 ^ (signbit ^ 1);
	uint32_t value = (signbit << 26) | (j1 << 13) | (j2 << 11) | (imm10 << 16) | imm11;
	value |= 0xF000D000;  // BL
	return THUMB_SHUFFLE(value);
}
