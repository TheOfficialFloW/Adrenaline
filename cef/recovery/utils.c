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

char *stristr(const char *str1, const char *str2) {
	#define MAXLEN 256

	static char temp1[MAXLEN+1], temp2[MAXLEN+1];
	temp1[MAXLEN] = 0;
	temp2[MAXLEN] = 0;

	strncpy(temp1, str1, MAXLEN);
	strncpy(temp2, str2, MAXLEN);

	int i;
	for (i = 0; i < MAXLEN && (temp1[i] != 0); i++) {
		temp1[i] = tolower(temp1[i]);
	}

	for (i = 0; i < MAXLEN && (temp2[i] != 0); i++) {
		temp2[i] = tolower(temp2[i]);
	}

	const char *pos = strstr(temp1, temp2);
	if (pos) {
		return (char *)(pos - temp1 + str1);
	}

	return 0;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int removePath(const char *path) {
	SceUID dfd = sceIoDopen(path);
	if (dfd >= 0) {
		int res = 0;

		do {
			SceIoDirent dir;
			memset(&dir, 0, sizeof(SceIoDirent));

			res = sceIoDread(dfd, &dir);
			if (res > 0) {
				if (dir.d_name[0] != '.') {
					char *new_path = malloc(strlen(path) + strlen(dir.d_name) + 2);
					sprintf(new_path, "%s/%s", path, dir.d_name);

					removePath(new_path);

					free(new_path);
				}
			}
		} while (res > 0);

		sceIoDclose(dfd);

		sceIoRmdir(path);
	} else {
		sceIoRemove(path);
	}

	return 0;
}

int CreateDirectories(char **directories, int n) {
	int i;
	for (i = 0; i < n; i++) {
		int res = sceIoMkdir(directories[i], 0777);
		if (res < 0 && res != 0x80010011)
			return res;
	}

	return 0;
}

int GetRegistryData(const char *dir, const char *name, int type, void *data, int data_size) {
	int ret = 0;

	struct RegParam reg;
	memset(&reg, 0, sizeof(reg));
	reg.regtype = 1;
	strcpy(reg.name, "/system");
	reg.namelen = strlen(reg.name);
	reg.unk2 = 1;
	reg.unk3 = 1;

	REGHANDLE h;
	if (sceRegOpenRegistry(&reg, 2, &h) == 0) {
		REGHANDLE hd;
		if (sceRegOpenCategory(h, dir, 2, &hd) == 0) {
			u32 reg_type, reg_size;

			REGHANDLE hk;
			if (sceRegGetKeyInfo(hd, name, &hk, &reg_type, &reg_size) == 0) {
				if (type == reg_type) {
					if (sceRegGetKeyValue(hd, hk, data, data_size) == 0) {
						ret = 1;
					}
				}
			}

			sceRegFlushCategory(hd);
			sceRegCloseCategory(hd);
		}

		sceRegFlushRegistry(h);
		sceRegCloseRegistry(h);
	}

	return ret;
}

int SetRegistryData(const char *dir, const char *name, int type, void *data, int data_size) {
	int ret = 0;

	struct RegParam reg;
	memset(&reg, 0, sizeof(reg));
	reg.regtype = 1;
	strcpy(reg.name, "/system");
	reg.namelen = strlen(reg.name);
	reg.unk2 = 1;
	reg.unk3 = 1;

	REGHANDLE h;
	if (sceRegOpenRegistry(&reg, 2, &h) == 0) {
		REGHANDLE hd;
		if (sceRegOpenCategory(h, dir, 2, &hd) == 0) {
			if (sceRegSetKeyValue(hd, name, data, data_size) == 0) {
				ret = 1;
			} else
			{
				sceRegCreateKey(hd, name, type, data_size);
				sceRegSetKeyValue(hd, name, data, data_size);
				ret = 1;
			}

			sceRegFlushCategory(hd);
			sceRegCloseCategory(hd);
		}

		sceRegFlushRegistry(h);
		sceRegCloseRegistry(h);
	}

	return ret;
}