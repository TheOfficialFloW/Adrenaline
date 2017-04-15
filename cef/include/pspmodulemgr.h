/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspmodulemgr.h - Prototypes to manage modules.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: pspmodulemgr.h 1148 2005-10-12 19:08:27Z tyranid $
 */

/* Note: Some of the structures, types, and definitions in this file were
   extrapolated from symbolic debugging information found in the Japanese
   version of Puzzle Bobble. */

#ifndef __MODLOAD_H__
#define __MODLOAD_H__

#include <pspkerneltypes.h>

/** @defgroup ModuleMgr Module Manager Library
  * This module contains the imports for the kernel's module management routines.
  */

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup ModuleMgr Module Manager Library */
/*@{*/

#define PSP_MEMORY_PARTITION_KERNEL 1
#define PSP_MEMORY_PARTITION_USER 2

typedef struct SceKernelLMOption {
	SceSize 		size;
	SceUID 			mpidtext;
	SceUID 			mpiddata;
	unsigned int 	flags;
	char 			position;
	char 			access;
	char 			creserved[2];
} SceKernelLMOption;

typedef struct SceKernelSMOption {
	SceSize 		size;
	SceUID 			mpidstack;
	SceSize 		stacksize;
	int 			priority;
	unsigned int 	attribute;
} SceKernelSMOption;


/**
  * Load a module.
  * @note This function restricts where it can load from (such as from flash0) 
  * unless you call it in kernel mode. It also must be called from a thread.
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID sceKernelLoadModule(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a module from MS.
  * @note This function restricts what it can load, e.g. it wont load plain executables.
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, set to 0.
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID sceKernelLoadModuleMs(const char *path, int flags, SceKernelLMOption *option);

/**
 * Load a module from the given file UID.
 *
 * @param fid - The module's file UID.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 */
SceUID sceKernelLoadModuleByID(SceUID fid, int flags, SceKernelLMOption *option);

/**
 * Load a module from a buffer using the USB/WLAN API.
 *
 * Can only be called from kernel mode, or from a thread that has attributes of 0xa0000000.
 *
 * @param bufsize - Size (in bytes) of the buffer pointed to by buf.
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 */
SceUID sceKernelLoadModuleBufferUsbWlan(SceSize bufsize, void *buf, int flags, SceKernelLMOption *option);

/**
  * Start a loaded module.
  *
  * @param modid - The ID of the module returned from LoadModule.
  * @param argsize - Length of the args.
  * @param argp - A pointer to the arguments to the module.
  * @param status - Returns the status of the start.
  * @param option - Pointer to an optional ::SceKernelSMOption structure.
  *
  * @return ??? on success, otherwise one of ::PspKernelErrorCodes.
  */
int sceKernelStartModule(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

/**
 * Stop a running module.
 *
 * @param modid - The UID of the module to stop.
 * @param argsize - The length of the arguments pointed to by argp.
 * @param argp - Pointer to arguments to pass to the module's module_stop() routine.
 * @param status - Return value of the module's module_stop() routine.
 * @param option - Pointer to an optional ::SceKernelSMOption structure.
 *
 * @returns ??? on success, otherwise one of ::PspKernelErrorCodes.
 */
int sceKernelStopModule(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

/**
 * Unload a stopped module.
 *
 * @param modid - The UID of the module to unload.
 *
 * @returns ??? on success, otherwise one of ::PspKernelErrorCodes.
 */
int sceKernelUnloadModule(SceUID modid);

/**
 * Stop and unload the current module.
 *
 * @param unknown - Unknown (I've seen 1 passed).
 * @param argsize - Size (in bytes) of the arguments that will be passed to module_stop().
 * @param argp - Pointer to arguments that will be passed to module_stop().
 *
 * @return ??? on success, otherwise one of ::PspKernelErrorCodes.
 */
int sceKernelSelfStopUnloadModule(int unknown, SceSize argsize, void *argp);

/**
 * Stop and unload the current module.
 *
 * @param argsize - Size (in bytes) of the arguments that will be passed to module_stop().
 * @param argp - Poitner to arguments that will be passed to module_stop().
 * @param status - Return value from module_stop().
 * @param option - Pointer to an optional ::SceKernelSMOption structure.
 *
 * @returns ??? on success, otherwise one of ::PspKernelErrorCodes.
 */
int sceKernelStopUnloadSelfModule(SceSize argsize, void *argp, int *status, SceKernelSMOption *option);


typedef struct SceKernelModuleInfo {
	SceSize 		size;
	char 			nsegment;
	char 			reserved[3];
	int 			segmentaddr[4];
	int 			segmentsize[4];
	unsigned int 	entry_addr;
	unsigned int 	gp_value;
	unsigned int 	text_addr;
	unsigned int 	text_size;
	unsigned int 	data_size;
	unsigned int 	bss_size;
	/* The following is only available in the v1.5 firmware and above,
	   but as sceKernelQueryModuleInfo is broken in v1.0 is doesn't matter ;) */
	unsigned short  attribute;
	unsigned char   version[2];
	char            name[28];
} SceKernelModuleInfo;

/**
  * Query the information about a loaded module from its UID.
  * @note This fails on v1.0 firmware (and even it worked has a limited structure)
  * so if you want to be compatible with both 1.5 and 1.0 (and you are running in 
  * kernel mode) then call this function first then ::pspSdkQueryModuleInfoV1 
  * if it fails, or make separate v1 and v1.5+ builds.
  *
  * @param modid - The UID of the loaded module.
  * @param info - Pointer to a ::SceKernelModuleInfo structure.
  * 
  * @return 0 on success, otherwise one of ::PspKernelErrorCodes.
  */
int sceKernelQueryModuleInfo(SceUID modid, SceKernelModuleInfo *info);

/**
  * Get a list of module IDs. NOTE: This is only available on 1.5 firmware
  * and above. For V1 use ::pspSdkGetModuleIdList.
  *
  * @param readbuf - Buffer to store the module list.
  * @param readbufsize - Number of elements in the readbuffer.
  * @param idcount - Returns the number of module ids
  *
  * @return >= 0 on success
  */
int sceKernelGetModuleIdList(SceUID *readbuf, int readbufsize, int *idcount);

/**
  * Load a module protected by DRM...
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID ModuleMgrForUser_FEF27DC1(const char *path, int flags, SceKernelLMOption *option);

#define sceKernelLoadModuleDNAS	ModuleMgrForUser_FEF27DC1

/**
 * Stop and unload the current module with the specified exit status code
 *
 * @param exitcode - The exitcode for the module
 * @param argsize - Size (in bytes) of the arguments that will be passed to module_stop().
 * @param argp - Poitner to arguments that will be passed to module_stop().
 * @param status - Return value from module_stop().
 * @param option - Pointer to an optional ::SceKernelSMOption structure.
 *
 * @returns ??? on success, otherwise one of ::PspKernelErrorCodes.
 */
int	ModuleMgrForUser_8F2DF740(int exitcode, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

#define sceKernelStopUnloadSelfModuleWithStatus ModuleMgrForUser_8F2DF740

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
