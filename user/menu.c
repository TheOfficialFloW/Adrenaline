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

#include <psp2/appmgr.h>
#include <psp2/apputil.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/system_param.h>
#include <psp2/sysmodule.h>
#include <psp2/power.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/dmac.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <vita2d.h>

#include "main.h"
#include "menu.h"
#include "states.h"
#include "usb.h"
#include "utils.h"
#include "math_utils.h"

#include <lcd3x_v.h>
#include <lcd3x_f.h>
#include <texture_v.h>
#include <texture_f.h>
#include <opaque_v.h>
#include <bicubic_f.h>
#include <sharp_bilinear_f.h>
#include <sharp_bilinear_v.h>
#include <sharp_bilinear_simple_f.h>
#include <sharp_bilinear_simple_v.h>
#include <advanced_aa_v.h>
#include <advanced_aa_f.h>
#include "vflux_f.h"
#include "vflux_v.h"

static const SceGxmProgram *const gxm_program_vflux_v = (SceGxmProgram*)&vflux_v;
static const SceGxmProgram *const gxm_program_vflux_f = (SceGxmProgram*)&vflux_f;

extern SceGxmContext *_vita2d_context;
int sceCommonDialogIsRunning();

vita2d_pgf *font;

int language = 0, enter_button = 0, date_format = 0, time_format = 0;

static int EnterStandbyMode();
static int OpenOfficialSettings();
static int ExitPspEmuApplication();
static int ResetAdrenalineSettings();

// RGB colors for the filter box used by f.lux
static float flux_colors[] = {
  1.0f, 0.5f, 0.0f, 0.0f, // Yellow
  0.0f, 0.0f, 0.5f, 0.0f, // Blue
  0.0f, 0.0f, 0.0f, 0.0f  // Black
};

static char *graphics_options[] = { "Original", "Bilinear", "Sharp bilinear", "Advanced AA", "LCD3x", "Sharp bilinear without scanlines" };
static char *flux_mode_options[] = { "None", "Yellow", "Blue", "Black" };
static char *no_yes_options[] = { "No", "Yes" };
static char *yes_no_options[] = { "Yes", "No" };
static char *ms_location_options[] = { "ux0:pspemu", "ur0:pspemu", "imc0:pspemu", "xmc0:pspemu", "uma0:pspemu" };
static char *usbdevice_options[] = { "Memory Card", "Internal Storage", "sd2vita", "psvsd" };

static MenuEntry main_entries[] = {
  { "Enter Standby Mode",        MENU_ENTRY_TYPE_CALLBACK, 0, EnterStandbyMode, NULL, NULL, 0 },
  { "Open Official Settings",    MENU_ENTRY_TYPE_CALLBACK, 0, OpenOfficialSettings, NULL, NULL, 0 },
  { "Exit PspEmu Application",   MENU_ENTRY_TYPE_CALLBACK, 0, ExitPspEmuApplication, NULL, NULL, 0 },
  { "Exit Adrenaline Menu",      MENU_ENTRY_TYPE_CALLBACK, 0, ExitAdrenalineMenu, NULL, NULL, 0 },
};

static MenuEntry settings_entries[] = {
  { "Graphics Filtering",        MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.graphics_filtering, graphics_options, sizeof(graphics_options) / sizeof(char **) },
  { "Smooth Graphics",           MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.no_smooth_graphics, yes_no_options, sizeof(yes_no_options) / sizeof(char **) },
  { "f.lux Filter Color",        MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.flux_mode, flux_mode_options, sizeof(flux_mode_options) / sizeof(char **) },
  { "Screen Scale X (PSP)",      MENU_ENTRY_TYPE_SCALE,  0, NULL, (int *)&config.psp_screen_scale_x, NULL, 0 },
  { "Screen Scale Y (PSP)",      MENU_ENTRY_TYPE_SCALE,  0, NULL, (int *)&config.psp_screen_scale_y, NULL, 0 },
  { "Screen Scale X (PS1)",      MENU_ENTRY_TYPE_SCALE,  0, NULL, (int *)&config.ps1_screen_scale_x, NULL, 0 },
  { "Screen Scale Y (PS1)",      MENU_ENTRY_TYPE_SCALE,  0, NULL, (int *)&config.ps1_screen_scale_y, NULL, 0 },
  { "Memory Stick Location",     MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.ms_location, ms_location_options, sizeof(ms_location_options) / sizeof(char **) },
  { "USB device",                MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.usbdevice, usbdevice_options, sizeof(usbdevice_options) / sizeof(char **) },
  { "Use DS3/DS4 controller",    MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.use_ds3_ds4, no_yes_options, sizeof(no_yes_options) / sizeof(char **) },
  { "Skip Adrenaline Boot Logo", MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.skip_logo, no_yes_options, sizeof(no_yes_options) / sizeof(char **) },
  { "Reset Adrenaline Settings", MENU_ENTRY_TYPE_CALLBACK, 0, ResetAdrenalineSettings, NULL, NULL, 0 },
};

