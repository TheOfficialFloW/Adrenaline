#include <common.h>
#include "psperror.h"

#include "isofs_driver.h"

#define MAX_HANDLERS	32
#define SIZE_OF_SECTORS	(SECTOR_SIZE*8)+64

Iso9660DirectoryRecord main_record;
FileHandle handlers[MAX_HANDLERS];
u8 *sectors;

int g_lastLBA, g_lastReadSize;

int IsofsReadSectors(int lba, int nsectors, void *buf) {
	if (buf == sectors) {
		if (nsectors > 8) return SCE_ERROR_ERRNO_EFBIG;
		memset(sectors + (nsectors * SECTOR_SIZE), 0, 64);
	}

	return Umd9660ReadSectors2(lba, nsectors, buf);
}

static inline int SizeToSectors(int size) {
	int len = size / SECTOR_SIZE;
	if ((size % SECTOR_SIZE) != 0) len++;
	return len;
}

__attribute__((noinline)) int GetFreeHandle() {
	// Lets ignore handler 0 to avoid problems with bad code...
	int i;
	for (i = 1; i < MAX_HANDLERS; i++) {
		if (handlers[i].opened == 0) return i;
	}

	return SCE_ERROR_ERRNO_EMFILE;
}

void UmdNormalizeName(char *filename) {
	char *p = strstr(filename, ";1");
	if (p) *p = 0;
}

int GetPathAndName(char *fullpath, char *path, char *filename) {
	static char fullpath2[256];
	strcpy(fullpath2, fullpath);

	if (fullpath2[strlen(fullpath2)-1] == '/') {
		fullpath2[strlen(fullpath2)-1] = 0;
	}

	char *p = strrchr(fullpath2, '/');

	if (!p) {
		if (strlen(fullpath2)+1 > 32) {
			// filename too big for ISO9660
			return SCE_ERROR_ERRNO_ENAMETOOLONG;
		}

		memset(path, 0, 256);
		UmdNormalizeName(fullpath2);
		strcpy(filename, fullpath2);

		return 0;
	}

	if (strlen(p+1)+1 > 32) {
		// filename too big for ISO9660
		return SCE_ERROR_ERRNO_ENAMETOOLONG;
	}

	strcpy(filename, p+1);
	p[1] = 0;

	if (fullpath2[0] == '/') {
		strcpy(path, fullpath2+1);
	} else
	{
		strcpy(path, fullpath2);
	}

	UmdNormalizeName(filename);

	return 0;
}

int FindFileLBA(char *filename, int lba, int dirSize, int isDir, Iso9660DirectoryRecord *retRecord) {
	char name[32];
	int oldDirLen = 0;
	int res;
	int remaining = 0;

	int pos = lba * SECTOR_SIZE;

	if (SizeToSectors(dirSize) <= 8) {
		res = IsofsReadSectors(lba, SizeToSectors(dirSize), sectors);
	} else
	{
		remaining = SizeToSectors(dirSize) - 8;
		res = IsofsReadSectors(lba, 8, sectors);
	}

	if (res < 0) return res;

	u8 *p = sectors;
	Iso9660DirectoryRecord *record = (Iso9660DirectoryRecord *)p;

	while (1) {
		if (record->len_dr == 0) {
			if (SECTOR_SIZE - (pos % SECTOR_SIZE) <= oldDirLen) {
				p += (SECTOR_SIZE - (pos % SECTOR_SIZE));
				pos += (SECTOR_SIZE - (pos % SECTOR_SIZE));
				record = (Iso9660DirectoryRecord *)p;

				if (record->len_dr == 0) {
					return SCE_ERROR_ERRNO_ENOENT;
				}
			} else
			{
				return SCE_ERROR_ERRNO_ENOENT;
			}
		}

		if (record->len_fi > 32) {
			return SCE_ERROR_ERRNO_EINVAL;
		}

		if (record->fi == 0) {
			if (strcmp(filename, ".") == 0) {
				memcpy(retRecord, record, sizeof(Iso9660DirectoryRecord));
				return record->lsbStart;
			}
		} else if (record->fi == 1) {
			if (strcmp(filename, "..") == 0) {
				memcpy(retRecord, record, sizeof(Iso9660DirectoryRecord));
				return record->lsbStart;
			}
		} else
		{
			memset(name, 0, 32);
			memcpy(name, &record->fi, record->len_fi);
			UmdNormalizeName(name);

			if (strcmp(name, filename) == 0) {
				if (isDir) {
					if (!(record->fileFlags & ISO9660_FILEFLAGS_DIR)) {
						return SCE_ERROR_ERRNO_ENOTDIR;
					}
				}

				memcpy(retRecord, record, sizeof(Iso9660DirectoryRecord));
				return record->lsbStart;
			}
		}

		pos += record->len_dr;
		p += record->len_dr;
		oldDirLen = record->len_dr;
		record = (Iso9660DirectoryRecord *)p;
		
		if (remaining > 0) {
			int offset = (p - sectors);

			if ((offset + sizeof(Iso9660DirectoryRecord) + 0x60) >= (8*SECTOR_SIZE)) {
				lba += (offset / SECTOR_SIZE);

				res = IsofsReadSectors(lba, 8, sectors);
				if (res < 0) return res;

				if (offset >= (8*SECTOR_SIZE)) {
					remaining -= 8;
				} else
				{
					remaining -= 7;
				}

				p = sectors + (offset % SECTOR_SIZE);
				record = (Iso9660DirectoryRecord *)p;
			}
		}
	}

	return -1;
}

