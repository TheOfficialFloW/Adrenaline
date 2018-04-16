/*
 * PSP Software Development Kit - http://www.psvdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspdebug.h - Prototypes for the psvDebug library
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: pspdebug.h 2450 2009-01-04 23:53:02Z oopo $
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup Debug Debug Utility Library */

/** @addtogroup Debug */
/*@{*/

/** 
  * Initialise the debug screen
  */
void psvDebugScreenInit(void);

/**
 * Extended debug screen init
 *
 * @param vram_base - Base address of frame buffer, if NULL then sets a default
 * @param mode - Colour mode
 * @param setup - Setup the screen if 1
 */
void psvDebugScreenInitEx(void *vram_base, int mode, int setup);

/**
  * Do a printf to the debug screen.
  *
  * @param fmt - Format string to print
  * @param ... - Arguments
  */
void psvDebugScreenPrintf(const char *fmt, ...) __attribute__((format(printf,1,2)));

/**
 * Enable or disable background colour writing (defaults to enabled)
 * 
 * @param enable - Set 1 to to enable background color, 0 for disable
 */
void psvDebugScreenEnableBackColor(int enable);

/** 
  * Set the background color for the text
  * @note To reset the entire screens bg colour you need to call psvDebugScreenClear
  *
  * @param color - A 32bit RGB colour
  */
void psvDebugScreenSetBackColor(uint32_t color);

/**
  * Set the text color 
  *
  * @param color - A 32 bit RGB color
  */
void psvDebugScreenSetTextColor(uint32_t color);

/**
 * Set the color mode (you must have switched the frame buffer appropriately)
 *
 * @param mode - Color mode
 */
void psvDebugScreenSetColorMode(int mode);

/** 
  * Draw a single character to the screen.
  *
  * @param x - The x co-ordinate to draw to (pixel units)
  * @param y - The y co-ordinate to draw to (pixel units)
  * @param color - The text color to draw
  * @param ch - The character to draw
  */
void psvDebugScreenPutChar(int x, int y, uint32_t color, uint8_t ch);

/**
  * Set the current X and Y co-ordinate for the screen (in character units)
  */
void psvDebugScreenSetXY(int x, int y);

/**
  * Set the video ram offset used for the screen
  *
  * @param offset - Offset in bytes
  */
void psvDebugScreenSetOffset(int offset);

/**
 * Set the video ram base used for the screen
 *
 * @param base - Base address in bytes
 */
void psvDebugScreenSetBase(uint32_t* base);

/** 
  * Get the current X co-ordinate (in character units)
  *
  * @return The X co-ordinate
  */
int psvDebugScreenGetX(void);

/** 
  * Get the current Y co-ordinate (in character units)
  *
  * @return The Y co-ordinate
  */
int psvDebugScreenGetY(void);

/**
  * Clear the debug screen.
  */
void psvDebugScreenClear(void);

/**
  * Print non-nul terminated strings.
  * 
  * @param buff - Buffer containing the text.
  * @param size - Size of the data
  *
  * @return The number of characters written
  */
int psvDebugScreenPrintData(const char *buff, int size);

/**
 * Print a string
 *
 * @param str - String
 *
 * @return The number of characters written
 */
int psvDebugScreenPuts(const char *str);

/**
  * Get a MIPS stack trace (might work :P)
  *
  * @param results - List of points to store the results of the trace, (up to max)
  * @param max - Maximum number of back traces
  *
  * @return The number of frames stored in results.
*/
int psvDebugGetStackTrace(unsigned int* results, int max);

/**
 * Enable the clear line function that allows debug to clear the screen
*/
void psvDebugScreenClearLineEnable(void);

/**
 * Disable the clear line function that causes flicker on constant refreshes
*/
void psvDebugScreenClearLineDisable(void);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif