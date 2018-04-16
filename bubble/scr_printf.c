/*
 * PSP Software Development Kit - http://www.psvdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * scr_printf.c - Debug screen functions.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: scr_printf.c 2450 2009-01-04 23:53:02Z oopo $
 */
#include <vitasdk.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "pspdebug.h"

#define PSV_SCREEN_WIDTH 960
#define PSV_SCREEN_HEIGHT 544
#define PSV_LINE_SIZE 960

/* baseado nas libs do Duke... */

void  _psvDebugScreenClearLine( int Y);

static int X = 0, Y = 0;
static int MX=68, MY=34;
static uint32_t bg_col = 0, fg_col = 0xFFFFFFFF;
static int bg_enable = 1;
static void* g_vram_base = NULL;
static int g_vram_offset = 0;
static int g_vram_mode = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
static int init = 0;
static int clearline_en = 1;

static void clear_screen_32(uint32_t color)
{
  int x;
  uint32_t *vram = g_vram_base;
  vram +=  (g_vram_offset>>2);

  for(x = 0; x < (PSV_LINE_SIZE * PSV_SCREEN_HEIGHT); x++)
  {
    *vram++ = color; 
  }
}

static void clear_screen(uint32_t color)
{
  if(g_vram_mode == SCE_DISPLAY_PIXELFORMAT_A8B8G8R8)
  {
    clear_screen_32(color);
  }
}

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

void psvDebugScreenInitEx(void *vram_base, int mode, int setup)
{
  switch(mode)
  {
    case SCE_DISPLAY_PIXELFORMAT_A8B8G8R8:
      break;
    default: mode = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
  };

  X = Y = 0;
  /* Place vram in uncached memory */
  if(vram_base == NULL)
  {
    int block = sceKernelAllocMemBlock("", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, ALIGN(PSV_LINE_SIZE * PSV_SCREEN_HEIGHT * 4, 256 * 1024), NULL);
    sceKernelGetMemBlockBase(block, &vram_base);
  }
  g_vram_base = vram_base;
  g_vram_offset = 0;
  g_vram_mode = mode;
  if(setup)
  {
    SceDisplayFrameBuf framebuf;
    framebuf.size = sizeof(SceDisplayFrameBuf);
    framebuf.base = (void *)g_vram_base;
    framebuf.pitch = PSV_LINE_SIZE;
    framebuf.width = PSV_SCREEN_WIDTH;
    framebuf.height = PSV_SCREEN_HEIGHT;
    framebuf.pixelformat = mode;
    sceDisplaySetFrameBuf(&framebuf, 1);
  }
  clear_screen(bg_col);
  init = 1;
}

void psvDebugScreenInit()
{
  X = Y = 0;
  psvDebugScreenInitEx(NULL, SCE_DISPLAY_PIXELFORMAT_A8B8G8R8, 1);
}

void psvDebugScreenEnableBackColor(int enable) 
{
  bg_enable = enable;
}

void psvDebugScreenSetBackColor(uint32_t colour)
{
  bg_col = colour;
}

void psvDebugScreenSetTextColor(uint32_t colour)
{
  fg_col = colour;
}

void psvDebugScreenSetColorMode(int mode)
{
  switch(mode)
  {
    case SCE_DISPLAY_PIXELFORMAT_A8B8G8R8:
      break;
    default: mode = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
  };

  g_vram_mode = mode;
}

int psvDebugScreenGetX()
{
  return X;
}

int psvDebugScreenGetY()
{
  return Y;
}

void psvDebugScreenClear()
{
  int y;

  if(!init)
  {
    return;
  }

  for(y=0;y<MY;y++)
  {
    _psvDebugScreenClearLine(y);
  }

  psvDebugScreenSetXY(0,0);
  clear_screen(bg_col);
}

void psvDebugScreenSetXY(int x, int y)
{
  if( x<MX && x>=0 ) X=x;
  if( y<MY && y>=0 ) Y=y;
}

void psvDebugScreenSetOffset(int offset)
{
  g_vram_offset = offset;
}

void psvDebugScreenSetBase(uint32_t* base)
{
  g_vram_base = base;
}

extern uint8_t msx[];

static void debug_put_char_32(int x, int y, uint32_t color, uint32_t bgc, uint8_t ch)
{
  int   i,j, l;
  uint8_t  *font;
  uint32_t *vram_ptr;
  uint32_t *vram;

  if(!init)
  {
    return;
  }

  x *= 2;
  y *= 2;

  vram = g_vram_base;
  vram += (g_vram_offset >> 2) + x;
  vram += (y * PSV_LINE_SIZE);

  font = &msx[ (int)ch * 8];
  for (i=l=0; i < 8; i++, l+= 8, font++)
  {
    vram_ptr  = vram;
    for (j=0; j < 8; j++)
    {
      if ((*font & (128 >> j))) {
        vram_ptr[0] = color; 
        vram_ptr[1] = color; 
        vram_ptr[0 + PSV_LINE_SIZE] = color; 
        vram_ptr[1 + PSV_LINE_SIZE] = color; 
      } else if(bg_enable) {
        vram_ptr[0] = bgc; 
        vram_ptr[1] = bgc; 
        vram_ptr[0 + PSV_LINE_SIZE] = bgc; 
        vram_ptr[1 + PSV_LINE_SIZE] = bgc; 
      }

      vram_ptr += 2;
    }
    vram += 2 * PSV_LINE_SIZE;
  }
}

void
psvDebugScreenPutChar( int x, int y, uint32_t color, uint8_t ch)
{
  if(g_vram_mode == SCE_DISPLAY_PIXELFORMAT_A8B8G8R8)
  {
    debug_put_char_32(x, y, color, bg_col, ch);
  }
}

void  _psvDebugScreenClearLine( int Y)
{
  if(clearline_en)
  {
    int i;
    if(bg_enable)
    {
      for (i=0; i < MX; i++)
      {
        psvDebugScreenPutChar( i*7 , Y * 8, bg_col, 219);
      }
    }
  }
  return;
}

void psvDebugScreenClearLineEnable(void)
{
  clearline_en = 1;
  return;
}

void psvDebugScreenClearLineDisable(void)
{
  clearline_en = 0;
  return;
}

/* Print non-nul terminated strings */
int psvDebugScreenPrintData(const char *buff, int size)
{
  int i;
  int j;
  char c;

  if(!init)
  {
    return 0;
  }

  for (i = 0; i < size; i++)
  {
    c = buff[i];
    switch (c)
    {
      case '\r':
            X = 0;
            break;
      case '\n':
            X = 0;
            Y ++;
            if (Y == MY)
              Y = 0;
            _psvDebugScreenClearLine(Y);
            break;
      case '\t':
            for (j = 0; j < 5; j++) {
              psvDebugScreenPutChar( X*7 , Y * 8, fg_col, ' ');
              X++;
            }
            break;
      default:
            psvDebugScreenPutChar( X*7 , Y * 8, fg_col, c);
            X++;
            if (X == MX)
            {
              X = 0;
              Y++;
              if (Y == MY)
                Y = 0;
              _psvDebugScreenClearLine(Y);
            }
    }
  }

  return i;
}

int psvDebugScreenPuts(const char *str)
{
  return psvDebugScreenPrintData(str, sceClibStrnlen(str, 2048));
}

void psvDebugScreenPrintf(const char *format, ...)
{
  va_list  opt;
  char     buff[2048];
  int    bufsz;

  va_start(opt, format);
  bufsz = sceClibVsnprintf( buff, (size_t) sizeof(buff), format, opt);
  (void) psvDebugScreenPrintData(buff, bufsz);
}
