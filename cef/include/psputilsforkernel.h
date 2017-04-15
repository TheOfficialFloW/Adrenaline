/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * psputilsforkernel.h - Include file for UtilsForKernel
 *
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 * Copyright (c) 2005 adresd
 *
 * $Id: psputilsforkernel.h 1636 2005-12-30 02:04:09Z tyranid $
 */

#ifndef __PSPUTILSFORKERNEL_H__
#define __PSPUTILSFORKERNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Decompress gzip'd data (requires kernel mode)
 *
 * @param dest - pointer to destination buffer
 * @param destSize - size of destination buffer
 * @param src - pointer to source (compressed) data
 * @param unknown - unknown, pass NULL
 * @return size decompressed on success, < 0 on error
 */
int sceKernelGzipDecompress(u8 *dest, u32 destSize, const u8 *src, void *unknown);

/**
 * Decompress RLZ data (requires kernel mode)
 *
 * @param dest - pointer to destination buffer
 * @param destSize - size of destination buffer
 * @param src - pointer to source (compressed) data
 * @param unknown - unknown, pass NULL
 * @return size decompressed on success, < 0 on error
 */
int sceKernelLzrcDecode(u8 *dest, u32 destSize, const u8 *src, void *unknown);

/**
 * Decompress inflate data (requires kernel mode)
 *
 * @param dest - pointer to destination buffer
 * @param destSize - size of destination buffer
 * @param src - pointer to source (compressed) data
 * @param unknown - unknown, pass NULL
 * @return size decompressed on success, < 0 on error
 */
int sceKernelDeflateDecompress(u8 *dest, u32 destSize, const u8 *src, void *unknown);

/**
 * Invalidate the entire data cache
 */ 
void sceKernelDcacheInvalidateAll(void);

/**
 * Check whether the specified address is in the data cache
 * @param addr - The address to check
 *
 * @return 0 = not cached, 1 = cache
 */
int  sceKernelDcacheProbe(void *addr);

/**
 * Invalidate the entire instruction cache
 */
void sceKernelIcacheInvalidateAll(void);

/**
 * Invalidate a instruction cache range.
 * @param addr - The start address of the range.
 * @param size - The size in bytes
 */
void sceKernelIcacheInvalidateRange(const void *addr, unsigned int size);

/**
 * Check whether the specified address is in the instruction cache
 * @param addr - The address to check
 *
 * @return 0 = not cached, 1 = cache
 */
int  sceKernelIcacheProbe(const void *addr);

#ifdef __cplusplus
}
#endif

#endif
