/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef LIBS_H
#define LIBS_H

typedef struct
{
	const char *name;
	unsigned short version;
	unsigned short attribute;
	unsigned char entLen;
	unsigned char varCount;
	unsigned short funcCount;
	unsigned int *fnids;
	unsigned int *funcs;
	unsigned int *vnids;
	unsigned int *vars;
}PspModuleImport;

PspModuleImport *find_import_lib(SceModule *pMod, char *library);

unsigned int find_import_bynid(SceModule *pMod, char *library, unsigned int nid);

void api_hook_addr(int addr, void *func);
void api_hook_import(int addr, void *func);

int hook_import_bynid(SceModule *pMod, char *library, unsigned int nid, void *func, int syscall);

#endif
