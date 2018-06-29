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

#ifndef __MAIN_H__
#define __MAIN_H__

typedef struct {
    u32 addr;
    u32 size;
} SceSysmemPartInfo;

typedef struct {
    u32 memSize;
    u32 unk4;
    u32 unk8;
    SceSysmemPartInfo other1; // 12
    SceSysmemPartInfo other2; // 20
    SceSysmemPartInfo vshell; // 28
    SceSysmemPartInfo scUser; // 36
    SceSysmemPartInfo meUser; // 44
    SceSysmemPartInfo extSc2Kernel; // 52
    SceSysmemPartInfo extScKernel; // 60
    SceSysmemPartInfo extMeKernel; // 68
    SceSysmemPartInfo extVshell; // 76
} SceSysmemPartTable;

#endif