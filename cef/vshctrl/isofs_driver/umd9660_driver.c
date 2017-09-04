#include <common.h>
#include "psperror.h"

#include "umd9660_driver.h"
#include "csoread.h"

SceUID umdfd;
int umd_open;
int umd_is_cso;

char umdfilename[256];

void VshCtrlSetUmdFile(char *file) {
	SetUmdFile(file);
	strncpy(umdfilename, file, 255);
	sceIoClose(umdfd);
	umd_open = 0;
	umdfd = -1;
}

int IsofileReadSectors(int lba, int nsectors, void *buf) {
	int read = ReadUmdFileRetry(buf, SECTOR_SIZE * nsectors, lba * SECTOR_SIZE);
	if (read < 0) return read;

	return read / SECTOR_SIZE;
}

int OpenIso() {
	umd_open = 0;
	sceIoClose(umdfd);

	if ((umdfd = sceIoOpen(umdfilename, PSP_O_RDONLY | 0xF0000, 0)) < 0) {
		return -1;
	}

	umd_is_cso = 0;
	if (CisoOpen(umdfd) >= 0) umd_is_cso = 1;
	umd_open = 1;

	return 0;
}

int ReadUmdFileRetry(void *buf, int size, u32 offset) {
	int i;
	for (i = 0; i < 0x10; i++) {
		if (sceIoLseek32(umdfd, offset, PSP_SEEK_SET) >= 0) {
			for (i = 0; i < 0x10; i++) {
				int read = sceIoRead(umdfd, buf, size);
				if (read >= 0) return read;

				OpenIso();
			}

			return 0x80010013;
		}

		OpenIso();
	}

	return 0x80010013;
}

int Umd9660ReadSectors2(int lba, int nsectors, void *buf) {
	if (umd_open == 0) {
		int i;
		for (i = 0; i < 0x10; i++) {
			if (sceIoLseek32(umdfd, 0, PSP_SEEK_CUR) >= 0) {
				break;
			}

			OpenIso();
		}

		if (umd_open == 0) {
			return 0x80010013;
		}
	}

	if (umd_is_cso == 0) {
		return IsofileReadSectors(lba, nsectors, buf);
	} else
	{
		return CisofileReadSectors(lba, nsectors, buf);
	}
}