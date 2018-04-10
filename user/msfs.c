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

#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/sysmem.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "main.h"
#include "utils.h"
#include "msfs.h"

static ScePspemuMsfsDescriptor descriptor_list[MAX_DESCRIPTORS];

static void buildPspemuMsfsPath(char *out_path, const char *in_path) {
  if (in_path[0] == '/')
    in_path++;

  if (strncmp(in_path, "__ADRENALINE__", 14) == 0) {
    snprintf(out_path, MAX_PATH_LENGTH, "ux0:app/" ADRENALINE_TITLEID "/%s", in_path+14);
  } else {
    snprintf(out_path, MAX_PATH_LENGTH, "%s/%s", getPspemuMemoryStickLocation(), in_path);
  }
}

static void convertFileStat(SceIoStat *stat) {
  if (stat->st_mode & SCE_S_IFDIR)
    stat->st_attr |= SCE_SO_IFDIR;

  if (stat->st_mode & SCE_S_IFREG)
    stat->st_attr |= SCE_SO_IFREG;

  ScePspemuConvertStatTimeToLocaltime(stat);
}

static int ScePspemuMsfsAddDescriptor(SceUID fd, char *path, int flags, int trunc, int folder) {
  int i;
  for (i = 0; i < MAX_DESCRIPTORS; i++) {
    if (!descriptor_list[i].fd)
      break;
  }

  if (i == MAX_DESCRIPTORS)
    return -1;

  memset(&descriptor_list[i], 0, sizeof(ScePspemuMsfsDescriptor));
  strcpy(descriptor_list[i].path, path);
  descriptor_list[i].fd = fd;
  descriptor_list[i].flags = flags;
  descriptor_list[i].trunc = trunc;
  descriptor_list[i].folder = folder;
  descriptor_list[i].extra = folder ? 0 : -1;

  // Root
  if (path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
    descriptor_list[i].extra = -1;
  }

  return i;
}

static ScePspemuMsfsDescriptor *ScePspemuMsfsGetDescriptor(int index) {
  if (index < 0 || index >= MAX_DESCRIPTORS)
    return NULL;

  return &descriptor_list[index];
}

static int ScePspemuMsfsRemoveDescriptor(int index) {
  ScePspemuMsfsDescriptor *descriptor = ScePspemuMsfsGetDescriptor(index);
  if (!descriptor)
    return -1;

  memset(descriptor, 0, sizeof(ScePspemuMsfsDescriptor));

  return 0;
}

static void ScePspemuMsfsCloseAllDescriptors() {
  int i;
  for (i = 0; i < MAX_DESCRIPTORS; i++) {
    if (descriptor_list[i].fd) {
      int res = sceIoClose(descriptor_list[i].fd);
      if (res < 0)
        res = sceIoDclose(descriptor_list[i].fd);
    }
  }

  memset(descriptor_list, 0, sizeof(descriptor_list));
}