int FindPathLBA(char *path, Iso9660DirectoryRecord *retRecord) {
	char curdir[32];
	int level = 0;

	int lba = main_record.lsbStart;
	memcpy(retRecord, &main_record, sizeof(Iso9660DirectoryRecord));

	char *p = strchr(path, '/');
	char *curpath = path;

	while (p) {
		if (p-curpath+1 > 32) {
			return SCE_ERROR_ERRNO_ENAMETOOLONG;
		}

		memset(curdir, 0, sizeof(curdir));
		strncpy(curdir, curpath, p-curpath);

		if (strcmp(curdir, ".") == 0) {
		} else if (strcmp(curdir, "..") == 0) {
			level--;
		} else
		{
			level++;
		}

		if (level > 8) {
			return SCE_ERROR_ERRNO_EINVAL;
		}
		
		lba = FindFileLBA(curdir, lba, retRecord->lsbDataLength, 1, retRecord);
		if (lba < 0) return lba;
		
		curpath = p+1;
		p = strchr(curpath, '/');
	}
	
	return lba;
}

int isofs_init() {
	sectors = oe_malloc(SIZE_OF_SECTORS);
	if (!sectors) return -1;

	memset(sectors, 0, SIZE_OF_SECTORS);

	int res = IsofsReadSectors(0x10, 1, sectors);
	if (res < 0) return res;

	if (memcmp(sectors + 1, "CD001", 5) != 0) {
		return SCE_ERROR_ERRNO_EINVAL;
	}

	memcpy(&main_record, sectors + 0x9C, sizeof(Iso9660DirectoryRecord));
	memset(handlers, 0, sizeof(handlers));

	g_lastLBA = -1;
	g_lastReadSize = 0;

	return 0;
}

int isofs_fastinit() {
	sectors = oe_malloc(SIZE_OF_SECTORS);
	if (!sectors) return -1;

	memset(sectors, 0, SIZE_OF_SECTORS);

	memset(&main_record, 0, sizeof(Iso9660DirectoryRecord));
	memset(handlers, 0, sizeof(handlers));

	g_lastLBA = -1;
	g_lastReadSize = 0;

	return 0;
}

int isofs_exit() {
	if (sectors) {
		oe_free(sectors);
		sectors = NULL;
	}

	g_lastLBA = -1;
	g_lastReadSize = 0;

	return 0;
}

int isofs_reset() {
	memset(&main_record, 0, sizeof(Iso9660DirectoryRecord));
	memset(handlers, 0, sizeof(handlers));

	g_lastLBA = -1;
	g_lastReadSize = 0;

	return 0;
}

