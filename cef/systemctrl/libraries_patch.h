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

#ifndef __LIBRARIES_PATCH_H__
#define __LIBRARIES_PATCH_H__

typedef struct {
	u32 nid;
	void *function;
} MissingNid;

typedef struct {
	char *libname;
	MissingNid *nid;
	int n_nid;
} MissingNidList;

extern MissingNid LoadCoreForKernel_nids[];

extern int setjmp_clone();
extern void longjmp_clone();

extern int (* aLinkLibEntries)(void *lib);
extern int (* search_nid_in_entrytable)(void *lib, u32 nid, int unk, int nidSearchOption);

int aLinkLibEntriesPatched(void *lib);
int search_nid_in_entrytable_patched(void *lib, u32 nid, void *stub, int count);

#endif