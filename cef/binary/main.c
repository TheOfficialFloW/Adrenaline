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

#include <pspsdk.h>
#include <pspkernel.h>

#include "../../adrenaline_compat.h"

volatile SceAdrenaline *adrenaline = (SceAdrenaline *)ADRENALINE_ADDRESS;

#define REG32(ADDR) (*(volatile u32 *)(ADDR))

void ClearCaches() {
	asm("\
	.word 0x40088000; .word 0x24091000; .word 0x7D081240;\
	.word 0x01094804; .word 0x4080E000; .word 0x4080E800;\
	.word 0x00004021; .word 0xBD010000; .word 0xBD030000;\
	.word 0x25080040; .word 0x1509FFFC; .word 0x00000000;\
	"::);
	
	asm("\
	.word 0x40088000; .word 0x24090800; .word 0x7D081180;\
	.word 0x01094804; .word 0x00004021; .word 0xBD140000;\
	.word 0xBD140000; .word 0x25080040; .word 0x1509FFFC;\
	.word 0x00000000; .word 0x0000000F; .word 0x00000000;\
	"::);
}

inline static void pspSync() {
	asm volatile ("sync\n");
}

inline static void sceKermitSetRegister(u32 reg, u32 val) {
	u32 old_val = REG32(reg);
	REG32(reg) = old_val & ~val;
	REG32(reg) = old_val | val;
}

__attribute__((noinline)) void sceKermitWait() {
	pspSync();

	REG32(0xBD000004) = 0xF;

	volatile u32 val;
	while ((val = REG32(0xBD000004)) != 0);
	while ((val = REG32(0xBD000000)) != 0xF);
}

void sceKermitSendRequest(u32 mode, u32 cmd) {
	// Kermit request
	SceKermitRequest *request = (SceKermitRequest *)0xABCC0000;
	request->cmd = cmd;
	request->sema_id = 0x12345678;
	request->response = (uint64_t *)request;

	// Kermit command
	SceKermitCommand *command = (SceKermitCommand *)0xBFC00800;
	command->cmd = (mode << 16) | cmd;
	command->request = request;

	// Wait
	sceKermitWait();

	// Set register
	sceKermitSetRegister(0xBC300050, (1 << (4 + 0)));
}

u64 _main(u32 sp, u32 ra) {
	adrenaline->sp = sp;
	adrenaline->ra = ra;

	if (adrenaline->savestate_mode == SAVESTATE_MODE_SAVE) {
		sceKermitSendRequest(KERMIT_MODE_EXTRA_2, ADRENALINE_VITA_CMD_SAVESTATE);

		while (adrenaline->vita_response != ADRENALINE_VITA_RESPONSE_SAVED);
		adrenaline->vita_response = ADRENALINE_VITA_RESPONSE_NONE;
	} else if (adrenaline->savestate_mode == SAVESTATE_MODE_LOAD) {
		sceKermitSendRequest(KERMIT_MODE_EXTRA_2, ADRENALINE_VITA_CMD_LOADSTATE);

		while (adrenaline->vita_response != ADRENALINE_VITA_RESPONSE_LOADED);
		adrenaline->vita_response = ADRENALINE_VITA_RESPONSE_NONE;
	}

	ClearCaches();

	return (((u64)adrenaline->sp << 32) | (u64)adrenaline->ra);
}