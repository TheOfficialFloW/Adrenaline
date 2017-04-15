/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspmodulemgr_kernel.h - Prototypes to manage modules.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: pspmodulemgr.h 792 2005-07-27 09:03:36Z warren $
 */

#ifndef __MODMGRKERNEL_H__
#define __MODMGRKERNEL_H__

#include <pspkerneltypes.h>
#include <pspmodulemgr.h>
#include <pspinit.h>

/** @defgroup ModuleMgrKern Kernel Module Manager Library
  * This module contains the imports for the kernel's module management routines.
  */

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup ModuleMgrKern Kernel Module Manager Library */
/*@{*/

enum PSPLoadModuleApitype
{
	PSP_LOADMODULE_APITYPE_KERNEL = 0, /* ModuleMgrForKernel */
	PSP_LOADMODULE_APITYPE_USER = 0x10, /* ModuleMgrForUser */
	PSP_LOADMODULE_APITYPE_DNAS = 0x13,  
	PSP_LOADMODULE_APITYPE_VSH = 0x20, 
	PSP_LOADMODULE_APITYPE_DISC = 0x120,
	PSP_LOADMODULE_APITYPE_DISC_UPDATER = 0x121,
	PSP_LOADMODULE_APITYPE_MS1 = 0x140, 
	PSP_LOADMODULE_APITYPE_MS2 = 0x141, 
	PSP_LOADMODULE_APITYPE_MS3 = 0x142, 
	PSP_LOADMODULE_APITYPE_VSH_MAIN1 = 0x210, /* ExitGame */
	PSP_LOADMODULE_APITYPE_VSH_MAIN2 = 0x220, /* ExitVSH */
};

/**
  * Gets the current module list.
  * 
  * @param readbufsize - The size of the read buffer.
  * @param readbuf     - Pointer to a buffer to store the IDs
  *
  * @return < 0 on error.
  */
int sceKernelGetModuleList(int readbufsize, SceUID *readbuf);

/**
  * Get the number of loaded modules.
  *
  * @return The number of loaded modules.
  */
int sceKernelModuleCount(void);

/**
 * Load a module from a buffer
 *
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param bufsize - Size (in bytes) of the buffer pointed to by buf.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 */
SceUID sceKernelLoadModuleBuffer(void *buf, SceSize bufsize, int flags, SceKernelLMOption *option);

/**
  * Load a module with the VSH apitype.
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID sceKernelLoadModuleVSH(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load the executable of a disc (EBOOT.BIN) (0x120)
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID ModuleMgrForKernel_A1A78C58(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load the updater executable of a disc (UPDATE/EBOOT.BIN) (0x121)
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID ModuleMgrForKernel_896C15B6(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used for updater pbp's) (0x140)
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID ModuleMgrForKernel_6723BBFF(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used for non updater pbp's) (0x141)
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID ModuleMgrForKernel_49C5B9E1(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used in Ms3) (0x142)
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID ModuleMgrForKernel_ECC2EAA9(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used in Ms4 function to load psx games) (0x143)
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID ModuleMgrForKernel_F07E1A2F(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a module with a specific apitype
  * 
  * @param apìtype - The apitype
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID ModuleMgrForKernel_6DE9FF11(int apitype, const char *path, int flags, SceKernelLMOption *option);

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
int	ModuleMgrForKernel_2BE4807D(int exitcode, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

#define sceKernelLoadModuleDisc ModuleMgrForKernel_A1A78C58
#define sceKernelLoadModuleDiscUpdater ModuleMgrForKernel_896C15B6
#define sceKernelLoadModuleMs1 ModuleMgrForKernel_6723BBFF
#define sceKernelLoadModuleMs2 ModuleMgrForKernel_49C5B9E1
#define sceKernelLoadModuleMs3 ModuleMgrForKernel_ECC2EAA9
#define sceKernelLoadModuleMs4 ModuleMgrForKernel_F07E1A2F
#define sceKernelLoadModuleWithApitype2 ModuleMgrForKernel_6DE9FF11

#define sceKernelStopUnloadSelfModuleWithStatusKernel ModuleMgrForKernel_2BE4807D



/*@}*/

#ifdef __cplusplus
}
#endif

#endif
