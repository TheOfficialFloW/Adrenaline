/*
  Adrenaline
  Copyright (C) 2016-2018, TheFloW

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MENU_H__
#define __MENU_H__

#include <vita2d.h>

enum Colors {
    // Primary colors
    RED             = 0xFF0000FF,
    GREEN           = 0xFF00FF00,
    BLUE            = 0xFFFF0000,
    // Secondary colors
    CYAN            = 0xFFFFFF00,
    MAGENTA         = 0xFFFF00FF,
    YELLOW          = 0xFF00FFFF,
    // Tertiary colors
    AZURE           = 0xFFFF7F00,
    VIOLET          = 0xFFFF007F,
    ROSE            = 0xFF7F00FF,
    ORANGE          = 0xFF007FFF,
    CHARTREUSE      = 0xFF00FF7F,
    SPRING_GREEN    = 0xFF7FFF00,
    // Grayscale
    WHITE           = 0xFFFFFFFF,
    LITEGRAY        = 0xFFBFBFBF,
    GRAY            = 0xFF7F7F7F,
    DARKGRAY        = 0xFF3F3F3F,
    BLACK           = 0xFF000000
};

#define NOALPHA 0xFF

#define COLOR_ALPHA(color, alpha) (color & 0x00FFFFFF) | ((alpha & 0xFF) << 24)

#define SCREEN_LINE 1024
#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544

#define ALIGN_CENTER(a, b) ((a - b) / 2)
#define ALIGN_RIGHT(x, w) ((x) - (w))

#define WINDOW_WIDTH 840.0f
#define WINDOW_HEIGHT 476.0f
#define WINDOW_X ALIGN_CENTER(SCREEN_WIDTH, WINDOW_WIDTH)
#define WINDOW_Y ALIGN_CENTER(SCREEN_HEIGHT, WINDOW_HEIGHT)
#define WINDOW_COLOR COLOR_ALPHA(BLACK, 0xAF)

#define FONT_SIZE 1.0f
#define FONT_Y_SPACE 23.0f

#define FONT_Y_LINE(y) (40.0f + (y) * FONT_Y_SPACE)

#define pgf_draw_text(x, y, color, scale, text) \
  vita2d_pgf_draw_text(font, x, (y) + 20, color, scale, text)

#define pgf_draw_textf(x, y, color, scale, ...) \
  vita2d_pgf_draw_textf(font, x, (y) + 20, color, scale, __VA_ARGS__)

enum MenuEntryTypes {
  MENU_ENTRY_TYPE_TEXT,
  MENU_ENTRY_TYPE_OPTION,
  MENU_ENTRY_TYPE_SCALE,
  MENU_ENTRY_TYPE_CALLBACK,
};

typedef struct {
  char *name;
  int type;
  uint32_t color;
  int (* callback)();
  int *value;
  char **options;
  int n_options;
} MenuEntry;

typedef struct {
  char *name;
  MenuEntry *entries;
  int n_entries;
  int moveable;
} TabEntry;

extern vita2d_pgf *font;

extern int language, enter_button, date_format, time_format;

int ExitAdrenalineMenu();

int AdrenalineDraw(SceSize args, void *argp);
int ScePspemuCustomSettingsHandler(int a1, int a2, int a3, int a4);

#endif
