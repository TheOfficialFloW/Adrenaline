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

#include <common.h>

#include "main.h"
#include "virtualpbpmgr.h"
#include "isofs_driver/umd9660_driver.h"
#include "isofs_driver/isofs_driver.h"

PSP_MODULE_INFO("VshControl", 0x1007, 1, 0);

#define BOOT_BIN "disc0:/PSP_GAME/SYSDIR/BOOT.BIN"
#define EBOOT_BIN "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define EBOOT_OLD "disc0:/PSP_GAME/SYSDIR/EBOOT.OLD"

#define DUMMY_CAT_ISO_EXTENSION "     "

#define MAX_FILES 128

char categorypath[256];
SceUID categorydfd = -1;

SceUID gamedfd = -1, isodfd = -1, overiso = 0;
int vpbpinited = 0, isoindex = 0, cachechanged = 0;
VirtualPbp *cache = NULL;

STMOD_HANDLER previous;

AdrenalineConfig config;

void ClearCaches() {
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

void KXploitString(char *str) {
	if (str) {
		char *perc = strchr(str, '%');
		if (perc) {
			strcpy(perc, perc + 1);
		}
	}
}

int CorruptIconPatch(char *name) {
	char path[256];
	sprintf(path, "ms0:/PSP/GAME/%s%%/EBOOT.PBP", name);

	SceIoStat stat;
	memset(&stat, 0, sizeof(stat));
	if (sceIoGetstat(path, &stat) >= 0) {
		strcpy(name, "__SCE"); // hide icon
		return 1;
	}

	return 0;
}

int HideDlc(char *name) {
	char path[256];
	sprintf(path, "ms0:/PSP/GAME/%s/PARAM.PBP", name);

	SceIoStat stat;
	memset(&stat, 0, sizeof(stat));
	if (sceIoGetstat(path, &stat) >= 0) {
		sprintf(path, "ms0:/PSP/GAME/%s/EBOOT.PBP", name);

		memset(&stat, 0, sizeof(stat));
		if (sceIoGetstat(path, &stat) < 0) {
			strcpy(name, "__SCE"); // hide icon
			return 1;
		}
	}

	return 0;
}

int GetIsoIndex(const char *file) {
	char *p = strstr(file, "/MMMMMISO");
	if (!p)
		return -1;

	char *q = strchr(p + 9, '/');
	if (!q)
		return strtol(p + 9, NULL, 10);

	char number[5];
	memset(number, 0, 5);
	strncpy(number, p + 9, q - (p + 9));

	return strtol(number, NULL, 10);
}

int LoadExecVSHCommonPatched(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2) {
	int k1 = pspSdkSetK1(0);

	VshCtrlSetUmdFile("");

	int index = GetIsoIndex(file);
	if (index >= 0) {
		VshCtrlSetUmdFile(virtualpbp_getfilename(index));

		int uses_prometheus = 0;

		// Execute patched ISOs
		if (isofs_init() < 0) {
			isofs_exit();
			return -1;
		}

		SceIoStat stat;
		if (isofs_getstat("/PSP_GAME/SYSDIR/EBOOT.OLD", &stat) >= 0) {
			uses_prometheus = 1;
		}

		isofs_exit();

		if (uses_prometheus) {
			param->argp = EBOOT_OLD;
		} else {
			if (config.executebootbin)
				param->argp = BOOT_BIN;
			else
				param->argp = EBOOT_BIN;
		}

		// Update path and key
		file = param->argp;
		param->key = "umdemu";

		apitype = PSP_INIT_APITYPE_DISC;

		// Set umdmode
		if (config.umdmode == MODE_INFERNO) {
			sctrlSESetBootConfFileIndex(BOOT_INFERNO);
		} else if (config.umdmode == MODE_MARCH33) {
			sctrlSESetBootConfFileIndex(BOOT_MARCH33);
		} else if (config.umdmode == MODE_NP9660) {
			sctrlSESetBootConfFileIndex(BOOT_NP9660);
		}
	}

	// Enable 1.50 homebrews boot
	if (strstr(file, "ms0:/PSP/GAME/")) {
		KXploitString(param->argp);
		file = param->argp;
	}

	param->args = strlen(param->argp) + 1; //Update length

	pspSdkSetK1(k1);
	return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}

SceUID sceIoDopenPatched(const char *dirname) {
	int res, game = 0;
	int k1 = pspSdkSetK1(0);

	int index = GetIsoIndex(dirname);
	if (index >= 0) {
		int res = virtualpbp_open(index);
		pspSdkSetK1(k1);
		return res;
	}

	if (strcmp(dirname, "ms0:/PSP/GAME") == 0) {
		game = 1;
	}

	if (strstr(dirname, DUMMY_CAT_ISO_EXTENSION)) {
		char *p = strrchr(dirname, '/');
		if (p) {
			strcpy(categorypath, "ms0:/ISO");
			strcat(categorypath, p);
			categorypath[8 + strlen(p) - 5] = '\0';

			categorydfd = sceIoDopen(categorypath);
			pspSdkSetK1(k1);
			return categorydfd;
		}
	}

	pspSdkSetK1(k1);
	res = sceIoDopen(dirname);
	pspSdkSetK1(0);

	if (game) {
		gamedfd = res;
		overiso = 0;
	}

	pspSdkSetK1(k1);
	return res;
}

void ApplyIsoNamePatch(SceIoDirent *dir) {
	if (dir->d_name[0] != '.') {
		memset(dir->d_name, 0, 256);
		sprintf(dir->d_name, "MMMMMISO%d", isoindex++);
	}
}

int ReadCache() {
	SceUID fd;
	int i;

	if (!cache)
		cache = (VirtualPbp *)oe_malloc(MAX_FILES * sizeof(VirtualPbp));

	memset(cache, 0, sizeof(VirtualPbp) * MAX_FILES);

	for (i = 0; i < 0x10; i++) {
		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache2.bin", PSP_O_RDONLY, 0);
		if (fd >= 0)
			break;
	}

	if (i == 0x10)
		return -1;

	sceIoRead(fd, cache, sizeof(VirtualPbp) * MAX_FILES);
	sceIoClose(fd);

	return 0;
}

int SaveCache() {
	SceUID fd;
	int i;

	if (!cache)
		return -1;

	for (i = 0; i < MAX_FILES; i++) {
		if (cache[i].isofile[0] != 0) {
			SceIoStat stat;
			memset(&stat, 0, sizeof(stat));
			if (sceIoGetstat(cache[i].isofile, &stat) < 0) {
				cachechanged = 1;
				memset(&cache[i], 0, sizeof(VirtualPbp));
			}
		}
	}

	if (!cachechanged)
		return 0;

	cachechanged = 0;

	sceIoMkdir("ms0:/PSP", 0777);
	sceIoMkdir("ms0:/PSP/SYSTEM", 0777);

	for (i = 0; i < 0x10; i++) {
		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache2.bin", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fd >= 0)
			break;
	}

	if (i == 0x10)
		return -1;

	sceIoWrite(fd, cache, sizeof(VirtualPbp) * MAX_FILES);
	sceIoClose(fd);

	return 0;
}

int IsCached(char *isofile, ScePspDateTime *mtime, VirtualPbp *res) {
	int i;
	for (i = 0; i < MAX_FILES; i++) {
		if (cache[i].isofile[0] != 0) {
			if (strcmp(cache[i].isofile, isofile) == 0) {
				if (memcmp(mtime, &cache[i].mtime, sizeof(ScePspDateTime)) == 0) {
					memcpy(res, &cache[i], sizeof(VirtualPbp));
					return 1;
				}
			}
		}
	}

	return 0;
}

int Cache(VirtualPbp *pbp) {
	int i;
	for (i = 0; i < MAX_FILES; i++) {
		if (cache[i].isofile[0] == 0) {
			memcpy(&cache[i], pbp, sizeof(VirtualPbp));
			cachechanged = 1;
			return 1;
		}
	}

	return 0;
}

int AddIsoDirent(char *path, SceUID fd, SceIoDirent *dir, int readcategories) {
	int res;

NEXT:
	if ((res = sceIoDread(fd, dir)) > 0) {
		static VirtualPbp vpbp;
		static char fullpath[256];
		int res2 = -1;
		int docache;

		if (!FIO_S_ISDIR(dir->d_stat.st_mode)) {
			strcpy(fullpath, path);
			strcat(fullpath, "/");
			strcat(fullpath, dir->d_name);

			if (IsCached(fullpath, &dir->d_stat.st_mtime, &vpbp)) {
				res2 = virtualpbp_fastadd(&vpbp);
				docache = 0;
			} else {
				res2 = virtualpbp_add(fullpath, &dir->d_stat.st_mtime, &vpbp);
				docache = 1;
			}
			
			if (res2 >= 0) {
				ApplyIsoNamePatch(dir);

				// Fake the entry from file to directory
				dir->d_stat.st_mode = 0x11FF;
				dir->d_stat.st_attr = 0x0010;
				dir->d_stat.st_size = 0;
				
				// Change the modifcation time to creation time
				memcpy(&dir->d_stat.st_mtime, &dir->d_stat.st_ctime, sizeof(ScePspDateTime));

				if (docache) {
					Cache(&vpbp);
				}
			}
		} else {
			if (readcategories && dir->d_name[0] != '.' && strcmp(dir->d_name, "VIDEO") != 0) {
				strcat(dir->d_name, DUMMY_CAT_ISO_EXTENSION);
			} else {
				goto NEXT;
			}
		}

		return res;
	}

	return -1;
}

int sceIoDreadPatched(SceUID fd, SceIoDirent *dir) {
	int res;
	int k1 = pspSdkSetK1(0);

	if (vpbpinited) {
		res = virtualpbp_dread(fd, dir);
		if (res >= 0) {
			pspSdkSetK1(k1);
			return res;
		}
	}
	
	if (fd >= 0) {
		if (fd == gamedfd) {
			if (isodfd < 0 && !overiso) {
				isodfd = sceIoDopen("ms0:/ISO");
				if (isodfd >= 0) {
					if (!vpbpinited) {
						virtualpbp_init();
						vpbpinited = 1;
					} else {
						virtualpbp_reset();
					}

					ReadCache();
					isoindex = 0;
				} else {
					overiso = 1;
				}
			}

			if (isodfd >= 0) {
				res = AddIsoDirent("ms0:/ISO", isodfd, dir, 1);
				if (res > 0) {
					pspSdkSetK1(k1);
					return res;
				}
			}
		} else if (fd == categorydfd) {
			res = AddIsoDirent(categorypath, categorydfd, dir, 0);
			if (res > 0) {
				pspSdkSetK1(k1);
				return res;
			}
		}
	}

	res = sceIoDread(fd, dir);

	if (res > 0) {
		if (config.hidecorrupt)
			CorruptIconPatch(dir->d_name);
		
		if (config.hidedlcs)
			HideDlc(dir->d_name);		
	}

	pspSdkSetK1(k1);
	return res;
}

int sceIoDclosePatched(SceUID fd) {
	int k1 = pspSdkSetK1(0);
	int res;

	if (vpbpinited) {
		res = virtualpbp_close(fd);	
		if (res >= 0) {
			pspSdkSetK1(k1);
			return res;
		}
	}
	
	if (fd == categorydfd) {
		categorydfd = -1;
	} else if (fd == gamedfd) {
		sceIoDclose(isodfd);
		isodfd = -1;
		gamedfd = -1;
		overiso = 0;
		SaveCache();
	}

	pspSdkSetK1(k1);
	return sceIoDclose(fd);
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
	int k1 = pspSdkSetK1(0);

	int index = GetIsoIndex(file);
	if (index >= 0) {
		int res = virtualpbp_open(index);	
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);
	return sceIoOpen(file, flags, mode);
}

int sceIoClosePatched(SceUID fd) {
	int k1 = pspSdkSetK1(0);
	int res = -1;

	if (vpbpinited) {
		res = virtualpbp_close(fd);
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoClose(fd);

	return res;
}

int sceIoReadPatched(SceUID fd, void *data, SceSize size) {
	int k1 = pspSdkSetK1(0);
	int res = -1;
	
	if (vpbpinited) {
		res = virtualpbp_read(fd, data, size);
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoRead(fd, data, size);

	return res;
}

SceOff sceIoLseekPatched(SceUID fd, SceOff offset, int whence) {
	int k1 = pspSdkSetK1(0);
	int res = -1;

	if (vpbpinited) {
		res = virtualpbp_lseek(fd, offset, whence);
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoLseek(fd, offset, whence);

	return res;
}

int sceIoLseek32Patched(SceUID fd, int offset, int whence) {
	int k1 = pspSdkSetK1(0);
	int res = -1;

	if (vpbpinited) {
		res = virtualpbp_lseek(fd, offset, whence);	
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoLseek32(fd, offset, whence);

	return res;
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat) {
	int k1 = pspSdkSetK1(0);

	int index = GetIsoIndex(file);
	if (index >= 0) {
		int res = virtualpbp_getstat(index, stat);
		pspSdkSetK1(k1);
		return res;
	}

	int game = 0;
	if (strcmp(file, "ms0:/PSP/GAME") == 0)
		game = 1;

	pspSdkSetK1(k1);

	int res = sceIoGetstat(file, stat);

	if (game && res < 0) {
		pspSdkSetK1(0);
		sceIoMkdir("ms0:/PSP", 0777);
		sceIoMkdir("ms0:/PSP/GAME", 0777);
		pspSdkSetK1(k1);

		res = sceIoGetstat(file, stat);
	}

	return res;
}

int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits) {
	int k1 = pspSdkSetK1(0);

	int index = GetIsoIndex(file);
	if (index >= 0) {
		int res = virtualpbp_chstat(index, stat, bits);
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);
	return sceIoChstat(file, stat, bits);
}

int sceIoRemovePatched(const char *file) {
	int k1 = pspSdkSetK1(0);

	int index = GetIsoIndex(file);
	if (index >= 0) {
		int res = virtualpbp_remove(index);
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);
	return sceIoRemove(file);
}

int sceIoRmdirPatched(const char *path) {
	int k1 = pspSdkSetK1(0);

	int index = GetIsoIndex(path);
	if (index >= 0) {
		int res = virtualpbp_rmdir(index);
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);
	return sceIoRmdir(path);
}

int sceIoMkdirPatched(const char *dir, SceMode mode) {
	int k1 = pspSdkSetK1(0);

	if (strcmp(dir, "ms0:/PSP/GAME") == 0) {
		sceIoMkdir("ms0:/ISO", mode);
	}

	pspSdkSetK1(k1);
	return sceIoMkdir(dir, mode);
}

///////////////////////////////////////////////////////

u32 firsttick;
u8 set;

int (* vshmenu_ctrl)(SceCtrlData *pad_data, int count);

SceUID modid = -1;

int cpu_list[] = { 0, 20, 75, 100, 133, 222, 266, 300, 333 };
int bus_list[] = { 0, 10, 37,  50,  66, 111, 133, 150, 166 };

#define N_CPU (sizeof(cpu_list) / sizeof(int))

int sceCtrlReadBufferPositivePatched(SceCtrlData *pad_data, int count) {
	int res = sceCtrlReadBufferPositive(pad_data, count);
	int k1 = pspSdkSetK1(0);

	if (!set && config.vshcpuspeed != 0) {
		u32 curtick = sceKernelGetSystemTimeLow();
		curtick -= firsttick;

		u32 t = (u32)curtick;
		if (t >= (10 * 1000 * 1000)) {
			set = 1;
			SetSpeed(cpu_list[config.vshcpuspeed % N_CPU], bus_list[config.vshcpuspeed % N_CPU]);
		}
	}

	if (!sceKernelFindModuleByName("VshCtrlSatelite")) {
		if (pad_data->Buttons & PSP_CTRL_SELECT) {
			if (!sceKernelFindModuleByName("htmlviewer_plugin_module") &&
			   !sceKernelFindModuleByName("sceVshOSK_Module") &&
			   !sceKernelFindModuleByName("camera_plugin_module")) {
				modid = sceKernelLoadModule("flash0:/vsh/module/satelite.prx", 0, NULL);
				if (modid >= 0) {
					sceKernelStartModule(modid, 0, NULL, NULL, NULL);
					pad_data->Buttons &= ~PSP_CTRL_SELECT;
				}
			}
		}
	} else {
		if (vshmenu_ctrl) {
			vshmenu_ctrl(pad_data, count);
		} else if (modid >= 0) {
			if (sceKernelStopModule(modid, 0, NULL, NULL, NULL) >= 0) {
				sceKernelUnloadModule(modid);
			}
		}
	}

	pspSdkSetK1(k1);
	return res;
}

int vctrlVSHRegisterVshMenu(int (* ctrl)(SceCtrlData *, int)) {
	int k1 = pspSdkSetK1(0);
	vshmenu_ctrl = (void *)((u32)ctrl | 0x80000000);
	pspSdkSetK1(k1);
	return 0;
}

int vctrlVSHExitVSHMenu(AdrenalineConfig *conf) {
	int k1 = pspSdkSetK1(0);
	int oldspeed = config.vshcpuspeed;

	vshmenu_ctrl = NULL;
	memcpy(&config, conf, sizeof(AdrenalineConfig));
	SetConfig(&config);

	if (set) {
		if (config.vshcpuspeed != oldspeed) {
			if (config.vshcpuspeed) {
				SetSpeed(cpu_list[config.vshcpuspeed % N_CPU], bus_list[config.vshcpuspeed % N_CPU]);
			} else {
				SetSpeed(222, 111);
			}
		}
	}

	pspSdkSetK1(k1);
	return 0;
}

u32 MakeSyscallStub(void *function) {
	SceUID block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_High, 2 * sizeof(u32), NULL);
	u32 stub = (u32)sceKernelGetBlockHeadAddr(block_id);
	_sw(0x03E00008, stub);
	_sw(0x0000000C | (sceKernelQuerySystemCall(function) << 6), stub + 4);
	return stub;
}

int InitUsbPatched() {
	return sctrlStartUsb();
}

int ShutdownUsbPatched() {
	return sctrlStopUsb();
}

int GetUsbStatusPatched() {	
	int state = sctrlGetUsbState();

	if (state & 0x20)
		return 1; // Connected

	return 2; // Not connected
}

typedef struct {
	void *import;
	void *patched;
} ImportPatch;

ImportPatch import_patch[] = {
	// Directory functions
	{ &sceIoDopen, sceIoDopenPatched },
	{ &sceIoDread, sceIoDreadPatched },
	{ &sceIoDclose, sceIoDclosePatched },

	// File functions
	{ &sceIoOpen, sceIoOpenPatched },
	{ &sceIoClose, sceIoClosePatched },
	{ &sceIoRead, sceIoReadPatched },
	{ &sceIoLseek, sceIoLseekPatched },
	{ &sceIoLseek32, sceIoLseek32Patched },
	{ &sceIoGetstat, sceIoGetstatPatched },
	{ &sceIoChstat, sceIoChstatPatched },
	{ &sceIoRemove, sceIoRemovePatched },
	{ &sceIoRmdir, sceIoRmdirPatched },
	{ &sceIoMkdir, sceIoMkdirPatched },
};

void IoPatches() {
	int i;
	for (i = 0; i < (sizeof(import_patch) / sizeof(ImportPatch)); i++) {
		sctrlHENPatchSyscall(K_EXTRACT_IMPORT(import_patch[i].import), import_patch[i].patched); 
	}
}

void PatchVshMain(u32 text_addr) {
	// Allow old sfo's
	_sw(0, text_addr + 0x122B0);
	_sw(0, text_addr + 0x12058); //DISC_ID
	_sw(0, text_addr + 0x12060); //DISC_ID

	IoPatches();

	SceModule2 *mod = sceKernelFindModuleByName("sceVshBridge_Driver");
	MAKE_CALL(mod->text_addr + 0x25C, sceCtrlReadBufferPositivePatched);
	sctrlHENPatchSyscall(K_EXTRACT_IMPORT(&sceCtrlReadBufferPositive), sceCtrlReadBufferPositivePatched);

	// Dummy usb detection functions
	MAKE_DUMMY_FUNCTION(text_addr + 0x38C94, 0);
	MAKE_DUMMY_FUNCTION(text_addr + 0x38D68, 0);

	if (config.skipgameboot) {
		// Disable sceDisplaySetHoldMode
		_sw(0, text_addr + 0xCA88);
	}

	if (config.useextendedcolors == 1) {
		_sh(0x1000, text_addr + 0x3174A);
	}

	ClearCaches();	
}

wchar_t verinfo[] = L"6.61 Adrenaline    ";
wchar_t macinfo[] = L"00:00:00:00:00:00";

void PatchSysconfPlugin(u32 text_addr) {
	int version = sctrlSEGetVersion();
	int version_major = version >> 16;
	int version_minor = version & 0xFFFF;

	if (version_major > 1) {
		verinfo[15] = '-';
		verinfo[16] = '0' + version_major;

		if (version_minor > 0) {
			verinfo[17] = '.';
			verinfo[18] = '0' + version_minor;
		}
	}

	memcpy((void *)text_addr + 0x2A62C, verinfo, sizeof(verinfo));

	_sw(0x3C020000 | ((u32)(text_addr + 0x2A62C) >> 16), text_addr + 0x192E0);
	_sw(0x34420000 | ((u32)(text_addr + 0x2A62C) & 0xFFFF), text_addr + 0x192E4);

	if (config.hidemacaddr) {
		memcpy((void *)text_addr + 0x2E9A0, macinfo, sizeof(macinfo));
	}

	// Allow slim colors
	if (config.useextendedcolors != 0) {
		_sw(_lw(text_addr + 0x76F0), text_addr + 0x76EC);
		_sw(0x24020001, text_addr + 0x76F0);
	}

	// Dummy all vshbridge usbstor functions
	_sw(0x24020001, text_addr + 0xCD78); // sceVshBridge_ED978848 - vshUsbstorMsSetWorkBuf
	_sw(0x00001021, text_addr + 0xCDAC); // sceVshBridge_EE59B2B7
	_sw(0x00001021, text_addr + 0xCF0C); // sceVshBridge_6032E5EE - vshUsbstorMsSetProductInfo
	_sw(0x00001021, text_addr + 0xD218); // sceVshBridge_360752BF - vshUsbstorMsSetVSHInfo

	// Dummy LoadUsbModules, UnloadUsbModules
	MAKE_DUMMY_FUNCTION(text_addr + 0xCC70, 0);
	MAKE_DUMMY_FUNCTION(text_addr + 0xD2C4, 0);

	// Redirect USB functions
	REDIRECT_FUNCTION(text_addr + 0xAE9C, MakeSyscallStub(InitUsbPatched));
	REDIRECT_FUNCTION(text_addr + 0xAFF4, MakeSyscallStub(ShutdownUsbPatched));
	REDIRECT_FUNCTION(text_addr + 0xB4A0, MakeSyscallStub(GetUsbStatusPatched));

	// Ignore wait thread end failure
	_sw(0, text_addr + 0xB264);

	ClearCaches();
}

void PatchGamePlugin(u32 text_addr) {
	// Allow homebrew launch
	MAKE_DUMMY_FUNCTION(text_addr + 0x20528, 0);

	// Allow PSX launch
	MAKE_DUMMY_FUNCTION(text_addr + 0x20E6C, 0);

	// Allow custom multi-disc PSX
	_sw(0, text_addr + 0x14850);

	_sw(0x00001021, text_addr + 0x20620);

	if (config.hidepic0pic1) {
		_sw(0x00601021, text_addr + 0x1D858);
		_sw(0x00601021, text_addr + 0x1D864);
	}

	if (config.skipgameboot) {
		MAKE_CALL(text_addr + 0x19130, text_addr + 0x194B0);
		_sw(0x24040002, text_addr + 0x19134);
	}

	ClearCaches();	
}

int sceUpdateDownloadSetVersionPatched(int version) {
	int k1 = pspSdkSetK1(0);

	int (* sceUpdateDownloadSetVersion)(int version) = (void *)FindProc("SceUpdateDL_Library", "sceLibUpdateDL", 0xC1AF1076);
	int (* sceUpdateDownloadSetUrl)(const char *url) = (void *)FindProc("SceUpdateDL_Library", "sceLibUpdateDL", 0xF7E66CB4);

	sceUpdateDownloadSetUrl("http://adrenaline.abertschi.ch/psp-updatelist.txt");
	int res = sceUpdateDownloadSetVersion(sctrlSEGetVersion());

	pspSdkSetK1(k1);
	return res;
}

void PatchUpdatePlugin(u32 text_addr) {
	MAKE_CALL(text_addr + 0x82A8, MakeSyscallStub(sceUpdateDownloadSetVersionPatched));
	ClearCaches();
}

int OnModuleStart(SceModule2 *mod) {
	char *modname = mod->modname;
	u32 text_addr = mod->text_addr;

	if (strcmp(modname, "vsh_module") == 0) {
		PatchVshMain(text_addr);
	} else if (strcmp(modname, "sysconf_plugin_module") == 0) {
		PatchSysconfPlugin(text_addr);
	} else if (strcmp(modname, "game_plugin_module") == 0) {
		PatchGamePlugin(text_addr);
	} else if (strcmp(modname, "update_plugin_module") == 0) {
		PatchUpdatePlugin(text_addr);
	}

	if (!previous)
		return 0;

	return previous(mod);
}

int module_start(SceSize args, void *argp) {
	SceModule2 *mod = sceKernelFindModuleByName("sceLoadExec");
	u32 text_addr = mod->text_addr;

	MAKE_CALL(text_addr + 0x1DC0, LoadExecVSHCommonPatched); //sceKernelLoadExecVSHMs2
	MAKE_CALL(text_addr + 0x1BE0, LoadExecVSHCommonPatched); //sceKernelLoadExecVSHMsPboot

	sctrlSEGetConfig(&config);

	if (config.vshcpuspeed != 0) {
		firsttick = sceKernelGetSystemTimeLow();
	}

	ClearCaches();

	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	return 0;
}