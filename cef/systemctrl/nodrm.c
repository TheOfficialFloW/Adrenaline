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

int (* do_open)(char *file, int flags, SceMode mode, int async, int retAddr, int oldK1);
int (* do_ioctl)(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen, int async);

SceUID (* _sceKernelLoadModuleNpDrm)(const char *path, int flags, SceKernelLMOption *option);

int (* _sceNpDrmRenameCheck)(const char *file_name);
int (* _sceNpDrmEdataSetupKey)(SceUID fd);
int (* _sceNpDrmEdataGetDataSize)(SceUID fd);

int IsPlainDrmFd(SceUID fd) {
	int k1 = pspSdkSetK1(0);

	int pos = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
	sceIoLseek32(fd, 0, PSP_SEEK_SET);

	char buf[8];
	if (sceIoRead(fd, buf, sizeof(buf)) == sizeof(buf)) {
		sceIoLseek32(fd, pos, PSP_SEEK_SET);

		// Encrypted DRM file
		if (memcmp(buf, "\0PSPEDAT", 8) == 0 || memcmp(buf, "\0PGD", 4) == 0) {
			pspSdkSetK1(k1);
			return 0;
		}
	} else {
		// Read error
		pspSdkSetK1(k1);
		return -1;
	}

	// Plain DRM file
	pspSdkSetK1(k1);
	return 1;
}

int IsPlainDrmPath(const char *path) {
	int k1 = pspSdkSetK1(0);

	SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0);
	if (fd >= 0) {
		if (IsPlainDrmFd(fd)) {
			sceIoClose(fd);
			pspSdkSetK1(k1);
			return 1;
		}

		sceIoClose(fd);
	}

	pspSdkSetK1(k1);
	return 0;
}

int do_open_patched(char *file, int flags, SceMode mode, int async, int retAddr, int oldK1) {
	if (flags & 0x40000000) {
		if (IsPlainDrmPath(file)) {
			flags &= ~0x40000000;
		}
	}

	return do_open(file, flags, mode, async, retAddr, oldK1);
}

int do_ioctl_patched(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen, int async) {
	int res = do_ioctl(fd, cmd, indata, inlen, outdata, outlen, async);

	if (res < 0) {
		if (cmd == 0x04100001 || cmd == 0x04100002) {
			if (IsPlainDrmFd(fd)) {
				return 0;
			}
		}
	}

	return res;
}

SceUID sceKernelLoadModuleNpDrmPatched(const char *path, int flags, SceKernelLMOption *option) {
	int res = _sceKernelLoadModuleNpDrm(path, flags, option);

	if (res < 0) {
		if (IsPlainDrmPath(path)) {
			return sceKernelLoadModule661(path, flags, option);
		}
	}

	return res;
}

int sceNpDrmRenameCheckPatched(const char *file_name) {
	int res = _sceNpDrmRenameCheck(file_name);

	if (res < 0) {
		if (IsPlainDrmPath(file_name)) {
			return 0;
		}
	}

	return res;
}

int sceNpDrmEdataSetupKeyPatched(SceUID fd) {
	int res = _sceNpDrmEdataSetupKey(fd);

	if (res < 0) {
		if (IsPlainDrmFd(fd)) {
			return 0;
		}
	}

	return res;
}

int sceNpDrmEdataGetDataSizePatched(SceUID fd) {
	int res = _sceNpDrmEdataGetDataSize(fd);

	if (res < 0) {
		if (IsPlainDrmFd(fd)) {
			int pos = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
			int size = sceIoLseek32(fd, 0, PSP_SEEK_END);
			sceIoLseek32(fd, pos, PSP_SEEK_SET);
			return size;
		}
	}

	return res;
}

void PatchNpDrmDriver(u32 text_addr) {
	if (!config.notusenodrmengine) {
		if (sceKernelBootFrom() == PSP_BOOT_DISC) {
			SceModule2 *mod = sceKernelFindModuleByName661("sceIOFileManager");

			int i;
			for (i = 0; i < mod->text_size; i += 4) {
				u32 addr = mod->text_addr + i;

				if (_lw(addr) == 0x03641824) {
					HIJACK_FUNCTION(addr - 4, do_open_patched, do_open);
					continue;
				}

				if (_lw(addr) == 0x00C75821) {
					HIJACK_FUNCTION(addr - 4, do_ioctl_patched, do_ioctl);
					continue;
				}
			}

			HIJACK_FUNCTION(text_addr + 0x1590, sceNpDrmRenameCheckPatched, _sceNpDrmRenameCheck);
			HIJACK_FUNCTION(text_addr + 0x1714, sceNpDrmEdataSetupKeyPatched, _sceNpDrmEdataSetupKey);
			HIJACK_FUNCTION(text_addr + 0x1514, sceNpDrmEdataGetDataSizePatched, _sceNpDrmEdataGetDataSize);

			HIJACK_FUNCTION(FindProc("sceModuleManager", "ModuleMgrForUser", 0xF2D8D1B4), sceKernelLoadModuleNpDrmPatched, _sceKernelLoadModuleNpDrm);

			ClearCaches();
		}
	}
}