static MenuEntry about_entries[] = {
#if (ADRENALINE_VERSION_MINOR > 0)
  { "6.61 Adrenaline-" ADRENALINE_VERSION_MAJOR_STR "." ADRENALINE_VERSION_MINOR_STR, MENU_ENTRY_TYPE_TEXT, ORANGE, NULL, NULL, NULL, 0 },
#else
  { "6.61 Adrenaline-" ADRENALINE_VERSION_MAJOR_STR, MENU_ENTRY_TYPE_TEXT, ORANGE, NULL, NULL, NULL, 0 },
#endif
  { "by TheFloW", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
  { "", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
  { "", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
  { "Credits", MENU_ENTRY_TYPE_TEXT, ORANGE, NULL, NULL, NULL, 0 },
  { "Team molecule for HENkaku", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
  { "frangarcj for graphics filtering", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
  { "Rinnegatamante for f.lux", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
  { "xerpi for vita2dlib", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
};

static TabEntry tab_entries[] = {
  { "Main", main_entries, sizeof(main_entries) / sizeof(MenuEntry), 1 },
  { "States", NULL, 0, 0 },
  { "Settings", settings_entries, sizeof(settings_entries) / sizeof(MenuEntry), 1 },
  { "About", about_entries, sizeof(about_entries) / sizeof(MenuEntry), 0 },
};

#define N_TABS (sizeof(tab_entries) / sizeof(TabEntry))
#define TAB_SIZE (WINDOW_WIDTH / N_TABS)

static AdrenalineConfig old_config;
static int tab_sel = 0;
static int menu_sel = 0;
int menu_open = 0;

static int changed = 0;
static int open_official_settings = 0;

static SceUID settings_semaid = -1;

static int EnterStandbyMode() {
  stopUsb(usbdevice_modid);
  ExitAdrenalineMenu();
  scePowerRequestSuspend();
  return 0;
}

static int OpenOfficialSettings() {
  open_official_settings = 1;
  ExitAdrenalineMenu();
  return 0;
}

static int ExitPspEmuApplication() {
  stopUsb(usbdevice_modid);
  ScePspemuErrorExit(0);
  return 0;
}

static int EnterAdrenalineMenu() {
  initStates();

  memcpy(&old_config, &config, sizeof(AdrenalineConfig));

  changed = 0;
  menu_open = 1;
  open_official_settings = 0;

  SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_INPUT_MODE, ADRENALINE_SIZE);
  if (adrenaline->pops_mode)
    ScePspemuPausePops(1);

  return 0;
}

int ExitAdrenalineMenu() {
  if (changed) {
    config.magic[0] = ADRENALINE_CFG_MAGIC_1;
    config.magic[1] = ADRENALINE_CFG_MAGIC_2;
    WriteFile("ux0:app/" ADRENALINE_TITLEID "/adrenaline.bin", &config, sizeof(AdrenalineConfig));
  }

  SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, KERMIT_INPUT_MODE, ADRENALINE_SIZE);
  if (adrenaline->pops_mode)
    ScePspemuPausePops(0);

  menu_open = 0;

  if (old_config.ms_location != config.ms_location)
    SendAdrenalineRequest(ADRENALINE_PSP_CMD_REINSERT_MS);

  SetPspemuFrameBuffer((void *)SCE_PSPEMU_FRAMEBUFFER);
  sceDisplayWaitVblankStart();

  sceKernelSignalSema(settings_semaid, 1);

  finishStates();

  return 0;
}

int ResetAdrenalineSettings() {
  memset(&config, 0, sizeof(AdrenalineConfig));
  config.magic[0] = ADRENALINE_CFG_MAGIC_1;
  config.magic[1] = ADRENALINE_CFG_MAGIC_2;
  config.psp_screen_scale_x = 2.0f;
  config.psp_screen_scale_y = 2.0f;
  config.ps1_screen_scale_x = 1.0f;
  config.ps1_screen_scale_y = 1.0f;
  WriteFile("ux0:app/" ADRENALINE_TITLEID "/adrenaline.bin", &config, sizeof(AdrenalineConfig));

  return 0;
}

void drawMenu() {
  // Draw window
  vita2d_draw_rectangle(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_COLOR);

  // Draw title
  vita2d_draw_rectangle(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, 38.0f, COLOR_ALPHA(GRAY, 0x7F));
  char *title = "Adrenaline Menu";
  pgf_draw_textf(WINDOW_X + ALIGN_CENTER(WINDOW_WIDTH, vita2d_pgf_text_width(font, FONT_SIZE, title)), FONT_Y_LINE(0), WHITE, FONT_SIZE, title);

  // Draw tabs
  int i;
  for (i = 0; i < N_TABS; i++) {
    vita2d_draw_rectangle(WINDOW_X + (i * TAB_SIZE), FONT_Y_LINE(19) - 5.0f, TAB_SIZE, 38.0f, tab_sel == i ? 0xFF7F3F00 : COLOR_ALPHA(GRAY, 0x8F));

    if (i != 0)
      vita2d_draw_rectangle(WINDOW_X + (i * TAB_SIZE) - 2.0f, FONT_Y_LINE(19) - 5.0f, 4.0f, 38.0f, COLOR_ALPHA(BLACK, 0x8F));

    float x = WINDOW_X + (i * TAB_SIZE) + ALIGN_CENTER(TAB_SIZE, vita2d_pgf_text_width(font, FONT_SIZE, tab_entries[i].name));
    pgf_draw_text(x, FONT_Y_LINE(19), WHITE, FONT_SIZE, tab_entries[i].name);
  }

  // Draw entries
  MenuEntry *menu_entries = tab_entries[tab_sel].entries;
  if (menu_entries) {
    for (i = 0; i < tab_entries[tab_sel].n_entries; i++) {
      float y = FONT_Y_LINE(2 + i);

      uint32_t color = WHITE;
      if (menu_entries[i].type == MENU_ENTRY_TYPE_TEXT)
        color = menu_entries[i].color;

      // Focus
      if (i == menu_sel && tab_entries[tab_sel].moveable)
        vita2d_draw_rectangle(WINDOW_X, y + 3.0f, WINDOW_WIDTH, FONT_Y_SPACE, COLOR_ALPHA(0xFFFF1F7F, 0x8F));

      if (menu_entries[i].type == MENU_ENTRY_TYPE_TEXT || menu_entries[i].type == MENU_ENTRY_TYPE_CALLBACK) {
        // Item
        float x = vita2d_pgf_text_width(font, FONT_SIZE, menu_entries[i].name);
        pgf_draw_text(ALIGN_CENTER(SCREEN_WIDTH, x), y, color, FONT_SIZE, menu_entries[i].name);
      } else {
        // Item
        float x = vita2d_pgf_text_width(font, FONT_SIZE, menu_entries[i].name);
        pgf_draw_text(ALIGN_RIGHT((SCREEN_WIDTH / 2.0f) - 10.0f, x), y, color, FONT_SIZE, menu_entries[i].name);

        if (menu_entries[i].type == MENU_ENTRY_TYPE_OPTION) {
          // Option
          int value = *(menu_entries[i].value);
          pgf_draw_text((SCREEN_WIDTH / 2.0f) + 10.0f, y, GREEN, FONT_SIZE, menu_entries[i].options[value]);
        } else if (menu_entries[i].type == MENU_ENTRY_TYPE_SCALE) {
            // Option
          float *value = (float *)menu_entries[i].value;
          pgf_draw_textf((SCREEN_WIDTH / 2.0f) + 10.0f, y, GREEN, FONT_SIZE, "%.03f", *value);
        }
      }
    }

    // Info about Original filter
    if (tab_sel == 2 && menu_sel == 0 && config.graphics_filtering == 0) {
      char *title = "All graphics related options are not taking effect with the Original rendering mode.";
      pgf_draw_textf(WINDOW_X + ALIGN_CENTER(WINDOW_WIDTH, vita2d_pgf_text_width(font, FONT_SIZE, title)), FONT_Y_LINE(17), WHITE, FONT_SIZE, title);
    }
  } else {
    drawStates();
  }
}

void ctrlMenu() {
  if (released_pad[PAD_PSBUTTON]) {
    ExitAdrenalineMenu();
  }

  if (!open_options) {
    if (released_pad[PAD_CANCEL]) {
      ExitAdrenalineMenu();
    }

    if (pressed_pad[PAD_LTRIGGER]) {
      if (tab_sel > 0) {
        menu_sel = 0;
        tab_sel--;
      }
    }

    if (pressed_pad[PAD_RTRIGGER]) {
      if (tab_sel < N_TABS-1) {
        menu_sel = 0;
        tab_sel++;
      }
    }
  }

  // Savestates
  if (tab_sel == 1) {
    ctrlStates();
  } else {
    if (tab_entries[tab_sel].moveable) {
      MenuEntry *menu_entries = tab_entries[tab_sel].entries;
      if (menu_entries) {
        if (menu_entries[menu_sel].type == MENU_ENTRY_TYPE_CALLBACK) {
          if (released_pad[PAD_ENTER]) {
            if (menu_entries[menu_sel].callback)
              menu_entries[menu_sel].callback();
          }
        } else if (menu_entries[menu_sel].type == MENU_ENTRY_TYPE_OPTION) {
          if (released_pad[PAD_RIGHT] || released_pad[PAD_LEFT_ANALOG_RIGHT] || released_pad[PAD_ENTER]) {
            if ((*menu_entries[menu_sel].value) < menu_entries[menu_sel].n_options-1)
              (*menu_entries[menu_sel].value)++;
            else
              (*menu_entries[menu_sel].value) = 0;
          }

          if (released_pad[PAD_LEFT] || released_pad[PAD_LEFT_ANALOG_LEFT]) {
            if ((*menu_entries[menu_sel].value) > 0)
              (*menu_entries[menu_sel].value)--;
            else
              (*menu_entries[menu_sel].value) = menu_entries[menu_sel].n_options-1;
          }

          changed = 1;
        } else if (menu_entries[menu_sel].type == MENU_ENTRY_TYPE_SCALE) {
          if (hold2_pad[PAD_RIGHT] || hold2_pad[PAD_LEFT_ANALOG_RIGHT] || hold2_pad[PAD_ENTER]) {
            float *a = (float *)menu_entries[menu_sel].value;
            (*a) += 0.005f;
          }

          if (hold2_pad[PAD_LEFT] || hold2_pad[PAD_LEFT_ANALOG_LEFT]) {
            float *a = (float *)menu_entries[menu_sel].value;
            if (*a > 0.005f)
              (*a) -= 0.005f;
          }

          changed = 1;
        }
      }

      if (hold_pad[PAD_UP] || hold_pad[PAD_LEFT_ANALOG_UP]) {
        if (menu_sel > 0)
          menu_sel--;
      }

      if (hold_pad[PAD_DOWN] || hold_pad[PAD_LEFT_ANALOG_DOWN]) {
        if (menu_sel < tab_entries[tab_sel].n_entries-1)
          menu_sel++;
      }
    }
  }
}

void getPspScreenScale(float *scale_x, float *scale_y) {
  *scale_x = config.psp_screen_scale_x;
  *scale_y = config.psp_screen_scale_y;
}

void getPopsScreenScale(float *scale_x, float *scale_y) {
  *scale_x = config.ps1_screen_scale_x;
  *scale_y = config.ps1_screen_scale_y;
}

void *pops_data = NULL;

int AdrenalineDraw(SceSize args, void *argp) {
  sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &language);
  sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &enter_button);
  sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_DATE_FORMAT, &date_format);
  sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_TIME_FORMAT, &time_format);

  vita2d_init();
  font = vita2d_load_default_pgf();

  vita2d_texture *native_tex = vita2d_create_empty_texture_data(SCREEN_WIDTH, SCREEN_HEIGHT, (void*)CONVERT_ADDRESS(NATIVE_FRAMEBUFFER), SCE_GXM_TEXTURE_FORMAT_U5U6U5_BGR);
  if (!native_tex)
    return -1;

  vita2d_texture *psp_tex = vita2d_create_empty_texture(PSP_SCREEN_LINE, PSP_SCREEN_HEIGHT);
  if (!psp_tex)
    return -1;

  void *psp_data = vita2d_texture_get_datap(psp_tex);

  vita2d_texture *pops_tex = vita2d_create_empty_texture_format(SCREEN_LINE, SCREEN_HEIGHT, SCE_GXM_TEXTURE_FORMAT_X8U8U8U8_1BGR);
  if (!pops_tex)
    return -1;

  pops_data = vita2d_texture_get_datap(pops_tex);

  vita2d_shader *opaque_shader = vita2d_create_shader((SceGxmProgram *)opaque_v, (SceGxmProgram *)texture_f);
  vita2d_shader *sharp_shader = vita2d_create_shader((SceGxmProgram *)sharp_bilinear_v, (SceGxmProgram *)sharp_bilinear_f);
  vita2d_shader *advanced_aa_shader = vita2d_create_shader((SceGxmProgram *)advanced_aa_v, (SceGxmProgram *)advanced_aa_f);
  vita2d_shader *lcd3x_shader = vita2d_create_shader((SceGxmProgram *)lcd3x_v, (SceGxmProgram *)lcd3x_f);
  vita2d_shader *sharp_simple_shader = vita2d_create_shader((SceGxmProgram *)sharp_bilinear_simple_v, (SceGxmProgram *)sharp_bilinear_simple_f);

  // f.lux shader
  vita2d_shader *flux_shader = vita2d_create_shader_untextured((SceGxmProgram *)gxm_program_vflux_v, (SceGxmProgram *)gxm_program_vflux_f);

  // f.lux vertices
  SceUID flux_vertices_id = sceKernelAllocMemBlock(
    "flux vertices",
    SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,
    ALIGN(sizeof(float)*12, 4 * 1024),
    NULL);
  float *flux_vertices;
  sceKernelGetMemBlockBase(flux_vertices_id, (void **)&flux_vertices);
  flux_vertices[0] = 0.0f;
  flux_vertices[1] = 0.0f;
  flux_vertices[2] = 0.5f;
  flux_vertices[3] = 0.0f;
  flux_vertices[4] = 544.0f;
  flux_vertices[5] = 0.5f;
  flux_vertices[6] = 960.0f;
  flux_vertices[7] = 544.0f;
  flux_vertices[8] = 0.5f;
  flux_vertices[9] = 960.0f;
  flux_vertices[10] = 0.0f;
  flux_vertices[11] = 0.5f;
  sceGxmMapMemory(flux_vertices, ALIGN(sizeof(float)*12, 4 * 1024), SCE_GXM_MEMORY_ATTRIB_READ);

  // f.lux indices
  SceUID flux_indices_id = sceKernelAllocMemBlock(
    "flux indices",
    SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,
    ALIGN(sizeof(uint16_t)*4, 4 * 1024),
    NULL);
  uint16_t *flux_indices;
  sceKernelGetMemBlockBase(flux_indices_id, (void **)&flux_indices);
  int i;
  for (i=0;i<4;i++){
    flux_indices[i] = i;
  }
  sceGxmMapMemory(flux_indices, ALIGN(sizeof(uint16_t)*4, 4 * 1024), SCE_GXM_MEMORY_ATTRIB_READ);

  // f.lux model-view-projection matrix
  matrix4x4 projection, modelview, mvp;
  matrix4x4_identity(modelview);
  matrix4x4_init_orthographic(projection, 0, 960, 544, 0, -1, 1);
  matrix4x4_multiply(mvp, projection, modelview);

  // f.lux shaders uniforms
  SceGxmProgramParameter *vflux_color_param = sceGxmProgramFindParameterByName(gxm_program_vflux_f, "color");
  SceGxmProgramParameter *vflux_wvp_param = sceGxmProgramFindParameterByName(gxm_program_vflux_v, "wvp");

  vita2d_shader *shader = opaque_shader;

  settings_semaid = sceKernelCreateSema("AdrenalineSettingsSemaphore", 0, 0, 1, NULL);
  if (settings_semaid < 0)
    return settings_semaid;

  #ifdef SHOW_FPS
  // FPS counting
  SceUInt64 cur_micros = 0, delta_micros = 0, last_micros = 0;
  uint32_t frames = 0;
  float fps = 0.0f;
  #endif

  // keep track of entering pops mode
  int lastPops = 0;

  while (1) {
    int draw_native = *(uint32_t *)CONVERT_ADDRESS(DRAW_NATIVE);
    SceAdrenaline *adrenaline = (SceAdrenaline *)CONVERT_ADDRESS(ADRENALINE_ADDRESS);

    // pause/unpause pops once after switching from psp to pops mode
    // this pause/unpause fixes slowdown in PS1 games that used to require manually entering/exiting menu
    if (!adrenaline->pops_mode) {
      lastPops = 0;
    }

    if (adrenaline->pops_mode && lastPops == 0) {
      ScePspemuPausePops(1);
      sceDisplayWaitVblankStart();
      ScePspemuPausePops(0);
      sceDisplayWaitVblankStart();
      lastPops = 1;
    }

    // Draw savestate screen
    if (adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
      vita2d_start_drawing();
      vita2d_clear_screen();

      char *title = "Please wait...";
      pgf_draw_textf(ALIGN_CENTER(SCREEN_WIDTH, vita2d_pgf_text_width(font, FONT_SIZE, title)), FONT_Y_LINE(8), WHITE, FONT_SIZE, title);

      // End drawing
      vita2d_end_drawing();
      vita2d_swap_buffers();

      // Sync
      if (!adrenaline->pops_mode)
        sceCompatLCDCSync();

      continue;
    }

    // Read pad
    readPad();

    // Double click detection
    if (menu_open == 0) {
      if (doubleClick(SCE_CTRL_PSBUTTON, 300 * 1000)) {
        stopUsb(usbdevice_modid);

        if (sceAppMgrLaunchAppByName2(ADRENALINE_TITLEID, NULL, NULL) < 0)
          ScePspemuErrorExit(0);
      }
    }

    // Fast forward in pops
    if (adrenaline->pops_mode) {
      // FSM for button combination
      static int combo_state = 0;
      if (current_pad[PAD_LTRIGGER] && current_pad[PAD_SELECT]) {
        combo_state = 1;
      } else {
        if (combo_state == 1)
          combo_state = 2;
        else
          combo_state = 0;
      }

      if (combo_state == 2) {
        uint8_t *val = (uint8_t *)ScePspemuConvertAddress(0xABCD00A9, KERMIT_OUTPUT_MODE, 1);
        *val = !(*val);
        ScePspemuWritebackCache(val, 1);
        combo_state = 0;
      }
    }

    // Do not draw if dialog is running
    if (sceCommonDialogIsRunning() || (config.graphics_filtering == 0 && menu_open == 0 && draw_native == 0)) {
      sceDisplayWaitVblankStart();
      continue;
    }

    // Draw display
    vita2d_start_drawing();
    vita2d_clear_screen();

    // Select shader
    if (config.graphics_filtering == 1)
      shader = opaque_shader;
    else if (config.graphics_filtering == 2)
      shader = sharp_shader;
    else if (config.graphics_filtering == 3)
      shader = advanced_aa_shader;
    else if (config.graphics_filtering == 4)
      shader = lcd3x_shader;
    else if (config.graphics_filtering == 5)
      shader = sharp_simple_shader;
    else
      shader = opaque_shader;

    vita2d_texture_set_program(shader->vertexProgram, shader->fragmentProgram);
    vita2d_texture_set_wvp(shader->wvpParam);
    vita2d_texture_set_vertexInput(&shader->vertexInput);
    vita2d_texture_set_fragmentInput(&shader->fragmentInput);

    // Smooth
    if (config.no_smooth_graphics == 0) {
      vita2d_texture_set_filters(psp_tex, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
      vita2d_texture_set_filters(pops_tex, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
    } else {
      vita2d_texture_set_filters(psp_tex, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
      vita2d_texture_set_filters(pops_tex, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
    }

    if ((!adrenaline->pops_mode && !draw_native) || adrenaline->draw_psp_screen_in_pops) {
      // Copy PSP framebuffer
      sceDmacMemcpy(psp_data, (void *)SCE_PSPEMU_FRAMEBUFFER, SCE_PSPEMU_FRAMEBUFFER_SIZE);

      // Draw psp screen
      float scale_x = 2.00f;
      float scale_y = 2.00f;
      getPspScreenScale(&scale_x, &scale_y);
      vita2d_draw_texture_scale_rotate_hotspot(psp_tex, 480.0f, 272.0f, scale_x, scale_y, 0.0, 240.0, 136.0);
    } else if (draw_native) {
      vita2d_draw_texture_scale_rotate_hotspot(native_tex, 480.0f, 272.0f, 1.0f, 1.0f, 0.0, 480.0, 272.0);
    } else {
      // Draw pops screen
      float scale_x = 1.0f;
      float scale_y = 1.0f;
      getPopsScreenScale(&scale_x, &scale_y);
      vita2d_draw_texture_scale_rotate_hotspot(pops_tex, 480.0f, 272.0f, scale_x, scale_y, 0.0, 480.0, 272.0);
    }

    // Draw Menu
    if (menu_open)
      drawMenu();

    // f.lux filter drawing
    if (config.flux_mode != 0) {
      uint8_t flux_idx = (config.flux_mode * 4) - 1;

      // Updating our rectangle alpha value depending on daytime
      SceDateTime time;
      sceRtcGetCurrentClockLocalTime(&time);
      if (time.hour < 6)       // Night/Early Morning
        flux_colors[flux_idx] = 0.25f;
      else if (time.hour < 10) // Morning/Early Day
        flux_colors[flux_idx] = 0.1f;
      else if (time.hour < 15) // Mid day
        flux_colors[flux_idx] = 0.05f;
      else if (time.hour < 19) // Late day
        flux_colors[flux_idx] = 0.15f;
      else                     // Evening/Night
        flux_colors[flux_idx] = 0.2f;

      // Setting vertex and fragment program for f.lux
      sceGxmSetVertexProgram(_vita2d_context, flux_shader->vertexProgram);
      sceGxmSetFragmentProgram(_vita2d_context, flux_shader->fragmentProgram);

      // Setting color uniform
      void *rgba_buffer, *wvp_buffer;
      sceGxmReserveFragmentDefaultUniformBuffer(_vita2d_context, &rgba_buffer);
      sceGxmSetUniformDataF(rgba_buffer, vflux_color_param, 0, 4, &flux_colors[(config.flux_mode - 1) * 4]);

      // Setting wvp uniform
      sceGxmReserveVertexDefaultUniformBuffer(_vita2d_context, &wvp_buffer);
      sceGxmSetUniformDataF(wvp_buffer, vflux_wvp_param, 0, 16, (const float*)mvp);

      // Performing f.lux filter draw
      sceGxmSetVertexStream(_vita2d_context, 0, flux_vertices);
      sceGxmDraw(_vita2d_context, SCE_GXM_PRIMITIVE_TRIANGLE_FAN, SCE_GXM_INDEX_FORMAT_U16, flux_indices, 4);
    }

    #ifdef SHOW_FPS
    // Show FPS
    pgf_draw_textf(0.0f, 0.0f, WHITE, FONT_SIZE, "FPS: %.2f", fps);

    // Calculate FPS
    cur_micros = sceKernelGetProcessTimeWide();
    if (cur_micros >= (last_micros + 1000000)) {
      delta_micros = cur_micros - last_micros;
      last_micros = cur_micros;
      fps = (frames / (double)delta_micros) * 1000000.0f;
      frames = 0;
    }
    #endif

    // End drawing
    vita2d_end_drawing();
    vita2d_swap_buffers();
    
    #ifdef SHOW_FPS
    frames++;
    #endif

    // Sync
    if ((!adrenaline->pops_mode && !draw_native) || adrenaline->draw_psp_screen_in_pops)
      sceCompatLCDCSync();
    else
      sceDisplayWaitVblankStart();
      
    // Ctrl
    if (menu_open)
      ctrlMenu();
  }

  return sceKernelExitDeleteThread(0);
}

int ScePspemuCustomSettingsHandler(int a1, int a2, int a3, int a4) {
  if (a2 == 3) {
    EnterAdrenalineMenu();
  } else if (a2 == 1) {
    sceKernelWaitSema(settings_semaid, 1, NULL);

    if (!open_official_settings) {
      ScePspemuSetDisplayConfig();
      return 0;
    }
  }

  return ScePspemuSettingsHandler(a1, a2, a3, a4);
}
