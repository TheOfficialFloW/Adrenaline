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

#include <pspkernel.h>
#include <pspsdk.h>
#include <systemctrl_se.h>
#include "pspcrypt.h"

u32 sctrlKernelRand(void)
{
        u32 k1, result;
        u8 *alloc, *ptr;

        enum {
                KIRK_PRNG_CMD=0xE,
        };

        k1 = pspSdkSetK1(0);

        alloc = oe_malloc(20 + 4);

        if(alloc == NULL) {
                asm("break");
        }

        /* output ptr has to be 4 bytes aligned */
        ptr = (void*)(((u32)alloc & (~(4-1))) + 4);
        sceUtilsBufferCopyWithRange(ptr, 20, NULL, 0, KIRK_PRNG_CMD);
        result = *(u32*)ptr;
        oe_free(alloc);
        pspSdkSetK1(k1);

        return result;
}
