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
#include "libraries_patch.h"
#include "string_clone.h"

#include "nid_table.h"

int (* aLinkLibEntries)(void *lib);
int (* search_nid_in_entrytable)(void *lib, u32 nid, int unk, int nidSearchOption);

#define N_MISSING_NID(x) (sizeof(x) / sizeof(MissingNid))
#define N_MISSING_NID_LIST (sizeof(missing_nid_list) / sizeof(MissingNidList))

MissingNid SysclibForKernel_nids[] = {
	{ 0x1AB53A58, strtok_r_clone },
	{ 0x87F8D2DA, strtok_clone },
	{ 0x1D83F344, atob_clone },
	{ 0x62AE052F, strspn_clone },
	{ 0x89B79CB1, strcspn_clone },
	{ 0xD3D1A3B9, strncat_clone },
	{ 0x909C228B, &setjmp_clone },
	{ 0x18FE80DB, &longjmp_clone },
};

MissingNid LoadCoreForKernel_nids[] = {
	{ 0x2952F5AC, 0 }, //sceKernelDcacheWBinvAll
	{ 0xD8779AC6, 0 }, //sceKernelIcacheClearAll
};

MissingNidList missing_nid_list[] = {
	{ "SysclibForKernel", SysclibForKernel_nids, N_MISSING_NID(SysclibForKernel_nids) },
	{ "LoadCoreForKernel", LoadCoreForKernel_nids, N_MISSING_NID(LoadCoreForKernel_nids) },
};

void *ResolveMissingNIDs(const char *libname, u32 nid) {
	int i;
	for (i = 0; i < N_MISSING_NID_LIST; i++) {
		if (strcmp(missing_nid_list[i].libname, libname) == 0) {
			int j;
			for (j = 0; j < missing_nid_list[i].n_nid; j++) {
				if (missing_nid_list[i].nid[j].nid == nid) {
					return missing_nid_list[i].nid[j].function;
				}
			}
		}
	}

	return NULL;
}

u32 ResolveOldNIDs(const char *libname, u32 nid) {
	int i;
	for (i = 0; i < N_NID_TABLE; i++) {
		if (strcmp(nid_table[i].libname, libname) == 0) {
			int j;
			for (j = 0; j < nid_table[i].n_nid; j++) {
				if (nid_table[i].nid[j].old_nid == nid) {
					return nid_table[i].nid[j].new_nid;
				}
			}
		}
	}

	return 0;
}

u32 sctrlHENFindFunction(const char *szMod, const char *szLib, u32 nid) {
	SceModule2 *mod = sceKernelFindModuleByName661(szMod);
	if (!mod) {
		mod = sceKernelFindModuleByAddress661((SceUID)szMod);
		if (!mod)
			return 0;
	}

	u32 new_nid = ResolveOldNIDs(szLib, nid);
	if (new_nid)
		nid = new_nid;

	int i = 0;
	while (i < mod->ent_size) {
		SceLibraryEntryTable *entry = (SceLibraryEntryTable *)(mod->ent_top + i);

        if (!szLib || (entry->libname && strcmp(entry->libname, szLib) == 0)) {
			u32 *table = entry->entrytable;
			int total = entry->stubcount + entry->vstubcount;

			int j;
			for (j = 0; j < total; j++) {
				if (table[j] == nid) {
					return table[j + total];
				}
			}
		}

		i += (entry->len * 4);
	}

	return 0;
}

u32 sctrlHENFindImport(const char *szMod, const char *szLib, u32 nid) {
	SceModule2 *mod = sceKernelFindModuleByName661(szMod);
	if (!mod) {
		mod = sceKernelFindModuleByAddress661((SceUID)szMod);
		if (!mod)
			return 0;
	}
/*
	u32 new_nid = ResolveOldNIDs(szLib, nid);
	if (new_nid)
		nid = new_nid;
*/
	int i = 0;
	while (i < mod->stub_size) {
		SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

		if (stub->libname && strcmp(stub->libname, szLib) == 0) {
			u32 *table = (u32 *)stub->nidtable;

			int j;
			for (j = 0; j < stub->stubcount; j++) {
				if (table[j] == nid) {
					return ((u32)stub->stubtable + (j * 8));
				}
			}
		}

		i += (stub->len * 4);
	}

	return 0;
}

int aLinkLibEntriesPatched(void *lib) {
	char *libname = (char *)((u32 *)lib)[8/4];

	// Fix Prometheus patch
	if (strcmp(libname, "Kernel_LibrarZ") == 0 || strcmp(libname, "Kernel_Librar0") == 0) {
		libname[13] = 'y';
	}

	if (strcmp(libname, "sceUtilitO") == 0) {
		libname[9] = 'y';
	}

	return aLinkLibEntries(lib);
}

int search_nid_in_entrytable_patched(void *lib, u32 nid, void *stub, int count) {
	char *libname = (char *)((u32 *)lib)[0x44/4];
	u32 stubtable = ((u32 *)stub)[0x18/4];
	u32 original_stub = ((u32 *)stub)[0x24/4];
	int is_user_mode = ((u32 *)stub)[0x34/4];
	u32 stub_addr = stubtable + (count * 8);

	u32 module_sdk_version = FindProc((char *)original_stub, NULL, 0x11B97506);
	if (module_sdk_version && (FIRMWARE_TO_FW(*(u32 *)module_sdk_version) == 0x660 || FIRMWARE_TO_FW(*(u32 *)module_sdk_version) == 0x661)) {
		// Sony module
	} else {
		if (!is_user_mode) {
			// Resolve missing NIDs
			void *function = ResolveMissingNIDs(libname, nid);
			if (function) {
				REDIRECT_FUNCTION(stub_addr, function);
				return -1;
			}
		}

		// Resolve old NIDs
		u32 new_nid = ResolveOldNIDs(libname, nid);
		if (new_nid)
			nid = new_nid;
	}

	int res = search_nid_in_entrytable(lib, nid, -1, 0);

	// Not linked yet
	if (res < 0) {
		// log("Missing nid %s_%08X\n", libname, (unsigned int)nid);

		_sw(0x0000054C, stub_addr);
		_sw(0x00000000, stub_addr + 4);

		return -1;
	}

	return res;
}