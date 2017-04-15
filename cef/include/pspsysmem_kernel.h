/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspsysmem_kernel.h - Interface to the system memory manager (kernel).
 *
 * Copyright (c) 2005 James F.
 *
 * $Id: pspsysmem.h 1095 2005-09-27 21:02:16Z jim $
 */

/* Note: Some of the structures, types, and definitions in this file were
   extrapolated from symbolic debugging information found in the Japanese
   version of Puzzle Bobble. */

#ifndef PSPSYSMEMKERNEL_H
#define PSPSYSMEMKERNEL_H

#include <pspkerneltypes.h>
#include <pspsysmem.h>

/** @defgroup SysMemKern System Memory Manager Kernel
  * This module contains routines to manage heaps of memory.
  */

/** @addtogroup SysMemKern System Memory Manager Kernel */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _PspSysmemPartitionInfo
{
	SceSize size;
	unsigned int startaddr;
	unsigned int memsize;
	unsigned int attr;
} PspSysmemPartitionInfo;

enum PspModel
{
	PSP_MODEL_STANDARD = 0,
	PSP_MODE_SLIM_AND_LITE = 1
};

/**
 * Query the parition information
 *
 * @param pid  - The partition id
 * @param info - Pointer to the ::PspSysmemPartitionInfo structure
 *
 * @returns 0 on success.
 */
int sceKernelQueryMemoryPartitionInfo(int pid, PspSysmemPartitionInfo *info);

/**
 * Get the total amount of free memory.
 *
 * @param pid - The partition id
 *
 * @returns The total amount of free memory, in bytes.
 */
SceSize sceKernelPartitionTotalFreeMemSize(int pid);

/**
 * Get the size of the largest free memory block.
 *
 * @param pid - The partition id
 *
 * @returns The size of the largest free memory block, in bytes.
 */
SceSize sceKernelPartitionMaxFreeMemSize(int pid);

/**
 * Get the kernel to dump the internal memory table to Kprintf
 */
void sceKernelSysMemDump(void);

/**
 * Dump the list of memory blocks
 */
void sceKernelSysMemDumpBlock(void);

/**
 * Dump the tail blocks
 */
void sceKernelSysMemDumpTail(void);

/**
 * Set the protection of a block of ddr memory
 *
 * @param addr - Address to set protection on
 * @param size - Size of block
 * @param prot - Protection bitmask
 *
 * @return < 0 on error
 */
int sceKernelSetDdrMemoryProtection(void *addr, int size, int prot);

/**
 * Create a heap.
 *
 * @param partitionid - The UID of the partition where allocate the heap.
 * @param size - The size in bytes of the heap.
 * @param unk - Unknown, probably some flag or type, pass 1.
 * @param name - Name assigned to the new heap.
 *
 * @returns The UID of the new heap, or if less than 0 an error. 
*/
SceUID sceKernelCreateHeap(SceUID partitionid, SceSize size, int unk, const char *name);

/**
 * Allocate a memory block from a heap.
 *
 * @param heapid - The UID of the heap to allocate from.
 * @param size - The number of bytes to allocate.
 *
 * @returns The address of the allocated memory block, or NULL on error.
*/
void *sceKernelAllocHeapMemory(SceUID heapid, SceSize size);

/**
 * Free a memory block allocated from a heap.
 *
 * @param heapid - The UID of the heap where block belongs.
 * @param block - The block of memory to free from the heap.
 *
 * @returns 0 on success, < 0 on error.
 */
int sceKernelFreeHeapMemory(SceUID heapid, void *block);

/**
 * Delete a heap.
 *
 * @param heapid - The UID of the heap to delete.
 *
 * @returns 0 on success, < 0 on error.
*/
int sceKernelDeleteHeap(SceUID heapid);

/**
 * Get the amount of free size of a heap, in bytes.
 *
 * @param heapid - The UID of the heap
 *
 * @returns the free size of the heap, in bytes. < 0 on error.
*/
SceSize sceKernelHeapTotalFreeSize(SceUID heapid);

/** Structure of a UID control block */
struct _uidControlBlock {
    struct _uidControlBlock *parent;
    struct _uidControlBlock *nextChild;
    struct _uidControlBlock *type;   //(0x8)
    u32 UID;					//(0xC)
    char *name;					//(0x10)
	unsigned char unk;
	unsigned char size;			// Size in words
    short attribute;
    struct _uidControlBlock *nextEntry;
} __attribute__((packed));
typedef struct _uidControlBlock uidControlBlock;

/**
 * Get a UID control block
 *
 * @param uid - The UID to find
 * @param block - Pointer to hold the pointer to the block
 *
 * @return 0 on success
 */
int sceKernelGetUIDcontrolBlock(SceUID uid, uidControlBlock** block);

/**
 * Get a UID control block on a particular type
 *
 * @param uid - The UID to find
 * @param type - Pointer to the type UID block
 * @param block - Pointer to hold the pointer to the block
 *
 * @return 0 on success
 */
int sceKernelGetUIDcontrolBlockWithType(SceUID uid, uidControlBlock* type, uidControlBlock** block);

/**
 * Get the root of the UID tree (1.5+ only)
 *
 * @return Pointer to the UID tree root
 */
uidControlBlock* SysMemForKernel_536AD5E1(void);

/**
 * Delete a UID
 *
 * @param uid - The UID to delete
 *
 * @return 0 on success
 */
int sceKernelDeleteUID(SceUID uid);

/**
 * Gets the model of the PSP.
 *
 * @returns one of PspModel values.
*/
int sceKernelGetModel(void);

#ifdef __cplusplus
}
#endif

/*@}*/

#endif /* PSPSYSMEMKERNEL_H */
