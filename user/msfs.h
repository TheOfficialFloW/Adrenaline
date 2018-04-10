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

#ifndef __MSFS_H__
#define __MSFS_H__

#define SCE_ERROR_ERRNO_ENOENT      0x80010002
#define SCE_ERROR_ERRNO_EEXIST      0x80010011
#define SCE_ERROR_ERRNO_ENODEV      0x80010013
#define SCE_ERROR_ERRNO_ENOTDIR      0x80010014
#define SCE_ERROR_ERRNO_EINVAL      0x80010016

#define PSP_SECTOR_SIZE 512

/* for chstat cbit */
#define PSP_CST_MODE    0x0001
#define PSP_CST_ATTR    0x0002
#define PSP_CST_SIZE    0x0004
#define PSP_CST_CT      0x0008
#define PSP_CST_AT      0x0010
#define PSP_CST_MT      0x0020
#define PSP_CST_PRVT    0x0040

#define MAX_IO_SIZE (50 * 1024 * 1024)
#define TRANSFER_SIZE (64 * 1024)

#define KERMIT_CMD_MSFS_INIT      (0x00000C63)
#define KERMIT_CMD_MSFS_EXIT      (0x00000C6E)
#define KERMIT_CMD_MSFS_OPEN      (0x00000C67)
#define KERMIT_CMD_MSFS_CLOSE      (0x00000C6B)
#define KERMIT_CMD_MSFS_READ      (0x00000C5E)
#define KERMIT_CMD_MSFS_WRITE      (0x00000C6A)
#define KERMIT_CMD_MSFS_LSEEK      (0x00000C62)
#define KERMIT_CMD_MSFS_IOCTL      (0x00000C6F)
#define KERMIT_CMD_MSFS_REMOVE      (0x00000C6C)
#define KERMIT_CMD_MSFS_MKDIR      (0x00000C66)
#define KERMIT_CMD_MSFS_RMDIR      (0x00000C61)
#define KERMIT_CMD_MSFS_DOPEN      (0x00000C5F)
#define KERMIT_CMD_MSFS_DCLOSE      (0x00000C64)
#define KERMIT_CMD_MSFS_DREAD      (0x00000C65)
#define KERMIT_CMD_MSFS_GETSTAT      (0x00000C69)
#define KERMIT_CMD_MSFS_CHSTAT      (0x00000C5D)
#define KERMIT_CMD_MSFS_RENAME      (0x00000C68)
#define KERMIT_CMD_MSFS_CHDIR      (0x00000C6D)
#define KERMIT_CMD_MSFS_DEVCTL      (0x00000C60)

typedef struct {
  char path[MAX_PATH_LENGTH];
  char filter[MAX_NAME_LENGTH];
  SceUID fd;
  SceOff offset;
  SceOff first_write_offset;
  int first_write;
  int flags;
  int trunc;
  int folder;
  int extra;
} ScePspemuMsfsDescriptor;

#define MAX_DESCRIPTORS 32

ScePspemuMsfsDescriptor *ScePspemuMsfsGetFileDescriptors();
void ScePspemuMsfsSetFileDescriptors(ScePspemuMsfsDescriptor *descriptors);

int ScePspemuMsfsGetstat(const char *file, SceIoStat *stat);

int ScePspemuRemoteMsfs(SceSize args, void *argp);

#endif