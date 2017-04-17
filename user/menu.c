/*
	Adrenaline
	Copyright (C) 2016-2017, TheFloW

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

#include "includes/lcd3x_v.h"
#include "includes/lcd3x_f.h"
#include "includes/texture_v.h"
#include "includes/texture_f.h"
#include "includes/opaque_v.h"
#include "includes/bicubic_f.h"
#include "includes/sharp_bilinear_f.h"
#include "includes/sharp_bilinear_v.h"
#include "includes/advanced_aa_v.h"
#include "includes/advanced_aa_f.h"

int sceCommonDialogIsRunning();

vita2d_pgf *font;

int language = 0, enter_button = 0, date_format = 0, time_format = 0;

static int EnterStandbyMode();
static int OpenOfficialSettings();
static int ExitPspEmuApplication();

static char *graphics_options[] = { "Official", "Bilinear (GPU)", "Sharp bilinear", "Advanced AA", "LCD3x" };
static char *no_yes_options[] = { "No", "Yes" };
static char *yes_no_options[] = { "Yes", "No" };
static char *screen_size_options[] = { "2.0x", "1.75x", "1.5x", "1.25x", "1.0x" };
static char *ms_location_options[] = { "ux0:pspemu", "ur0:pspemu" };

static MenuEntry main_entries[] = {
	{ "Enter Standby Mode", MENU_ENTRY_TYPE_CALLBACK, 0, EnterStandbyMode, NULL, NULL, 0 },
	{ "Open Official Settings", MENU_ENTRY_TYPE_CALLBACK, 0, OpenOfficialSettings, NULL, NULL, 0 },
	{ "Exit PspEmu Application", MENU_ENTRY_TYPE_CALLBACK, 0, ExitPspEmuApplication, NULL, NULL, 0 },
	{ "Exit Adrenaline Menu", MENU_ENTRY_TYPE_CALLBACK, 0, ExitAdrenalineMenu, NULL, NULL, 0 },
};

static MenuEntry settings_entries[] = {
	{ "Graphics Filtering", MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.graphics_filtering, graphics_options, sizeof(graphics_options) / sizeof(char **) },
	{ "Smooth Graphics", MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.no_smooth_graphics, yes_no_options, sizeof(yes_no_options) / sizeof(char **) },
	{ "Screen Size", MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.screen_size, screen_size_options, sizeof(screen_size_options) / sizeof(char **) },
	{ "Memory Stick Location", MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.ms_location, ms_location_options, sizeof(ms_location_options) / sizeof(char **) },
	{ "Use DS3/DS4 controller", MENU_ENTRY_TYPE_OPTION, 0, NULL, &config.use_ds3_ds4, no_yes_options, sizeof(no_yes_options) / sizeof(char **) },
};

static MenuEntry about_entries[] = {
	{ "6.61 Adrenaline", MENU_ENTRY_TYPE_TEXT, ORANGE, NULL, NULL, NULL, 0 },
	{ "by TheFloW", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
	{ "", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
	{ "", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
	{ "Credits", MENU_ENTRY_TYPE_TEXT, ORANGE, NULL, NULL, NULL, 0 },
	{ "Team molecule for HENkaku", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
	{ "frangarcj for graphics filtering", MENU_ENTRY_TYPE_TEXT, WHITE, NULL, NULL, NULL, 0 },
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

	SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, SCE_PSPEMU_CACHE_NONE, ADRENALINE_SIZE);
	if (adrenaline->pops_mode)
		ScePspemuPausePops(1);

	return 0;
}

int ExitAdrenalineMenu() {
	if (changed) {
		config.magic[0] = ADRENALINE_CFG_MAGIC_1;
		config.magic[1] = ADRENALINE_CFG_MAGIC_2;
		WriteFile("ux0:adrenaline/adrenaline.bin", &config, sizeof(AdrenalineConfig));
	}

	SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, SCE_PSPEMU_CACHE_NONE, ADRENALINE_SIZE);
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

		pgf_draw_text(WINDOW_X + (i * TAB_SIZE) + ALIGN_CENTER(TAB_SIZE, vita2d_pgf_text_width(font, FONT_SIZE, tab_entries[i].name)), FONT_Y_LINE(19), WHITE, FONT_SIZE, tab_entries[i].name);
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

			if (menu_entries[i].type != MENU_ENTRY_TYPE_OPTION) {
				// Item
				float x = vita2d_pgf_text_width(font, FONT_SIZE, menu_entries[i].name);
				pgf_draw_text(ALIGN_CENTER(SCREEN_WIDTH, x), y, color, FONT_SIZE, menu_entries[i].name);
			} else {
				// Item
				float x = vita2d_pgf_text_width(font, FONT_SIZE, menu_entries[i].name);
				pgf_draw_text(ALIGN_RIGHT((SCREEN_WIDTH / 2.0f) - 10.0f, x), y, color, FONT_SIZE, menu_entries[i].name);

				// Option
				int value = *(menu_entries[i].value);
				pgf_draw_text((SCREEN_WIDTH / 2.0f) + 10.0f, y, GREEN, FONT_SIZE, menu_entries[i].options[value]);
			}
		}
	} else {
		drawStates();
	}
}

void ctrlMenu() {
	readPad();

	if (released_buttons & SCE_CTRL_PS_BTN) {
		ExitAdrenalineMenu();
	}

	if (!open_options) {
		if (released_buttons & SCE_CTRL_CANCEL) {
			ExitAdrenalineMenu();
		}

		if (pressed_buttons & SCE_CTRL_LTRIGGER) {
			if (tab_sel > 0) {
				menu_sel = 0;
				tab_sel--;
			}
		}

		if (pressed_buttons & SCE_CTRL_RTRIGGER) {
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
			if ((released_buttons & (SCE_CTRL_ENTER | SCE_CTRL_LEFT | SCE_CTRL_RIGHT))) {
				MenuEntry *menu_entries = tab_entries[tab_sel].entries;
				if (menu_entries) {
					if (menu_entries[menu_sel].type == MENU_ENTRY_TYPE_CALLBACK) {
						if (released_buttons & SCE_CTRL_ENTER) {
							if (menu_entries[menu_sel].callback)
								menu_entries[menu_sel].callback();
						}
					} else if (menu_entries[menu_sel].type == MENU_ENTRY_TYPE_OPTION) {
						if (released_buttons & (SCE_CTRL_ENTER | SCE_CTRL_RIGHT)) {
							if ((*menu_entries[menu_sel].value) < menu_entries[menu_sel].n_options-1)
								(*menu_entries[menu_sel].value)++;
							else
								(*menu_entries[menu_sel].value) = 0;
						}

						if (released_buttons & SCE_CTRL_LEFT) {
							if ((*menu_entries[menu_sel].value) > 0)
								(*menu_entries[menu_sel].value)--;
							else
								(*menu_entries[menu_sel].value) = menu_entries[menu_sel].n_options-1;
						}

						changed = 1;
					}
				}
			}

			if (hold_buttons & SCE_CTRL_UP) {
				if (menu_sel > 0)
					menu_sel--;
			}

			if (hold_buttons & SCE_CTRL_DOWN) {
				if (menu_sel < tab_entries[tab_sel].n_entries-1)
					menu_sel++;
			}
		}
	}
}

void *pops_data = NULL;

int AdrenalineDraw(SceSize args, void *argp) {
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &language);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &enter_button);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_DATE_FORMAT, &date_format);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_TIME_FORMAT, &time_format);

	if (enter_button == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE) {
		SCE_CTRL_ENTER = SCE_CTRL_CIRCLE;
		SCE_CTRL_CANCEL = SCE_CTRL_CROSS;
	}

	vita2d_init();
	font = vita2d_load_default_pgf();

	vita2d_texture *fbo = vita2d_create_empty_texture(SCREEN_WIDTH, SCREEN_HEIGHT);
	if (!fbo)
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

	vita2d_shader *shader = opaque_shader;

	settings_semaid = sceKernelCreateSema("AdrenalineSettingsSemaphore", 0, 0, 1, NULL);
	if (settings_semaid < 0)
		return settings_semaid;

	SceAdrenaline *adrenaline = (SceAdrenaline *)ScePspemuConvertAddress(ADRENALINE_ADDRESS, SCE_PSPEMU_CACHE_NONE, ADRENALINE_SIZE);

	while (1) {
		if (adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
			vita2d_start_drawing_advanced(fbo, VITA_2D_RESET_POOL | VITA_2D_SCENE_FRAGMENT_SET_DEPENDENCY);
			vita2d_clear_screen();
			vita2d_end_drawing();

			vita2d_start_drawing_advanced(NULL, VITA_2D_SCENE_VERTEX_WAIT_FOR_DEPENDENCY);
			vita2d_clear_screen();

			char *title = "Please wait...";
			pgf_draw_textf(ALIGN_CENTER(SCREEN_WIDTH, vita2d_pgf_text_width(font, FONT_SIZE, title)), FONT_Y_LINE(8), WHITE, FONT_SIZE, title);

			// End drawing
			vita2d_end_drawing();
			vita2d_swap_buffers();

			// Sync
			sceCompatLCDCSync();

			continue;
		}

		// Continue if dialog is running or official screen is used or pops mode is on
		if (sceCommonDialogIsRunning() || (config.graphics_filtering == 0 && menu_open == 0) || (adrenaline->pops_mode && menu_open == 0)) {
			sceDisplayWaitVblankStart();
			continue;
		}

		// Draw PSP framebuffer
		vita2d_start_drawing_advanced(fbo, VITA_2D_RESET_POOL | VITA_2D_SCENE_FRAGMENT_SET_DEPENDENCY);
		vita2d_clear_screen();

		// Do custom rendering if not pops mode and graphics filtering is used
		if (!adrenaline->pops_mode && config.graphics_filtering != 0) {
			// Copy PSP framebuffer
			sceDmacMemcpy(psp_data, (void *)SCE_PSPEMU_FRAMEBUFFER, SCE_PSPEMU_FRAMEBUFFER_SIZE);

			// Smooth
			if (config.no_smooth_graphics == 0)
				vita2d_texture_set_filters(psp_tex, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
			else
				vita2d_texture_set_filters(psp_tex, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);

			// Shader
			if (config.graphics_filtering == 1)
				shader = opaque_shader;
			else if (config.graphics_filtering == 2)
				shader = sharp_shader;
			else if (config.graphics_filtering == 3)
				shader = advanced_aa_shader;
			else if (config.graphics_filtering == 4)
				shader = lcd3x_shader;
			else
				shader = opaque_shader;

			float scale = 2.00f;
		
			switch (config.screen_size) {
				case SCREEN_SIZE_2_00:
					scale = 2.00f;
					break;
					
				case SCREEN_SIZE_1_75:
					scale = 1.75f;
					break;
					
				case SCREEN_SIZE_1_50:
					scale = 1.50f;
					break;
					
				case SCREEN_SIZE_1_25:
					scale = 1.25f;
					break;
					
				case SCREEN_SIZE_1_00:
					scale = 1.00f;
					break;
			}

			vita2d_texture_set_program(shader->vertexProgram, shader->fragmentProgram);
			vita2d_texture_set_wvp(shader->wvpParam);
			vita2d_texture_set_vertexInput(&shader->vertexInput);
			vita2d_texture_set_fragmentInput(&shader->fragmentInput);
			vita2d_draw_texture_scale_rotate_hotspot(psp_tex, 480.0f, 272.0f, scale, scale, 0.0, 240.0, 136.0);
		}

		vita2d_end_drawing();

		// Start drawing fbo
		vita2d_start_drawing_advanced(NULL, VITA_2D_SCENE_VERTEX_WAIT_FOR_DEPENDENCY);
		vita2d_clear_screen();
		vita2d_texture_set_program(opaque_shader->vertexProgram, opaque_shader->fragmentProgram);
		vita2d_texture_set_wvp(opaque_shader->wvpParam);
		vita2d_texture_set_vertexInput(&opaque_shader->vertexInput);
		vita2d_texture_set_fragmentInput(&opaque_shader->fragmentInput);
		vita2d_draw_texture(fbo, 0, 0);

		// Draw Menu
		if (menu_open) {
			if (adrenaline->pops_mode) {
				int (* ScePspemuDrawPopsDisplay)(void *data, int unk) = (void *)text_addr + 0x300D8 + 0x1;
				ScePspemuDrawPopsDisplay(pops_data, 0);

				vita2d_draw_texture(pops_tex, 0, 0);
			} else {
				if (config.graphics_filtering == 0) {
					int (* ScePspemuDrawPspDisplay)(void *data, void *frame_buffer) = (void *)text_addr + 0x1161C + 0x1;
					void *(* ScePspemuGetFramebuffer)() = (void *)text_addr + 0x3F90 + 0x1;
					ScePspemuDrawPspDisplay(pops_data, ScePspemuGetFramebuffer());

					vita2d_draw_texture(pops_tex, 0, 0);
				}
			}

			drawMenu();
		}

		// End drawing
		vita2d_end_drawing();
		vita2d_swap_buffers();

		// Sync
		sceCompatLCDCSync();

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
			int (* ScePspemuSetDisplayConfig)() = (void *)text_addr + 0x20E50 + 0x1;
			ScePspemuSetDisplayConfig();
			return 0;
		}
	}

	return ScePspemuSettingsHandler(a1, a2, a3, a4);
}