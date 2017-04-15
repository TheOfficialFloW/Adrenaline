/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspctrl.h - Prototypes for the sceCtrl library.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: pspctrl.h 2433 2008-10-15 10:00:27Z iwn $
 */

/* Note: Some of the structures, types, and definitions in this file were
   extrapolated from symbolic debugging information found in the Japanese
   version of Puzzle Bobble. */

#ifndef __CTRL_H__
#define __CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup Ctrl Controller Kernel Library */
/*@{*/

/**
 * Enumeration for the digital controller buttons.
 *
 * @note PSP_CTRL_HOME, PSP_CTRL_NOTE, PSP_CTRL_SCREEN, PSP_CTRL_VOLUP, PSP_CTRL_VOLDOWN, PSP_CTRL_DISC, PSP_CTRL_WLAN_UP, PSP_CTRL_REMOTE, PSP_CTRL_MS can only be read in kernel mode
 */
enum PspCtrlButtons
{
	/** Select button. */
	PSP_CTRL_SELECT     = 0x000001,
	/** L3 button. */
	PSP_CTRL_L3			= 0x000002,
	/** R3 button. */
	PSP_CTRL_R3			= 0x000004,
	/** Start button. */
	PSP_CTRL_START      = 0x000008,
	/** Up D-Pad button. */
	PSP_CTRL_UP         = 0x000010,
	/** Right D-Pad button. */
	PSP_CTRL_RIGHT      = 0x000020,
	/** Down D-Pad button. */
	PSP_CTRL_DOWN      	= 0x000040,
	/** Left D-Pad button. */
	PSP_CTRL_LEFT      	= 0x000080,
	/** Left trigger. */
	PSP_CTRL_LTRIGGER   = 0x000100,
	/** Right trigger. */
	PSP_CTRL_RTRIGGER   = 0x000200,
	/** L2 button. */
	PSP_CTRL_L2			= 0x000100,
	/** R2 button. */
	PSP_CTRL_R2			= 0x000200,
	/** L1 button. */
	PSP_CTRL_L1			= 0x000400,
	/** R1 button. */
	PSP_CTRL_R1			= 0x000800,
	/** Triangle button. */
	PSP_CTRL_TRIANGLE   = 0x001000,
	/** Circle button. */
	PSP_CTRL_CIRCLE     = 0x002000,
	/** Cross button. */
	PSP_CTRL_CROSS      = 0x004000,
	/** Square button. */
	PSP_CTRL_SQUARE     = 0x008000,
	/** Home button. In user mode this bit is set if the exit dialog is visible. */
	PSP_CTRL_HOME       = 0x010000,
	/** Hold button. */
	PSP_CTRL_HOLD       = 0x020000,
	/** Music Note button. */
	PSP_CTRL_NOTE       = 0x800000,
	/** Screen button. */
	PSP_CTRL_SCREEN     = 0x400000,
	/** Volume up button. */
	PSP_CTRL_VOLUP      = 0x100000,
	/** Volume down button. */
	PSP_CTRL_VOLDOWN    = 0x200000,
	/** Wlan switch up. */
	PSP_CTRL_WLAN_UP    = 0x040000,
	/** Remote hold position. */
	PSP_CTRL_REMOTE     = 0x080000,	
	/** Disc present. */
	PSP_CTRL_DISC       = 0x1000000,
	/** Memory stick present. */
	PSP_CTRL_MS         = 0x2000000,
};

/** Controller mode. */
enum PspCtrlMode
{
	/* Digitial. */
	PSP_CTRL_MODE_DIGITAL = 0,
	/* Analog. */
	PSP_CTRL_MODE_ANALOG
};

/** Returned controller data */
typedef struct SceCtrlData {
	/** The current read frame. */
	unsigned int 	TimeStamp;
	/** Bit mask containing zero or more of ::PspCtrlButtons. */
	unsigned int 	Buttons;
	/** Left analogue stick, X axis. */
	unsigned char 	Lx;
	/** Left analogue stick, Y axis. */
	unsigned char 	Ly;
	/** Right analogue stick, X axis. */
	unsigned char 	Rx;
	/** Right analogue stick, Y axis. */
	unsigned char 	Ry;
	/** Reserved. */
	unsigned int	Reserved;
} SceCtrlData;

typedef struct SceCtrlLatch {
	unsigned int 	uiMake;
	unsigned int 	uiBreak;
	unsigned int 	uiPress;
	unsigned int 	uiRelease;
} SceCtrlLatch;

/**
 * Set the controller cycle setting.
 *
 * @param cycle - Cycle.  Normally set to 0.
 *
 * @return The previous cycle setting.
 */
int sceCtrlSetSamplingCycle(int cycle);

/**
 * Get the controller current cycle setting.
 *
 * @param pcycle - Return value.
 *
 * @return 0.
 */
int sceCtrlGetSamplingCycle(int *pcycle);

/**
 * Set the controller mode.
 *
 * @param mode - One of ::PspCtrlMode.
 *
 * @return The previous mode.
 */
int sceCtrlSetSamplingMode(int mode);

/**
 * Get the current controller mode.
 *
 * @param pmode - Return value.
 *
 * @return 0.
 */
int sceCtrlGetSamplingMode(int *pmode);

int sceCtrlPeekBufferPositive(SceCtrlData *pad_data, int count);

int sceCtrlPeekBufferNegative(SceCtrlData *pad_data, int count);

/**
 * Read buffer positive
 *
 * @par Example:
 * @code
 * SceCtrlData pad;

 * sceCtrlSetSamplingCycle(0);
 * sceCtrlSetSamplingMode(1);
 * sceCtrlReadBufferPositive(&pad, 1);
 * // Do something with the read controller data
 * @endcode
 *
 * @param pad_data - Pointer to a ::SceCtrlData structure used hold the returned pad data.
 * @param count - Number of ::SceCtrlData buffers to read.
 */
int sceCtrlReadBufferPositive(SceCtrlData *pad_data, int count);

int sceCtrlReadBufferNegative(SceCtrlData *pad_data, int count);

int sceCtrlPeekLatch(SceCtrlLatch *latch_data);

int sceCtrlReadLatch(SceCtrlLatch *latch_data);

/**
 * Set analog threshold relating to the idle timer.
 *
 * @param idlereset - Movement needed by the analog to reset the idle timer.
 * @param idleback - Movement needed by the analog to bring the PSP back from an idle state.
 *
 * Set to -1 for analog to not cancel idle timer.
 * Set to 0 for idle timer to be cancelled even if the analog is not moved.
 * Set between 1 - 128 to specify the movement on either axis needed by the analog to fire the event.
 *
 * @return < 0 on error.
 */
int sceCtrlSetIdleCancelThreshold(int idlereset, int idleback);

/**
 * Get the idle threshold values.
 *
 * @param idlerest - Movement needed by the analog to reset the idle timer.
 * @param idleback - Movement needed by the analog to bring the PSP back from an idle state.
 *
 * @return < 0 on error.
 */
int sceCtrlGetIdleCancelThreshold(int *idlerest, int *idleback);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
