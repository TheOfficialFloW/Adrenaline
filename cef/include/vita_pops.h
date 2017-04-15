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

#ifndef __VITA_POPS_H__
#define __VITA_POPS_H__

#define POPS_REGISTER 0x49FE0000

#define POPS_MAX_DISPLAYS 2
#define POPS_MAX_FRAME_BUFFERS 3
#define POPS_MAX_CONTROLLERS 8

#define POPS_SCREEN_WIDTH 320
#define POPS_SCREEN_HEIGHT 240
#define POPS_SCREEN_LINE_SIZE 640

#define POPS_VRAM 0x490C0000
#define POPS_VRAM_SIZE 0x140000 // 640 * 512 * 4

enum PopsGpuFlags {
	POPS_GPU_HEIGHT_240 = 0x00,
	POPS_GPU_HEIGHT_480 = 0x04,

	POPS_GPU_WIDTH_256 = 0x00,
	POPS_GPU_WIDTH_320 = 0x01,
	POPS_GPU_WIDTH_512 = 0x02,
	POPS_GPU_WIDTH_640 = 0x03,
	POPS_GPU_WIDTH_368 = 0x40,

	POPS_GPU_PAL = 0x08,
	POPS_GPU_RGB24 = 0x10,
	POPS_GPU_INTERLACED = 0x20,
	POPS_GPU_REVERSE = 0x80,
};

typedef struct {
	short x; // 0x0
	short y; // 0x2
	short width; // 0x4
	short height; // 0x6
	char mode; // 0x8
	char current_frame_buffer; // 0x9
} PopsDisplay; // 0xA

typedef struct {
	u32 Buttons; // 0x0
	u8 Lx; // 0x4
	u8 Ly; // 0x5
	u8 Rx; // 0x6
	u8 Ry; // 0x7
	u32 Reserved[2]; // 0x8
} PopsController; // 0x10

typedef struct {
	u8 disable_spu; // 0x0
	u8 enable_screenshot; // 0x1
	u8 disable_gpu; // 0x2
	u8 disc_change; // 0x3
	u8 disc_number; // 0x4
	u8 disc_numbers; // 0x5
	u8 value_6; // 0x6 // region? 0, 1, 2
	u8 controller_port; // 0x7 // 0 or 1 on PS Vita, 2 on Vita TV
	u8 controller_mode; // 0x8
	u8 no_vblank_wait; // 0x9
	u8 disc_load_speed; // 0xA
} PopsInformation; // 0xB

typedef struct {
	PopsDisplay display[POPS_MAX_DISPLAYS]; // 0x0
	u8 update_frame_buffer; // 0x14
	u8 ctrl_modes[POPS_MAX_CONTROLLERS]; // 0x15
	u8 padding_0[3]; // 0x1D
	PopsController ctrl[POPS_MAX_CONTROLLERS]; // 0x20
	PopsInformation info; // 0xA0
	u8 use_memory_card_utility; // 0xAB
	u16 screenshot_signature[120]; // 0xAC
	u8 signature_value_0; // 0x19C
	u8 signature_value_1; // 0x19D
	u8 reserved[2]; // 0x19E
	u8 unk; // 0x1A0
	u8 padding_1[3]; // 0x1A1
	void *savedata[2]; // 0x1A4
	u32 error_code; // 0x1AC
} PopsRegister; // 0x1B0

#endif