int isofs_open(char *file, int flags, SceMode mode) {
	Iso9660DirectoryRecord record;
	static char fullpath[256], path[256], filename[32];

	int res, lba, i;
	int notallowedflags = PSP_O_WRONLY | PSP_O_APPEND | PSP_O_CREAT | PSP_O_TRUNC | PSP_O_EXCL;

	if (!file) return SCE_ERROR_ERRNO_EINVAL;

	if (strcmp(file, "/") == 0) {
		i = GetFreeHandle();
		if (i < 0) return i;

		handlers[i].opened = 1;
		handlers[i].lba = main_record.lsbStart;
		handlers[i].filesize = main_record.lsbDataLength;
		handlers[i].filepointer = 0;

		return i;
	}

	memset(fullpath, 0, 256);
	strncpy(fullpath, file, 256);

	if (fullpath[strlen(fullpath)-1] == '/') {
		fullpath[strlen(fullpath)-1] = 0;
	}

	if (strlen(fullpath)+1 > 256) {
		// path too big for ISO9660 
		return SCE_ERROR_ERRNO_ENAMETOOLONG;
	}
	
	if (flags & notallowedflags) return SCE_ERROR_ERRNO_EFLAG;

	if (strncmp(fullpath, "/sce_lbn", 8) != 0) {
		if ((res = GetPathAndName(fullpath, path, filename)) < 0) {
			return res;
		}

		if (path[0]) {
			lba = FindPathLBA(path, &record);
		} else
		{
			memcpy(&record, &main_record, sizeof(Iso9660DirectoryRecord));
			lba = record.lsbStart;
		}

		if (lba < 0) return lba;

		lba = FindFileLBA(filename, lba, record.lsbDataLength, 0, &record);
		if (lba < 0) return lba;
		
		i = GetFreeHandle();
		if (i < 0) return i;

		handlers[i].opened = 1;
		handlers[i].lba = lba;
		handlers[i].filesize = record.lsbDataLength;
		handlers[i].filepointer = 0;
	} else
	{
		// lba  access
		char str[11];
		
		char *p = strstr(fullpath, "_size");
		if (!p) return SCE_ERROR_ERRNO_EINVAL;

		if ((p-(fullpath+8)) > 10) {
			return SCE_ERROR_ERRNO_ENAMETOOLONG;
		}

		memset(str, 0, 11);
		strncpy(str, fullpath+8, p-(fullpath+8));
		
		lba = strtol(str, NULL, 0);
		if (lba < 0) return SCE_ERROR_ERRNO_EINVAL;

		if ((p+strlen(p)-(p+5)) > 10) {
			return SCE_ERROR_ERRNO_ENAMETOOLONG;
		}

		memset(str, 0, 11);
		strncpy(str, p+5, p+strlen(p)-(p+5));

		int size = strtol(str, NULL, 0);
		if (size < 0) return SCE_ERROR_ERRNO_EINVAL;

		i = GetFreeHandle();
		if (i < 0) return i;

		handlers[i].opened = 1;
		handlers[i].lba = lba;
		handlers[i].filesize = size;
		handlers[i].filepointer = 0;
	}
	
	return i;
}

int isofs_close(SceUID fd) {
	
	if (fd < 0 || fd >= MAX_HANDLERS) {
		return SCE_ERROR_ERRNO_EBADF;
	}

	if (!handlers[fd].opened) {
		return SCE_ERROR_ERRNO_EBADF;
	}
	
	handlers[fd].opened = 0;
	
	return 0;
}

