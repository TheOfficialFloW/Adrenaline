/*
 * File for sceVshBridge functions 
 * Note: vshbridge lets vsh threads to call to some functions that
 * are kernel-exports only. 
*/

#ifndef __VSHBRIDGE__
#define __VSHBRIDGE__

#include "psploadexec_kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Executes a new executable from a buffer.
 *
 * @param bufsize - Size in bytes of the buffer pointed by buf.
 * @param buf - Pointer to a buffer containing the module to execute.
 * @param param - Pointer to a ::SceKernelLoadExecParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int vshKernelLoadExecBufferPlain(SceSize bufsize, void *buf, struct SceKernelLoadExecParam *param);

/**
 * Restart the vsh.
 *
 * @param unk - Unknown, I haven't checked it. Set it to NULL
 *
 * @returns < 0 on some errors.
 *
 * @note - when called in game mode it will have the same effect that sceKernelExitGame
 *
*/
int vshKernelExitVSHVSH(void *unk);

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute the EBOOT.BIN from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int vshKernelLoadVSHDisc(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute an updater from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int vshKernelLoadExecVSHDiscUpdater(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute an updater from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int vshKernelLoadExecVSHMs1(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute games (and homebrew :P) from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int vshKernelLoadExecVSHMs2(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute ... ?
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors. 
*/
int vshKernelLoadExecVSHMs3(const char *file, struct SceKernelLoadExecVSHParam *param);


/**
 * Performs a logical format in a flash partition.
 *
 * @param argc - The number of parameters
 * @param argv - The parameters
 * @return < 0 on error
 */
int vshLflashFatfmtStartFatfmt(int argc, char *argv[]);

/**
  * Load a module.
  * 
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  */
SceUID vshKernelLoadModuleVSH(const char *path, int flags, SceKernelLMOption *option);

#ifdef __cplusplus
}
#endif

#endif