static SceUID ScePspemuMsfsReopenFile(ScePspemuMsfsDescriptor *descriptor) {
  char msfs_path[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(msfs_path, descriptor->path);

  if (descriptor->folder) {
    descriptor->fd = sceIoDopen(msfs_path);
  } else {
    descriptor->fd = sceIoOpen(msfs_path, descriptor->flags, 0777);
    sceIoLseek(descriptor->fd, descriptor->offset, SCE_SEEK_SET);
  }

  return descriptor->fd;
}

ScePspemuMsfsDescriptor *ScePspemuMsfsGetFileDescriptors() {
  return descriptor_list;
}

void ScePspemuMsfsSetFileDescriptors(ScePspemuMsfsDescriptor *descriptors) {
  ScePspemuMsfsCloseAllDescriptors();

  memcpy(descriptor_list, descriptors, MAX_DESCRIPTORS * sizeof(ScePspemuMsfsDescriptor));

  int i;
  for (i = 0; i < MAX_DESCRIPTORS; i++) {
    if (descriptor_list[i].fd)
      ScePspemuMsfsReopenFile(&descriptor_list[i]);
  }
}

static int ScePspemuMsfsInit() {
  ScePspemuMsfsCloseAllDescriptors();
  return 0;
}

static int ScePspemuMsfsExit() {
  ScePspemuMsfsCloseAllDescriptors();
  return 0;
}

static SceUID ScePspemuMsfsOpen(const char *file, int flags, SceMode mode) {
  int trunc = 0;

  char msfs_path[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(msfs_path, file);

  if (file[0] == '\0')
    return SCE_ERROR_ERRNO_EINVAL;

  if (flags & SCE_O_TRUNC) {
    char msfs_path_trunc[MAX_PATH_LENGTH];
    strcpy(msfs_path_trunc, msfs_path);
    strcat(msfs_path_trunc, "__TRUNC__");
    if (sceIoRename(msfs_path, msfs_path_trunc) >= 0) {
      // Create empty file
      SceUID fd = sceIoOpen(msfs_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
      if (fd >= 0) {
        sceIoClose(fd);
        trunc = 1;
      }
    }
  }

  SceUID fd = sceIoOpen(msfs_path, flags, 0777);
  if (fd < 0)
    return fd;

  return ScePspemuMsfsAddDescriptor(fd, (char *)file, flags, trunc, 0);
}

static int ScePspemuMsfsTruncateFix(ScePspemuMsfsDescriptor *descriptor) {
  int res = 0;
  SceUID fd = -1;
  char *buf = NULL;

  char msfs_path_trunc[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(msfs_path_trunc, descriptor->path);
  strcat(msfs_path_trunc, "__TRUNC__");

  fd = sceIoOpen(msfs_path_trunc, SCE_O_RDONLY, 0);
  if (fd < 0) {
    res = fd;
    goto EXIT;
  }

  sceIoLseek(descriptor->fd, 0, SCE_SEEK_SET);

  buf = malloc(TRANSFER_SIZE);
  if (!buf) {
    res = -1;
    goto EXIT;
  }

  SceOff seek = 0;

  do {
    SceOff remain = descriptor->first_write_offset - seek;
    int buf_size = remain < TRANSFER_SIZE ? (int)remain : TRANSFER_SIZE;

    if (buf_size == 0)
      break;

    int read = sceIoRead(fd, buf, buf_size);
    if (read < 0) {
      res = read;
      goto EXIT;
    }

    int written = sceIoWrite(descriptor->fd, buf, buf_size);
    if (written < 0) {
      res = written;
      goto EXIT;
    }

    seek += buf_size;
  } while (seek < descriptor->first_write_offset);

EXIT:
  if (buf)
    free(buf);

  if (fd >= 0)
    sceIoClose(fd);

  sceIoRemove(msfs_path_trunc);

  return res;
}

static int ScePspemuMsfsClose(SceUID fd) {
  ScePspemuMsfsDescriptor *descriptor = ScePspemuMsfsGetDescriptor(fd);
  if (!descriptor)
    return SCE_ERROR_ERRNO_EINVAL;

  // Truncation on vita sets everything to 0
  // This doesn't happen on PSP though, so we need to copy back the data
  if (descriptor->trunc) {
    int res = ScePspemuMsfsTruncateFix(descriptor);
    if (res < 0)
      return res;
  }

  int res = sceIoClose(descriptor->fd);

  // Fake close success
  if (res == SCE_ERROR_ERRNO_ENODEV)
    res = 0;

  if (res >= 0)
    ScePspemuMsfsRemoveDescriptor(fd);

  return res;
}

static int ScePspemuMsfsRead(SceUID fd, void *data, SceSize size) {
  ScePspemuMsfsDescriptor *descriptor = ScePspemuMsfsGetDescriptor(fd);
  if (!descriptor)
    return SCE_ERROR_ERRNO_EINVAL;

  int seek = 0;

  do {
    int remain = size - seek;
    int buf_size = (remain < MAX_IO_SIZE) ? remain : MAX_IO_SIZE;

    int res = sceIoRead(descriptor->fd, data + seek, buf_size);

    if (res == SCE_ERROR_ERRNO_ENODEV) {
      if (ScePspemuMsfsReopenFile(descriptor) >= 0)
        res = sceIoRead(descriptor->fd, data + seek, buf_size);
    }

    if (res < 0)
      return res;

    if (res == 0)
      break;

    seek += res;
    descriptor->offset += res;
  } while (seek < size);

  return seek;
}

static int ScePspemuMsfsWrite(SceUID fd, const void *data, SceSize size) {
  ScePspemuMsfsDescriptor *descriptor = ScePspemuMsfsGetDescriptor(fd);
  if (!descriptor)
    return SCE_ERROR_ERRNO_EINVAL;

  // Set first write offset for truncation hack
  if (!descriptor->first_write && size > 0) {
    descriptor->first_write_offset = descriptor->offset;
    descriptor->first_write = 1;
  }

  int seek = 0;

  do {
    int remain = size - seek;
    int buf_size = (remain < MAX_IO_SIZE) ? remain : MAX_IO_SIZE;

    int res = sceIoWrite(descriptor->fd, data + seek, buf_size);

    if (res == SCE_ERROR_ERRNO_ENODEV) {
      if (ScePspemuMsfsReopenFile(descriptor) >= 0)
        res = sceIoWrite(descriptor->fd, data + seek, buf_size);
    }

    if (res < 0)
      return res;

    if (res == 0)
      break;

    seek += res;
    descriptor->offset += res;
  } while (seek < size);

  return seek;
}

static SceOff ScePspemuMsfsLseek(SceUID fd, SceOff offset, int whence) {
  ScePspemuMsfsDescriptor *descriptor = ScePspemuMsfsGetDescriptor(fd);
  if (!descriptor)
    return SCE_ERROR_ERRNO_EINVAL;

  SceOff res = sceIoLseek(descriptor->fd, offset, whence);

  if ((int)res == SCE_ERROR_ERRNO_ENODEV) {
    if (ScePspemuMsfsReopenFile(descriptor) >= 0)
      res = sceIoLseek(descriptor->fd, offset, whence);
  }

  if ((int)res >= 0)
    descriptor->offset = res;

  return res;
}

static int ScePspemuMsfsIoctl(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
  ScePspemuMsfsDescriptor *descriptor = ScePspemuMsfsGetDescriptor(fd);
  if (!descriptor)
    return SCE_ERROR_ERRNO_EINVAL;

  // Directory filter command
  if (cmd == 0x02415050 && indata && inlen == sizeof(uint32_t)) {
    char *name = (char *)ScePspemuConvertAddress(*(uint32_t *)indata, SCE_COMPAT_CACHE_NONE, 0x4000);
    strcpy(descriptor->filter, name);
    return 0;
  }

  return SCE_ERROR_ERRNO_EINVAL;
}

static int ScePspemuMsfsRemove(const char *file) {
  char msfs_path[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(msfs_path, file);

  // Invalid path
  if (file[0] == '\0')
    return SCE_ERROR_ERRNO_EINVAL;

  return sceIoRemove(msfs_path);
}

static int ScePspemuMsfsMkdir(const char *dir, SceMode mode) {
  char msfs_path[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(msfs_path, dir);

  // Invalid path
  if (dir[0] == '\0')
    return SCE_ERROR_ERRNO_EINVAL;

  return sceIoMkdir(msfs_path, 0777);
}

static int ScePspemuMsfsRmdir(const char *path) {
  char msfs_path[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(msfs_path, path);

  // Invalid path
  if (path[0] == '\0')
    return SCE_ERROR_ERRNO_EINVAL;

  return sceIoRmdir(msfs_path);
}

static SceUID ScePspemuMsfsDopen(const char *dirname) {
  char msfs_path[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(msfs_path, dirname);

  SceUID dfd = sceIoDopen(msfs_path);
  if (dfd < 0)
    return dfd;

  return ScePspemuMsfsAddDescriptor(dfd, (char *)dirname, 0, 0, 1);
}

static int ScePspemuMsfsDclose(SceUID fd) {
  ScePspemuMsfsDescriptor *descriptor = ScePspemuMsfsGetDescriptor(fd);
  if (!descriptor)
    return SCE_ERROR_ERRNO_EINVAL;

  int res = sceIoDclose(descriptor->fd);

  // Fake close success
  if (res == SCE_ERROR_ERRNO_ENODEV)
    res = 0;

  if (res >= 0)
    ScePspemuMsfsRemoveDescriptor(fd);

  return res;
}

static int ScePspemuMsfsDirectoryFilter(char *name, char *filter) {
  uint8_t *p = (uint8_t *)name;
  uint8_t *q = (uint8_t *)filter;

  // Loop until one of the strings ends
  while (*p && *q) {
    // Same character / any character
    if (*p == *q || *q == '?') {
      // Next characters
      p++, q++;
      continue;
    }

    // Any sequence
    if (*q == '*') {
      // Next character
      q++;

      // Skip all * and ?
      while (*q == '*' || *q == '?')
        q++;

      // Search same character after *
      while (*p && *p != *q)
        p++;
    }

    // Different character
    if (*p != *q)
      break;
  }

  // Skip all * and ?
  while (*q == '*' || *q == '?')
    q++;

  // Filter result
  return *p == '\0' && *q == '\0';
}

static int ScePspemuMsfsExtraDread(ScePspemuMsfsDescriptor *descriptor, SceIoDirent *dir) {
  if (descriptor->extra >= 0 && descriptor->extra < 2) {
    switch (descriptor->extra) {
      case 0:
        strcpy(dir->d_name, ".");
        descriptor->extra = 1;
        break;

      case 1:
        strcpy(dir->d_name, "..");
        descriptor->extra = -1;
        break;
    }

    ScePspemuMsfsGetstat(descriptor->path, &dir->d_stat);

    dir->d_stat.st_mode = (SCE_S_IFDIR | SCE_S_IRWXO | SCE_S_IRWXG | SCE_S_IRWXU);
    dir->d_stat.st_attr = SCE_SO_IFDIR;

    return 1;
  }

  return 0;
}

static int ScePspemuMsfsDread(SceUID fd, SceIoDirent *dir) {
  int res = 0;

  ScePspemuMsfsDescriptor *descriptor = ScePspemuMsfsGetDescriptor(fd);
  if (!descriptor)
    return SCE_ERROR_ERRNO_EINVAL;

  void *private = dir->d_private;

  do {
    if (descriptor->extra >= 0) {
      res = ScePspemuMsfsExtraDread(descriptor, dir);
    } else {
      res = sceIoDread(descriptor->fd, dir);
    }
  } while (res != 0 && descriptor->filter[0] && ScePspemuMsfsDirectoryFilter(dir->d_name, descriptor->filter) == 0);

  convertFileStat(&dir->d_stat);

  // sceIoDread clears this, so we have to backup and restore it manually
  dir->d_private = private;

  if (dir->d_private) {
    SceFatMsDirent *ms_dirent = (SceFatMsDirent *)ScePspemuConvertAddress((uint32_t)dir->d_private, SCE_COMPAT_CACHE_NONE | SCE_COMPAT_CACHE_INVALIDATE, sizeof(SceFatMsDirent));
    if (ms_dirent && ms_dirent->size == sizeof(SceFatMsDirent)) {
      snprintf(ms_dirent->longFileName, MAX_PATH_LENGTH, dir->d_name);
    }
  }

  return res;
}

int ScePspemuMsfsGetstat(const char *file, SceIoStat *stat) {
  char msfs_path[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(msfs_path, file);

  // Invalid path
  if (file[0] == '\0')
    return SCE_ERROR_ERRNO_EINVAL;

  int res = sceIoGetstat(msfs_path, stat);

  convertFileStat(stat);

  return res;
}

static int ScePspemuMsfsChstat(const char *file, SceIoStat *stat, int bits) {
  char msfs_path[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(msfs_path, file);

  // Invalid path
  if (file[0] == '\0')
    return SCE_ERROR_ERRNO_EINVAL;

  bits &= ~(PSP_CST_MODE | PSP_CST_ATTR | PSP_CST_SIZE | PSP_CST_PRVT);

  stat->st_attr = 0;

  ScePspemuConvertStatTimeToUtc(stat);

  return sceIoChstat(msfs_path, stat, bits);
}

static int ScePspemuMsfsRename(const char *oldname, const char *newname) {
  char old_path[MAX_PATH_LENGTH], new_path[MAX_PATH_LENGTH];
  buildPspemuMsfsPath(old_path, oldname);
  buildPspemuMsfsPath(new_path, newname);

  // Invalid path
  if (oldname[0] == '\0' || newname[0] == '\0') {
    return SCE_ERROR_ERRNO_EINVAL;
  }

  // Allow renaming from '/PSP/GAME/_UPDATE' to 'UPDATE'
  if (!strchr(newname, '/')) {
    char *p = strrchr(old_path, '/');
    if (!p)
      return SCE_ERROR_ERRNO_EINVAL;

    *p = '\0';
    snprintf(new_path, MAX_PATH_LENGTH, "%s/%s", old_path, newname);
    *p = '/';
  }

  return sceIoRename(old_path, new_path);
}

static int ScePspemuMsfsChdir(const char *path) {
  return -1;
}

static int ScePspemuMsfsDevctl(const char *dev, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
  // Device information command
  if (cmd == 0x02425818 && outdata && outlen == sizeof(ScePspemuIoDevInfo)) {
    char *path;
    SceIoDevInfo devinfo;
    memset(&devinfo, 0, sizeof(SceIoDevInfo));
    switch (config.ms_location) {
      case MEMORY_STICK_LOCATION_UR0:
        path = "ur0:";
        break;
      case MEMORY_STICK_LOCATION_IMC0:
        path = "imc0:";
        break;
      case MEMORY_STICK_LOCATION_UMA0:
        path = "uma0:";
        break;
      default:
        path = "ux0:";
        break;
    }

    int res = sceIoDevctl(path, 0x3001, NULL, 0, &devinfo, sizeof(SceIoDevInfo));
    if (res < 0)
      return res;

    uint32_t max_clusters = ScePspemuDivide(devinfo.max_size, devinfo.cluster_size);
    uint32_t free_clusters = ScePspemuDivide(devinfo.free_size, devinfo.cluster_size);

    ScePspemuIoDevInfo *pspemu_devinfo = (ScePspemuIoDevInfo *)outdata;
    pspemu_devinfo->max_clusters = max_clusters;
    pspemu_devinfo->free_clusters = free_clusters;
    pspemu_devinfo->max_sectors = free_clusters;
    pspemu_devinfo->sector_size = PSP_SECTOR_SIZE;
    pspemu_devinfo->sector_count = devinfo.cluster_size / PSP_SECTOR_SIZE;

    return 0;
  }

  return SCE_ERROR_ERRNO_EINVAL;
}

int ScePspemuRemoteMsfs(SceSize args, void *argp) {
  while (1) {
    // Wait and get kermit request
    SceKermitRequest *request;
    ScePspemuKermitWaitAndGetRequest(KERMIT_MODE_MSFS, &request);

    int64_t res = 0;

    switch (request->cmd) {
      case KERMIT_CMD_MSFS_INIT:
      {
        res = ScePspemuMsfsInit();
        break;
      }

      case KERMIT_CMD_MSFS_EXIT:
      {
        res = ScePspemuMsfsExit();
        break;
      }

      case KERMIT_CMD_MSFS_OPEN:
      {
        char *file = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        int flags = request->args[1];
        SceMode mode = (SceMode)request->args[2];
        res = ScePspemuMsfsOpen(file, flags, mode);
        break;
      }

      case KERMIT_CMD_MSFS_CLOSE:
      {
        SceUID fd = request->args[0];
        res = ScePspemuMsfsClose(fd);
        break;
      }

      case KERMIT_CMD_MSFS_READ:
      {
        SceUID fd = request->args[0];
        SceSize size = request->args[2];
        void *data = (void *)ScePspemuConvertAddress(request->args[1], SCE_COMPAT_CACHE_INVALIDATE, size);

        res = ScePspemuMsfsRead(fd, data, size);
        if (res >= 0)
          ScePspemuWritebackCache(data, size);

        break;
      }

      case KERMIT_CMD_MSFS_WRITE:
      {
        SceUID fd = request->args[0];
        SceSize size = request->args[2];
        void *data = (void *)ScePspemuConvertAddress(request->args[1], SCE_COMPAT_CACHE_NONE, size);
        res = ScePspemuMsfsWrite(fd, data, size);
        break;
      }

      case KERMIT_CMD_MSFS_LSEEK:
      {
        SceUID fd = request->args[0];
        SceOff offset = (SceOff)request->args[1];
        int whence = request->args[2];
        res = ScePspemuMsfsLseek(fd, offset, whence);
        break;
      }

      case KERMIT_CMD_MSFS_IOCTL:
      {
        SceUID fd = request->args[0];
        unsigned int cmd = request->args[1];
        int inlen = request->args[3];
        int outlen = request->args[5];
        void *indata = (void *)ScePspemuConvertAddress(request->args[2], SCE_COMPAT_CACHE_NONE, inlen);
        void *outdata = (void *)ScePspemuConvertAddress(request->args[4], SCE_COMPAT_CACHE_INVALIDATE, outlen);

        res = ScePspemuMsfsIoctl(fd, cmd, indata, inlen, outdata, outlen);
        if (res >= 0)
          ScePspemuWritebackCache(outdata, outlen);

        break;
      }

      case KERMIT_CMD_MSFS_REMOVE:
      {
        char *file = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        res = ScePspemuMsfsRemove(file);
        break;
      }

      case KERMIT_CMD_MSFS_MKDIR:
      {
        char *dir = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        SceMode mode = (SceMode)request->args[1];
        res = ScePspemuMsfsMkdir(dir, mode);
        break;
      }

      case KERMIT_CMD_MSFS_RMDIR:
      {
        char *path = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        res = ScePspemuMsfsRmdir(path);
        break;
      }

      case KERMIT_CMD_MSFS_DOPEN:
      {
        char *dirname = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        res = ScePspemuMsfsDopen(dirname);
        break;
      }

      case KERMIT_CMD_MSFS_DCLOSE:
      {
        SceUID fd = request->args[0];
        res = ScePspemuMsfsDclose(fd);
        break;
      }

      case KERMIT_CMD_MSFS_DREAD:
      {
        SceUID fd = request->args[0];
        SceIoDirent *dir = (SceIoDirent *)ScePspemuConvertAddress(request->args[1], SCE_COMPAT_CACHE_NONE | SCE_COMPAT_CACHE_INVALIDATE, sizeof(SceIoDirent));

        res = ScePspemuMsfsDread(fd, dir);
        if (res >= 0)
          ScePspemuWritebackCache(dir, sizeof(SceIoDirent));

        break;
      }

      case KERMIT_CMD_MSFS_GETSTAT:
      {
        char *file = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        SceIoStat *stat = (SceIoStat *)ScePspemuConvertAddress(request->args[1], SCE_COMPAT_CACHE_INVALIDATE, sizeof(SceIoStat));

        res = ScePspemuMsfsGetstat(file, stat);
        if (res >= 0)
          ScePspemuWritebackCache(stat, sizeof(SceIoStat));

        break;
      }

      case KERMIT_CMD_MSFS_CHSTAT:
      {
        char *file = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        SceIoStat *stat = (SceIoStat *)ScePspemuConvertAddress(request->args[1], SCE_COMPAT_CACHE_NONE, sizeof(SceIoStat));
        int bits = request->args[2];
        res = ScePspemuMsfsChstat(file, stat, bits);
        break;
      }

      case KERMIT_CMD_MSFS_RENAME:
      {
        char *oldname = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        char *newname = (char *)ScePspemuConvertAddress(request->args[1], SCE_COMPAT_CACHE_NONE, 0x4000);
        res = ScePspemuMsfsRename(oldname, newname);
        break;
      }

      case KERMIT_CMD_MSFS_CHDIR:
      {
        char *path = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        res = ScePspemuMsfsChdir(path);
        break;
      }

      case KERMIT_CMD_MSFS_DEVCTL:
      {
        char *dev = (char *)ScePspemuConvertAddress(request->args[0], SCE_COMPAT_CACHE_NONE, 0x4000);
        unsigned int cmd = request->args[1];
        int inlen = request->args[3];
        int outlen = request->args[5];
        void *indata = (void *)ScePspemuConvertAddress(request->args[2], SCE_COMPAT_CACHE_NONE, inlen);
        void *outdata = (void *)ScePspemuConvertAddress(request->args[4], SCE_COMPAT_CACHE_INVALIDATE, outlen);

        res = ScePspemuMsfsDevctl(dev, cmd, indata, inlen, outdata, outlen);
        if (res >= 0)
          ScePspemuWritebackCache(outdata, outlen);

        break;
      }
    }

    // Send back response to kermit
    ScePspemuKermitSendResponse(KERMIT_MODE_MSFS, request, (uint64_t)res);
  }

  return 0;
}