int isofs_read(SceUID fd, char *data, int len) {
	int sectorscanbewritten = 0; // Sectors that can be written directly to the output buffer
	int res, read = 0;

	if (fd < 0 || fd >= MAX_HANDLERS) return SCE_ERROR_ERRNO_EBADF;
	if (!handlers[fd].opened) return SCE_ERROR_ERRNO_EBADF;
	if (len < 0) return SCE_ERROR_ERRNO_EINVAL;

	u8 *p = (u8 *)data;
	int remaining = len;

	if (remaining+handlers[fd].filepointer > handlers[fd].filesize) {
		remaining -= (remaining+handlers[fd].filepointer)-handlers[fd].filesize;
	}

	if (remaining <= 0) return 0;

	int nextsector = handlers[fd].lba + (handlers[fd].filepointer / SECTOR_SIZE);
	
	if ((handlers[fd].filepointer % SECTOR_SIZE) != 0) {
		res = IsofsReadSectors(nextsector, 1, sectors);
		if (res != 1) return res;

		read = SECTOR_SIZE-(handlers[fd].filepointer % SECTOR_SIZE);
		
		if (read > remaining) {
			read = remaining;
		}

		memcpy(p, sectors+(handlers[fd].filepointer % SECTOR_SIZE), read);

		remaining -= read;
		p += read;
		handlers[fd].filepointer += read;
		nextsector++;
	}

	if (remaining <= 0) return read;
	
	int sectorstoread = SizeToSectors(remaining);

	if ((remaining % SECTOR_SIZE) != 0) {
		sectorscanbewritten = sectorstoread-1;
	} else
	{
		sectorscanbewritten = sectorstoread;
	}

	if (sectorscanbewritten > 0) {
		res = IsofsReadSectors(nextsector, sectorscanbewritten, p);
		if (res < 0) return res;

		remaining -= (res*SECTOR_SIZE);
		p += (res*SECTOR_SIZE);
		handlers[fd].filepointer += (res*SECTOR_SIZE);
		read += (res*SECTOR_SIZE);
		nextsector += res;
	}

	if (remaining <= 0) return read;
	
	res = IsofsReadSectors(nextsector, 1, sectors);
	if (res < 0) return res;

	memcpy(p, sectors, (remaining % SECTOR_SIZE));
	read += (remaining % SECTOR_SIZE);
	handlers[fd].filepointer += (remaining % SECTOR_SIZE);
	remaining -= (remaining % SECTOR_SIZE);
	
	if (remaining > 0) return SCE_ERROR_ERRNO_EIO;
	
	return read;
}

SceOff isofs_lseek(SceUID fd, SceOff ofs, int whence) {
	if (fd < 0 || fd >= MAX_HANDLERS) {
		return SCE_ERROR_ERRNO_EBADF;
	}

	if (!handlers[fd].opened) {
		return SCE_ERROR_ERRNO_EBADF;
	}

	if (whence == PSP_SEEK_SET) {
		handlers[fd].filepointer = (int)ofs;
	} else if (whence == PSP_SEEK_CUR) {
		handlers[fd].filepointer += (int)ofs;
	} else if (whence == PSP_SEEK_END) {
		handlers[fd].filepointer = handlers[fd].filesize - (int)ofs;
	} else
	{
		return SCE_ERROR_ERRNO_EINVAL;
	}

	return handlers[fd].filepointer;
}

int isofs_getstat(const char *file, SceIoStat *stat) {
	Iso9660DirectoryRecord record;
	static char fullpath[256], path[256], filename[32];
	int res, lba;

	if (!file || !stat) return SCE_ERROR_ERRNO_EINVAL;

	if (strlen(file)+1 > 256) {
		// path too big for ISO9660 
		return SCE_ERROR_ERRNO_ENAMETOOLONG;
	}

	memset(fullpath, 0, 256);
	strncpy(fullpath, file, 256);

	if (fullpath[strlen(fullpath)-1] == '/') {
		fullpath[strlen(fullpath)-1] = 0;
	}

	if (strncmp(fullpath, "/sce_lbn", 8) != 0) {
		if ((res = GetPathAndName(fullpath, path, filename)) < 0) {
			return res;
		}

		if (path[0]) {
			lba = FindPathLBA(path, &record); 
		} else
		{
			memcpy(&record, &main_record, sizeof(Iso9660DirectoryRecord));
			lba = record.lsbStart;
		}

		if (lba < 0) return lba;

		lba = FindFileLBA(filename, lba, record.lsbDataLength, 0, &record);
		if (lba < 0) return lba;

		memset(stat, 0, sizeof(SceIoStat));
		stat->st_size = record.lsbDataLength;
		stat->st_private[0] = lba;
	}

	return 0;
}