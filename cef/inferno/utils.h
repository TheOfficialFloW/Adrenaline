/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef UTILS_H
#define UTILS_H

typedef unsigned int uint;

#define FW_661 0x06060110
#define FW_660 0x06060010
#define FW_639 0x06030910
#define FW_635 0x06030510
#define FW_620 0x06020010
#define FW_610 0x06010010
#define FW_600 0x06000010
#define FW_551 0x05050110
#define FW_550 0x05050010
#define FW_503 0x05000310
#define FW_501 0x05000110
#define FW_500 0x05000010
#define FW_401 0x04000110
#define FW_396 0x03090610
#define FW_395 0x03090510
#define FW_393 0x03090310
#define FW_390 0x03090010
#define FW_380 0x03080010
#define FW_372 0x03070210
#define FW_371 0x03070110
#define FW_352 0x03050210
#define FW_351 0x03050110
#define FW_350 0x03050010
#define FW_340 0x03040010
#define FW_330 0x03030010
#define FW_311 0x03010110
#define FW_310 0x03010010
#define FW_303 0x03000310
#define FW_302 0x03000210

enum {
	PSP_1000 = 0,
	PSP_2000 = 1,
	PSP_3000 = 2,
	PSP_4000 = 3,
	PSP_GO   = 4,
	PSP_7000 = 6,
	PSP_9000 = 8,
	PSP_11000 = 10,
};

#define SYSMEM_TEXT_ADDR 0x88000000

#define MAKE_JUMP(f) (0x08000000 | (((u32)(f) >> 2) & 0x03ffffff))
#define MAKE_CALL(f) (0x0C000000 | (((u32)(f) >> 2) & 0x03ffffff))
#define MAKE_SYSCALL(n) ((n<<6)|12)
#define NOP 0

#define REDIRECT_FUNCTION(new_func, original) \
	do { \
		_sw(MAKE_JUMP(new_func), (original)); \
		_sw(NOP, (original)+4); \
	} while ( 0 );

#define MAKE_DUMMY_FUNCTION_RETURN_0(a) do {\
	_sw(0x03E00008, a);\
	_sw(0x00001021, a + 4);\
} while (0)

#define MAKE_DUMMY_FUNCTION_RETURN_1(a) do {\
	_sw(0x03E00008, a);\
	_sw(0x24020001, a + 4);\
} while (0)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define NELEMS(a) (sizeof(a) / sizeof(a[0]))

#define PTR_ALIGN_64(p) ((void*)((((u32)p)+64-1)&(~(64-1))))

int sceKernelGetModel(void);

void sync_cache(void);

/**
 * @return interrupted status
 * 0 - we are definitely in interrupt disabled status. And the interrupt status won't change as long as our code didn't
 * 1 - we are in interrupt enable status. but the interrupt status would change in later code
 */
int is_cpu_intr_enable(void);

#ifdef DEBUG
void hexdump(void *addr, int size);
void fill_vram(u32 color);
#else
static inline void hexdump(void *addr, int size)
{
}
static inline void fill_vram(u32 color)
{
}
#endif

int get_device_name(char *device, int size, const char* path);

SceUID get_thread_id(const char *name);

/** Check if user syscall didn't pass kernel memory, as OFW did */
int check_memory(const void *addr, int size);

#endif
