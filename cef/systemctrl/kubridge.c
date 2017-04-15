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

#include <common.h>

#include "main.h"

int kuKernelGetModel() {
	return 1; // Fake slim model
}

int kuKernelSetDdrMemoryProtection(void *addr, int size, int prot) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelSetDdrMemoryProtection661(addr, size, prot);
	pspSdkSetK1(k1);
	return res;
}

int kuKernelGetUserLevel(void) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelGetUserLevel();
	pspSdkSetK1(k1);
	return res;
}

int kuKernelInitKeyConfig() {
	return sceKernelInitKeyConfig();
}

int kuKernelBootFrom() {
	return sceKernelBootFrom();
}

int kuKernelInitFileName(char *filename) {
	int k1 = pspSdkSetK1(0);
	strcpy(filename, sceKernelInitFileName());
	pspSdkSetK1(k1);
	return 0;
}

int kuKernelInitApitype() {
	return sceKernelInitApitype();
}

SceUID kuKernelLoadModuleWithApitype2(int apitype, const char *path, int flags, SceKernelLMOption *option) {
 	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadModuleWithApitype2661(apitype, path, flags, option);
	pspSdkSetK1(k1);
	return res;
}

SceUID kuKernelLoadModule(const char *path, int flags, SceKernelLMOption *option) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadModule661(path, flags, option);
	pspSdkSetK1(k1);
	return res;
}